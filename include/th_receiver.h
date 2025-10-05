#pragma once
#include <vector>
#include <cstdint>

bool start_receiver(const char*);
void stop_receiver();
bool read_message(std::vector<uint8_t>&); // need to use vectors because while dealing hexadecimals
                                          // like 0x00 the std::string will transform that value in
                                          // '\0' that will terminate the string leading to
                                          // inconsistency with the lenght of the one