#include <stdio.h>
#include <iostream>
#include <cstring>
#include "can_message.h"
#include "parser.h"

extern "C"{
    #include "fake_receiver.h"
}

int main(void){

    std::cout << "Welcome to Project 2" << std::endl;

    std::string test = "0A0#6601";
    CAN_Message out = parse_message(test);

    std::cout<<"CAN message: \n"
             <<"ID: "<<out.ID<<"\n"
             <<"Payload: ";
    for (uint8_t byte : out.Payload) {
        std::cout<<byte; // uint8_t is printed as char!!
    }
    std::cout<<"\n"
             <<"Unix timestamp: "<<out.timestamp<<"\n";

    return 0;
}