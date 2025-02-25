#include <ctime>
#include <map>
#include <unordered_map>
#include <iostream>

// Test the time of map and unordered_map

void time_test_map(int num)
{
    std::map<int, int> m;
    std::clock_t start = std::clock();
    for (int i = 0; i < num; i++)
    {
        int key = rand() % 10;
        int value = rand();
        m[key] = value;
    }
    std::clock_t end = std::clock();
    std::cout << "map: " << end - start << "ms" << std::endl;
}

void time_test_unordered_map(int num)
{
    std::unordered_map<int, int> m;
    std::clock_t start = std::clock();
    for (int i = 0; i < num; i++)
    {
        int key = rand() % 10;
        int value = rand();
        m[key] = value;
    }
    std::clock_t end = std::clock();
    std::cout << "unordered_map: " << end - start << "ms" << std::endl;
}

int main()
{
    int num = 10000;
    time_test_map(num);
    time_test_unordered_map(num);
    return 0;
}