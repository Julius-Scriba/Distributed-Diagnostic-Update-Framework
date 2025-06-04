#include "Loader.h"
#include "Fingerprint.h"
#include "Globals.h"
#include <iostream>
#include <thread>
#include <chrono>
#include <curl/curl.h>

static size_t write_cb(void* contents, size_t size, size_t nmemb, void* userp) {
    return size * nmemb;
}

static void heartbeat(const std::string& server) {
    CURL* curl = curl_easy_init();
    if(!curl) return;
    while(true) {
        std::string url = server + "/heartbeat";
        std::string payload = "{\"uuid\":\"" + g_uuid + "\"}";
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, payload.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_cb);
        curl_easy_perform(curl);
        std::this_thread::sleep_for(std::chrono::minutes(1));
    }
    curl_easy_cleanup(curl);
}

int main() {
    auto fp = collect_fingerprint();
    g_uuid = fp.uuid;
    std::thread hb(heartbeat, "http://localhost:5000");
    hb.detach();

    Loader loader("./plugins");
    loader.load();
    for (const auto& mod : loader.modules()) {
        if(g_safe_mode.load()) break;
        std::cout << "Loaded module: " << mod->name() << std::endl;
        mod->init();
    }
    while(!g_safe_mode.load()) {
        std::this_thread::sleep_for(std::chrono::seconds(5));
    }
    // safe mode active; keep heartbeat alive
    return 0;
}
