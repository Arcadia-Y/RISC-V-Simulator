#include "../include/program_counter.hpp"

int Program_Counter::get() const
{
    return counter;
} 

int Program_Counter::get_next() const
{
    return next_counter;
}

void Program_Counter::set(int goal)
{
    next_counter = goal;
}

void Program_Counter::update()
{
    counter = next_counter;
    next_counter = counter + 4;
}

int Program_Counter::branch(int goal)
{
    if (status > 1)
    {
        next_counter = goal;
        return counter + 4;
    }
    else
    {
        next_counter = counter + 4;
        return goal;
    }
}

bool Program_Counter::verify(bool res)
{
    if (status > 1)
    {
        status += res ? 1 : -1;
        status = status > 3 ? 3 : status;
        return res;
    }
    else
    {
        status += res ? 1 : -1;
        status = status < 0 ? 0 : status;
        return !res;
    }
}
