#include "IPC.h"
#ifdef _WIN32
#include <iostream>

IPCServer::IPCServer() {}

void IPCServer::run() {
    running_ = true;
    while(running_) {
        pipe_ = CreateNamedPipeA("\\\\.\\pipe\\US_IPC_CORE",
            PIPE_ACCESS_DUPLEX,
            PIPE_TYPE_MESSAGE | PIPE_READMODE_MESSAGE | PIPE_WAIT,
            PIPE_UNLIMITED_INSTANCES, 4096, 4096, 0, NULL);
        if (pipe_ == INVALID_HANDLE_VALUE) {
            std::cerr << "Failed to create pipe" << std::endl;
            return;
        }
        BOOL connected = ConnectNamedPipe(pipe_, NULL) ? TRUE : (GetLastError() == ERROR_PIPE_CONNECTED);
        if(!connected) {
            CloseHandle(pipe_);
            continue;
        }
        char buffer[4096];
        DWORD read = 0;
        while (ReadFile(pipe_, buffer, sizeof(buffer)-1, &read, NULL) && read > 0) {
            buffer[read] = '\0';
            try {
                auto msg = nlohmann::json::parse(buffer);
                std::cout << "IPC received: " << msg.dump() << std::endl;
            } catch(const std::exception& e) {
                std::cerr << "Invalid IPC message: " << e.what() << std::endl;
            }
        }
        DisconnectNamedPipe(pipe_);
        CloseHandle(pipe_);
    }
}

void IPCServer::init() {
    thread_ = std::thread(&IPCServer::run, this);
    thread_.detach();
}

void IPCServer::stop() {
    running_ = false;
}

#else
IPCServer::IPCServer() {}
void IPCServer::init() {}
void IPCServer::stop() {}
#endif

bool IPCClient::send(const nlohmann::json& msg) {
#ifdef _WIN32
    HANDLE h = CreateFileA("\\\\.\\pipe\\US_IPC_CORE", GENERIC_WRITE, 0, NULL, OPEN_EXISTING, 0, NULL);
    if (h == INVALID_HANDLE_VALUE) return false;
    std::string data = msg.dump();
    DWORD written = 0;
    BOOL ok = WriteFile(h, data.c_str(), (DWORD)data.size(), &written, NULL);
    CloseHandle(h);
    return ok && written == data.size();
#else
    (void)msg; return false;
#endif
}

extern "C" Module* create_module() {
    return new IPCServer();
}
