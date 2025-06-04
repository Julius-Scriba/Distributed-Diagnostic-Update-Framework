#pragma once
#include <functional>
#include <string>
#include <map>
#include "json.hpp"

class CommandRegistry {
public:
    using Handler = std::function<void(const nlohmann::json&)>;
    static CommandRegistry& instance();
    void register_handler(const std::string& name, Handler h);
    bool dispatch(const nlohmann::json& cmd);
private:
    std::map<std::string, Handler> handlers_;
};
