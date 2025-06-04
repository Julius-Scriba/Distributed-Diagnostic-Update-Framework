#pragma once
#include <string>
#include <array>

template<size_t N, char Key>
class XorStr {
public:
    std::array<char, N> data;
    constexpr XorStr(const char (&s)[N]) : data{} {
        for(size_t i=0; i<N; ++i)
            data[i] = s[i] ^ Key;
    }
    std::string decode() const {
        std::string out(N-1, '\0');
        for(size_t i=0; i<N-1; ++i)
            out[i] = data[i] ^ Key;
        return out;
    }
};

#define OBFUSCATE(s) (XorStr<sizeof(s), 0x5A>(s).decode())
