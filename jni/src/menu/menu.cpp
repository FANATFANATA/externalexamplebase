#include "menu.h"
#include "watermark/watermark.h"
#include <imgui.h>
#include <imgui_internal.h>

namespace menu
{

    static int current_tab = 0;
    static int prev_tab = 0;
    static float tab_switch_time = 0.0f;
    static bool menu_animating = false;
    static bool menu_target_visible = false;
    static float menu_anim_start = 0.0f;
    static float menu_alpha = 1.0f;
    static const float ANIM_DURATION = 0.25f;
    static const float TAB_FADE_DURATION = 0.15f;

    static const char *tabs[] = {"ESP", "Aim", "Skins", "Misc", "Config", "About us"};
    static const int tab_count = sizeof(tabs) / sizeof(tabs[0]);

    void ApplyStyle()
    {
        ImGuiStyle &s = ImGui::GetStyle();
        s.WindowRounding = 12.0f;
        s.ChildRounding = 8.0f;
        s.FrameRounding = 6.0f;
        s.GrabRounding = 6.0f;
        s.PopupRounding = 6.0f;
        s.ScrollbarRounding = 10.0f;
        s.TabRounding = 8.0f;
        s.WindowBorderSize = 0.0f;
        s.FrameBorderSize = 0.0f;
        s.WindowPadding = ImVec2(8, 8);
        s.FramePadding = ImVec2(6, 4);

        ImVec4 *colors = s.Colors;
        colors[ImGuiCol_WindowBg] = ImVec4(0.0235f, 0.0784f, 0.1059f, 0.95f);
        colors[ImGuiCol_ChildBg] = ImVec4(0.0667f, 0.1294f, 0.1765f, 1.00f);
        colors[ImGuiCol_FrameBg] = ImVec4(0.1451f, 0.2157f, 0.2706f, 0.54f);
        colors[ImGuiCol_FrameBgHovered] = ImVec4(0.2902f, 0.3608f, 0.4157f, 0.40f);
        colors[ImGuiCol_FrameBgActive] = ImVec4(0.6078f, 0.6588f, 0.6706f, 0.67f);
        colors[ImGuiCol_TitleBg] = ImVec4(0.0235f, 0.0784f, 0.1059f, 1.00f);
        colors[ImGuiCol_TitleBgActive] = ImVec4(0.0667f, 0.1294f, 0.1765f, 1.00f);
        colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.0235f, 0.0784f, 0.1059f, 0.51f);
        colors[ImGuiCol_ScrollbarBg] = ImVec4(0.0235f, 0.0784f, 0.1059f, 0.53f);
        colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.1451f, 0.2157f, 0.2706f, 1.00f);
        colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.2902f, 0.3608f, 0.4157f, 1.00f);
        colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.6078f, 0.6588f, 0.6706f, 1.00f);
        colors[ImGuiCol_CheckMark] = ImVec4(0.8000f, 0.8157f, 0.8235f, 1.00f);
        colors[ImGuiCol_SliderGrab] = ImVec4(0.8000f, 0.8157f, 0.8235f, 1.00f);
        colors[ImGuiCol_SliderGrabActive] = ImVec4(0.6078f, 0.6588f, 0.6706f, 1.00f);
        colors[ImGuiCol_Button] = ImVec4(0.1451f, 0.2157f, 0.2706f, 1.00f);
        colors[ImGuiCol_ButtonHovered] = ImVec4(0.2902f, 0.3608f, 0.4157f, 1.00f);
        colors[ImGuiCol_ButtonActive] = ImVec4(0.6078f, 0.6588f, 0.6706f, 1.00f);
        colors[ImGuiCol_Header] = ImVec4(0.1451f, 0.2157f, 0.2706f, 1.00f);
        colors[ImGuiCol_HeaderHovered] = ImVec4(0.2902f, 0.3608f, 0.4157f, 1.00f);
        colors[ImGuiCol_HeaderActive] = ImVec4(0.6078f, 0.6588f, 0.6706f, 1.00f);
        colors[ImGuiCol_Separator] = ImVec4(0.1451f, 0.2157f, 0.2706f, 1.00f);
        colors[ImGuiCol_Text] = ImVec4(0.8000f, 0.8157f, 0.8235f, 1.00f);
        colors[ImGuiCol_TextDisabled] = ImVec4(0.6078f, 0.6588f, 0.6706f, 1.00f);
        colors[ImGuiCol_Border] = ImVec4(0.1451f, 0.2157f, 0.2706f, 1.00f);
    }

    void ShowMenu(bool *p_open)
    {
        if (*p_open != menu_target_visible)
        {
            menu_target_visible = *p_open;
            menu_animating = true;
            menu_anim_start = ImGui::GetTime();
        }

        if (menu_animating)
        {
            float elapsed = ImGui::GetTime() - menu_anim_start;
            if (elapsed >= ANIM_DURATION)
            {
                menu_animating = false;
                *p_open = menu_target_visible;
                menu_alpha = menu_target_visible ? 1.0f : 0.0f;
            }
            else
            {
                float t = elapsed / ANIM_DURATION;
                if (!menu_target_visible)
                    t = 1.0f - t;
                menu_alpha = t;
            }
        }
        else
        {
            menu_alpha = *p_open ? 1.0f : 0.0f;
        }

        if (!*p_open && !menu_animating)
            return;

        ImGui::PushStyleVar(ImGuiStyleVar_Alpha, menu_alpha);

        ImGuiStyle old_style = ImGui::GetStyle();
        ImGui::GetStyle().ScaleAllSizes(1.5f);

        ImGui::SetNextWindowSize(ImVec2(580, 420), ImGuiCond_FirstUseEver);
        ImGui::SetNextWindowPos(ImVec2(ImGui::GetIO().DisplaySize.x * 0.5f, ImGui::GetIO().DisplaySize.y * 0.5f),
                                ImGuiCond_FirstUseEver, ImVec2(0.5f, 0.5f));

        ImGui::Begin("Sdeeb", p_open, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar);

        if (menu_animating && !menu_target_visible)
        {
            ImGui::GetCurrentWindow()->Flags |= ImGuiWindowFlags_NoInputs;
        }

        ImDrawList *draw_list = ImGui::GetWindowDrawList();
        ImVec2 win_pos = ImGui::GetWindowPos();
        ImVec2 win_size = ImGui::GetWindowSize();
        ImU32 col_top = IM_COL32(0x06, 0x14, 0x1B, 0xF2);
        ImU32 col_bottom = IM_COL32(0x11, 0x21, 0x2D, 0xF2);
        draw_list->AddRectFilledMultiColor(win_pos, ImVec2(win_pos.x + win_size.x, win_pos.y + win_size.y),
                                           col_top, col_top, col_bottom, col_bottom);

        ImU32 border_col = IM_COL32(0x25, 0x37, 0x45, 0xFF);
        draw_list->AddRect(win_pos, ImVec2(win_pos.x + win_size.x, win_pos.y + win_size.y),
                           border_col, 12.0f, ImDrawFlags_RoundCornersAll, 1.2f);

        ImGui::Columns(2, "menu_columns", false);
        ImGui::SetColumnWidth(0, 160);

        for (int i = 0; i < tab_count; i++)
        {
            bool selected = (current_tab == i);
            if (ImGui::Selectable(tabs[i], selected))
            {
                if (current_tab != i)
                {
                    prev_tab = current_tab;
                    current_tab = i;
                    tab_switch_time = ImGui::GetTime();
                }
            }
            if (selected)
            {
                ImGui::SetItemDefaultFocus();
            }
        }

        ImGui::NextColumn();

        ImGui::BeginChild("tab_content", ImVec2(0, 0), false);

        float tab_alpha = 1.0f;
        if (tab_switch_time > 0.0f)
        {
            float elapsed = ImGui::GetTime() - tab_switch_time;
            if (elapsed < TAB_FADE_DURATION)
            {
                tab_alpha = elapsed / TAB_FADE_DURATION;
            }
            else
            {
                tab_switch_time = 0.0f;
            }
        }

        ImGui::PushStyleVar(ImGuiStyleVar_Alpha, tab_alpha);

        switch (current_tab)
        {
        case 0:
            ImGui::Text("Coming soon");
            break;
        case 1:
            ImGui::Text("Coming soon");
            break;
        case 2:
            ImGui::Text("Coming soon");
            break;
        case 3:
        {
            auto &cfg = watermark::GetWatermarkConfig();
            ImGui::Text("Watermark");
            ImGui::Separator();
            ImGui::Checkbox("Show version", &cfg.showVersion);
            ImGui::Checkbox("Show type", &cfg.showType);
            ImGui::Checkbox("Show FPS", &cfg.showFps);
            break;
        }
        case 4:
            ImGui::Text("Coming soon");
            break;
        case 5:
            ImGui::Text("Coming soon");
            break;
        }

        ImGui::PopStyleVar();
        ImGui::EndChild();
        ImGui::Columns(1);

        ImGui::End();

        ImGui::GetStyle() = old_style;
        ImGui::PopStyleVar();
    }

}
