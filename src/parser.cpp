#pragma once
#include <stdint.h>
#include <vector>
#include "can_message.h"
#include <string>
#include <iostream>


void throw_error(char* err_str) {
    throw std::runtime_error(err_str);
}

/*
 * parse_message: std::string& --> CAN_Message
 * ( no memory leaks ;] )
 * 
 * std::string instead of char* for wrapping functions and std::stoul
*/
CAN_Message parse_message(const std::string& raw_message) {

    size_t pos = raw_message.find('#'); // need to split the ID / Payload
    if (pos == std::string::npos) throw_error("Invalid format!");  // std::string::npos is a value returned in case
                                                                   // functions such as find don't find the wanted value

    uint16_t id = std::stoul(raw_message.substr(0, pos), nullptr, 16);
    // std::stoul (string to unsigned long): https://en.cppreference.com/w/cpp/string/basic_string/stoul

    std::string raw_payload = raw_message.substr(pos+1); // cut from # excluded to the end
    if (raw_payload.length() % 2 != 0) throw_error("Invalid Payload format!"); // odd digits
    std::vector<uint8_t> Payload;

    // The payload is expressed as hexadecimal, it means that every digit is 4 bit data
    // So we get 2 digits at once in order to get a full byte expressable as ascii character
    for(size_t i = 0; i < raw_payload.size(); i+=2 /* iterate by 2 chars */) {
        Payload.push_back(std::stoul(raw_payload.substr(i, 2), nullptr, 16));  // adds 0xXX (1 byte) to the array
    }

    uint64_t timestamp = static_cast<uint64_t>(time(nullptr) * 1000 ); // time in ms
    /* difference between the normal cast and static_cast:
     *  - the normal explicit cast can make illegal type conversion
     *  - static_cast is more readable and is "cast-safe", the compile blocks any type
     *    of illegal conversion
    */
    return CAN_Message{id, Payload, timestamp};
}