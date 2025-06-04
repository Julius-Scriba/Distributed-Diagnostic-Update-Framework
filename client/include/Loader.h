#pragma once
#include <string>
#include <vector>
#include <memory>
#include "Module.h"
class Loader {
public:
    Loader(const std::string& path, const std::string& aes_key);
    void load();
    const std::vector<std::unique_ptr<Module>>& modules() const;
private:
    std::string path_;
    std::string key_;
    std::vector<void*> handles_;
    std::vector<std::unique_ptr<Module>> modules_;
};
