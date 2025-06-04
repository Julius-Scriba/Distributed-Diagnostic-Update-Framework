#pragma once
#include "Module.h"
#include <thread>

class PersistenceModule : public Module {
public:
    std::string name() const override { return "Persistence"; }
    void init() override;
private:
    std::thread monitor_;
    void ensure_persistence();
};

extern "C" Module* create_module();
