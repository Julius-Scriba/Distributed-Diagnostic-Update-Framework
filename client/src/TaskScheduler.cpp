#include "TaskScheduler.h"
#include "CommandRegistry.h"
#include <cstdlib>
#include <iostream>

TaskScheduler* g_task_scheduler = nullptr;

void TaskScheduler::init() {
    g_task_scheduler = this;
    worker_ = std::thread(&TaskScheduler::loop, this);
    worker_.detach();
}

void TaskScheduler::schedule(const nlohmann::json& cmd, const nlohmann::json& sched) {
    if(!sched.contains("type")) return;
    std::string type = sched["type"].get<std::string>();
    if(type != "delayed") return;
    int delay = sched.value("delay_seconds", 0);
    int rand_off = sched.value("random_offset", 0);
    if(rand_off > 0) {
        delay += std::rand() % rand_off;
    }
    nlohmann::json clean = cmd;
    clean.erase("schedule");
    ScheduledTask task{clean, std::chrono::steady_clock::now() + std::chrono::seconds(delay)};
    std::lock_guard<std::mutex> lock(mtx_);
    tasks_.push_back(std::move(task));
}

void TaskScheduler::loop() {
    while(running_) {
        std::this_thread::sleep_for(std::chrono::seconds(1));
        std::vector<nlohmann::json> ready;
        auto now = std::chrono::steady_clock::now();
        {
            std::lock_guard<std::mutex> lock(mtx_);
            auto it = tasks_.begin();
            while(it != tasks_.end()) {
                if(it->execute_at <= now) {
                    ready.push_back(it->cmd);
                    it = tasks_.erase(it);
                } else ++it;
            }
        }
        for(const auto& c : ready) {
            CommandRegistry::instance().dispatch(c);
        }
    }
}

extern "C" Module* create_module() { return new TaskScheduler(); }
