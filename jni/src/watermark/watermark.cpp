#include "watermark.h"
#include <imgui.h>
#include <cstdint>
#include <string>

namespace watermark
{

    static WatermarkConfig config;

    WatermarkConfig &GetWatermarkConfig()
    {
        return config;
    }

    void DrawWatermark(bool &menu_visible)
    {
        ImGuiIO &io = ImGui::GetIO();
        std::string text = "Sdeeb";
        if (config.showVersion)
            text += " | 0.39.1";
        if (config.showType)
            text += " | External";
        if (config.showFps)
            text += " | " + std::to_string(static_cast<int>(io.Framerate)) + " FPS";

        ImVec2 pos(20.0f, 20.0f);
        ImVec2 textSize = ImGui::CalcTextSize(text.c_str());
        ImDrawList *dl = ImGui::GetForegroundDrawList();
        ImVec2 bg_min(pos.x - 8.0f, pos.y - 6.0f);
        ImVec2 bg_max(pos.x + textSize.x + 8.0f, pos.y + textSize.y + 6.0f);
        dl->AddRectFilled(bg_min, bg_max, IM_COL32(0x06, 0x14, 0x1B, 0xD0), 8.0f);
        dl->AddRect(bg_min, bg_max, IM_COL32(0x25, 0x37, 0x45, 0xFF), 8.0f, ImDrawFlags_RoundCornersAll, 1.5f);
        dl->AddText(pos, IM_COL32(0xCC, 0xD0, 0xCF, 0xFF), text.c_str());

        if (ImGui::IsMouseClicked(0))
        {
            ImVec2 mouse = ImGui::GetMousePos();
            if (mouse.x >= bg_min.x && mouse.y >= bg_min.y &&
                mouse.x <= bg_max.x && mouse.y <= bg_max.y)
            {
                menu_visible = !menu_visible;
            }
        }
    }

}
