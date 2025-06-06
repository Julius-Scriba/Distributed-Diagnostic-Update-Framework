#include "AntiForensic/PPIDSpoofingStub.h"
#include "json.hpp"
#include <iostream>

bool PPIDSpoofingStub::spawn_with_ppid(const std::string& target_exe,
                                       uint32_t ppid) {
    (void)target_exe;
    (void)ppid;
#ifdef DEBUG
    std::cout << "[DEBUG] spawn_with_ppid stub: " << target_exe
              << " <- PPID " << ppid << std::endl;
#endif
    return false;
}

void PPIDSpoofingStub::init() {
    register_command("PPID_SPOOF", [this](const nlohmann::json& cmd){
#ifdef DEBUG
        std::cout << "[DEBUG] PPID_SPOOF command received" << std::endl;
#endif
        nlohmann::json p = cmd.value("parameters", nlohmann::json::object());
        if(!p.contains("target") || !p.contains("ppid")) return;
        std::string target = p["target"].get<std::string>();
        uint32_t id = p["ppid"].get<uint32_t>();
        spawn_with_ppid(target, id);
    });
}

extern "C" Module* create_module() { return new PPIDSpoofingStub(); }
