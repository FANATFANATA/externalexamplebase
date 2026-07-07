#include "memory.h"
#include <cstdio>
#include <cstring>
#include <cstdlib>

namespace memory
{

    std::vector<MemoryRegion> GetProcessMaps(pid_t pid)
    {
        std::vector<MemoryRegion> regions;
        char mapsPath[64];
        snprintf(mapsPath, sizeof(mapsPath), "/proc/%d/maps", pid);
        FILE *fp = fopen(mapsPath, "r");
        if (!fp)
            return regions;

        char line[512];
        while (fgets(line, sizeof(line), fp))
        {
            MemoryRegion region{};
            char perms[5];
            unsigned long long start, end;
            unsigned int dev_major, dev_minor;
            unsigned long inode;
            char pathname[256];
            pathname[0] = '\0';
            int n = sscanf(line, "%llx-%llx %4s %*x %x:%x %lu %255s",
                           &start, &end, perms, &dev_major, &dev_minor, &inode, pathname);
            if (n >= 3)
            {
                region.start = static_cast<uintptr_t>(start);
                region.end = static_cast<uintptr_t>(end);
                region.readable = (perms[0] == 'r');
                region.writable = (perms[1] == 'w');
                region.executable = (perms[2] == 'x');
                if (n >= 7)
                {
                    region.pathname = pathname;
                }
                regions.push_back(region);
            }
        }
        fclose(fp);
        return regions;
    }

    uintptr_t GetModuleBase(pid_t pid, const char *moduleName)
    {
        auto regions = GetProcessMaps(pid);
        for (const auto &r : regions)
        {
            if (!r.pathname.empty() && strstr(r.pathname.c_str(), moduleName) != nullptr)
            {
                if (r.readable && r.executable)
                {
                    return r.start;
                }
            }
        }
        return 0;
    }

}
