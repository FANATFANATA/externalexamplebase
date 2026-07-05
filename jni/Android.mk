LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE := example.sh

LOCAL_SRC_FILES := src/main.cpp

include $(BUILD_EXECUTABLE)
