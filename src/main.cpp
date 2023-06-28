#include "../include/chip.hpp"
#include <iostream>

int main()
{
    Chip chip;
    chip.read_ins();
    std::cout << chip.run() << '\n';
    return 0;
}
