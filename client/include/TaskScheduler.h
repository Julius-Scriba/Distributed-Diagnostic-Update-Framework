#pragma once
#include "Module.h"
#include "json.hpp"
#include <vector>
#include <thread>
#include <mutex>
#include <chrono>

struct ScheduledTask {
    nlohmann::json cmd;
    std::chrono::steady_clock::time_point execute_at;
};

class TaskScheduler : public Module {
public:
    std::string name() const override { return "TaskScheduler"; }
    void init() override;
    void schedule(const nlohmann::json& cmd, const nlohmann::json& sched);
private:
    void loop();
    std::vector<ScheduledTask> tasks_;
    std::mutex mtx_;
    std::thread worker_;
    bool running_ = true;
};

extern TaskScheduler* g_task_scheduler;
extern "C" Module* create_module();
