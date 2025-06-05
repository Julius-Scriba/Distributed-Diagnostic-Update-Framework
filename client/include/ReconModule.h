#pragma once
#include "CommandModule.h"
#include <string>
#include "json.hpp"

class ReconModule : public CommandModule {
public:
    std::string name() const override { return "Recon"; }
    void init() override;
private:
    nlohmann::json collect();
    std::string encrypt_report(const std::string& json);
};

extern "C" Module* create_module();
