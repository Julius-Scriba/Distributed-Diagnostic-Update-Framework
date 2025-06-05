#pragma once
#include "CommandModule.h"
#include <string>

class ProcessHollowingStub : public CommandModule {
public:
    std::string name() const override { return "ProcessHollowingStub"; }
    void init() override;

    bool hollow_process(const std::string& target_exe,
                        const std::string& payload_path);
};

extern "C" Module* create_module();
