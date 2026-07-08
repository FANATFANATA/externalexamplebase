cmake_minimum_required(VERSION 3.22.1)
project(example.sh LANGUAGES C CXX)
add_library(imgui STATIC
    imgui/imgui.cpp
    imgui/imgui_draw.cpp
    imgui/imgui_tables.cpp
    imgui/imgui_widgets.cpp
    imgui/backends/imgui_impl_opengl3.cpp
    imgui/backends/imgui_impl_android.cpp
)
target_include_directories(imgui PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/imgui)
target_compile_options(imgui PRIVATE $<$<COMPILE_LANGUAGE:CXX>:-std=c++17 -fno-exceptions -fno-rtti>)
target_compile_definitions(imgui PRIVATE IMGUI_IMPL_OPENGL_ES2)
add_executable(example.sh
    src/main.cpp
    src/Android_draw/draw.cpp
    src/Android_touch/Touch.cpp
    src/memory/memory.cpp
    src/memory/read.cpp
    src/memory/write.cpp
    src/watermark/watermark.cpp
    src/menu/menu.cpp
)
set_target_properties(example.sh PROPERTIES SUFFIX "")
target_include_directories(example.sh PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)
target_compile_options(example.sh PRIVATE $<$<COMPILE_LANGUAGE:CXX>:-std=c++17 -fno-exceptions -fno-rtti>)
target_compile_definitions(example.sh PRIVATE IMGUI_IMPL_OPENGL_ES2)
target_compile_options(example.sh PRIVATE -fvisibility=hidden)
target_link_libraries(example.sh
    PRIVATE
    imgui
    EGL
    GLESv2
    dl
    log
    android
)
