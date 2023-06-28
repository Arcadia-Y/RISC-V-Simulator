#ifndef ROB_HPP
#define ROB_HPP

#include "ins.hpp"
#include "myqueue.hpp"
#include <unordered_map>

// under the consideration that ROB is highly related to other
// parts of the chip, functions of ROB are implemented in Chip
struct Reorder_Buffer
{
    struct Entry
    {
        Ins ins;
        char state = 0; // 1 means ready to be committed
        int dest; // dest for address to jump when prediction fails, address for store
        int value; // address for load
    };
    Myqueue<Entry, 8> buffer, next;
    void update()
    {
        buffer = next;
    }
    void clear()
    {
        next.clear();
    }
};

#endif