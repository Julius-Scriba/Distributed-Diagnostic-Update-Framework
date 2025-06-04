#include "HttpHeaderRandomizer.h"
#include <random>

HttpHeaderRandomizer::HttpHeaderRandomizer() {
    user_agents_ = {
        "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/114.0 Safari/537.36",
        "Mozilla/5.0 (Macintosh; Intel Mac OS X 10_15_7) AppleWebKit/605.1.15 (KHTML, like Gecko) Version/16.0 Safari/605.1.15",
        "Mozilla/5.0 (X11; Ubuntu; Linux x86_64; rv:109.0) Gecko/20100101 Firefox/115.0",
        "Mozilla/5.0 (Windows NT 10.0; Win64; x64; rv:109.0) Gecko/20100101 Firefox/115.0",
        "Mozilla/5.0 (X11; Linux x86_64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/114.0 Safari/537.36",
        "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Edge/115.0",
        "Mozilla/5.0 (Macintosh; Intel Mac OS X 10_14_6) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/113.0 Safari/537.36",
        "Mozilla/5.0 (X11; Linux x86_64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/113.0 Safari/537.36",
        "Mozilla/5.0 (iPhone; CPU iPhone OS 16_0 like Mac OS X) AppleWebKit/605.1.15 (KHTML, like Gecko) Version/16.0 Mobile/15E148 Safari/604.1",
        "Mozilla/5.0 (Windows NT 6.1; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/112.0 Safari/537.36"
    };
    accept_lang_ = {
        "en-US,en;q=0.9",
        "de-DE,de;q=0.8,en-US;q=0.5,en;q=0.3",
        "en-GB,en;q=0.9",
        "fr-FR,fr;q=0.8,en-US;q=0.5,en;q=0.3",
        "es-ES,es;q=0.8,en-US;q=0.5,en;q=0.3"
    };
    cache_control_ = {
        "no-cache",
        "no-store",
        "max-age=0",
        "max-age=0, no-cache",
        "private"
    };
    connection_ = { "keep-alive", "close" };
    accept_encoding_ = {
        "gzip, deflate",
        "gzip, deflate, br",
        "identity",
        "deflate, gzip",
        "br, gzip, deflate"
    };

    std::random_device rd;
    std::mt19937 gen(rd());
    ua_ = user_agents_[gen() % user_agents_.size()];
    lang_ = accept_lang_[gen() % accept_lang_.size()];
    cache_ = cache_control_[gen() % cache_control_.size()];
    conn_ = connection_[gen() % connection_.size()];
    encoding_ = accept_encoding_[gen() % accept_encoding_.size()];

    headers_.push_back("User-Agent: " + ua_);
    headers_.push_back("Accept-Language: " + lang_);
    headers_.push_back("Cache-Control: " + cache_);
    headers_.push_back("Connection: " + conn_);
    headers_.push_back("Accept-Encoding: " + encoding_);
}

struct curl_slist* HttpHeaderRandomizer::build_list() const {
    struct curl_slist* list = nullptr;
    for(const auto& h : headers_) list = curl_slist_append(list, h.c_str());
    return list;
}

HttpHeaderRandomizer g_header_randomizer;

