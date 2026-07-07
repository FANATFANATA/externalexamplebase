LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)
LOCAL_MODULE := imgui
LOCAL_SRC_FILES := imgui/imgui.cpp \
                   imgui/imgui_draw.cpp \
                   imgui/imgui_tables.cpp \
                   imgui/imgui_widgets.cpp \
                   imgui/backends/imgui_impl_opengl3.cpp \
                   imgui/backends/imgui_impl_android.cpp
LOCAL_C_INCLUDES := $(LOCAL_PATH)/imgui
LOCAL_EXPORT_C_INCLUDES := $(LOCAL_PATH)/imgui
LOCAL_CPPFLAGS := -std=c++17 -fno-exceptions -fno-rtti -DIMGUI_IMPL_OPENGL_ES2
include $(BUILD_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := example.sh
LOCAL_SRC_FILES := src/main.cpp \
                   src/Android_draw/draw.cpp \
                   src/Android_touch/Touch.cpp \
                   src/memory/memory.cpp \
                   src/memory/read.cpp \
                   src/memory/write.cpp \
                   src/watermark/watermark.cpp \
                   src/menu/menu.cpp
LOCAL_C_INCLUDES := $(LOCAL_PATH)/src
LOCAL_CPPFLAGS := -std=c++17 -fno-exceptions -fno-rtti -DIMGUI_IMPL_OPENGL_ES2
LOCAL_CFLAGS := -fvisibility=hidden
LOCAL_STATIC_LIBRARIES := imgui
LOCAL_LDLIBS := -lEGL -lGLESv2 -ldl -llog -landroid
include $(BUILD_EXECUTABLE)
