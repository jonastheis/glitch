# Hello Native Development Kit (NDK)
This is a short introduction on how to compile, transfer and execute native code on an Android emulator. 
Derived from: https://software.intel.com/en-us/articles/building-an-android-command-line-application-using-the-ndk-build-tools

Prerequisites: 
- install Android Studio
- install NDK (SDK manager)
- install emulator

Add Android to your PATH
```bash
export ANDROID_HOME=/Users/{username}/Library/Android/sdk
export PATH=$PATH:$ANDROID_HOME/tools/bin
export PATH=$PATH/:$ANDROID_HOME/platform-tools
export PATH=$PATH/:$ANDROID_HOME/ndk-bundle
export PATH=$PATH/:$ANDROID_HOME/emulator
```

Now create the actual application.
```bash
# Create necessary Makefiles Android.mk and Application.mk

# Adjust contents of Android.mk file (LOCAL_MODULE, LOCAL_SRC_FILES)

# Compile program
NDK_PROJECT_PATH=. ndk-build NDK_APPLICATION_MK=./Application.mk

# Start emulator
emulator @Nexus_5X_API_28_x86 -writable-system & 

# Transfer file to emulator
adb root
adb push obj/local/x86/hello.out /data/exec-test/hello.out

adb shell
cd /data/exec-test/
./hello.out
```

### Useful when coding

```bash
NDK_PROJECT_PATH=. ndk-build NDK_APPLICATION_MK=./Application.mk && adb push obj/local/x86/hello.out /data/exec-test/hello.out
```


### Resources 

- https://developer.android.com/ndk/guides/stable_apis#ogles-3
- https://stackoverflow.com/questions/18529021/android-initialise-opengl2-0-context-with-egl
- https://stackoverflow.com/questions/49212540/android-ndk-opengles-not-rendering-triangles