#include "Android_draw/draw.h"
#include "Android_touch/Touch.hpp"
#include "watermark/watermark.h"
#include "menu/menu.h"
#include "other/memory.hpp"
#include "game/game.hpp"
#include "game/math.hpp"
#include "func/visuals.hpp"
#include "protect/oxorany.hpp"
#include <csignal>
#include <unistd.h>
#include <android/log.h>
#include <imgui.h>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <chrono>
#include <thread>

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

static void launch_standoff() {
    system(oxorany("am start -n com.standoff/com.standoff.MainActivity"));
}

int main()
{
    if (getuid() != 0)
    {
        __android_log_print(ANDROID_LOG_FATAL, LOG_TAG, "Root required");
        return 1;
    }

    struct sigaction sa{};
    sa.sa_handler = signal_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    sigaction(SIGINT, &sa, &old_sa_int);
    sigaction(SIGTERM, &sa, &old_sa_term);
    sigaction(SIGQUIT, &sa, &old_sa_quit);

    launch_standoff();

    game::init();

    android::ANativeWindowCreator::DisplayInfo dispInfo = android::ANativeWindowCreator::GetDisplayInfo();
    if (!initGUI_draw(dispInfo.width, dispInfo.height, false))
    {
        LOGE("Failed to init GUI draw");
        cleanup_signals();
        return 1;
    }

    g_sw = (float)(dispInfo.width > dispInfo.height ? dispInfo.width : dispInfo.height);
    g_sh = (float)(dispInfo.width < dispInfo.height ? dispInfo.width : dispInfo.height);

    if (!touch::init(dispInfo.width, dispInfo.height, dispInfo.orientation))
    {
        LOGI("Touch input not available, overlay will be non-interactive");
    }

    ImGuiIO &io = ImGui::GetIO();
    io.DisplaySize = ImVec2((float)dispInfo.width, (float)dispInfo.height);

    menu::ApplyStyle();

    bool menu_visible = false;

    while (g_running)
    {
        if (access("/data/local/tmp/stop_overlay", F_OK) == 0)
        {
            unlink("/data/local/tmp/stop_overlay");
            g_running = false;
            break;
        }

        drawBegin();

        if (game::valid() && proc::lib != 0) {
            game::check_lib(get_player_manager());
            visuals::draw();
        }

        watermark::DrawWatermark(menu_visible);
        menu::ShowMenu(&menu_visible);

        drawEnd();
        usleep(menu_visible ? 1500 : 4000);
    }

    shutdown();
    touch::shutdown();
    cleanup_signals();
    return 0;
}
