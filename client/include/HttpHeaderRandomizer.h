#pragma once
#include <curl/curl.h>
#include <string>
#include <vector>

class HttpHeaderRandomizer {
public:
    HttpHeaderRandomizer();
    ~HttpHeaderRandomizer();
    void apply(CURL* curl) const;
private:
    std::vector<std::string> user_agents_;
    std::vector<std::string> accept_lang_;
    std::vector<std::string> cache_control_;
    std::vector<std::string> connection_;
    std::vector<std::string> accept_encoding_;

    std::string ua_;
    std::string lang_;
    std::string cache_;
    std::string conn_;
    std::string encoding_;
    struct curl_slist* header_list_;
};

extern HttpHeaderRandomizer g_header_randomizer;
