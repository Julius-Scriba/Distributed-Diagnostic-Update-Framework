#include "AntiForensic/ProcessHollowingStub.h"
#include "json.hpp"
#include <iostream>

bool ProcessHollowingStub::hollow_process(const std::string& target_exe,
                                          const std::string& payload_path) {
    (void)target_exe;
    (void)payload_path;
#ifdef DEBUG
    std::cout << "[DEBUG] hollow_process stub: " << target_exe
              << " <- " << payload_path << std::endl;
#endif
    return false;
}

void ProcessHollowingStub::init() {
    register_command("PROCESS_HOLLOW", [this](const nlohmann::json& p){
#ifdef DEBUG
        std::cout << "[DEBUG] PROCESS_HOLLOW command received" << std::endl;
#endif
        if(!p.contains("target") || !p.contains("payload")) return;
        std::string target = p["target"].get<std::string>();
        std::string pay = p["payload"].get<std::string>();
        hollow_process(target, pay);
    });
}

extern "C" Module* create_module() { return new ProcessHollowingStub(); }
