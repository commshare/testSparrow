APP_ABI:=armeabi-v7a

DEBUG := $(NDK_DEBUG)

ifndef NDK_DEBUG
	DEBUG := 0
endif
ifeq ($(DEBUG),true)
	DEBUG := 1
endif

ifeq ($(DEBUG),1)
	APP_CFLAGS += -O0 -g
	APP_OPTIM := debug
else
	APP_CFLAGS += -O2
	APP_OPTIM := release
endif

###FIX :error: format not a string literal and no format arguments [-Werror=format-security]
APP_CFLAGS += -Wno-error=format-security



###setenvv4 use api 17 toolchain
APP_PLATFORM := android-17
NDK_TOOLCHAIN_VERSION := 4.8