#include "../include/chip.hpp"
#include <iostream>

int main()
{
    #ifdef COUNT_SUCCESS
    std::string testcases[] = {"array_test1", "array_test2", "basicopt1", "bulgarian",
        "expr", "gcd", "hanoi", "lvalue2", "magic", "manyarguments", "multiarray", 
        "naive", "pi", "qsort", "queens", "statement_test", "superloop", "tak"};
    freopen("result.txt", "w", stdout);
    unsigned long total = 0, success = 0;
    for (int i = 0; i < 18; i++)
    {
        std::string name = "../testcases/";
        name += testcases[i];
        name += ".data";
        freopen(name.c_str(), "r", stdin);
        Chip chip;
        chip.read_ins();
        chip.run();
        std::cout << testcases[i] << ":\n";
        auto info = chip.count_success();
        total += info.total;
        success += info.success;
        std::cout << "Total: " << info.total << "\n";
        std::cout << "Success " << info.success << "\n";
        std::cout << "Rate: " << info.rate * 100 << "%\n";
        std::cout << '\n';
        std::cin.clear();
    }
    std::cout << "Allcases:\n";
    std::cout << "Total: " << total << '\n';
    std::cout << "Success: " << success << '\n';
    std::cout << "Rate: " << (double) success / total * 100 << "%\n";

    #else
    Chip chip;
    chip.read_ins();
    std::cout << chip.run() << '\n';
    #endif

    return 0;
}
