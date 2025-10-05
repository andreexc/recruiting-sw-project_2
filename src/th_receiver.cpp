#include <thread>
#include <mutex>
#include <condition_variable>
#include <queue>
#include <vector>
#include <cstdint>
#include <cstring>
#include <iostream>
#include "FSM.h"
#include "th_receiver.h"

// fake_receiver is written in C and the linking is
// managed differently so this way i'm notifying the compiler
// that it's a C file and not C++
extern "C" {
    #include "fake_receiver.h"
}

static std::queue<std::vector<uint8_t>> raw_mess_queue; // still to be parsed
static std::mutex queueMutex;
static bool running = false;                   // static vars are visibile only in the file
                                               // scope so it doesn't need a mutex
static std::thread th_receiver;

// thread function
void receiveMessage() {
    // problems if in heap
    // char* raw_buffer = (char*) malloc(sizeof(char)*MAX_CAN_MESSAGE_SIZE); // allocating memory in the heap
                                                                          // for the buffer
    //memset(raw_buffer, 0, MAX_CAN_MESSAGE_SIZE);
    uint8_t raw_buffer[MAX_CAN_MESSAGE_SIZE];
    while (running) {
        memset(raw_buffer, 0, MAX_CAN_MESSAGE_SIZE);                // the buffer needs to be cleared by dirty bits
        int len = can_receive(reinterpret_cast<char*>(raw_buffer)); // need to cast it to char
                                                                    // can't use static_cast because there's
                                                                    // no logic correlation with uint8_t* and char*
        if (len <= 0) continue;                                     // error reading the message / no data
        std::vector<uint8_t> raw_msg(raw_buffer, raw_buffer+len);   // copying the memory from the pointer raw_buffer (first element) to the
                                                                    // last element (pointers math) because its continous in memory like arrays
        std::lock_guard<std::mutex> lock(queueMutex);               // guarding the queue access during the scope
        raw_mess_queue.push(raw_msg);                               // sharing the new message (needed a copy because the buffer is a classic array)
                                                                    // could have changed the raw_buffer type and then used std::move (?)
    }
    // free(raw_buffer);                                            // de-allocating memory
}

bool start_receiver(const char* path) {
    if (running) return false; // already running
    if (open_can(path) < 0) {
        std::cerr<<"Error while opening the CAN bus"<<std::endl;
        return false;
    }
    running = !running;  // (set to true but cooler)
    th_receiver = std::thread(receiveMessage);  /* the function doesn't have brackets because
                                                 * i'm passing its address and not calling it
                                                 */    
    return true;
}

void stop_receiver() {
    if (!running) return; // not running
    close_can();
    running = !running;
    if (th_receiver.joinable()) th_receiver.join();
}

// the bool is useful to see if there's a message
// ready to be processed
bool read_message(std::vector<uint8_t>& mess_out) {
    /* I could have used queueMutex.lock() / .unlock()
     * but lock_guard guarantees that in the exit of the scope
     * it immediatly call queueMutex.unlock() making the code safer
     * in case of exception avoiding possibile deadlocks
     * 
     * in this case lock_guard is a template (generics) that handles 
     * a standard mutex. search for the definition in std_mutex.h
     */ 
    std::lock_guard<std::mutex> lock(queueMutex);

    if (!raw_mess_queue.empty()) {
        mess_out = raw_mess_queue.front();  // get first element
                                            // could have used std::move (?)
        raw_mess_queue.pop();               // remove first element
        return true;
    }
    return false;
}