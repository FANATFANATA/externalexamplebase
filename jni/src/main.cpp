#include "Android_draw/draw.h"
#include "Android_touch/Touch.hpp"
#include "watermark/watermark.h"
#include "menu/menu.h"
#include <csignal>
#include <sys/stat.h>
#include <unistd.h>
#include <android/log.h>
#include <imgui.h>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <signal.h>
#include <chrono>
#include <thread>

#define LOG_TAG "ImguiOverlay"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)

bool g_running = true;
struct sigaction old_sa_int;
struct sigaction old_sa_term;
struct sigaction old_sa_quit;
pid_t game_pid = -1;

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

pid_t get_game_pid()
{
    FILE *fp = popen("su -c 'pidof com.axlebolt.standoff2'", "r");
    if (!fp)
        return -1;
    char buf[64];
    if (fgets(buf, sizeof(buf), fp))
    {
        pclose(fp);
        return atoi(buf);
    }
    pclose(fp);
    return -1;
}

bool start_game()
{
    LOGI("Starting game");
    int ret = system("su -c 'am start -n com.axlebolt.standoff2/com.google.firebase.MessagingUnityPlayerActivity' 2>/dev/null");
    if (ret != 0)
    {
        LOGE("Failed to start game");
        return false;
    }
    return true;
}

bool ensure_game_running()
{
    game_pid = get_game_pid();
    if (game_pid > 0)
    {
        LOGI("Game already running");
        return true;
    }
    if (!start_game())
        return false;
    auto start = std::chrono::steady_clock::now();
    while (std::chrono::steady_clock::now() - start < std::chrono::seconds(15))
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
        game_pid = get_game_pid();
        if (game_pid > 0)
        {
            LOGI("Game started");
            return true;
        }
    }
    LOGE("Timeout waiting for game process");
    return false;
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

    if (!ensure_game_running())
    {
        cleanup_signals();
        return 1;
    }

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

    io.Fonts->AddFontFromFileTTF("/system/fonts/Roboto-Regular.ttf", 16.0f);
    io.FontDefault = io.Fonts->Fonts.back();

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

        if (kill(game_pid, 0) != 0)
        {
            LOGI("Game process died, exiting");
            g_running = false;
            break;
        }

        drawBegin();

        watermark::DrawWatermark(menu_visible);
        menu::ShowMenu(&menu_visible);

        drawEnd();
    }

    shutdown();
    touch::shutdown();
    cleanup_signals();
    return 0;
}
