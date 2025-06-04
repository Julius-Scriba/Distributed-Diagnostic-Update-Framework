#pragma once
#include "Module.h"
#include "Fingerprint.h"
class FingerprintModule : public Module {
public:
    std::string name() const override { return "Fingerprint"; }
    void init() override {
        auto data = collect_fingerprint();
        // placeholder: print
    }
};
extern "C" Module* create_module();
