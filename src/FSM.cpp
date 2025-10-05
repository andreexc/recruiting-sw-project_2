#include <iostream>
#include <fstream>
#include <string>
#include <ctime>
#include <unordered_map>
#include "parser.h"
#include "can_message.h"
#include "stat_data.h"
#include "FSM_codes.h"
#include "FSM.h"

FSM::FSM() { this->state = State::IDLE; }

void FSM::handleMessage(const std::string& raw_message) {
            
    // Message parsing:
    CAN_Message message;
    try {  // need a try for handling syntax errors
        message = parse_message(raw_message);
    }
    catch (std::exception& e) {
        std::cerr<<"Error: "
                    << e.what()
                    << std::endl;
        return;
    }

    // State-driven actions:
    switch(this->state) {
        case State::IDLE:
            this->handleOnIDLE(message, raw_message);
            break;
        case State::RUN:
            this->handleOnRUN(message, raw_message);
            break;
    }
}

void FSM::handleOnIDLE(const CAN_Message& msg, const std::string& raw_msg) {
    if (this->isStartMessage(msg)) {
        this->startRunSession(/*msg*/);
        this->do_log(msg, raw_msg);  // logging also the start message
    }
    // in this scope i'm idling so i don't log the message
}

void FSM::handleOnRUN(const CAN_Message& msg, const std::string& raw_msg) {
    // writing the raw message
    this->do_log(msg, raw_msg);
    if (this->isStopMessage(msg)) {
        this->stopRunSession();
    }
    this->updateStats(msg);
}

void FSM::do_log(const CAN_Message& msg, const std::string& raw_msg) {
    this->log_file<<"("<<std::to_string(msg.timestamp)<<") "<<raw_msg<<std::endl;
}

bool FSM::isStartMessage(const CAN_Message& msg) {
    // with this for-each i check if the message is a start message inside
    // the codes "database"
    for (const CAN_Message& start_wannabe : START_MESSAGES) {
        // in C++ the std::vector comparison is well defined and can be assured just with "=="
        // in fact, while operating with std::vector it checks if they have the same dimension
        // and if the element in the same positions are equal
        if (msg.ID == start_wannabe.ID && start_wannabe.Payload == msg.Payload) {
            return true;
        }
    }
    return false;
}

// same as isStartMessage but with the stop-codes "database"
bool FSM::isStopMessage(const CAN_Message& msg) {
    for (const CAN_Message& stop_wannabe : STOP_MESSAGES) {
        if (msg.ID == stop_wannabe.ID && stop_wannabe.Payload == msg.Payload) {
            return true;
        }
    }
    return false;
}

void FSM::startRunSession(/*const CAN_Message& msg*/) {
    std::string session_filename = "../static_data/session_" + std::to_string(time(nullptr)) + ".log"; // the log file is recognized by its creation date
    this->log_file.open(session_filename, std::ios::out);
    if (this->log_file.fail()) {
        std::cerr<<"Error while creating the file."
                    << std::endl;
        return;
    }
    this->session_statistics.clear();  // clearing the previous session data
    this->state = State::RUN;
    /*
        * if i chose to set the run state before the file creation i could get logging
        * problems if the file could not be created (the FSM switchs to RUN anyway)
    */
    std::cout<<"Going from IDLE to RUN"
                <<std::endl;
}

void FSM::stopRunSession() {
    this->log_file.close();
    this->saveStatsOnCSV();
    this->state = State::IDLE;
    std::cout<<"Going from RUN to IDLE"
                <<std::endl;
}

void FSM::updateStats(const CAN_Message& msg) {

    // here i get the statistics of messages with the same ID
    // (coded in an unordered key-value map)
    StatData& /* (need address reference for editing values) */
              msg_class_data = this->session_statistics[msg.ID];
    msg_class_data.count++; // increase the number of messages with that ID

    // if the value in the key msg.ID is not defined the operator std::vector<X,Y>[msg.ID] creates
    // a new key-value with the msg.ID key and default values ( {0,0,0} )

    // if not the first message
    if (msg_class_data.lastTimestamp != 0) {
        double elapsed_time = msg.timestamp - msg_class_data.lastTimestamp;
        msg_class_data.mean_time += (elapsed_time - msg_class_data.mean_time) / msg_class_data.count;
    } // no need to save the data in the map because i'm referring to the same memory slot
    /*
     *                        (new_value - old_mean)
     * new_mean = old_mean + ------------------------
     *                                  n
     *
     * proof: https://math.stackexchange.com/questions/106700/incremental-averaging
     * (with this formula i just have to save the old mean saving
     *  memory and time to calculate the new mean everytime)
     */
    msg_class_data.lastTimestamp = msg.timestamp; // overriding last message timestamp
}

void FSM::saveStatsOnCSV() {
    std::string sessionCSV_filename = "../static_data/stats_" + std::to_string(time(nullptr)) + ".csv";
    std::fstream csv_file; csv_file.open(sessionCSV_filename, std::ios::app);

    if (csv_file.fail()) {
        std::cerr<<"Error. Cannot open the stats file."
                 << std::endl;
        return; // yes, it will not save the stats
    }

    // writing in the CSV
    csv_file<<"ID,number_of_messages,mean_time"
            << std::endl; // heading
    for (const auto& ID_Stats : this->session_statistics) { /* for-each key-value (ID : Stats)*/
        uint16_t id = ID_Stats.first; // taking the key (ID)
        const StatData& stats_by_ID = ID_Stats.second; // taking the stats given the ID
        
        // std::hex changes the stream in hex base (need to be changed again to dec) 
        csv_file<<std::hex << id << ","
                <<std::dec << stats_by_ID.count << ","
                <<stats_by_ID.mean_time
                <<std::endl;
    }
    csv_file.close();
}