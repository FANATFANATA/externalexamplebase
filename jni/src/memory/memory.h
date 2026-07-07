#ifndef MEMORY_H
#define MEMORY_H

#include <cstdint>
#include <vector>
#include <string>
#include <sys/types.h>

namespace memory
{

    struct MemoryRegion
    {
        uintptr_t start;
        uintptr_t end;
        bool readable;
        bool writable;
        bool executable;
        std::string pathname;
    };

    std::vector<MemoryRegion> GetProcessMaps(pid_t pid);
    uintptr_t GetModuleBase(pid_t pid, const char *moduleName);

}

#endif
