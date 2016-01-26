#!/bin/sh

trap exit ERR

LIB_DIR=arm-neon
FFMPEG_DIR=ffmpeg-$LIB_DIR
SCRIPT_DIR=$( (cd -P $(dirname $0) && pwd) )

rm -rf $FFMPEG_DIR
mkdir $FFMPEG_DIR
cd $FFMPEG_DIR
mkdir -p dist

NDK=D:/android/android-ndk
SYSROOT=$NDK/platforms/android-14/arch-arm

FLAGS="--prefix="dist" --cross-prefix=arm-linux-androideabi- --enable-cross-compile --target-os=linux"
FLAGS="$FLAGS --arch=arm --enable-neon --enable-thumb " # --enable-thumb 
FLAGS="$FLAGS --enable-asm --disable-debug --enable-optimizations --enable-pthreads"
FLAGS="$FLAGS --disable-ffserver --disable-ffmpeg --disable-ffprobe --disable-encoders --disable-muxers --disable-avdevice --disable-hwaccels "
FLAGS="$FLAGS --enable-swscale --enable-encoder=flv,mpeg4,aac"

EXTRA_CFLAGS="-DANDROID -O3 -fasm -ffast-math -mthumb" # -marm -mthumb -fpic
EXTRA_CFLAGS="$EXTRA_CFLAGS -ftree-vectorize -mvectorize-with-neon-quad"
EXTRA_CFLAGS="$EXTRA_CFLAGS -mcpu=cortex-a8 -mfloat-abi=softfp -mfpu=neon --sysroot=$SYSROOT"

EXTRA_LDFLAGS="-Wl,--fix-cortex-a8 --sysroot=$SYSROOT"

EXTRA_CXXFLAGS="-Wno-multichar -fno-exceptions -fno-rtti"

echo "FLAGS: $FLAGS --extra-cflags=$EXTRA_CFLAGS --extra-ldflags=$EXTRA_LDFLAGS --extra-cxxflags=$EXTRA_CXXFLAGS "

../../ffmpeg/configure $FLAGS --extra-cflags="$EXTRA_CFLAGS" --extra-ldflags="$EXTRA_LDFLAGS" --extra-cxxflags="$EXTRA_CXXFLAGS" 

make clean
make -j4

cd ..
# arm-linux-androideabi-gcc -shared -I../ffmpeg -I$FFMPEG_DIR -o FFmpeg.so FFmpeg.c $FFMPEG_DIR/libswscale/libswscale.a $FFMPEG_DIR/libavformat/libavformat.a $FFMPEG_DIR/libavcodec/libavcodec.a $FFMPEG_DIR/libavutil/libavutil.a -lz --sysroot=$SYSROOT
# arm-linux-androideabi-strip FFmpeg.so

rm -rf $LIB_DIR
mkdir $LIB_DIR
cp $FFMPEG_DIR/libavcodec/*.a $LIB_DIR
cp $FFMPEG_DIR/libavformat/*.a $LIB_DIR
cp $FFMPEG_DIR/libavutil/*.a $LIB_DIR
cp $FFMPEG_DIR/libswscale/*.a $LIB_DIR
cp $FFMPEG_DIR/libswresample/*.a $LIB_DIR

echo "Installed: $FFMPEG_DIR/dist"
