#include "menu.h"
#include "watermark/watermark.h"
#include <imgui.h>
#include <imgui_internal.h>

namespace menu
{

    static int current_tab = 0;
    static const char *tabs[] = {"ESP", "Aim", "Skins", "Misc", "Config", "About us"};
    static const int tab_count = sizeof(tabs) / sizeof(tabs[0]);

    void ApplyStyle()
    {
        ImGuiStyle &s = ImGui::GetStyle();
        s.WindowRounding = 6.0f;
        s.ChildRounding = 6.0f;
        s.FrameRounding = 4.0f;
        s.GrabRounding = 4.0f;
        s.PopupRounding = 4.0f;
        s.ScrollbarRounding = 6.0f;
        s.TabRounding = 6.0f;

        ImVec4 *colors = s.Colors;
        colors[ImGuiCol_WindowBg] = ImVec4(0.0235f, 0.0784f, 0.1059f, 1.00f);       // #06141B
        colors[ImGuiCol_ChildBg] = ImVec4(0.0667f, 0.1294f, 0.1765f, 1.00f);        // #11212D
        colors[ImGuiCol_FrameBg] = ImVec4(0.1451f, 0.2157f, 0.2706f, 0.54f);        // #253745
        colors[ImGuiCol_FrameBgHovered] = ImVec4(0.2902f, 0.3608f, 0.4157f, 0.40f); // #4A5C6A
        colors[ImGuiCol_FrameBgActive] = ImVec4(0.6078f, 0.6588f, 0.6706f, 0.67f);  // #9BA8AB
        colors[ImGuiCol_TitleBg] = ImVec4(0.0235f, 0.0784f, 0.1059f, 1.00f);
        colors[ImGuiCol_TitleBgActive] = ImVec4(0.0667f, 0.1294f, 0.1765f, 1.00f);
        colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.0235f, 0.0784f, 0.1059f, 0.51f);
        colors[ImGuiCol_ScrollbarBg] = ImVec4(0.0235f, 0.0784f, 0.1059f, 0.53f);
        colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.1451f, 0.2157f, 0.2706f, 1.00f);
        colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.2902f, 0.3608f, 0.4157f, 1.00f);
        colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.6078f, 0.6588f, 0.6706f, 1.00f);
        colors[ImGuiCol_CheckMark] = ImVec4(0.8000f, 0.8157f, 0.8235f, 1.00f); // #CCD0CF
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
    }

    void ShowMenu(bool *p_open)
    {
        if (!*p_open)
            return;

        ImGui::SetNextWindowSize(ImVec2(550, 400), ImGuiCond_FirstUseEver);
        ImGui::SetNextWindowPos(ImVec2(ImGui::GetIO().DisplaySize.x * 0.5f, ImGui::GetIO().DisplaySize.y * 0.5f),
                                ImGuiCond_FirstUseEver, ImVec2(0.5f, 0.5f));

        ImGui::Begin("Sdeeb", p_open, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar);

        ImGui::Columns(2, "menu_columns", false);
        ImGui::SetColumnWidth(0, 100);

        for (int i = 0; i < tab_count; i++)
        {
            bool selected = (current_tab == i);
            if (ImGui::Selectable(tabs[i], selected))
            {
                current_tab = i;
            }
            if (selected)
            {
                ImGui::SetItemDefaultFocus();
            }
        }

        ImGui::NextColumn();

        ImGui::BeginChild("tab_content", ImVec2(0, 0), false);

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

        ImGui::EndChild();
        ImGui::Columns(1);

        ImGui::End();
    }

}
