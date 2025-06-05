#pragma once
#include "CommandModule.h"
#include <string>
#include <cstdint>

#ifdef _WIN32
#include <windows.h>
extern "C" __declspec(dllimport) NTSTATUS NtCreateProcessEx(
    HANDLE*, ACCESS_MASK, void*, HANDLE, ULONG, HANDLE, HANDLE, HANDLE, BOOLEAN);
#endif

class PPIDSpoofingStub : public CommandModule {
public:
    std::string name() const override { return "PPIDSpoofingStub"; }
    void init() override;

    bool spawn_with_ppid(const std::string& target_exe, uint32_t ppid);
};

extern "C" Module* create_module();
