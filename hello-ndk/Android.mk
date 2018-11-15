LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)

LOCAL_MODULE := hello.out
LOCAL_SRC_FILES := \
	hello.c
include $(BUILD_EXECUTABLE)