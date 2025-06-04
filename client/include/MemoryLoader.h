#pragma once
#include <vector>

class MemoryLoader {
public:
    static void* load(const std::vector<unsigned char>& data);
};
