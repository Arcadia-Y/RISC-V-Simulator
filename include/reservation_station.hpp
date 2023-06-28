#ifndef RS_HPP
#define RS_HPP

#include "ins.hpp"
#include <unordered_map>
#include "myqueue.hpp"
#include "reorder_buffer.hpp"

class Reservation_Station
{
public:
    struct Entry
    {
        Ins op;
        int vj, vk; // computation result is stored in vj
        int qj = -1, qk = -1;
        int a = 0; // offset for load/store
        int dest;
        bool status = 0;
    };
    Reservation_Station(std::unordered_map<int, int>* _cdb):
        cdb(_cdb){}
    bool available() {return buffer.available();}
    void push(const Entry& x);
    void execute();
    void update();
    void clear();

protected: 
    bool exec_flag = false, write_flag = false;
    Myqueue<Entry, 4> buffer, next;
    std::unordered_map<int, int> *cdb;

    void calculate(Entry& x);
};

class Load_Store_Buffer : public Reservation_Station
{
public:
    Load_Store_Buffer(std::unordered_map<int, int>* _cdb, Reorder_Buffer* _rob):
        Reservation_Station(_cdb), rob(_rob) {}
    void execute();
private:
    Reorder_Buffer* rob;
};

#endif
