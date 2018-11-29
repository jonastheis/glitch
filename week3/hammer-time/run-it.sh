#!/bin/bash

NDK_PROJECT_PATH=. ndk-build NDK_APPLICATION_MK=./Application.mk 

adb push obj/local/x86/main.out /data/exec-test/main.out
adb push shaders/tr.fs /data/exec-test/tr.fs
adb push shaders/tr.vs /data/exec-test/tr.vs

adb shell  /data/exec-test/main.out 