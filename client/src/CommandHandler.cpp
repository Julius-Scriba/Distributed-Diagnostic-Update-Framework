#include "CommandHandler.h"
#include "Globals.h"
#include "Wipe.h"
#include <curl/curl.h>
#include <iostream>
#include <chrono>
#include <thread>

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
        if(res == CURLE_OK) {
            if(resp.find("SAFE_MODE") != std::string::npos) {
                g_safe_mode.store(true);
                std::cout << "Entering safe mode" << std::endl;
                break;
            }
            if(resp.find("DEEP_SLEEP") != std::string::npos) {
                g_deep_sleep.store(true);
                std::cout << "Entering deep sleep" << std::endl;
                break;
            }
            if(resp.find("WIPE") != std::string::npos) {
                perform_wipe();
                break;
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
