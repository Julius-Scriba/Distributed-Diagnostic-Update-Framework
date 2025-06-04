#pragma once
#include <string>
#include <vector>

class Crypto {
public:
    static std::vector<unsigned char> encrypt(const std::vector<unsigned char>& data, const std::string& key, const std::string& iv);
    static std::vector<unsigned char> decrypt(const std::vector<unsigned char>& data, const std::string& key, const std::string& iv);
};
