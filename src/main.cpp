#include <iostream>
#include "can_message.h"
#include "parser.h"
#include "FSM.h"

extern "C"{
    #include "fake_receiver.h"
}

int main(void){

    FSM* fsm = new FSM();

    std::string mess;
    std::cout<<"Insert code:"<<std::endl;
    while (true) {
        std::getline(std::cin, mess);    // reads the word until a space
        if (mess.empty()) continue;
        fsm->handleMessage(mess);
    }

    delete fsm;                          // watchout for memory leaks
    return 0;
}