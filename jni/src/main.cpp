#include <EGL/egl.h>
#include <GLES2/gl2.h>
#include <linux/input.h>
#include <fcntl.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/ioctl.h>
#include <sys/system_properties.h>
#include <dlfcn.h>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cmath>

#include "imgui.h"

namespace
{
    using android_String8_ctor = void (*)(void *, const char *);
    using android_String8_dtor = void (*)(void *);
    using android_RefBase_incStrong = void (*)(void *, const void *);
    using android_RefBase_decStrong = void (*)(void *, const void *);
    using android_SurfaceComposerClient_ctor = void (*)(void *);
    using android_SurfaceComposerClient_initCheck = int (*)(void *);
    using android_SurfaceComposerClient_createSurface = void *(*)(void *, void *, uint32_t, uint32_t, int32_t, uint32_t, const void *, int32_t, int32_t);
    using android_SurfaceControl_getSurface = void *(*)(void *);
    using android_Transaction_ctor = void (*)(void *);
    using android_Transaction_dtor = void (*)(void *);
    using android_Transaction_setLayer = void *(*)(void *, const void *, int32_t);
    using android_Transaction_setPosition = void *(*)(void *, const void *, float, float);
    using android_Transaction_show = void *(*)(void *, const void *);
    using android_Transaction_apply = int (*)(void *, bool);

    struct android_sp
    {
        void *m_ptr;
        android_sp() : m_ptr(nullptr) {}
        void assign(void *ptr, android_RefBase_incStrong inc, android_RefBase_decStrong dec)
        {
            if (ptr)
                inc(ptr, nullptr);
            if (m_ptr)
                dec(m_ptr, nullptr);
            m_ptr = ptr;
        }
        ~android_sp() = default;
    };

    void *g_libgui = nullptr;
    void *g_libutils = nullptr;
    android_String8_ctor String8_ctor = nullptr;
    android_String8_dtor String8_dtor = nullptr;
    android_RefBase_incStrong RefBase_incStrong = nullptr;
    android_RefBase_decStrong RefBase_decStrong = nullptr;
    android_SurfaceComposerClient_ctor SCC_ctor = nullptr;
    android_SurfaceComposerClient_initCheck SCC_initCheck = nullptr;
    android_SurfaceComposerClient_createSurface SCC_createSurface = nullptr;
    android_SurfaceControl_getSurface SC_getSurface = nullptr;
    android_Transaction_ctor Transaction_ctor = nullptr;
    android_Transaction_dtor Transaction_dtor = nullptr;
    android_Transaction_setLayer Transaction_setLayer = nullptr;
    android_Transaction_setPosition Transaction_setPosition = nullptr;
    android_Transaction_show Transaction_show = nullptr;
    android_Transaction_apply Transaction_apply = nullptr;

    EGLDisplay egl_display = EGL_NO_DISPLAY;
    EGLSurface egl_surface = EGL_NO_SURFACE;
    EGLContext egl_context = EGL_NO_CONTEXT;
    int g_screen_w = 0;
    int g_screen_h = 0;
    int g_input_fd = -1;

    GLuint shader_prog = 0;
    GLuint vbo = 0;
    GLuint ibo = 0;

    const char *vertex_src = R"(
attribute vec2 pos;
attribute vec2 uv;
attribute vec4 color;
varying vec2 frag_uv;
varying vec4 frag_color;
uniform mat4 proj;
void main() {
    frag_uv = uv;
    frag_color = color;
    gl_Position = proj * vec4(pos, 0.0, 1.0);
}
)";

    const char *fragment_src = R"(
precision mediump float;
varying vec2 frag_uv;
varying vec4 frag_color;
uniform sampler2D tex;
void main() {
    gl_FragColor = frag_color * texture2D(tex, frag_uv);
}
)";

    bool load_symbols()
    {
        g_libgui = dlopen("libgui.so", RTLD_NOW);
        g_libutils = dlopen("libutils.so", RTLD_NOW);
        if (!g_libgui || !g_libutils)
            return false;

        String8_ctor = (android_String8_ctor)dlsym(g_libutils, "_ZN7android7String8C1EPKc");
        String8_dtor = (android_String8_dtor)dlsym(g_libutils, "_ZN7android7String8D1Ev");
        RefBase_incStrong = (android_RefBase_incStrong)dlsym(g_libutils, "_ZNK7android7RefBase9incStrongEPKv");
        RefBase_decStrong = (android_RefBase_decStrong)dlsym(g_libutils, "_ZNK7android7RefBase9decStrongEPKv");
        if (!String8_ctor || !String8_dtor || !RefBase_incStrong || !RefBase_decStrong)
            return false;

        SCC_ctor = (android_SurfaceComposerClient_ctor)dlsym(g_libgui, "_ZN7android20SurfaceComposerClientC1Ev");
        SCC_initCheck = (android_SurfaceComposerClient_initCheck)dlsym(g_libgui, "_ZNK7android20SurfaceComposerClient9initCheckEv");
        SC_getSurface = (android_SurfaceControl_getSurface)dlsym(g_libgui, "_ZNK7android14SurfaceControl10getSurfaceEv");
        Transaction_ctor = (android_Transaction_ctor)dlsym(g_libgui, "_ZN7android20SurfaceComposerClient11TransactionC1Ev");
        Transaction_dtor = (android_Transaction_dtor)dlsym(g_libgui, "_ZN7android20SurfaceComposerClient11TransactionD1Ev");
        Transaction_setLayer = (android_Transaction_setLayer)dlsym(g_libgui, "_ZN7android20SurfaceComposerClient11Transaction8setLayerERKNS_2spINS_14SurfaceControlEEEi");
        Transaction_setPosition = (android_Transaction_setPosition)dlsym(g_libgui, "_ZN7android20SurfaceComposerClient11Transaction11setPositionERKNS_2spINS_14SurfaceControlEEEff");
        Transaction_show = (android_Transaction_show)dlsym(g_libgui, "_ZN7android20SurfaceComposerClient11Transaction4showERKNS_2spINS_14SurfaceControlEEE");
        Transaction_apply = (android_Transaction_apply)dlsym(g_libgui, "_ZN7android20SurfaceComposerClient11Transaction5applyEb");
        if (!SCC_ctor || !SCC_initCheck || !SC_getSurface || !Transaction_ctor || !Transaction_dtor ||
            !Transaction_setLayer || !Transaction_setPosition || !Transaction_show || !Transaction_apply)
            return false;

        char sdk_ver[PROP_VALUE_MAX] = {0};
        __system_property_get("ro.build.version.sdk", sdk_ver);
        int sdk = atoi(sdk_ver);
        void *sym = nullptr;
        if (sdk <= 30)
        {
            sym = dlsym(g_libgui, "_ZN7android20SurfaceComposerClient13createSurfaceERKNS_7String8EjjiiRKNS_2spINS_7IBinderEEEii");
        }
        else
        {
            sym = dlsym(g_libgui, "_ZN7android20SurfaceComposerClient13createSurfaceERKNS_7String8EjjiiRKNS_2spINS_7IBinderEEENS_13LayerMetadataEPj");
            if (!sym)
            {
                sym = dlsym(g_libgui, "_ZN7android20SurfaceComposerClient13createSurfaceERKNS_7String8EjjiiRKNS_2spINS_7IBinderEEENS_13LayerMetadataEiPj");
            }
        }
        if (!sym)
            return false;
        SCC_createSurface = (android_SurfaceComposerClient_createSurface)sym;
        return true;
    }

    bool get_screen_size()
    {
        FILE *f = fopen("/sys/class/graphics/fb0/virtual_size", "r");
        if (!f)
            f = fopen("/sys/class/graphics/fb1/virtual_size", "r");
        if (!f)
            return false;
        int w = 0, h = 0;
        if (fscanf(f, "%d,%d", &w, &h) != 2)
        {
            fclose(f);
            return false;
        }
        fclose(f);
        if (w <= 0 || h <= 0)
            return false;
        g_screen_w = w;
        g_screen_h = h;
        return true;
    }

    int find_touch_fd()
    {
        DIR *d = opendir("/dev/input");
        if (!d)
            return -1;
        dirent *ent;
        while ((ent = readdir(d)))
        {
            if (strncmp(ent->d_name, "event", 5) != 0)
                continue;
            char path[256];
            snprintf(path, sizeof(path), "/dev/input/%s", ent->d_name);
            int fd = open(path, O_RDONLY | O_NONBLOCK);
            if (fd < 0)
                continue;
            unsigned long absbits = 0;
            if (ioctl(fd, EVIOCGBIT(EV_ABS, sizeof(absbits)), &absbits) < 0)
            {
                close(fd);
                continue;
            }
            bool mt_x = absbits & (1 << ABS_MT_POSITION_X);
            bool mt_y = absbits & (1 << ABS_MT_POSITION_Y);
            if (mt_x && mt_y)
            {
                closedir(d);
                return fd;
            }
            close(fd);
        }
        closedir(d);
        return -1;
    }

    GLuint compile_shader(GLenum type, const char *src)
    {
        GLuint s = glCreateShader(type);
        glShaderSource(s, 1, &src, nullptr);
        glCompileShader(s);
        GLint st = 0;
        glGetShaderiv(s, GL_COMPILE_STATUS, &st);
        if (!st)
        {
            glDeleteShader(s);
            return 0;
        }
        return s;
    }

    bool init_graphics()
    {
        GLuint vs = compile_shader(GL_VERTEX_SHADER, vertex_src);
        GLuint fs = compile_shader(GL_FRAGMENT_SHADER, fragment_src);
        if (!vs || !fs)
            return false;
        shader_prog = glCreateProgram();
        glAttachShader(shader_prog, vs);
        glAttachShader(shader_prog, fs);
        glLinkProgram(shader_prog);
        GLint st = 0;
        glGetProgramiv(shader_prog, GL_LINK_STATUS, &st);
        glDeleteShader(vs);
        glDeleteShader(fs);
        if (!st)
            return false;

        glGenBuffers(1, &vbo);
        glGenBuffers(1, &ibo);

        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGuiIO &io = ImGui::GetIO();
        io.DisplaySize = ImVec2((float)g_screen_w, (float)g_screen_h);
        io.DeltaTime = 1.0f / 60.0f;
        io.IniFilename = nullptr;
        io.LogFilename = nullptr;

        unsigned char *pixels;
        int w, h;
        io.Fonts->GetTexDataAsRGBA32(&pixels, &w, &h);
        GLuint tex;
        glGenTextures(1, &tex);
        glBindTexture(GL_TEXTURE_2D, tex);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        io.Fonts->TexID = (ImTextureID)(intptr_t)tex;
        return true;
    }

    void draw_frame()
    {
        ImGuiIO &io = ImGui::GetIO();
        io.DisplaySize = ImVec2((float)g_screen_w, (float)g_screen_h);

        ImGui::NewFrame();
        ImGui::Begin("Overlay", nullptr,
                     ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize |
                         ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar |
                         ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_AlwaysAutoResize);
        ImGui::Text("Hello World");
        ImGui::End();
        ImGui::Render();
        ImDrawData *dd = ImGui::GetDrawData();

        glViewport(0, 0, g_screen_w, g_screen_h);
        glClearColor(0, 0, 0, 0);
        glClear(GL_COLOR_BUFFER_BIT);

        if (dd->Valid)
        {
            float L = dd->DisplayPos.x;
            float R = dd->DisplayPos.x + dd->DisplaySize.x;
            float T = dd->DisplayPos.y;
            float B = dd->DisplayPos.y + dd->DisplaySize.y;
            float proj[4][4] = {
                {2.0f / (R - L), 0, 0, 0},
                {0, 2.0f / (T - B), 0, 0},
                {0, 0, -1.0f, 0},
                {(R + L) / (L - R), (T + B) / (B - T), 0, 1.0f}};

            glUseProgram(shader_prog);
            glUniformMatrix4fv(glGetUniformLocation(shader_prog, "proj"), 1, GL_FALSE, &proj[0][0]);
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, (GLuint)(intptr_t)dd->Fonts->TexID);
            glUniform1i(glGetUniformLocation(shader_prog, "tex"), 0);
            glEnable(GL_BLEND);
            glBlendEquation(GL_FUNC_ADD);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            glDisable(GL_DEPTH_TEST);
            glDisable(GL_CULL_FACE);
            glEnable(GL_SCISSOR_TEST);

            for (int n = 0; n < dd->CmdListsCount; ++n)
            {
                const ImDrawList *cl = dd->CmdLists[n];
                glBindBuffer(GL_ARRAY_BUFFER, vbo);
                glBufferData(GL_ARRAY_BUFFER, cl->VtxBuffer.Size * sizeof(ImDrawVert), cl->VtxBuffer.Data, GL_STREAM_DRAW);
                glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
                glBufferData(GL_ELEMENT_ARRAY_BUFFER, cl->IdxBuffer.Size * sizeof(ImDrawIdx), cl->IdxBuffer.Data, GL_STREAM_DRAW);

                GLint pos = glGetAttribLocation(shader_prog, "pos");
                GLint uv = glGetAttribLocation(shader_prog, "uv");
                GLint col = glGetAttribLocation(shader_prog, "color");
                glEnableVertexAttribArray(pos);
                glVertexAttribPointer(pos, 2, GL_FLOAT, GL_FALSE, sizeof(ImDrawVert), (void *)IM_OFFSETOF(ImDrawVert, pos));
                glEnableVertexAttribArray(uv);
                glVertexAttribPointer(uv, 2, GL_FLOAT, GL_FALSE, sizeof(ImDrawVert), (void *)IM_OFFSETOF(ImDrawVert, uv));
                glEnableVertexAttribArray(col);
                glVertexAttribPointer(col, 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(ImDrawVert), (void *)IM_OFFSETOF(ImDrawVert, col));

                for (int i = 0; i < cl->CmdBuffer.Size; ++i)
                {
                    const ImDrawCmd *cmd = &cl->CmdBuffer[i];
                    if (cmd->UserCallback)
                        continue;
                    glScissor((GLint)cmd->ClipRect.x, (GLint)(g_screen_h - cmd->ClipRect.w),
                              (GLsizei)(cmd->ClipRect.z - cmd->ClipRect.x),
                              (GLsizei)(cmd->ClipRect.w - cmd->ClipRect.y));
                    glDrawElements(GL_TRIANGLES, (GLsizei)cmd->ElemCount, GL_UNSIGNED_SHORT, (void *)(cmd->IdxOffset * sizeof(ImDrawIdx)));
                }
            }
            glDisable(GL_SCISSOR_TEST);
            glDisable(GL_BLEND);
        }
        eglSwapBuffers(egl_display, egl_surface);
    }

    void process_input()
    {
        if (g_input_fd < 0)
            return;
        ImGuiIO &io = ImGui::GetIO();
        input_event ev;
        while (read(g_input_fd, &ev, sizeof(ev)) == sizeof(ev))
        {
            if (ev.type == EV_ABS)
            {
                if (ev.code == ABS_MT_POSITION_X)
                    io.MousePos.x = ev.value * g_screen_w / 4096.0f;
                else if (ev.code == ABS_MT_POSITION_Y)
                    io.MousePos.y = ev.value * g_screen_h / 4096.0f;
            }
            else if (ev.type == EV_KEY && ev.code == BTN_TOUCH)
            {
                io.MouseDown[0] = (ev.value > 0);
            }
        }
    }

} // namespace

int main()
{
    if (!load_symbols())
        return 1;
    if (!get_screen_size())
        return 1;

    char client_buf[256] = {0};
    SCC_ctor(client_buf);
    if (SCC_initCheck(client_buf) != 0)
        return 1;

    char name_buf[64];
    String8_ctor(name_buf, "ImguiOverlay");

    void *parent_null = nullptr;
    android_sp surface_control_sp;
    void *sc_ptr = SCC_createSurface(client_buf, name_buf, (uint32_t)g_screen_w, (uint32_t)g_screen_h, 1, 4, &parent_null, -1, -1);
    surface_control_sp.assign(sc_ptr, RefBase_incStrong, RefBase_decStrong);
    String8_dtor(name_buf);
    if (!sc_ptr)
        return 1;

    void *surf_ptr = SC_getSurface(sc_ptr);
    if (!surf_ptr)
        return 1;

    char trans_buf[512] = {0};
    Transaction_ctor(trans_buf);

    void *sc_sp_ptr = &sc_ptr;
    Transaction_setLayer(trans_buf, sc_sp_ptr, 0x7FFFFFFF);
    Transaction_setPosition(trans_buf, sc_sp_ptr, 0.0f, 0.0f);
    Transaction_show(trans_buf, sc_sp_ptr);
    Transaction_apply(trans_buf, false);
    Transaction_dtor(trans_buf);

    egl_display = eglGetDisplay(EGL_DEFAULT_DISPLAY);
    if (egl_display == EGL_NO_DISPLAY)
        return 1;
    if (!eglInitialize(egl_display, nullptr, nullptr))
        return 1;

    EGLint config_attribs[] = {
        EGL_RENDERABLE_TYPE, EGL_OPENGL_ES2_BIT,
        EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
        EGL_RED_SIZE, 8, EGL_GREEN_SIZE, 8, EGL_BLUE_SIZE, 8, EGL_ALPHA_SIZE, 8,
        EGL_NONE};
    EGLConfig cfg;
    EGLint num;
    if (!eglChooseConfig(egl_display, config_attribs, &cfg, 1, &num))
        return 1;
    if (num < 1)
        return 1;

    EGLint ctx_attribs[] = {EGL_CONTEXT_CLIENT_VERSION, 2, EGL_NONE};
    egl_context = eglCreateContext(egl_display, cfg, EGL_NO_CONTEXT, ctx_attribs);
    if (egl_context == EGL_NO_CONTEXT)
        return 1;

    egl_surface = eglCreateWindowSurface(egl_display, cfg, (ANativeWindow *)surf_ptr, nullptr);
    if (egl_surface == EGL_NO_SURFACE)
        return 1;
    if (!eglMakeCurrent(egl_display, egl_surface, egl_surface, egl_context))
        return 1;

    if (!init_graphics())
        return 1;
    g_input_fd = find_touch_fd();

    while (true)
    {
        process_input();
        draw_frame();
        usleep(16666);
    }

    if (g_input_fd >= 0)
        close(g_input_fd);
    eglDestroySurface(egl_display, egl_surface);
    eglDestroyContext(egl_display, egl_context);
    eglTerminate(egl_display);
    if (g_libgui)
        dlclose(g_libgui);
    if (g_libutils)
        dlclose(g_libutils);
    return 0;
}
