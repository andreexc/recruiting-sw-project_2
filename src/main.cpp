#include <iostream>
#include <thread>
#include <chrono>
#include <vector>
#include <cstdint>
#include "FSM.h"
#include "th_receiver.h"


const char* FILEPATH = "../static_data/candump.log";
extern "C"{
    #include "fake_receiver.h"
}

int main(void){

    // memory allocation C-like
    FSM* fsm = (FSM*) malloc(sizeof(FSM));
    new (fsm) FSM;  // calling costructor in the indicated memory slot

    // path of simulated CAN stream
    // watchout from where you're starting the progamm!

    if (!start_receiver(FILEPATH)) { // error starting the listener
        std::cerr<<"Error while starting the CAN listener."<<std::endl;
        return -1;
    }

    std::vector<uint8_t> raw_msg;
    while (true) {
        if (read_message(raw_msg)) {
            std::string msg_str(reinterpret_cast<char*>(raw_msg.data()), raw_msg.size());
            std::cout << "RAW MESSAGE: [" << msg_str << "] (len=" << raw_msg.size() << ")\n";
            fsm->handleMessage(msg_str);
        }
        else {
            // if there's no message i sleep the FSM (main thread) in order
            // to avoid CPU spinlock
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
    }
    stop_receiver();
    fsm->~FSM();  // calling destructor for freeing internal objects
    free(fsm);    // freeing the memory
    return 0;
}