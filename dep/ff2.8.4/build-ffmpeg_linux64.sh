#!/bin/bash
####from https://gist.github.com/ayanamist/6282b01fbdd9c105791f
NDKROOT=~/android-ndk   # insert android ndk path
DEVICE_TYPE="arm_neon" 		# insert device type(arm_neon, arm_tegra2, x86)
OS=LINUX64                        # insert OS Version(LINUX64, LINUX32, WINDOWS_64, WINDOWS)

if [ $DEVICE_TYPE == "arm_neon" ];
then
		TOOLCHAIN=toolchain-arm
		ARCH=arm
		CPU=armv7-a
		TARGET_CFLAGS="-marm -march=armv7-a -mfloat-abi=softfp -mfpu=neon -mtune=cortex-a8 -mvectorize-with-neon-quad"
		TARGET_LDFLAGS="-Wl,--fix-cortex-a8"
		EABI="armeabi-v7a"
elif [ $DEVICE_TYPE == "arm_tegra2" ];
then
		TOOLCHAIN=toolchain-arm
		ARCH=arm
		CPU=armv7-a
		TARGET_CFLAGS="-marm -march=armv7-a -mfloat-abi=softfp -mfpu=vfpv3-d16"
		TARGET_LDFLAGS="-Wl,--fix-cortex-a8"
		EABI="armeabi"
elif [ $DEVICE_TYPE == "x86" ];
then
		TOOLCHAIN=toolchain-x86
		ARCH=x86
		TARGET_CFLAGS="-march=atom -mno-sse4 -mno-sse4.2 -O3 -DNDEBUG"
		EABI="x86"
fi


echo *Buliding for $1*

if [ $ARCH == "arm" ];
then
	if [ $OS == "LINUX64" ];
	then
		PREBUILT=$NDKROOT/toolchains/arm-linux-androideabi-4.6/prebuilt/linux-x86_64
	elif [ $OS == "LINUX32" ];
	then
		PREBUILT=$NDKROOT/toolchains/arm-linux-androideabi-4.6/prebuilt/linux-x86
	elif [ $OS == "WINDOWS_64" ];
	then
		PREBUILT=$NDKROOT/toolchains/arm-linux-androideabi-4.6/prebuilt/windows-x86_64
	elif [ $OS == "WINDOWS" ];
	then
		PREBUILT=$NDKROOT/toolchains/arm-linux-androideabi-4.6/prebuilt/windows
	fi
	CC="$PREBUILT/bin/arm-linux-androideabi-gcc"
	CXX="$PREBUILT/bin/arm-linux-androideabi-g++"
	STRIP="$PREBUILT/bin/arm-linux-androideabi-strip"
	CROSS_PREFIX="$PREBUILT/bin/arm-linux-androideabi-"
	HOST="arm-linux-androideabi"
	PLATFORM=$NDKROOT/platforms/android-8/arch-arm
	CONFIG_PARAMS=""
elif [ $ARCH == "x86" ];
then
	if [ $OS == "LINUX64" ];
	then
		PREBUILT=$NDKROOT/toolchains/x86-4.6/prebuilt/linux-x86_64
	elif [ $OS == "LINUX32" ];
	then
		PREBUILT=$NDKROOT/toolchains/x86-4.6/prebuilt/linux-x86
	elif [ $OS == "WINDOWS_64" ];
	then
		PREBUILT=$NDKROOT/toolchains/x86-4.6/prebuilt/windows-x86_64
	elif [ $OS == "WINDOWS" ];
	then
		PREBUILT=$NDKROOT/toolchains/x86-4.6/prebuilt/windows
	fi

	CC="$PREBUILT/bin/i686-linux-android-gcc"
	CXX="$PREBUILT/bin/i686-linux-android-g++"
	STRIP="$PREBUILT/bin/i686-linux-android-strip"
	CROSS_PREFIX="$PREBUILT/bin/i686-linux-android-"
	HOST="x86"
	PLATFORM=$NDKROOT/platforms/android-9/arch-x86
	CONFIG_PARAMS="--disable-asm"
fi


./configure --target-os=linux \
	--arch=$ARCH \
	--cpu=$CPU \
	--enable-shared \
	--disable-static \
	--enable-gpl \
	--enable-nonfree \
	--disable-doc \
	--disable-programs \
	--disable-ffmpeg \
	--disable-ffplay \
	--disable-ffserver \
	--disable-ffprobe \
	--disable-avdevice \
	--disable-encoders \
	--disable-muxers \
	--disable-devices \
	--disable-protocols \
	--disable-avfilter \
	--enable-optimizations \
	--enable-protocol=file \
	--enable-protocol=http \
	--enable-protocol=hls \
	--enable-protocol=mmsh \
	--enable-protocol=mmst \
	--enable-protocol=rtmp \
	--enable-protocol=rtmpe \
	--enable-protocol=rtmps \
	--enable-protocol=rtmpt \
	--enable-protocol=rtmpte \
	--enable-protocol=rtmpts \
	--enable-protocol=rtp \
	--enable-protocol=udp \
	--enable-protocol=https \
	--enable-protocol=tls \
	--disable-avdevice \
	$FFMPEG_EXTRA_CONFIG $CONFIG_PARAMS \
	--disable-symver \
	--enable-cross-compile \
	--sysroot=$PLATFORM \
	--cc=$CC \
	--cross-prefix=$CROSS_PREFIX \
	--extra-cflags="-I$PLATFORM/usr/include -fPIC -DANDROID -D_FILE_OFFSET_BITS=64 -D_LARGEFILE_SOURCE $TARGET_CFLAGS" \
	--extra-ldflags="$TARGET_LDFLAGS  -L$PLATFORM/usr/lib"

make clean
make -j8
if [ $? != 0 ]; then
	echo "Error FFmpeg build fail"
	exit
fi

LIBDIR=android
mkdir $LIBDIR 2>/dev/null
LIBDIR+=/$EABI
mkdir $LIBDIR 2>/dev/null

OBJ_FILES="$CC -shared -Wl,-soname,libffmpeg_dice.so -Wl,-Bsymbolic -Llibavcodec  -Llibavformat -Llibavutil  -Llibswscale -Llibswresample $TARGET_LDFLAGS  -L$PLATFORM/usr/lib  -lm -lz  --sysroot=$PLATFORM -Wl,--as-needed -Wl,--warn-common -Wl,-rpath-link=libswresample:libswscale:libavformat:libavcodec:libavutil -o libffmpeg_dice.so "

SRC_PATH=.
recurse_dir() {
 for i in "$1"/*;do
    if [ -d "$i" ];then
        recurse_dir "$i"
    elif [ -f "$i" ]; then
        if [ ${i: -2} == ".o" ]; then
        	ofile=" ${i#$SRC_PATH/}"
        	if [ $ofile != "libavutil/inverse.o" ] && \
        	[ $ofile != "libavformat/golomb.o" ] && \
        	[ $ofile != "libavformat/golomb_tab.o" ] && \
        	[ $ofile != "libswresample/log2_tab.o" ] && \
        	[ $ofile != "libavutil/log2_tab.o" ] && \
        	[ $ofile != "libavformat/log2_tab.o" ] && \
        	[ $ofile != "libswscale/log2_tab.o" ];
        	then
        		#echo "file: $ff"
        		OBJ_FILES+=$ofile
        	fi
        fi
    fi
 done
}

recurse_dir $SRC_PATH
echo $OBJ_FILES

$OBJ_FILES
$STRIP libffmpeg_dice.so
cp libffmpeg_dice.so $LIBDIR/libffmpeg_dice.so
$STRIP $LIBDIR/libffmpeg_dice.so
echo *DONE*
