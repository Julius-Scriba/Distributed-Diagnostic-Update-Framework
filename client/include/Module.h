#pragma once
#include <string>
class Module {
public:
    virtual ~Module() = default;
    virtual std::string name() const = 0;
    virtual void init() = 0;
};
extern "C" Module* create_module();
