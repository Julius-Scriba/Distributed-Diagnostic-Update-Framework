#include "Crypto.h"
#include <openssl/evp.h>
#include <vector>
#include <cstring>

std::vector<unsigned char> Crypto::encrypt(const std::vector<unsigned char>& data, const std::string& key, const std::string& iv) {
    EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();
    std::vector<unsigned char> out(data.size() + EVP_MAX_BLOCK_LENGTH);
    int outlen1 = 0, outlen2 = 0;
    EVP_EncryptInit_ex(ctx, EVP_aes_256_cbc(), nullptr, (const unsigned char*)key.data(), (const unsigned char*)iv.data());
    EVP_EncryptUpdate(ctx, out.data(), &outlen1, data.data(), data.size());
    EVP_EncryptFinal_ex(ctx, out.data()+outlen1, &outlen2);
    out.resize(outlen1 + outlen2);
    EVP_CIPHER_CTX_free(ctx);
    return out;
}

std::vector<unsigned char> Crypto::decrypt(const std::vector<unsigned char>& data, const std::string& key, const std::string& iv) {
    EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();
    std::vector<unsigned char> out(data.size());
    int outlen1 = 0, outlen2 = 0;
    EVP_DecryptInit_ex(ctx, EVP_aes_256_cbc(), nullptr, (const unsigned char*)key.data(), (const unsigned char*)iv.data());
    EVP_DecryptUpdate(ctx, out.data(), &outlen1, data.data(), data.size());
    EVP_DecryptFinal_ex(ctx, out.data()+outlen1, &outlen2);
    out.resize(outlen1 + outlen2);
    EVP_CIPHER_CTX_free(ctx);
    return out;
}
