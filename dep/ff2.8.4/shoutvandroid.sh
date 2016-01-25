#!/bin/bash

DEST=`pwd`/build/android && rm -rf $DEST
SOURCE=`pwd`

TOOLCHAIN=/tmp/Shou
SYSROOT=$TOOLCHAIN/sysroot
$ANDROID_NDK/build/tools/make-standalone-toolchain.sh --toolchain=arm-linux-androideabi-4.8 \
  --system=linux-x86_64 --platform=android-19 --install-dir=$TOOLCHAIN
SSL=$SOURCE/../openssl
SSL_BUILD=$SSL/obj/local/armeabi-v7a-hard
FDK_AAC=$SOURCE/../fdk-aac/build
RTMPDUMP=$SOURCE/../rtmpdump

export PATH=$TOOLCHAIN/bin:$PATH
export CC="ccache arm-linux-androideabi-gcc"
export AR=arm-linux-androideabi-ar
export LD=arm-linux-androideabi-ld
export STRIP=arm-linux-androideabi-strip

CFLAGS="-std=c99 -O3 -Wall -marm -pipe -fpic -fasm \
  -march=armv7-a -mfpu=neon -mfloat-abi=hard -mvectorize-with-neon-quad \
  -mhard-float -D_NDK_MATH_NO_SOFTFP=1 -fdiagnostics-color=always \
  -finline-limit=300 -ffast-math \
  -fstrict-aliasing -Werror=strict-aliasing \
  -fmodulo-sched -fmodulo-sched-allow-regmoves \
  -Wno-psabi -Wa,--noexecstack \
  -D__ARM_ARCH_5__ -D__ARM_ARCH_5E__ -D__ARM_ARCH_5T__ -D__ARM_ARCH_5TE__ \
  -DANDROID -DNDEBUG -I$SSL/include -I$FDK_AAC/include -I$RTMPDUMP"

LDFLAGS="-lm_hard -lz -Wl,--no-undefined -Wl,-z,noexecstack \
  -Wl,--no-warn-mismatch -Wl,--fix-cortex-a8"

FFMPEG_FLAGS="--target-os=linux \
  --arch=arm \
  --cross-prefix=arm-linux-androideabi- \
  --enable-cross-compile \
  --enable-static \
  --disable-shared \
  --disable-symver \
  --disable-doc \
  --disable-ffplay \
  --disable-ffmpeg \
  --disable-ffprobe \
  --disable-ffserver \
  --disable-avdevice \
  --disable-postproc \
  --disable-encoders \
  --disable-decoders \
  --disable-muxers \
  --disable-demuxers \
  --disable-filters \
  --disable-devices \
  --disable-everything \
  --disable-runtime-cpudetect \
  --enable-protocol=file \
  --enable-protocol=rtp \
  --enable-protocol=srtp \
  --enable-librtmp \
  --enable-filter=atempo \
  --enable-filter=volume \
  --enable-filter=aresample \
  --enable-filter=aconvert \
  --enable-filter=aformat \
  --enable-protocol=librtmp \
  --enable-protocol=librtmpe \
  --enable-protocol=librtmps \
  --enable-protocol=librtmpt \
  --enable-protocol=librtmpte \
  --enable-muxer=rtp \
  --enable-muxer=rtsp \
  --enable-muxer=mpegts \
  --enable-muxer=hls \
  --enable-muxer=matroska \
  --enable-muxer=mp4 \
  --enable-muxer=mov \
  --enable-muxer=tgp \
  --enable-muxer=flv \
  --enable-muxer=image2 \
  --enable-muxer=tee \
  --enable-encoder=mpeg4 \
  --enable-encoder=h263p \
  --enable-encoder=alac \
  --enable-encoder=pcm_s16le \
  --enable-libfdk-aac \
  --enable-encoder=libfdk_aac \
  --enable-asm \
  --enable-version3"


PREFIX="$DEST/neon" && mkdir -p $PREFIX
FFMPEG_FLAGS="$FFMPEG_FLAGS --prefix=$PREFIX"
FFMPEG_LIB_FLAGS="-L$SSL_BUILD -L$FDK_AAC/lib -L$RTMPDUMP/librtmp -lrtmp"
SSL_OBJS=`find $SSL_BUILD/objs/ssl $SSL_BUILD/objs/crypto -type f -name "*.o"`

./configure $FFMPEG_FLAGS --extra-cflags="$CFLAGS" --extra-ldflags="$LDFLAGS $FFMPEG_LIB_FLAGS" | tee $PREFIX/configuration.txt
cp config.* $PREFIX
[ $PIPESTATUS == 0 ] || exit 1

make clean
find . -name "*.o" -type f -delete
make -j7 || exit 1

mkdir -p $PREFIX/m_hard && cd $PREFIX/m_hard
$AR x $SYSROOT/usr/lib/libm_hard.a && cd -
$CC -o $PREFIX/libffmpeg.so -shared $CFLAGS $LDFLAGS $PREFIX/m_hard/*.o \
  libavutil/*.o libavutil/arm/*.o libavcodec/*.o libavcodec/arm/*.o libavformat/*.o \
  libavfilter/*.o libswscale/*.o libswscale/arm/*.o libswresample/*.o libswresample/arm/*.o compat/*.o \
  $SSL_OBJS $FDK_AAC/lib/libfdk-aac.a $RTMPDUMP/librtmp/librtmp.a

cp $PREFIX/libffmpeg.so $PREFIX/libffmpeg-debug.so
$STRIP --strip-unneeded $PREFIX/libffmpeg.so
