LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE := imgui

LOCAL_SRC_FILES := imgui/imgui.cpp \
                   imgui/imgui_draw.cpp \
                   imgui/imgui_tables.cpp \
                   imgui/imgui_widgets.cpp

LOCAL_EXPORT_C_INCLUDES := $(LOCAL_PATH)/imgui

LOCAL_CPPFLAGS := -std=c++17 -fno-exceptions -fno-rtti

include $(BUILD_STATIC_LIBRARY)

include $(CLEAR_VARS)

LOCAL_MODULE := example.sh

LOCAL_SRC_FILES := src/main.cpp

LOCAL_CPPFLAGS := -std=c++17

LOCAL_STATIC_LIBRARIES := imgui

LOCAL_LDLIBS := -lEGL -lGLESv2 -lgui -lbinder -lutils -lcutils -llog

LOCAL_EXPORT_C_INCLUDES := $(LOCAL_PATH)/imgui

include $(BUILD_EXECUTABLE)
