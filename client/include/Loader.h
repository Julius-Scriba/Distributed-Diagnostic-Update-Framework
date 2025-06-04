#pragma once
#include <string>
#include <vector>
#include <memory>
#include "Module.h"
class Loader {
public:
    explicit Loader(const std::string& path);
    void load();
    const std::vector<std::unique_ptr<Module>>& modules() const;
private:
    std::string path_;
    std::vector<void*> handles_;
    std::vector<std::unique_ptr<Module>> modules_;
};
