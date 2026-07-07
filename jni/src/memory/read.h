#ifndef MEMORY_READ_H
#define MEMORY_READ_H

#include <cstdint>
#include <sys/types.h>
#include <vector>

namespace memory
{

    struct MemorySegment
    {
        uintptr_t address;
        void *buffer;
        size_t size;
    };

    bool ReadProcessMemory(pid_t pid, uintptr_t address, void *buffer, size_t size);
    bool ReadProcessMemoryVec(pid_t pid, const std::vector<MemorySegment> &segments);

}

#endif
