#pragma once
#include <string>
struct FingerprintData {
    std::string uuid;
    std::string hostname;
};
FingerprintData collect_fingerprint();
