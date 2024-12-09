// aoc2024_09.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <map>
#include <set>

std::vector<int> ReadInput(std::ifstream & ifstream)
{
    std::vector<int> lines;
    std::string string;
    
    std::getline(ifstream, string);
    for (auto& c : string)
    {
        lines.push_back(c - '0');
    }
    return lines;
}

std::vector<int> Expand(std::vector<int> const& line)
{
    bool empty = false;
    int id = 0;
    std::vector<int> expanded;
    for (auto& val : line)
    {
        int c = empty ? -1 : (id++);
        for (int i = 0; i < val; ++i)
        {
            expanded.push_back(c);
        }

        empty = !empty;
    }
    return expanded;
}

std::vector<int> Condense(std::vector<int> const& loose)
{
    std::vector<int> condensed = loose;
    auto start = condensed.begin();
    auto end = condensed.end() - 1;

    while (true)
    {
        while (*start != -1) ++start;
        while (*end == -1) --end;
        if (start >= end) break;
        *start = *end;
        *end = -1;
    }
    return condensed;
}

int64_t CheckSum(std::vector<int> const& condensed)
{
    int64_t count = 0;
    for (int i = 0; i < condensed.size(); ++i)
    {
        if (condensed[i] < 0) continue;
        count += i * condensed[i];
    }
    return count;
}

void FindFreeBlocks(std::vector<int> const& line, std::vector<std::pair<size_t, int>> & freeBlocks, std::vector<std::tuple<int, int, size_t>>& fullBlocks)
{
    bool empty = false;
    int id = 0;
    size_t currentLocation = 0;
    for (auto& val : line)
    {
        if (empty)
        {
            freeBlocks.push_back(std::make_pair(currentLocation, val));
        }
        else
        {
            fullBlocks.push_back(std::make_tuple(id, val, currentLocation));
        }

        currentLocation += val;
        empty = !empty;
    }
    return;
}

void FindFreeBlocks(std::vector<int> const& line, std::map<int, std::set<size_t>>& packedFreeBlocks, std::vector<std::tuple<int, int, size_t>>& fullBlocks)
{
    bool empty = false;
    int id = 0;
    size_t currentLocation = 0;
    for (auto& val : line)
    {
        if (empty)
        {
            packedFreeBlocks[val].insert(currentLocation);
        }
        else
        {
            fullBlocks.push_back(std::make_tuple(id, val, currentLocation));
        }

        currentLocation += val;
        empty = !empty;
    }
    return;
}

void ProcessFreeBlocks(std::vector<std::pair<size_t, int>>& freeBlocks, std::map<int, std::set<size_t>>& packedFreeBlocks)
{
    for (auto& [location, size] : freeBlocks)
    {
        packedFreeBlocks[size].insert(location);
    }
}

size_t GetFreeSpace(std::map<int, std::set<size_t>>& packedFreeBlocks, int requiredSize, size_t currentLocation)
{
    // Find first size big enough and with space
    auto itr = packedFreeBlocks.lower_bound(requiredSize);
    if (itr == packedFreeBlocks.end())
    {
        return currentLocation;
    }
    auto bestLoc = std::min(currentLocation, *itr->second.begin());
    auto bestItr = itr;
    for(; itr != packedFreeBlocks.end(); ++itr)
    {
        auto& [size, locations] = *itr;
        auto location = *locations.begin();
        if (location < bestLoc)
        {
            bestLoc = location;
            bestItr = itr;
        }
    }

    if(bestItr != packedFreeBlocks.end())
    {
        auto& [size, locations] = *bestItr;

        auto copyTo = *locations.begin();
        auto newSize = size - requiredSize;
        
        if (locations.size() == 1)
        {
            packedFreeBlocks.erase(size);
        }
        else
        {
            locations.erase(locations.begin());
        }

        auto& newLocations = packedFreeBlocks[newSize];
        newLocations.insert(copyTo + requiredSize);
        return copyTo;
    }

    return currentLocation;
}

size_t GetFreeSpace(std::vector<std::pair<size_t, int>>& freeBlocks, int requiredSize, size_t currentLocation)
{
    for (auto itr = freeBlocks.begin(); itr != freeBlocks.end(); ++itr)
    {
        auto& [location, size] = *itr;
        if (location >= currentLocation) break;
        if (size >= requiredSize && location < currentLocation)
        {
            auto copyTo = location;
            size -= requiredSize;
            location += requiredSize;
            return copyTo;
        }
    }
    return currentLocation;
}


std::string CondensedToString(std::vector<int> condensed)
{
    std::string s;
    for (auto i : condensed)
    {
        if (i == -1) s += '.';
        else         s += '0' + i;
    }
    return s;
}

#define FAST_MODE 1

std::vector<int> ShiftBlocks(std::vector<int> const& line)
{
    std::vector<std::tuple<int, int, size_t>> fullBlocks;
#if FAST_MODE 
    std::map<int, std::set<size_t>> freeBlocks;
#else
    std::vector<std::pair<size_t, int>> freeBlocks;
#endif

    FindFreeBlocks(line, freeBlocks, fullBlocks);

    bool lastIsFull = line.size() % 2;
    std::vector<int> condensed = Expand(line);
    for (auto itr = fullBlocks.rbegin(); itr != fullBlocks.rend(); ++itr)
    {
        auto& [id, size, loc] = *itr;
        size_t nextLocation = GetFreeSpace(freeBlocks, size, loc);
        if (nextLocation >= loc) continue;
        for (int i = 0; i < size; ++i)
        {
            condensed[nextLocation + i] = condensed[loc + i];
            condensed[loc + i] = -1;
        }
    }
    return condensed;
}

#include <chrono>
struct Timer
{
    using time_t = std::chrono::time_point<std::chrono::high_resolution_clock>;

    void Begin()
    {
        start = std::chrono::high_resolution_clock::now();
    }

    void End(const char* s, int itrs = 1)
    {
        time_t stop = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(stop - start).count();
        total += duration;
        printf("%s: %llu us\n", s, duration / itrs);
    }

    void PrintTotal()
    {
        printf("Total: %llu us\n", total);
    }

    time_t start;
    size_t total = 0;
};

int64_t Part1(std::vector<int> const& inputLine)
{
    auto str = Expand(inputLine);
    auto condensed = Condense(str);
    return CheckSum(condensed);
}

int main()
{
    Timer timer;

    std::ifstream ifstream;
    ifstream.open("input.txt");

    timer.Begin();
    auto inputLine = ReadInput(ifstream);
    timer.End("Input");

    timer.Begin();
    auto p1 = Part1(inputLine);
    printf("%lld\n", p1);
    timer.End("P1");

    timer.Begin();
    int64_t count;
    int itrs = 1024;
    for (int i = 0; i < itrs; ++i)
    {
        std::vector<std::pair<size_t, int>> freeBlocks;
        std::vector<std::tuple<int, int, size_t>> fullBlocks;
        auto condensed = ShiftBlocks(inputLine);
        count = CheckSum(condensed);
    }
    timer.End("P2", itrs);
    printf("%lld\n", count);
    timer.PrintTotal();
}

// Run program: Ctrl + F5 or Debug > Start Without Debugging menu
// Debug program: F5 or Debug > Start Debugging menu

// Tips for Getting Started: 
//   1. Use the Solution Explorer window to add/manage files
//   2. Use the Team Explorer window to connect to source control
//   3. Use the Output window to see build output and other messages
//   4. Use the Error List window to view errors
//   5. Go to Project > Add New Item to create new code files, or Project > Add Existing Item to add existing code files to the project
//   6. In the future, to open this project again, go to File > Open > Project and select the .sln file
