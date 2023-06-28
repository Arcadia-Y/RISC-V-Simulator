#ifndef PC_HPP
#define PC_HPP

class Program_Counter
{
public:
    int get() const;
    int get_next() const;
    void set(int goal);
    // goal is address to jump, return the address to jump when prediction fails
    int branch(int goal);
    // res is result of boolean exprssion, return if prediction is true 
    bool verify(bool res); 
    void update();

private:
    int counter = 0;
    int next_counter = 0;
    int status = 2; // status for predicting jump, jump if >1
};

#endif