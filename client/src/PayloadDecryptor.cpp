#include "PayloadDecryptor.h"
#include "Crypto.h"
#include <fstream>

PayloadDecryptor::PayloadDecryptor(const std::string& key) : key_(key) {}

std::vector<unsigned char> PayloadDecryptor::decrypt_file(const std::string& path) const {
    std::ifstream f(path, std::ios::binary);
    if(!f) return {};
    std::vector<unsigned char> iv(16);
    f.read(reinterpret_cast<char*>(iv.data()), iv.size());
    std::vector<unsigned char> enc((std::istreambuf_iterator<char>(f)), std::istreambuf_iterator<char>());
    if(enc.empty()) return {};
    std::string iv_str(reinterpret_cast<char*>(iv.data()), iv.size());
    return Crypto::decrypt(enc, key_, iv_str);
}
