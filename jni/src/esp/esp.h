#ifndef ESP_H
#define ESP_H

#include <imgui.h>

namespace esp
{

    struct EspConfig
    {
        bool box = true;
        bool name = true;
        bool health = true;
        bool distance = true;
        int box_type = 0;
        float box_rounding = 3.0f;
        ImVec4 box_col = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
        ImVec4 health_col = ImVec4(0.0f, 1.0f, 0.0f, 1.0f);
        ImVec4 name_col = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
        ImVec4 distance_col = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
    };

    inline EspConfig &GetEspConfig()
    {
        static EspConfig cfg;
        return cfg;
    }

    void RenderEsp(pid_t game_pid, uint64_t game_assembly_base, int screen_w, int screen_h);

}

#endif
