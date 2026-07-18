#include "Android_draw/draw.h"
#include "ui/theme/theme.hpp"
#include "ui/menu.hpp"
#include "ui/bar.hpp"
#include "other/memory.hpp"
#include "game/game.hpp"
#include "func/visuals.hpp"
#include "protect/oxorany.hpp"
#include <cstdio>
#include <thread>
#include <chrono>

static void print_status(const char* status) {
    printf(oxorany("\033[2J\033[H\033[1;38;2;162;144;225m[@nnrpg]\033[0m \033[1;37m%s\033[0m\n"), status);
}

// Функция для запуска Standoff
static void launch_standoff() {
    system(oxorany("am start -n com.standoff/com.standoff.MainActivity"));
}

int main() {
    screen_config();

    int max_size = (displayInfo.height > displayInfo.width ? displayInfo.height : displayInfo.width);
    int min_size = (displayInfo.height < displayInfo.width ? displayInfo.height : displayInfo.width);

    g_sw = static_cast<float>(max_size);
    g_sh = static_cast<float>(min_size);

    native_window_screen_x = max_size;
    native_window_screen_y = max_size;

    if (!initGUI_draw(native_window_screen_x, native_window_screen_y, true)) return -1;

    touch::init(max_size, min_size, (uint8_t)displayInfo.orientation);

    // Автоматический запуск Standoff
    print_status(oxorany("Game detect ✅"));
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    print_status(oxorany("start cheat...."));
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    launch_standoff();
    
    game::init();

    static float alpha = 0.f;
    static bool prev = false;
    static bool game_started = false;

    while (true) {
        drawBegin();

        bool run = game::valid();

#if defined(__x86_64__)
        bool is_landscape = (displayInfo.orientation == 0 || displayInfo.orientation == 2);
#else
        bool is_landscape = (displayInfo.orientation == 1 || displayInfo.orientation == 3);
#endif

        if (run && !prev) {
            if (!game_started) {
                print_status(oxorany("Game detect ✅"));
                std::this_thread::sleep_for(std::chrono::milliseconds(300));
                print_status(oxorany("start cheat...."));
                game_started = true;
            } else {
                print_status(oxorany("Game detect ✅"));
            }
            prev = true;
        } else if (!run && prev) {
            print_status(oxorany("game closed"));
            prev = false;
            game_started = false;
        }

        if (is_landscape) {
            ImGuiIO& io = ImGui::GetIO();
            float dt = io.DeltaTime;
            if (dt <= 0.f || dt > 0.1f) dt = 0.016f;

            float target = run ? 1.f : 0.f;
            float spd = run ? 4.f : 6.f;

            if (alpha < target) {
                alpha += dt * spd;
                if (alpha > target) alpha = target;
            } else if (alpha > target) {
                alpha -= dt * spd;
                if (alpha < target) alpha = target;
            }

            ui::bar::set_game_alpha(alpha);

            if (alpha > 0.001f) {
                ui::menu::render();
            }

            if (run && proc::lib != 0) {
                game::check_lib(get_player_manager());
                visuals::draw();
            }
        }

        bool vis = ui::bar::g_open;
        drawEnd();
        usleep(vis ? 1500 : 4000);
    }

    shutdown();
    return 0;
}