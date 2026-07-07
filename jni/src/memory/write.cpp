#include "write.h"
#include "memory.h"
#include <cstdio>
#include <fcntl.h>
#include <unistd.h>
#include <cstring>

namespace memory
{

    static bool IsRegionValidForWrite(const std::vector<MemoryRegion> &regions, uintptr_t addr, size_t size)
    {
        for (const auto &r : regions)
        {
            if (addr >= r.start && addr + size <= r.end)
            {
                return r.writable;
            }
        }
        return false;
    }

    bool WriteProcessMemory(pid_t pid, uintptr_t address, const void *buffer, size_t size)
    {
        if (size == 0)
            return true;
        auto maps = GetProcessMaps(pid);
        if (!IsRegionValidForWrite(maps, address, size))
            return false;

        char memPath[64];
        snprintf(memPath, sizeof(memPath), "/proc/%d/mem", pid);
        int fd = open(memPath, O_RDWR);
        if (fd == -1)
            return false;

        lseek(fd, static_cast<off64_t>(address), SEEK_SET);
        const uint8_t *src = static_cast<const uint8_t *>(buffer);
        size_t remaining = size;
        while (remaining > 0)
        {
            ssize_t n = write(fd, src, remaining);
            if (n <= 0)
            {
                close(fd);
                return false;
            }
            src += n;
            remaining -= n;
        }
        close(fd);
        return true;
    }

    bool WriteProcessMemoryVec(pid_t pid, const std::vector<MemorySegment> &segments)
    {
        auto maps = GetProcessMaps(pid);
        for (const auto &seg : segments)
        {
            if (!IsRegionValidForWrite(maps, seg.address, seg.size))
                return false;
        }
        for (const auto &seg : segments)
        {
            if (!WriteProcessMemory(pid, seg.address, seg.buffer, seg.size))
                return false;
        }
        return true;
    }

}
