#include "menu.h"
#include "watermark/watermark.h"
#include <imgui.h>
#include <imgui_internal.h>
#include <cmath>
#include <vector>

namespace menu
{

    struct RippleState
    {
        ImVec2 center;
        float start_time;
        float strength;
    };

    struct TabAnimState
    {
        float hover_alpha = 0.0f;
        float scale = 1.0f;
        std::vector<RippleState> ripples;
    };

    struct ToggleAnimState
    {
        float position = 0.0f;
        std::vector<RippleState> ripples;
    };

    static int current_tab = 0;
    static int prev_tab = 0;
    static float tab_switch_time = 0.0f;
    static bool menu_animating = false;
    static bool menu_target_visible = false;
    static float menu_anim_start = 0.0f;
    static float menu_alpha = 1.0f;
    static const float ANIM_DURATION = 0.25f;
    static const float TAB_FADE_DURATION = 0.15f;
    static const float RIPPLE_DURATION = 0.5f;
    static const float HOVER_DURATION = 0.15f;
    static const float SCALE_HOVER = 1.05f;

    static const char *tabs[] = {"ESP", "Aim", "Skins", "Misc", "Config", "About us"};
    static const int tab_count = sizeof(tabs) / sizeof(tabs[0]);

    static std::vector<TabAnimState> tab_states(tab_count);
    static ToggleAnimState toggles[3];

    static ImU32 HexToCol32(unsigned int hex, float alpha)
    {
        return IM_COL32((hex >> 16) & 0xFF, (hex >> 8) & 0xFF, hex & 0xFF, (int)(alpha * 255));
    }

    static void UpdateRipples(std::vector<RippleState> &ripples)
    {
        float now = ImGui::GetTime();
        ripples.erase(std::remove_if(ripples.begin(), ripples.end(), [now](const RippleState &r)
                                     { return (now - r.start_time) > RIPPLE_DURATION; }),
                      ripples.end());
    }

    static void DrawRipples(const std::vector<RippleState> &ripples, ImVec2 center, float radius, ImU32 color)
    {
        float now = ImGui::GetTime();
        ImDrawList *dl = ImGui::GetWindowDrawList();
        for (const auto &r : ripples)
        {
            float elapsed = now - r.start_time;
            if (elapsed >= RIPPLE_DURATION)
                continue;
            float progress = elapsed / RIPPLE_DURATION;
            float alpha = 1.0f - progress;
            float size = radius * 1.5f * progress;
            ImU32 col = (color & 0x00FFFFFF) | ((int)(alpha * r.strength * 255) << 24);
            dl->AddCircleFilled(center, size, col);
        }
    }

    static void ProcessRipple(std::vector<RippleState> &ripples, ImVec2 center, bool hovered, bool clicked)
    {
        float now = ImGui::GetTime();
        if (clicked)
        {
            ripples.push_back({center, now, 0.6f});
        }
        else if (hovered && ripples.empty())
        {
            ripples.push_back({center, now, 0.15f});
        }
        UpdateRipples(ripples);
    }

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
        s.ItemSpacing = ImVec2(8, 8);

        ImVec4 *colors = s.Colors;
        colors[ImGuiCol_WindowBg] = ImVec4(0.0235f, 0.0784f, 0.1059f, 0.95f);
        colors[ImGuiCol_ChildBg] = ImVec4(0.0667f, 0.1294f, 0.1765f, 0.0f);
        colors[ImGuiCol_FrameBg] = ImVec4(0.1451f, 0.2157f, 0.2706f, 0.54f);
        colors[ImGuiCol_FrameBgHovered] = ImVec4(0.2902f, 0.3608f, 0.4157f, 0.40f);
        colors[ImGuiCol_FrameBgActive] = ImVec4(0.6078f, 0.6588f, 0.6706f, 0.67f);
        colors[ImGuiCol_Text] = ImVec4(0.8000f, 0.8157f, 0.8235f, 1.00f);
        colors[ImGuiCol_TextDisabled] = ImVec4(0.6078f, 0.6588f, 0.6706f, 1.00f);
        colors[ImGuiCol_Border] = ImVec4(0.1451f, 0.2157f, 0.2706f, 1.00f);
        colors[ImGuiCol_ScrollbarBg] = ImVec4(0.0235f, 0.0784f, 0.1059f, 0.53f);
        colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.1451f, 0.2157f, 0.2706f, 1.00f);
        colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.2902f, 0.3608f, 0.4157f, 1.00f);
        colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.6078f, 0.6588f, 0.6706f, 1.00f);
        colors[ImGuiCol_CheckMark] = ImVec4(0.8000f, 0.8157f, 0.8235f, 1.00f);
    }

    static void RenderTabButton(int index, bool selected)
    {
        ImGuiWindow *window = ImGui::GetCurrentWindow();
        ImGuiIO &io = ImGui::GetIO();
        ImDrawList *dl = ImGui::GetWindowDrawList();

        const char *label = tabs[index];
        ImVec2 pos = ImGui::GetCursorScreenPos();
        ImVec2 text_size = ImGui::CalcTextSize(label);
        float width = ImGui::CalcItemWidth();
        float height = text_size.y + 12.0f;
        ImVec2 size(width, height);
        ImVec2 center(pos.x + size.x * 0.5f, pos.y + size.y * 0.5f);

        ImGui::ItemSize(size);
        if (!ImGui::ItemAdd(ImRect(pos, ImVec2(pos.x + size.x, pos.y + size.y)), index))
            return;

        bool hovered = ImGui::IsItemHovered();
        bool clicked = ImGui::IsItemClicked();
        if (clicked)
            current_tab = index;

        TabAnimState &st = tab_states[index];
        float target_hover = (hovered || selected) ? 1.0f : 0.0f;
        float delta = io.DeltaTime / HOVER_DURATION;
        st.hover_alpha += (target_hover - st.hover_alpha) * (delta > 1.0f ? 1.0f : delta);

        float target_scale = hovered ? SCALE_HOVER : 1.0f;
        st.scale += (target_scale - st.scale) * (delta > 1.0f ? 1.0f : delta);

        ProcessRipple(st.ripples, center, hovered, clicked);

        ImU32 bg_col = HexToCol32(0x11212D, 1.0f);
        ImU32 hover_col = HexToCol32(0x253745, 1.0f);
        ImU32 sel_col = HexToCol32(0x4A5C6A, 1.0f);
        ImU32 text_col = HexToCol32(0xCCD0CF, 1.0f);
        ImU32 border_col = HexToCol32(0x253745, 0.8f);

        if (selected)
        {
            bg_col = sel_col;
        }
        else if (st.hover_alpha > 0.001f)
        {
            float a = st.hover_alpha;
            bg_col = IM_COL32(
                (int)(((bg_col >> 16) & 0xFF) + (((hover_col >> 16) & 0xFF) - ((bg_col >> 16) & 0xFF)) * a),
                (int)(((bg_col >> 8) & 0xFF) + (((hover_col >> 8) & 0xFF) - ((bg_col >> 8) & 0xFF)) * a),
                (int)((bg_col & 0xFF) + ((hover_col & 0xFF) - (bg_col & 0xFF)) * a),
                255);
        }

        ImVec2 pill_rect_min(pos.x + 4, pos.y + 2);
        ImVec2 pill_rect_max(pos.x + size.x - 4, pos.y + size.y - 2);
        dl->AddRectFilled(pill_rect_min, pill_rect_max, bg_col, size.y * 0.5f);
        dl->AddRect(pill_rect_min, pill_rect_max, border_col, size.y * 0.5f, 0, 1.5f);

        DrawRipples(st.ripples, center, size.y * 0.4f, hover_col);

        ImVec2 text_pos(center.x - text_size.x * 0.5f * st.scale, center.y - text_size.y * 0.5f * st.scale);
        dl->AddText(text_pos, text_col, label);
        if (st.scale != 1.0f)
        {
            dl->AddText(NULL, 0.0f, text_pos, text_col, label);
        }
    }

    static void RenderToggle(const char *label, bool *v)
    {
        ImGuiWindow *window = ImGui::GetCurrentWindow();
        ImDrawList *dl = ImGui::GetWindowDrawList();
        ImGuiIO &io = ImGui::GetIO();

        float label_width = ImGui::CalcTextSize(label).x;
        float toggle_width = 40.0f;
        float toggle_height = 22.0f;
        float total_width = label_width + 12.0f + toggle_width;
        float height = toggle_height + 4.0f;

        ImVec2 pos = ImGui::GetCursorScreenPos();
        ImGui::ItemSize(ImVec2(total_width, height));
        if (!ImGui::ItemAdd(ImRect(pos, ImVec2(pos.x + total_width, pos.y + height)), 0))
            return;

        ImVec2 toggle_pos(pos.x + label_width + 12.0f, pos.y + 3.0f);
        ImVec2 toggle_center(toggle_pos.x + toggle_width * 0.5f, toggle_pos.y + toggle_height * 0.5f);

        bool hovered = ImGui::IsMouseHoveringRect(toggle_pos, ImVec2(toggle_pos.x + toggle_width, toggle_pos.y + toggle_height));
        bool clicked = ImGui::IsItemClicked();

        static std::vector<RippleState> *ripples = &toggles[0].ripples;
        static float *pos_ptr = &toggles[0].position;
        int idx = (label[0] == 'S' && label[6] == 'v') ? 0 : (label[0] == 'S' && label[6] == 't') ? 1
                                                                                                  : 2;
        ripples = &toggles[idx].ripples;
        pos_ptr = &toggles[idx].position;

        ProcessRipple(*ripples, toggle_center, hovered, clicked);
        if (clicked)
            *v = !(*v);

        float target_pos = *v ? 1.0f : 0.0f;
        float delta = io.DeltaTime / 0.12f;
        *pos_ptr += (target_pos - *pos_ptr) * (delta > 1.0f ? 1.0f : delta);

        ImU32 bg_off = HexToCol32(0x253745, 0.8f);
        ImU32 bg_on = HexToCol32(0x4A5C6A, 1.0f);
        ImU32 knob_col = HexToCol32(0xCCD0CF, 1.0f);
        ImU32 border_col = HexToCol32(0x253745, 1.0f);

        ImU32 bg = bg_off;
        if (*pos_ptr > 0.001f)
        {
            float a = *pos_ptr;
            bg = IM_COL32(
                (int)(((bg_off >> 16) & 0xFF) + (((bg_on >> 16) & 0xFF) - ((bg_off >> 16) & 0xFF)) * a),
                (int)(((bg_off >> 8) & 0xFF) + (((bg_on >> 8) & 0xFF) - ((bg_off >> 8) & 0xFF)) * a),
                (int)((bg_off & 0xFF) + ((bg_on & 0xFF) - (bg_off & 0xFF)) * a),
                255);
        }

        dl->AddRectFilled(toggle_pos, ImVec2(toggle_pos.x + toggle_width, toggle_pos.y + toggle_height), bg, toggle_height * 0.5f);
        dl->AddRect(toggle_pos, ImVec2(toggle_pos.x + toggle_width, toggle_pos.y + toggle_height), border_col, toggle_height * 0.5f, 0, 1.5f);

        float knob_radius = toggle_height * 0.35f;
        float knob_x = toggle_pos.x + knob_radius + 4.0f + (*pos_ptr) * (toggle_width - knob_radius * 2.0f - 8.0f);
        ImVec2 knob_center(knob_x, toggle_center.y);
        dl->AddCircleFilled(knob_center, knob_radius, knob_col);

        DrawRipples(*ripples, toggle_center, toggle_height * 0.6f, HexToCol32(0x9BA8AB, 0.8f));

        dl->AddText(ImVec2(pos.x, pos.y + (height - ImGui::CalcTextSize(label).y) * 0.5f), IM_COL32(0xCC, 0xD0, 0xCF, 0xFF), label);
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

        ImGui::SetNextWindowSize(ImVec2(600, 450), ImGuiCond_FirstUseEver);
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
        ImGui::SetColumnWidth(0, 170);

        for (int i = 0; i < tab_count; i++)
        {
            RenderTabButton(i, current_tab == i);
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
            RenderToggle("Show version", &cfg.showVersion);
            RenderToggle("Show type", &cfg.showType);
            RenderToggle("Show FPS", &cfg.showFps);
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
        ImGui::PopStyleVar();
    }

}
