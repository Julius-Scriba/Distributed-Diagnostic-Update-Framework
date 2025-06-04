#include "MemoryLoader.h"
#include <vector>
#ifdef __linux__
#include <sys/mman.h>
#include <unistd.h>
#include <fcntl.h>
#include <dlfcn.h>
#include <stdio.h>
#endif

void* MemoryLoader::load(const std::vector<unsigned char>& data) {
#ifdef __linux__
    int fd = memfd_create("mod", MFD_CLOEXEC);
    if(fd < 0) return nullptr;
    ssize_t written = write(fd, data.data(), data.size());
    if(written != (ssize_t)data.size()) {
        close(fd);
        return nullptr;
    }
    lseek(fd, 0, SEEK_SET);
    char path[64];
    snprintf(path, sizeof(path), "/proc/self/fd/%d", fd);
    void* handle = dlopen(path, RTLD_NOW);
    close(fd);
    return handle;
#else
    (void)data;
    return nullptr;
#endif
}
