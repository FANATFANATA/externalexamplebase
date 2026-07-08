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

    static bool CustomCheckbox(const char *label, bool *v)
    {
        ImGuiWindow *window = ImGui::GetCurrentWindow();
        if (window->SkipItems)
            return false;
        ImGuiContext &g = *GImGui;
        const ImGuiStyle &style = g.Style;
        const ImGuiID id = window->GetID(label);
        const ImVec2 label_size = ImGui::CalcTextSize(label, NULL, true);
        const float square_sz = ImGui::GetFrameHeight();
        const ImVec2 pos = window->DC.CursorPos;
        const ImRect total_bb(pos, ImVec2(pos.x + square_sz + style.ItemInnerSpacing.x + label_size.x, pos.y + square_sz));
        ImGui::ItemSize(total_bb, style.FramePadding.y);
        if (!ImGui::ItemAdd(total_bb, id))
            return false;

        bool hovered, held;
        bool pressed = ImGui::ButtonBehavior(total_bb, id, &hovered, &held);
        if (pressed)
            *v = !*v;

        ImU32 col_bg;
        if (*v)
            col_bg = ImGui::ColorConvertFloat4ToU32(ImVec4(0.6078f, 0.6588f, 0.6706f, 1.0f));
        else
            col_bg = ImGui::ColorConvertFloat4ToU32(ImVec4(0.1451f, 0.2157f, 0.2706f, 1.0f));
        if (hovered)
            col_bg = ImGui::ColorConvertFloat4ToU32(ImVec4(0.2902f, 0.3608f, 0.4157f, 1.0f));

        const ImRect check_bb(pos, ImVec2(pos.x + square_sz, pos.y + square_sz));
        window->DrawList->AddRectFilled(check_bb.Min, check_bb.Max, col_bg, style.FrameRounding);

        if (*v)
        {
            const float pad = 2.0f;
            const float thickness = 2.0f;
            ImVec2 check_mark_bb_min(check_bb.Min.x + pad, check_bb.Min.y + pad);
            ImVec2 check_mark_bb_max(check_bb.Max.x - pad, check_bb.Max.y - pad);
            ImVec2 a(check_mark_bb_min.x, check_mark_bb_min.y + (check_mark_bb_max.y - check_mark_bb_min.y) * 0.5f);
            ImVec2 b(check_mark_bb_min.x + (check_mark_bb_max.x - check_mark_bb_min.x) * 0.4f, check_mark_bb_max.y);
            ImVec2 c(check_mark_bb_max.x, check_mark_bb_min.y);
            window->DrawList->AddLine(a, b, IM_COL32(0xCC, 0xD0, 0xCF, 0xFF), thickness);
            window->DrawList->AddLine(b, c, IM_COL32(0xCC, 0xD0, 0xCF, 0xFF), thickness);
        }

        ImGui::SameLine();
        ImGui::RenderText(ImVec2(check_bb.Max.x + style.ItemInnerSpacing.x, check_bb.Min.y), label);
        return pressed;
    }

    void ApplyStyle()
    {
        ImGuiStyle &s = ImGui::GetStyle();
        s.WindowRounding = 16.0f;
        s.ChildRounding = 12.0f;
        s.FrameRounding = 10.0f;
        s.GrabRounding = 10.0f;
        s.PopupRounding = 10.0f;
        s.ScrollbarRounding = 14.0f;
        s.TabRounding = 12.0f;
        s.WindowBorderSize = 0.0f;
        s.FrameBorderSize = 0.0f;
        s.WindowPadding = ImVec2(12, 12);
        s.FramePadding = ImVec2(10, 8);
        s.ItemSpacing = ImVec2(10, 8);
        s.ItemInnerSpacing = ImVec2(8, 8);

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

        ImGui::SetNextWindowSize(ImVec2(800, 600), ImGuiCond_FirstUseEver);
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
                           border_col, 16.0f, ImDrawFlags_RoundCornersAll, 2.0f);

        ImGui::Columns(2, "menu_columns", false);
        ImGui::SetColumnWidth(0, 220);

        ImVec4 tab_bg = ImVec4(0.1451f, 0.2157f, 0.2706f, 0.5f);
        ImVec4 tab_hovered = ImVec4(0.2902f, 0.3608f, 0.4157f, 0.7f);
        ImVec4 tab_active = ImVec4(0.6078f, 0.6588f, 0.6706f, 0.8f);
        ImVec2 tab_padding(14, 8);
        float tab_rounding = 10.0f;
        float tab_height = ImGui::GetTextLineHeight() + tab_padding.y * 2.0f;

        for (int i = 0; i < tab_count; i++)
        {
            ImGui::PushID(i);
            ImVec2 cursor_pos = ImGui::GetCursorScreenPos();
            ImVec2 avail = ImGui::GetContentRegionAvail();
            ImVec2 tab_size(avail.x - ImGui::GetStyle().ScrollbarSize - 6.0f, tab_height);

            bool is_selected = (current_tab == i);
            bool is_hovered = ImGui::IsMouseHoveringRect(cursor_pos, ImVec2(cursor_pos.x + tab_size.x, cursor_pos.y + tab_size.y));

            ImU32 bg_color;
            if (is_selected)
                bg_color = ImGui::ColorConvertFloat4ToU32(tab_active);
            else if (is_hovered)
                bg_color = ImGui::ColorConvertFloat4ToU32(tab_hovered);
            else
                bg_color = ImGui::ColorConvertFloat4ToU32(tab_bg);

            draw_list->AddRectFilled(cursor_pos, ImVec2(cursor_pos.x + tab_size.x, cursor_pos.y + tab_size.y), bg_color, tab_rounding);

            ImVec2 text_pos(cursor_pos.x + tab_padding.x, cursor_pos.y + tab_padding.y);
            draw_list->AddText(text_pos, ImGui::ColorConvertFloat4ToU32(ImGui::GetStyle().Colors[ImGuiCol_Text]), tabs[i]);

            if (ImGui::IsMouseClicked(0) && is_hovered)
            {
                if (current_tab != i)
                {
                    prev_tab = current_tab;
                    current_tab = i;
                    tab_switch_time = ImGui::GetTime();
                }
            }

            ImGui::Dummy(tab_size);
            ImGui::PopID();
        }

        ImGui::NextColumn();

        ImGui::BeginChild("tab_content", ImVec2(0, 0), false, ImGuiWindowFlags_NoScrollbar);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(16, 16));

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

        ImVec2 content_avail = ImGui::GetContentRegionAvail();
        float center_x = ImGui::GetCursorPosX() + content_avail.x * 0.5f;

        switch (current_tab)
        {
        case 0:
        case 1:
        case 2:
        case 4:
        case 5:
        {
            const char *msg = "Coming soon";
            float text_width = ImGui::CalcTextSize(msg).x;
            ImGui::SetCursorPosX(center_x - text_width * 0.5f);
            ImGui::Text("%s", msg);
            break;
        }
        case 3:
        {
            auto &cfg = watermark::GetWatermarkConfig();
            ImGui::Text("Watermark");
            ImGui::Separator();
            CustomCheckbox("Show version", &cfg.showVersion);
            CustomCheckbox("Show type", &cfg.showType);
            CustomCheckbox("Show FPS", &cfg.showFps);
            break;
        }
        }

        ImGui::PopStyleVar();
        ImGui::PopStyleVar();
        ImGui::EndChild();
        ImGui::Columns(1);

        ImGui::End();

        ImGui::PopStyleVar();
    }

}
