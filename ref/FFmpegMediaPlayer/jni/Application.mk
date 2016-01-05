APP_ABI :=  armeabi-v7a
#x86
# arm64-v8a x86
APP_PLATFORM := android-14
APP_STL := gnustl_static
#stlport_static

 APP_CPPFLAGS := -std=c++11 -Wno-literal-suffix
APP_CFLAGS := --std=c++11
#APP_OPTIM  := debug
#LOCAL_CPP_FEATURES += -Wliteral-suffix
NDK_TOOLCHAIN_VERSION := 4.8
