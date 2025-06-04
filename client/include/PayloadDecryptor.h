#pragma once
#include <string>
#include <vector>

class PayloadDecryptor {
public:
    PayloadDecryptor(const std::string& key);
    std::vector<unsigned char> decrypt_file(const std::string& path) const;
private:
    std::string key_;
};
