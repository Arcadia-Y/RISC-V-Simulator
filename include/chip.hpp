#ifndef CHIP_HPP
#define CHIP_HPP

#include <unordered_map>
#include <iostream>
#include "reservation_station.hpp"
#include "reorder_buffer.hpp"
#include "program_counter.hpp"

class Chip
{
public:
    Chip(): lsb(&cdb, &rob), alu(&cdb) {}
    int run();
    void read_ins();
    Prediction_Report count_success();

private:
    // control flow is implemented by function, so control unit is omitted
    struct Register
    {
        int status = -1; // -1 means available
        int data = 0;
    };
    int ins, next_ins;
    Program_Counter pc;
    Reorder_Buffer rob;
    Load_Store_Buffer lsb;
    Reservation_Station alu;
    Register reg[32];
    Register next_reg[32];
    std::unordered_map<int, int> cdb;
    std::unordered_map<int, unsigned char> ram;
    bool end_flag = false, clear_flag = false;

    void fetch();
    void decode_and_issue();
    void decode(Reservation_Station::Entry& rs_entry, Reorder_Buffer::Entry& rob_entry);
    void write_back();
    void update();
    void commit();
    int sign_extend(int x, int digit);
};

#endif