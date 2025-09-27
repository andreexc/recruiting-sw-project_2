#ifndef CAN_MESSAGE_H
#define CAN_MESSAGE_H

#include <cstdint>
#include <vector>

typedef struct {
    uint16_t ID;                    // ID (max 12 bits < uint16_t)
    std::vector<uint8_t> Payload;   // Payload (array of bytes)
                                    // (i chose vector instead of array for memory over speed)
    uint64_t timestamp;             // ms from epoch (Unix timestamp: 1/1/1970) 
} CAN_Message;

#endif // CAN_MESSAGE_