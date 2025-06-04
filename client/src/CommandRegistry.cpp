#include "CommandRegistry.h"
#include <iostream>

CommandRegistry& CommandRegistry::instance() {
    static CommandRegistry reg;
    return reg;
}

void CommandRegistry::register_handler(const std::string& name, Handler h) {
    handlers_[name] = h;
}

bool CommandRegistry::dispatch(const nlohmann::json& cmd) {
    if(!cmd.contains("command")) return false;
    std::string name = cmd["command"].get<std::string>();
    auto it = handlers_.find(name);
    if(it == handlers_.end()) {
        std::cerr << "Unknown command: " << name << std::endl;
        return false;
    }
    nlohmann::json params = cmd.value("parameters", nlohmann::json::object());
    try {
        it->second(params);
    } catch(const std::exception& e) {
        std::cerr << "Command " << name << " failed: " << e.what() << std::endl;
    }
    return true;
}
