#include "Loader.h"
#include <filesystem>
#include <dlfcn.h>
#include <iostream>

Loader::Loader(const std::string& path) : path_(path) {}

void Loader::load() {
    namespace fs = std::filesystem;
    for (const auto& entry : fs::directory_iterator(path_)) {
        if (entry.path().extension() == ".so") {
            void* handle = dlopen(entry.path().c_str(), RTLD_NOW);
            if (!handle) {
                std::cerr << "Failed to load " << entry.path() << "\n";
                continue;
            }
            auto create = (Module* (*)())dlsym(handle, "create_module");
            if (!create) {
                std::cerr << "Invalid module " << entry.path() << "\n";
                dlclose(handle);
                continue;
            }
            modules_.emplace_back(create());
            handles_.push_back(handle);
        }
    }
}

const std::vector<std::unique_ptr<Module>>& Loader::modules() const {
    return modules_;
}
