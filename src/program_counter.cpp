#include "../include/program_counter.hpp"

Program_Counter::Program_Counter()
{
    for (int i = 0; i < hsize; i++)
        status[i] = 1;
}

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
    int now = hash(counter) % hsize;
    history.push(now);
    if (status[now] > 1)
    {
        prediction.push(true);
        next_counter = goal;
        return counter + 4;
    }
    else
    {
        prediction.push(false);
        next_counter = counter + 4;
        return goal;
    }
}

bool Program_Counter::verify(bool res)
{
    int now = *history.begin();
    history.pop();   
    if (status[now] > 1)
    {
        status[now] += res ? 1 : -1;
        status[now] = status[now] > 3 ? 3 : status[now];
    }
    else
    {
        status[now] += res ? 1 : -1;
        status[now] = status[now] < 0 ? 0 : status[now];
    }
    bool last_predict = *prediction.begin();
    prediction.pop();
    bool ret = last_predict == res;
    ++total;
    success += ret;
    if (!ret)
    {
        history.clear();
        prediction.clear();
    }
    return ret;
}

unsigned int Program_Counter::hash(unsigned int x)
{
    x ^= 82589046;
    x ^= x << 13;
    x ^= x >> 7;
    x ^= x << 17;
    x ^= 82589046;
    return x;
}

void Program_Counter::count_success()
{
    std::cout << "Total: " << total << '\n';
    std::cout << "Success: " << success << '\n';
    if (!total) total = 1;
    std::cout << "Success Rate: " << (double) success / total * 100 << "%\n";
}
