#pragma once
#include <curl/curl.h>
#include <string>

class RequestSigner {
public:
    void set_key(const std::string& key);
    struct curl_slist* sign(struct curl_slist* list,
                            const std::string& method,
                            const std::string& url,
                            const std::string& body) const;
private:
    std::string key_;
    std::string gen_uuid() const;
    std::string base64_encode(const unsigned char* data, size_t len) const;
};

extern RequestSigner g_request_signer;
