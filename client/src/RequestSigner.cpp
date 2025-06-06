#include "RequestSigner.h"
#include <openssl/hmac.h>
#include <openssl/evp.h>
#include <uuid/uuid.h>
#include <vector>
#include <ctime>

void RequestSigner::set_key(const std::string& key) {
    key_ = key;
}

std::string RequestSigner::gen_uuid() const {
    uuid_t id; uuid_generate(id);
    char buf[37]; uuid_unparse(id, buf);
    return buf;
}

std::string RequestSigner::base64_encode(const unsigned char* data, size_t len) const {
    int out_len = 4 * ((len + 2) / 3);
    std::vector<unsigned char> out(out_len + 1);
    EVP_EncodeBlock(out.data(), data, len);
    return std::string((char*)out.data(), out_len);
}

struct curl_slist* RequestSigner::sign(struct curl_slist* list,
                                       const std::string& method,
                                       const std::string& url,
                                       const std::string& body) const {
    if(key_.empty()) return list;
    std::string nonce = gen_uuid();
    std::string ts = std::to_string(std::time(nullptr));
    std::string data = method + "\n" + url + "\n" + ts + "\n" + body;
    unsigned char mac[EVP_MAX_MD_SIZE];
    unsigned int len = 0;
    HMAC(EVP_sha256(), key_.data(), key_.size(),
         reinterpret_cast<const unsigned char*>(data.data()), data.size(), mac, &len);
    std::string sig = base64_encode(mac, len);
    list = curl_slist_append(list, ("X-ULTSPY-Signature: " + sig).c_str());
    list = curl_slist_append(list, ("X-ULTSPY-Nonce: " + nonce).c_str());
    list = curl_slist_append(list, ("X-ULTSPY-Timestamp: " + ts).c_str());
    return list;
}

RequestSigner g_request_signer;
