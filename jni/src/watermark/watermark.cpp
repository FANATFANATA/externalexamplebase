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
            text += " \xB7 0.39.1";
        if (config.showType)
            text += " \xB7 External";
        if (config.showFps)
            text += " \xB7 " + std::to_string(static_cast<int>(io.Framerate)) + " FPS";

        ImVec2 pos(10.0f, 10.0f);
        ImDrawList *dl = ImGui::GetForegroundDrawList();
        dl->AddText(pos, IM_COL32(0xCC, 0xD0, 0xCF, 0xFF), text.c_str());

        ImVec2 size = ImGui::CalcTextSize(text.c_str());
        if (ImGui::IsMouseClicked(0))
        {
            ImVec2 mouse = ImGui::GetMousePos();
            if (mouse.x >= pos.x && mouse.y >= pos.y &&
                mouse.x <= pos.x + size.x && mouse.y <= pos.y + size.y)
            {
                menu_visible = !menu_visible;
            }
        }
    }

}
