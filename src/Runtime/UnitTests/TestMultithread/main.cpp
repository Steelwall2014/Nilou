#include <async++/async++.h>
#include <iostream>

using namespace std;

int main()
{
    auto task2 = async::spawn([]() -> int {
        std::cout << "Task 2 executes in parallel with task 1" << std::endl;
        return 42;
    });
    auto task3 = task2.then([](int value) -> int {
        std::cout << "Task 3 executes after task 2, which returned "
                  << value << std::endl;
        return value * 3;
    });
    task3.get();
    return 0;
}