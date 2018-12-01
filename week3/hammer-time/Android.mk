LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)

LOCAL_LDLIBS := -lGLESv3 -lEGL
LOCAL_MODULE := main.out
LOCAL_SRC_FILES := \
	eglSetup.cpp shader.cpp mem_util.cpp allocator.cpp counters.cpp main.cpp
include $(BUILD_EXECUTABLE)