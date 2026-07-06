#ifndef ANDROID_DRAW_H
#define ANDROID_DRAW_H

#include <android/native_window.h>
#include <EGL/egl.h>
#include <GLES2/gl2.h>

#define IMGUI_IMPL_OPENGL_ES2

#include "imgui.h"
#include "imgui_internal.h"
#include "backends/imgui_impl_opengl3.h"
#include "backends/imgui_impl_android.h"
#include "../native_surface/ANativeWindowCreator.h"

bool init_egl(uint32_t _screen_x, uint32_t _screen_y, bool log);
bool initGUI_draw(uint32_t _screen_x, uint32_t _screen_y, bool log);
bool ImGui_init();
void drawBegin();
void drawEnd();
void shutdown();

extern ANativeWindow *native_window;
extern ImFont *fontDefault;
extern bool g_Initialized;
extern int native_window_screen_x;
extern int native_window_screen_y;

#endif
