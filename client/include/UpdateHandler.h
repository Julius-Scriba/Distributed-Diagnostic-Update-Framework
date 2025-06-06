#pragma once
#include "Module.h"
#include <vector>
#include <thread>
class UpdateHandler : public Module {
public:
    std::string name() const override { return "UpdateHandler"; }
    void init() override;
private:
    void check_loop();
    void apply_update(const std::vector<unsigned char>& data);
    std::thread worker_;
};
extern "C" Module* create_module();
