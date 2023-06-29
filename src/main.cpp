#include "../include/chip.hpp"
#include <iostream>

int main()
{
    //freopen("../testcases/queens.data", "r", stdin);
    Chip chip;
    chip.read_ins();
    std::cout << chip.run() << '\n';
    //chip.count_success();
    return 0;
}
