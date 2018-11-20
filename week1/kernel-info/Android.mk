LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)

LOCAL_LDLIBS := -lGLESv3 -lEGL
LOCAL_MODULE := main.out
# LOCAL_CFLAGS := -DDUMP
LOCAL_SRC_FILES := \
	eglSetup.c mem_util.c main.c
include $(BUILD_EXECUTABLE)