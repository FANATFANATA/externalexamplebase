#include <gui/SurfaceComposerClient.h>
#include <gui/SurfaceControl.h>
#include <gui/Surface.h>
#include <ui/DisplayInfo.h>
#include <ui/PixelFormat.h>
#include <EGL/egl.h>
#include <GLES2/gl2.h>
#include <linux/input.h>
#include <linux/uinput.h>
#include <fcntl.h>
#include <unistd.h>
#include <dirent.h>
#include <cmath>
#include <cstring>

#include "imgui.h"

static EGLDisplay display = EGL_NO_DISPLAY;
static EGLSurface surface = EGL_NO_SURFACE;
static EGLContext context = EGL_NO_CONTEXT;
static int input_fd = -1;
static int screen_width = 0;
static int screen_height = 0;

static const char *vertex_shader_source = R"(
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

static const char *fragment_shader_source = R"(
precision mediump float;
varying vec2 frag_uv;
varying vec4 frag_color;
uniform sampler2D tex;
void main() {
    gl_FragColor = frag_color * texture2D(tex, frag_uv);
}
)";

static GLuint shader_program = 0;
static GLuint vbo_handle = 0;
static GLuint elements_handle = 0;

static bool init_egl(android::sp<android::SurfaceComposerClient> &client,
                     android::sp<android::SurfaceControl> &control,
                     android::sp<android::Surface> &surface_obj)
{
    client = new android::SurfaceComposerClient();
    if (client->initCheck() != android::NO_ERROR)
        return false;

    android::DisplayInfo dinfo;
    android::sp<android::IBinder> dtoken = android::SurfaceComposerClient::getBuiltInDisplay(
        android::ISurfaceComposer::eDisplayIdMain);
    if (client->getDisplayInfo(dtoken, &dinfo) != android::NO_ERROR)
        return false;
    screen_width = dinfo.w;
    screen_height = dinfo.h;

    control = client->createSurface(
        android::String8("ImguiOverlay"),
        screen_width,
        screen_height,
        android::PIXEL_FORMAT_RGBA_8888,
        android::ISurfaceComposerClient::eOpaque | android::ISurfaceComposerClient::eHidden);
    if (!control.get())
        return false;

    android::SurfaceComposerClient::Transaction{}
        .setLayer(control, 0x7FFFFFFF)
        .setPosition(control, 0, 0)
        .show(control)
        .apply();

    surface_obj = control->getSurface();
    if (!surface_obj.get())
        return false;

    display = eglGetDisplay(EGL_DEFAULT_DISPLAY);
    if (display == EGL_NO_DISPLAY)
        return false;
    if (!eglInitialize(display, nullptr, nullptr))
        return false;

    EGLint attribs[] = {
        EGL_RENDERABLE_TYPE, EGL_OPENGL_ES2_BIT,
        EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
        EGL_RED_SIZE, 8,
        EGL_GREEN_SIZE, 8,
        EGL_BLUE_SIZE, 8,
        EGL_ALPHA_SIZE, 8,
        EGL_NONE};
    EGLConfig config;
    EGLint num_configs;
    if (!eglChooseConfig(display, attribs, &config, 1, &num_configs))
        return false;
    if (num_configs < 1)
        return false;

    context = eglCreateContext(display, config, EGL_NO_CONTEXT,
                               new EGLint[3]{EGL_CONTEXT_CLIENT_VERSION, 2, EGL_NONE});
    if (context == EGL_NO_CONTEXT)
        return false;

    surface = eglCreateWindowSurface(display, config, surface_obj.get(), nullptr);
    if (surface == EGL_NO_SURFACE)
        return false;

    if (!eglMakeCurrent(display, surface, surface, context))
        return false;
    return true;
}

static GLuint compile_shader(GLenum type, const char *source)
{
    GLuint shader = glCreateShader(type);
    glShaderSource(shader, 1, &source, nullptr);
    glCompileShader(shader);
    GLint status;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &status);
    if (!status)
    {
        glDeleteShader(shader);
        return 0;
    }
    return shader;
}

static bool init_graphics()
{
    GLuint vs = compile_shader(GL_VERTEX_SHADER, vertex_shader_source);
    GLuint fs = compile_shader(GL_FRAGMENT_SHADER, fragment_shader_source);
    if (!vs || !fs)
        return false;

    shader_program = glCreateProgram();
    glAttachShader(shader_program, vs);
    glAttachShader(shader_program, fs);
    glLinkProgram(shader_program);
    GLint status;
    glGetProgramiv(shader_program, GL_LINK_STATUS, &status);
    glDeleteShader(vs);
    glDeleteShader(fs);
    if (!status)
        return false;

    glGenBuffers(1, &vbo_handle);
    glGenBuffers(1, &elements_handle);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO &io = ImGui::GetIO();
    io.DisplaySize = ImVec2((float)screen_width, (float)screen_height);
    io.DeltaTime = 1.0f / 60.0f;
    io.IniFilename = nullptr;
    io.LogFilename = nullptr;

    unsigned char *pixels;
    int width, height;
    io.Fonts->GetTexDataAsRGBA32(&pixels, &width, &height);
    GLuint font_tex;
    glGenTextures(1, &font_tex);
    glBindTexture(GL_TEXTURE_2D, font_tex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    io.Fonts->TexID = (ImTextureID)(intptr_t)font_tex;

    return true;
}

static void draw_frame()
{
    ImGuiIO &io = ImGui::GetIO();
    io.DisplaySize = ImVec2((float)screen_width, (float)screen_height);

    ImGui::NewFrame();
    ImGui::Begin("Overlay", nullptr,
                 ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize |
                     ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar |
                     ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_AlwaysAutoResize);
    ImGui::Text("Hello World");
    ImGui::End();
    ImGui::Render();
    ImDrawData *draw_data = ImGui::GetDrawData();

    glViewport(0, 0, screen_width, screen_height);
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    if (draw_data->Valid)
    {
        float L = draw_data->DisplayPos.x;
        float R = draw_data->DisplayPos.x + draw_data->DisplaySize.x;
        float T = draw_data->DisplayPos.y;
        float B = draw_data->DisplayPos.y + draw_data->DisplaySize.y;
        const float ortho_projection[4][4] = {
            {2.0f / (R - L), 0.0f, 0.0f, 0.0f},
            {0.0f, 2.0f / (T - B), 0.0f, 0.0f},
            {0.0f, 0.0f, -1.0f, 0.0f},
            {(R + L) / (L - R), (T + B) / (B - T), 0.0f, 1.0f}};

        glUseProgram(shader_program);
        glUniformMatrix4fv(glGetUniformLocation(shader_program, "proj"), 1, GL_FALSE, &ortho_projection[0][0]);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, (GLuint)(intptr_t)draw_data->Fonts->TexID);
        glUniform1i(glGetUniformLocation(shader_program, "tex"), 0);

        glEnable(GL_BLEND);
        glBlendEquation(GL_FUNC_ADD);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glDisable(GL_DEPTH_TEST);
        glDisable(GL_CULL_FACE);
        glEnable(GL_SCISSOR_TEST);

        for (int n = 0; n < draw_data->CmdListsCount; ++n)
        {
            const ImDrawList *cmd_list = draw_data->CmdLists[n];
            const ImDrawVert *vtx_buffer = cmd_list->VtxBuffer.Data;
            const ImDrawIdx *idx_buffer = cmd_list->IdxBuffer.Data;

            glBindBuffer(GL_ARRAY_BUFFER, vbo_handle);
            glBufferData(GL_ARRAY_BUFFER, cmd_list->VtxBuffer.Size * sizeof(ImDrawVert), vtx_buffer, GL_STREAM_DRAW);
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, elements_handle);
            glBufferData(GL_ELEMENT_ARRAY_BUFFER, cmd_list->IdxBuffer.Size * sizeof(ImDrawIdx), idx_buffer, GL_STREAM_DRAW);

            GLint pos_loc = glGetAttribLocation(shader_program, "pos");
            GLint uv_loc = glGetAttribLocation(shader_program, "uv");
            GLint col_loc = glGetAttribLocation(shader_program, "color");

            glEnableVertexAttribArray(pos_loc);
            glVertexAttribPointer(pos_loc, 2, GL_FLOAT, GL_FALSE, sizeof(ImDrawVert), (void *)IM_OFFSETOF(ImDrawVert, pos));
            glEnableVertexAttribArray(uv_loc);
            glVertexAttribPointer(uv_loc, 2, GL_FLOAT, GL_FALSE, sizeof(ImDrawVert), (void *)IM_OFFSETOF(ImDrawVert, uv));
            glEnableVertexAttribArray(col_loc);
            glVertexAttribPointer(col_loc, 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(ImDrawVert), (void *)IM_OFFSETOF(ImDrawVert, col));

            for (int cmd_i = 0; cmd_i < cmd_list->CmdBuffer.Size; ++cmd_i)
            {
                const ImDrawCmd *cmd = &cmd_list->CmdBuffer[cmd_i];
                if (cmd->UserCallback)
                    continue;
                glScissor((GLint)cmd->ClipRect.x, (GLint)(screen_height - cmd->ClipRect.w),
                          (GLsizei)(cmd->ClipRect.z - cmd->ClipRect.x),
                          (GLsizei)(cmd->ClipRect.w - cmd->ClipRect.y));
                glDrawElements(GL_TRIANGLES, (GLsizei)cmd->ElemCount, GL_UNSIGNED_SHORT, (void *)(cmd->IdxOffset * sizeof(ImDrawIdx)));
            }
        }

        glDisable(GL_SCISSOR_TEST);
        glDisable(GL_BLEND);
    }

    eglSwapBuffers(display, surface);
}

static int find_touch_device()
{
    DIR *dir = opendir("/dev/input");
    if (!dir)
        return -1;
    int fd = -1;
    dirent *ent;
    while ((ent = readdir(dir)))
    {
        if (strncmp(ent->d_name, "event", 5) != 0)
            continue;
        char path[256];
        snprintf(path, sizeof(path), "/dev/input/%s", ent->d_name);
        int tmp = open(path, O_RDONLY | O_NONBLOCK);
        if (tmp < 0)
            continue;
        unsigned long absbits = 0;
        if (ioctl(tmp, EVIOCGBIT(EV_ABS, sizeof(absbits)), &absbits) < 0)
        {
            close(tmp);
            continue;
        }
        bool has_mt_x = (absbits & (1 << ABS_MT_POSITION_X)) != 0;
        bool has_mt_y = (absbits & (1 << ABS_MT_POSITION_Y)) != 0;
        if (has_mt_x && has_mt_y)
        {
            fd = tmp;
            break;
        }
        close(tmp);
    }
    closedir(dir);
    return fd;
}

static void process_input()
{
    if (input_fd < 0)
        return;
    ImGuiIO &io = ImGui::GetIO();
    struct input_event ev;
    while (read(input_fd, &ev, sizeof(ev)) == sizeof(ev))
    {
        if (ev.type == EV_ABS)
        {
            if (ev.code == ABS_MT_POSITION_X)
            {
                io.MousePos.x = ev.value * screen_width / 4096.0f;
            }
            else if (ev.code == ABS_MT_POSITION_Y)
            {
                io.MousePos.y = ev.value * screen_height / 4096.0f;
            }
        }
        else if (ev.type == EV_KEY && ev.code == BTN_TOUCH)
        {
            io.MouseDown[0] = (ev.value > 0);
        }
    }
}

int main()
{
    android::sp<android::SurfaceComposerClient> client;
    android::sp<android::SurfaceControl> control;
    android::sp<android::Surface> surface_obj;

    if (!init_egl(client, control, surface_obj))
        return 1;
    if (!init_graphics())
        return 1;

    input_fd = find_touch_device();

    while (true)
    {
        process_input();
        draw_frame();
        usleep(16666);
    }

    if (input_fd >= 0)
        close(input_fd);
    eglDestroySurface(display, surface);
    eglDestroyContext(display, context);
    eglTerminate(display);
    return 0;
}
