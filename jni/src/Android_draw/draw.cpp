#include "draw.h"

ANativeWindow *native_window = nullptr;
EGLDisplay display = EGL_NO_DISPLAY;
EGLConfig config = nullptr;
EGLSurface surface = EGL_NO_SURFACE;
EGLContext context = EGL_NO_CONTEXT;
bool g_Initialized = false;
int native_window_screen_x = 0;
int native_window_screen_y = 0;
ImFont *fontDefault = nullptr;
ImFont *espFont = nullptr;

bool init_egl(uint32_t _screen_x, uint32_t _screen_y, bool log)
{
    native_window = android::ANativeWindowCreator::Create("Overlay", _screen_x, _screen_y, false);
    if (!native_window)
        return false;
    ANativeWindow_acquire(native_window);
    display = eglGetDisplay(EGL_DEFAULT_DISPLAY);
    if (display == EGL_NO_DISPLAY)
        return false;
    if (eglInitialize(display, nullptr, nullptr) != EGL_TRUE)
        return false;

    const EGLint attribList[] = {
        EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
        EGL_RENDERABLE_TYPE, EGL_OPENGL_ES2_BIT,
        EGL_BLUE_SIZE, 8, EGL_GREEN_SIZE, 8, EGL_RED_SIZE, 8, EGL_ALPHA_SIZE, 8,
        EGL_DEPTH_SIZE, 16, EGL_STENCIL_SIZE, 8,
        EGL_NONE};
    EGLint numConfig = 0;
    if (eglChooseConfig(display, attribList, &config, 1, &numConfig) != EGL_TRUE)
        return false;

    EGLint eglFormat;
    eglGetConfigAttrib(display, config, EGL_NATIVE_VISUAL_ID, &eglFormat);
    ANativeWindow_setBuffersGeometry(native_window, 0, 0, eglFormat);

    EGLint ctxAttribs[] = {EGL_CONTEXT_CLIENT_VERSION, 2, EGL_NONE};
    context = eglCreateContext(display, config, EGL_NO_CONTEXT, ctxAttribs);
    if (context == EGL_NO_CONTEXT)
        return false;

    surface = eglCreateWindowSurface(display, config, native_window, nullptr);
    if (surface == EGL_NO_SURFACE)
        return false;

    if (!eglMakeCurrent(display, surface, surface, context))
        return false;
    native_window_screen_x = _screen_x;
    native_window_screen_y = _screen_y;
    return true;
}

bool ImGui_init()
{
    if (g_Initialized)
        return true;
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui::StyleColorsDark();

    ImGuiIO &io = ImGui::GetIO();
    io.IniFilename = nullptr;
    static const ImWchar ranges[] = {0x0020, 0x00FF, 0x0100, 0x017F, 0x0400, 0x04FF, 0};
    ImFontConfig font_config;
    font_config.SizePixels = 42.0f;
    font_config.OversampleH = 3;
    font_config.OversampleV = 3;
    font_config.PixelSnapH = true;
    const char *font_path = "/system/fonts/Roboto-Regular.ttf";
    FILE *f = fopen(font_path, "rb");
    if (f)
    {
        fclose(f);
        fontDefault = io.Fonts->AddFontFromFileTTF(font_path, 42.0f, &font_config, ranges);
    }
    else
    {
        font_path = "/system/fonts/DroidSans.ttf";
        f = fopen(font_path, "rb");
        if (f)
        {
            fclose(f);
            fontDefault = io.Fonts->AddFontFromFileTTF(font_path, 42.0f, &font_config, ranges);
        }
    }
    if (!fontDefault)
    {
        fontDefault = io.Fonts->AddFontDefault();
    }
    io.FontDefault = fontDefault;

    font_config.SizePixels = 18.0f;
    espFont = io.Fonts->AddFontFromFileTTF(font_path, 18.0f, &font_config, ranges);
    if (!espFont) espFont = fontDefault;

    ImGui_ImplAndroid_Init(native_window);
    ImGui_ImplOpenGL3_Init("#version 100");

    ImGui::GetStyle().ScaleAllSizes(1.0f);
    g_Initialized = true;
    return true;
}

bool initGUI_draw(uint32_t _screen_x, uint32_t _screen_y, bool log)
{
    if (!init_egl(_screen_x, _screen_y, log))
        return false;
    if (!ImGui_init())
        return false;
    return true;
}

void drawBegin()
{
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplAndroid_NewFrame();
    ImGui::NewFrame();
}

void drawEnd()
{
    ImGui::Render();
    glClear(GL_COLOR_BUFFER_BIT);
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    eglSwapBuffers(display, surface);
}

void shutdown()
{
    if (!g_Initialized)
        return;
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplAndroid_Shutdown();
    ImGui::DestroyContext();

    if (display != EGL_NO_DISPLAY)
    {
        eglMakeCurrent(display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
        if (context != EGL_NO_CONTEXT)
            eglDestroyContext(display, context);
        if (surface != EGL_NO_SURFACE)
            eglDestroySurface(display, surface);
        eglTerminate(display);
    }
    display = EGL_NO_DISPLAY;
    context = EGL_NO_CONTEXT;
    surface = EGL_NO_SURFACE;

    ANativeWindow_release(native_window);
    android::ANativeWindowCreator::Destroy(native_window);
    native_window = nullptr;
    g_Initialized = false;
}
