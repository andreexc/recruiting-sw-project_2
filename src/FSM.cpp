#include <iostream>
#include <fstream>
#include <string>
#include <ctime>
#include "parser.h"
#include "can_message.h"
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
    this->state = State::IDLE;
    std::cout<<"Going from RUN to IDLE"
                <<std::endl;
}