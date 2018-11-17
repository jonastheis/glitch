LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)

LOCAL_LDLIBS := -lGLESv3 -lEGL
LOCAL_MODULE := hello.out
LOCAL_SRC_FILES := \
	eglSetup.c hello.c
include $(BUILD_EXECUTABLE)