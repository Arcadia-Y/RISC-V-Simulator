#ifndef PC_HPP
#define PC_HPP

#include "myqueue.hpp"

class Program_Counter
{
public:
    Program_Counter();
    int get() const;
    int get_next() const;
    void set(int goal);
    // goal is address to jump, return the address to jump when prediction fails
    int branch(int goal);
    // res is result of boolean exprssion, return if prediction is true 
    bool verify(bool res); 
    void update();

private:
    const static int hsize = 32;
    bool last_predict;
    int counter = 0;
    int next_counter = 0;
    int status[hsize];
    Myqueue<int, 8> history;
    Myqueue<bool, 8> prediction;
    unsigned int hash(unsigned int x);
};

#endif