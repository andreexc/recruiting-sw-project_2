#pragma once
#include "can_message.h"

// Payload € [0, 255]
const CAN_Message START_MESSAGES[] = {
//  {  ID , { Payload  }, 0}    // (timestamp not required)
    {0x0A0, {0x66, 0x01}, 0},   // 0A0#6601
    {0x0A0, {0xFF, 0x01}, 0}    // 0A0#FF01
};

const CAN_Message STOP_MESSAGES[] = {
    {0x0A0, {0x66, 0xFF}, 0}    // 0A0#66FF
};