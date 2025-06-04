#include "Loader.h"
#include "PayloadDecryptor.h"
#include "MemoryLoader.h"
#include <filesystem>
#include <dlfcn.h>
#include <iostream>

Loader::Loader(const std::string& path, const std::string& aes_key)
    : path_(path), key_(aes_key) {}

void Loader::load() {
    namespace fs = std::filesystem;
    std::vector<fs::path> files;
    for (const auto& entry : fs::directory_iterator(path_)) {
        if(entry.path().extension() == ".so" || entry.path().extension() == ".enc")
            files.push_back(entry.path());
    }
    std::sort(files.begin(), files.end());
    PayloadDecryptor decryptor(key_);
    for (const auto& file : files) {
        void* handle = nullptr;
        if(file.extension() == ".enc") {
            auto data = decryptor.decrypt_file(file.string());
            if(data.empty()) {
                std::cerr << "Failed to decrypt " << file << "\n";
                continue;
            }
            handle = MemoryLoader::load(data);
        } else {
            handle = dlopen(file.c_str(), RTLD_NOW);
        }
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
