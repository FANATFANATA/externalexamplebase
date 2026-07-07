#include "Android_draw/draw.h"
#include "Android_touch/Touch.hpp"
#include <csignal>
#include <sys/stat.h>
#include <unistd.h>
#include <android/log.h>

#define LOG_TAG "ImguiOverlay"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)

bool g_running = true;
struct sigaction old_sa_int;
struct sigaction old_sa_term;
struct sigaction old_sa_quit;

void signal_handler(int)
{
    g_running = false;
}

void cleanup_signals()
{
    sigaction(SIGINT, &old_sa_int, nullptr);
    sigaction(SIGTERM, &old_sa_term, nullptr);
    sigaction(SIGQUIT, &old_sa_quit, nullptr);
}

int main()
{
    struct sigaction sa{};
    sa.sa_handler = signal_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    sigaction(SIGINT, &sa, &old_sa_int);
    sigaction(SIGTERM, &sa, &old_sa_term);
    sigaction(SIGQUIT, &sa, &old_sa_quit);

    android::ANativeWindowCreator::DisplayInfo dispInfo = android::ANativeWindowCreator::GetDisplayInfo();
    if (!initGUI_draw(dispInfo.width, dispInfo.height, false))
    {
        LOGE("Failed to init GUI draw");
        cleanup_signals();
        return 1;
    }

    if (!touch::init(dispInfo.width, dispInfo.height, dispInfo.orientation))
    {
        LOGI("Touch input not available, overlay will be non-interactive");
    }

    ImGuiIO &io = ImGui::GetIO();
    io.DisplaySize = ImVec2((float)dispInfo.width, (float)dispInfo.height);

    while (g_running)
    {
        if (access("/data/local/tmp/stop_overlay", F_OK) == 0)
        {
            unlink("/data/local/tmp/stop_overlay");
            g_running = false;
            break;
        }

        drawBegin();

        ImGui::Begin("Overlay", nullptr,
                     ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoScrollbar |
                         ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_AlwaysAutoResize);
        ImGui::Text("Hello World");
        ImGui::End();

        drawEnd();
    }

    shutdown();
    touch::shutdown();
    cleanup_signals();
    return 0;
}
