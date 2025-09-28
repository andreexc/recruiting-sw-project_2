#pragma once
#include <fstream>
#include <string>
#include <unordered_map>
#include "can_message.h"
#include "stat_data.h"
#include "FSM_codes.h"

enum State {
    IDLE, RUN
}; // Possible states of the FSM

class FSM {
    private:
        State state;
        std::unordered_map<uint16_t, StatData> session_statistics;
        std::fstream log_file;

    public:
        FSM();
        void handleMessage(const std::string& raw_message);

    private:
        void handleOnIDLE(const CAN_Message& msg, const std::string& raw_msg);
        void handleOnRUN(const CAN_Message& msg, const std::string& raw_msg);
        void do_log(const CAN_Message& msg, const std::string& raw_msg);
        bool isStartMessage(const CAN_Message& msg);
        bool isStopMessage(const CAN_Message& msg);
        void startRunSession();
        void stopRunSession();
        void updateStats(const CAN_Message& msg);
        void saveStatsOnCSV();
};