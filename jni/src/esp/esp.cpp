#include "esp.h"
#include "sdk/sdk.hpp"
#include <cmath>
#include <algorithm>
#include <cstdio>
#include <imgui.h>

extern ImFont *fontDefault;

namespace esp
{

    static EspConfig config;

    EspConfig &GetEspConfig()
    {
        return config;
    }

    static void draw_text_outlined(ImDrawList *dl, ImFont *font, float size, const ImVec2 &pos, ImU32 color, const char *text)
    {
        if (!font || !dl)
            return;
        int a = (color >> IM_COL32_A_SHIFT) & 0xFF;
        int s1 = static_cast<int>(a * 0.4f);
        int s2 = static_cast<int>(a * 0.7f);
        dl->AddText(font, size, ImVec2(pos.x + 2.f, pos.y + 2.f), IM_COL32(0, 0, 0, s1), text);
        dl->AddText(font, size, ImVec2(pos.x + 1.f, pos.y + 1.f), IM_COL32(0, 0, 0, s2), text);
        dl->AddText(font, size, pos, color, text);
    }

    static void draw_box(const ImVec2 &t, const ImVec2 &b, float a)
    {
        if (a < 0.01f)
            return;
        ImDrawList *dl = ImGui::GetForegroundDrawList();
        float x1 = t.x, y1 = t.y, x2 = b.x, y2 = b.y;
        float r = config.box_rounding;
        ImU32 col = IM_COL32(
            static_cast<int>(config.box_col.x * 255),
            static_cast<int>(config.box_col.y * 255),
            static_cast<int>(config.box_col.z * 255),
            static_cast<int>(config.box_col.w * 255 * a));
        if (config.box_type == 0)
        {
            dl->AddRect(ImVec2(x1 + 2.f, y1 + 2.f), ImVec2(x2 + 2.f, y2 + 2.f), IM_COL32(0, 0, 0, (int)(100 * a)), r, 0, 2.f);
            dl->AddRect(ImVec2(x1 - 1.f, y1 - 1.f), ImVec2(x2 + 1.f, y2 + 1.f), IM_COL32(0, 0, 0, (int)(180 * a)), r, 0, 1.f);
            dl->AddRect(ImVec2(x1, y1), ImVec2(x2, y2), col, r, 0, 1.f);
        }
        else
        {
            float w = x2 - x1, h = y2 - y1;
            float sz = std::min(w, h) * 0.25f;
            float cr = std::min(r, sz * 0.5f);
            ImU32 s1 = IM_COL32(0, 0, 0, (int)(100 * a));
            ImU32 s2 = IM_COL32(0, 0, 0, (int)(180 * a));
            auto corner = [&](float cx, float cy, float dx, float dy)
            {
                if (cr > 0.5f)
                {
                    float arc_cx = cx + dx * cr;
                    float arc_cy = cy + dy * cr;
                    float angle_start, angle_end;
                    if (dx > 0 && dy > 0)
                    {
                        angle_start = 3.14159265f;
                        angle_end = 4.71238898f;
                    }
                    else if (dx < 0 && dy > 0)
                    {
                        angle_start = 4.71238898f;
                        angle_end = 6.28318530f;
                    }
                    else if (dx > 0 && dy < 0)
                    {
                        angle_start = 1.57079632f;
                        angle_end = 3.14159265f;
                    }
                    else
                    {
                        angle_start = 0.f;
                        angle_end = 1.57079632f;
                    }
                    dl->PathArcTo(ImVec2(arc_cx + 2.f * (dx > 0 ? 1 : -1), arc_cy + 2.f * (dy > 0 ? 1 : -1)), cr, angle_start, angle_end, 8);
                    dl->PathStroke(s1, 0, 2.f);
                    dl->AddLine(ImVec2(cx + dx * cr, cy + 2.f * (dy > 0 ? 1 : -1)), ImVec2(cx + dx * sz, cy + 2.f * (dy > 0 ? 1 : -1)), s1, 2.f);
                    dl->AddLine(ImVec2(cx + 2.f * (dx > 0 ? 1 : -1), cy + dy * cr), ImVec2(cx + 2.f * (dx > 0 ? 1 : -1), cy + dy * sz), s1, 2.f);
                    dl->PathArcTo(ImVec2(arc_cx, arc_cy), cr, angle_start, angle_end, 8);
                    dl->PathStroke(s2, 0, 1.f);
                    dl->AddLine(ImVec2(cx + dx * cr, cy), ImVec2(cx + dx * sz, cy), s2, 1.f);
                    dl->AddLine(ImVec2(cx, cy + dy * cr), ImVec2(cx, cy + dy * sz), s2, 1.f);
                    dl->PathArcTo(ImVec2(arc_cx, arc_cy), cr, angle_start, angle_end, 8);
                    dl->PathStroke(col, 0, 1.f);
                    dl->AddLine(ImVec2(cx + dx * cr, cy), ImVec2(cx + dx * sz, cy), col, 1.f);
                    dl->AddLine(ImVec2(cx, cy + dy * cr), ImVec2(cx, cy + dy * sz), col, 1.f);
                }
                else
                {
                    dl->AddLine(ImVec2(cx + 2.f * (dx > 0 ? 1 : -1), cy + 2.f * (dy > 0 ? 1 : -1)), ImVec2(cx + dx * sz + 2.f * (dx > 0 ? 1 : -1), cy + 2.f * (dy > 0 ? 1 : -1)), s1, 2.f);
                    dl->AddLine(ImVec2(cx + 2.f * (dx > 0 ? 1 : -1), cy + 2.f * (dy > 0 ? 1 : -1)), ImVec2(cx + 2.f * (dx > 0 ? 1 : -1), cy + dy * sz + 2.f * (dy > 0 ? 1 : -1)), s1, 2.f);
                    dl->AddLine(ImVec2(cx, cy), ImVec2(cx + dx * sz, cy), s2, 1.f);
                    dl->AddLine(ImVec2(cx, cy), ImVec2(cx, cy + dy * sz), s2, 1.f);
                    dl->AddLine(ImVec2(cx, cy), ImVec2(cx + dx * sz, cy), col, 1.f);
                    dl->AddLine(ImVec2(cx, cy), ImVec2(cx, cy + dy * sz), col, 1.f);
                }
            };
            corner(x1, y1, 1, 1);
            corner(x2, y1, -1, 1);
            corner(x1, y2, 1, -1);
            corner(x2, y2, -1, -1);
        }
    }

    static void draw_health(int hp, const ImVec2 &t, const ImVec2 &b, float box_h, float a)
    {
        if (a < 0.01f)
            return;
        ImDrawList *dl = ImGui::GetForegroundDrawList();
        hp = std::clamp(hp, 0, 100);
        float pct = hp / 100.f;
        float bx = roundf(t.x - 6.f);
        float bw = 3.f;
        float bh = roundf(b.y - t.y);
        float fh = bh * pct;
        float top_y = t.y;
        float bot_y = b.y;
        float fill_top = roundf(bot_y - fh);
        dl->AddRectFilled(ImVec2(bx - 2.f, top_y - 2.f), ImVec2(bx + bw + 2.f, bot_y + 2.f), IM_COL32(0, 0, 0, (int)(120 * a)), 1.f);
        dl->AddRectFilled(ImVec2(bx - 1.f, top_y - 1.f), ImVec2(bx + bw + 1.f, bot_y + 1.f), IM_COL32(0, 0, 0, (int)(200 * a)), 0.f);
        dl->AddRect(ImVec2(bx - 1.f, top_y - 1.f), ImVec2(bx + bw + 1.f, bot_y + 1.f), IM_COL32(30, 30, 30, (int)(255 * a)), 0, 0, 1.f);
        ImU32 col_top = IM_COL32(
            static_cast<int>(config.health_col.x * 255),
            static_cast<int>(config.health_col.y * 255),
            static_cast<int>(config.health_col.z * 255),
            static_cast<int>(config.health_col.w * 255 * a));
        ImU32 col_bot = IM_COL32(
            static_cast<int>(config.health_col.x * 180),
            static_cast<int>(config.health_col.y * 180),
            static_cast<int>(config.health_col.z * 180),
            static_cast<int>(config.health_col.w * 255 * a));
        if (fh > 1.f)
        {
            dl->AddRectFilledMultiColor(ImVec2(bx, fill_top), ImVec2(bx + bw, bot_y), col_top, col_top, col_bot, col_bot);
        }
        if (hp < 100 && fontDefault && bh > 20.f)
        {
            char txt[8];
            snprintf(txt, sizeof(txt), "%d", hp);
            float fs = 10.f;
            ImVec2 ts = fontDefault->CalcTextSizeA(fs, FLT_MAX, 0.f, txt);
            float tx = roundf(bx + (bw - ts.x) * 0.5f);
            float ty = roundf(fill_top - ts.y * 0.5f);
            if (ty < top_y)
                ty = top_y;
            if (ty + ts.y > bot_y)
                ty = bot_y - ts.y;
            draw_text_outlined(dl, fontDefault, fs, ImVec2(tx, ty), IM_COL32(255, 255, 255, (int)(255 * a)), txt);
        }
    }

    static void draw_name(const char *name, const ImVec2 &box_min, float cx, float sz, float a)
    {
        if (!fontDefault || a < 0.01f || !name)
            return;
        ImDrawList *dl = ImGui::GetForegroundDrawList();
        ImVec2 ts = fontDefault->CalcTextSizeA(sz, FLT_MAX, 0.f, name);
        ImVec2 tp(roundf(cx - ts.x * 0.5f), roundf(box_min.y - ts.y - 4.f));
        ImU32 col = IM_COL32(
            static_cast<int>(config.name_col.x * 255),
            static_cast<int>(config.name_col.y * 255),
            static_cast<int>(config.name_col.z * 255),
            static_cast<int>(config.name_col.w * 255 * a));
        draw_text_outlined(dl, fontDefault, sz, tp, col, name);
    }

    static void draw_distance(float dist, float x, float y, float sz, float a)
    {
        if (!fontDefault || a < 0.01f)
            return;
        ImDrawList *dl = ImGui::GetForegroundDrawList();
        char txt[16];
        snprintf(txt, sizeof(txt), "%dm", static_cast<int>(dist));
        ImVec2 tp(roundf(x + 5.f), roundf(y));
        ImU32 col = IM_COL32(
            static_cast<int>(config.distance_col.x * 255),
            static_cast<int>(config.distance_col.y * 255),
            static_cast<int>(config.distance_col.z * 255),
            static_cast<int>(config.distance_col.w * 255 * a));
        draw_text_outlined(dl, fontDefault, sz, tp, col, txt);
    }

    void RenderEsp(pid_t game_pid, uint64_t game_assembly_base, int screen_w, int screen_h)
    {
        if (!config.box && !config.name && !config.health && !config.distance)
            return;

        uint64_t PlayerManager = sdk::get_player_manager_static(game_pid, game_assembly_base, offsets::PlayerManager::TypeOffset);
        if (!PlayerManager)
            return;

        uint64_t LocalPlayer = sdk::read_uint64(game_pid, PlayerManager + offsets::PlayerManager::LocalPlayer);
        if (!LocalPlayer)
            return;

        sdk::ViewMatrix ViewMatrix = sdk::get_view_matrix(game_pid, LocalPlayer);
        sdk::Vector3 LocalPosition = sdk::get_player_position(game_pid, LocalPlayer);

        uint64_t PlayerList = sdk::read_uint64(game_pid, PlayerManager + offsets::PlayerManager::PlayerList);
        if (!PlayerList)
            return;

        int PlayerCount = sdk::read_int32(game_pid, PlayerList + offsets::Dictionary::Count);
        if (PlayerCount <= 0 || PlayerCount > 64)
            return;

        uint64_t ListBuffer = sdk::read_uint64(game_pid, PlayerList + offsets::Dictionary::Entries);
        if (!ListBuffer)
            return;

        for (int i = 0; i < PlayerCount; i++)
        {
            uint64_t Player = sdk::get_player_by_index(game_pid, ListBuffer, i);
            if (!Player || Player == LocalPlayer)
                continue;

            sdk::Vector3 PlayerPosition = sdk::get_player_position(game_pid, Player);
            if (PlayerPosition.x == 0.f && PlayerPosition.y == 0.f && PlayerPosition.z == 0.f)
                continue;

            int Health = sdk::get_player_health(game_pid, Player);
            if (Health <= 0)
                continue;

            float dx = PlayerPosition.x - LocalPosition.x;
            float dy = PlayerPosition.y - LocalPosition.y;
            float dz = PlayerPosition.z - LocalPosition.z;
            float Distance = sqrtf(dx * dx + dy * dy + dz * dz);
            if (Distance > 500.f)
                continue;

            sdk::Vector3 HeadPosition{PlayerPosition.x, PlayerPosition.y + 1.67f, PlayerPosition.z};

            ImVec2 ScreenHead, ScreenFoot;
            bool HeadVisible = sdk::world_to_screen(HeadPosition, ViewMatrix, ScreenHead, (float)screen_w, (float)screen_h);
            bool FootVisible = sdk::world_to_screen(PlayerPosition, ViewMatrix, ScreenFoot, (float)screen_w, (float)screen_h);
            if (!HeadVisible || !FootVisible)
                continue;

            float x1 = roundf(ScreenHead.x);
            float y1 = roundf(fminf(ScreenHead.y, ScreenFoot.y));
            float x2 = roundf(ScreenFoot.x);
            float y2 = roundf(fmaxf(ScreenHead.y, ScreenFoot.y));

            float bh = fabsf(y2 - y1);
            float bw = roundf(bh * 0.25f);
            float cx = roundf((x1 + x2) * 0.5f);

            ImVec2 BoxMin(cx - bw, y1);
            ImVec2 BoxMax(cx + bw, y2);

            float inv_dist = 1.f / (Distance + 0.1f);
            float font_sz = std::clamp(400.f * inv_dist, 10.f, 18.f);

            if (config.box)
            {
                draw_box(BoxMin, BoxMax, 1.f);
            }
            if (config.health)
            {
                draw_health(Health, BoxMin, BoxMax, bh, 1.f);
            }
            if (config.name)
            {
                std::string PlayerName = sdk::get_player_name(game_pid, Player);
                if (!PlayerName.empty())
                {
                    draw_name(PlayerName.c_str(), BoxMin, cx, font_sz, 1.f);
                }
            }
            if (config.distance)
            {
                draw_distance(Distance, BoxMax.x, BoxMin.y, font_sz, 1.f);
            }
        }
    }

}
