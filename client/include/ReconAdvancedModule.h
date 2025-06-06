#pragma once
#include "CommandModule.h"
#include <string>
#include "json.hpp"

class ReconAdvancedModule : public CommandModule {
public:
    std::string name() const override { return "ReconAdvanced"; }
    void init() override;
private:
    nlohmann::json collect();
    std::string encrypt_report(const std::string& js);
};

extern "C" Module* create_module();
