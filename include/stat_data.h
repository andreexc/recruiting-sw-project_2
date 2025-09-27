#pragma once
#include <stdint.h>

struct StatData {
    size_t count;
    uint64_t lastTimestamp;
    double mean_time;
};