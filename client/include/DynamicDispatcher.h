#pragma once
#include "Module.h"
#include "CommandRegistry.h"
#include "json.hpp"
class DynamicDispatcher : public Module {
public:
    std::string name() const override { return "DynamicDispatcher"; }
    void init() override;
};

extern "C" Module* create_module();
