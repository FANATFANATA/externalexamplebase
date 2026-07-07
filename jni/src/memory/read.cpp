#include "read.h"
#include "memory.h"
#include <cstdio>
#include <fcntl.h>
#include <unistd.h>
#include <cstring>

namespace memory
{

    static bool IsRegionValidForRead(const std::vector<MemoryRegion> &regions, uintptr_t addr, size_t size)
    {
        for (const auto &r : regions)
        {
            if (addr >= r.start && addr + size <= r.end)
            {
                return r.readable;
            }
        }
        return false;
    }

    bool ReadProcessMemory(pid_t pid, uintptr_t address, void *buffer, size_t size)
    {
        if (size == 0)
            return true;
        auto maps = GetProcessMaps(pid);
        if (!IsRegionValidForRead(maps, address, size))
            return false;

        char memPath[64];
        snprintf(memPath, sizeof(memPath), "/proc/%d/mem", pid);
        int fd = open(memPath, O_RDONLY);
        if (fd == -1)
            return false;

        lseek(fd, static_cast<off64_t>(address), SEEK_SET);
        uint8_t *dst = static_cast<uint8_t *>(buffer);
        size_t remaining = size;
        while (remaining > 0)
        {
            ssize_t n = read(fd, dst, remaining);
            if (n <= 0)
            {
                close(fd);
                return false;
            }
            dst += n;
            remaining -= n;
        }
        close(fd);
        return true;
    }

    bool ReadProcessMemoryVec(pid_t pid, const std::vector<MemorySegment> &segments)
    {
        auto maps = GetProcessMaps(pid);
        for (const auto &seg : segments)
        {
            if (!IsRegionValidForRead(maps, seg.address, seg.size))
                return false;
        }
        for (const auto &seg : segments)
        {
            if (!ReadProcessMemory(pid, seg.address, seg.buffer, seg.size))
                return false;
        }
        return true;
    }

}
