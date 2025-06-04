#pragma once
#include "Module.h"
class UpdateHandler : public Module {
public:
    std::string name() const override { return "UpdateHandler"; }
    void init() override {} // placeholder
};
extern "C" Module* create_module();
