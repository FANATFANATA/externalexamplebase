#ifndef MEMORY_WRITE_H
#define MEMORY_WRITE_H

#include <cstdint>
#include <sys/types.h>
#include <vector>

namespace memory
{

    struct MemorySegment
    {
        uintptr_t address;
        const void *buffer;
        size_t size;
    };

    bool WriteProcessMemory(pid_t pid, uintptr_t address, const void *buffer, size_t size);
    bool WriteProcessMemoryVec(pid_t pid, const std::vector<MemorySegment> &segments);

}

#endif
