LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)

LOCAL_LDLIBS := -lGLESv3 -lEGL
LOCAL_MODULE := main.out
LOCAL_CPP_FEATURES += exceptions
LOCAL_SRC_FILES := \
	shader.cpp main.cpp
include $(BUILD_EXECUTABLE)