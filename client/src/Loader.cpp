#include "Loader.h"
#include <filesystem>
#include <dlfcn.h>
#include <iostream>

Loader::Loader(const std::string& path) : path_(path) {}

void Loader::load() {
    namespace fs = std::filesystem;
    std::vector<fs::path> files;
    for (const auto& entry : fs::directory_iterator(path_)) {
        if(entry.path().extension() == ".so")
            files.push_back(entry.path());
    }
    std::sort(files.begin(), files.end());
    for (const auto& file : files) {
        void* handle = dlopen(file.c_str(), RTLD_NOW);
        if (!handle) {
            std::cerr << "Failed to load " << file << "\n";
            continue;
        }
        auto create = (Module* (*)())dlsym(handle, "create_module");
        if (!create) {
            std::cerr << "Invalid module " << file << "\n";
            dlclose(handle);
            continue;
        }
        modules_.emplace_back(create());
        handles_.push_back(handle);
    }
}

const std::vector<std::unique_ptr<Module>>& Loader::modules() const {
    return modules_;
}
