#!/bin/bash

TOOLCHAIN=/home/zhangbin/android/android-toolchain/r10e-4.8/android-19
export PATH=$TOOLCHAIN/bin:$PATH


DEST=`pwd`/build/ffmpeg/one && rm -rf $DEST
FF=ffmpeg-2.8.5
SOURCE=`pwd`/$FF

if [ -d $FF ]; then
  echo "$FF exits!"
  cd $FF
else
  #git clone git://source.ffmpeg.org/ffmpeg.git ffmpeg
  cd $FF
fi

#git reset --hard
#git clean -f -d
#git checkout `cat ../ffmpeg-version`
#[ $PIPESTATUS == 0 ] || exit 1

#git log --pretty=format:%H -1 > ../ffmpeg-version

#TOOLCHAIN=/tmp/vplayer
SYSROOT=$TOOLCHAIN/sysroot
#$ANDROID_NDK/build/tools/make-standalone-toolchain.sh --platform=android-14 --install-dir=$TOOLCHAIN

#export PATH=$TOOLCHAIN/bin:$PATH

#https://github.com/appunite/AndroidFFmpeg/issues/22
###ccache
export CC="arm-linux-androideabi-gcc"
export LD=arm-linux-androideabi-ld
export AR=arm-linux-androideabi-ar
export STRIP=arm-linux-androideabi-strip

###https://github.com/shoutv/FFmpeg/blob/master/android.sh
###remove -mfpu=neon ,because of https://github.com/Bilibili/ijkplayer/issues/204
CFLAGS="-std=c99 -O3 -Wall -marm -pipe -fpic -fasm \
  -march=armv7-a  -mfloat-abi=hard -mvectorize-with-neon-quad \
  -mhard-float -D_NDK_MATH_NO_SOFTFP=1 -fdiagnostics-color=always \
  -finline-limit=300 -ffast-math \
  -fstrict-aliasing -Werror=strict-aliasing \
  -fmodulo-sched -fmodulo-sched-allow-regmoves \
  -Wno-psabi -Wa,--noexecstack \
  -DANDROID -DNDEBUG "
  
  ###remove this for support arch=arm(armv7-a) used in libavcodec/aacplus_float.o
  # -D__ARM_ARCH_5__ -D__ARM_ARCH_5E__ -D__ARM_ARCH_5T__ -D__ARM_ARCH_5TE__ 

  
  ###--enable-shared \
  #  --enable-static --disable-shared \
  ###from build-arm-neon.sh --enable-thumb 不支持啊  from https://trac.ffmpeg.org/attachment/ticket/1309/build-arm-neon.sh

FFMPEG_FLAGS="--target-os=linux \
  --arch=arm \
  --enable-cross-compile \
  --enable-static --disable-shared \
  --cross-prefix=arm-linux-androideabi- \
  --disable-symver \
  --disable-doc \
  --disable-ffplay \
  --disable-ffmpeg \
  --disable-ffprobe \
  --disable-ffserver \
  --disable-avdevice \
  --disable-encoders \
  --disable-muxers \
  --disable-devices \
  --enable-protocols  \
  --enable-parsers \
  --enable-demuxers \
  --enable-decoders \
  --enable-bsfs \
  --enable-network \
  --enable-swscale  \
  --disable-demuxer=sbg \
  --enable-asm \
  --enable-neon  \
  --enable-version3"

LDFLAGS="-lm_hard -lz -Wl,--no-undefined -Wl,-z,noexecstack \
  -Wl,--no-warn-mismatch -Wl,--fix-cortex-a8"
###
####from https://gist.github.com/ayanamist/6282b01fbdd9c105791f
#NDKROOT=~/android-ndk   # insert android ndk path
DEVICE_TYPE="arm_neon"    # insert device type(arm_neon, arm_tegra2, x86)
#OS=LINUX64                        # insert OS Version(LINUX64, LINUX32, WINDOWS_64, WINDOWS)

#####
if [ $DEVICE_TYPE == "arm_neon" ];
then
    TOOLCHAIN=toolchain-arm
    ARCH=arm
    CPU=armv7-a
    TARGET_CFLAGS="-marm -march=armv7-a -mfloat-abi=softfp -mfpu=neon -mtune=cortex-a8 -mvectorize-with-neon-quad"
    TARGET_LDFLAGS="-Wl,--fix-cortex-a8"
    EABI="armeabi-v7a"
fi

####
PLATFORM=$ANDROID_TOOLCHAIN/sysroot

###from  not work ！
OBJ_FILES="$CC -shared -Wl,-soname,libffm.so -Wl,-Bsymbolic -Llibavcodec  -Llibavformat -Llibavutil  -Llibswscale -Llibswresample \
 $TARGET_LDFLAGS  -L$PLATFORM/usr/lib  -lm -lz  \
  --sysroot=$PLATFORM -Wl,--as-needed -Wl,--warn-common \
  -Wl,-rpath-link=libswresample:libswscale:libavformat:libavcodec:libavutil -o libffm.so "


#for version in neon armv7 vfp armv6; do
for version in neon; do

  cd $SOURCE

  case $version in
    neon)
      EXTRA_CFLAGS="-march=armv7-a -mfpu=neon -mfloat-abi=softfp -mvectorize-with-neon-quad"
      EXTRA_LDFLAGS="-Wl,--fix-cortex-a8"
      ;;
    armv7)
      EXTRA_CFLAGS="-march=armv7-a -mfpu=vfpv3-d16 -mfloat-abi=softfp"
      EXTRA_LDFLAGS="-Wl,--fix-cortex-a8"
      ;;
    vfp)
      EXTRA_CFLAGS="-march=armv6 -mfpu=vfp -mfloat-abi=softfp"
      EXTRA_LDFLAGS=""
      ;;
    armv6)
      EXTRA_CFLAGS="-march=armv6"
      EXTRA_LDFLAGS=""
      ;;
    *)
      EXTRA_CFLAGS=""
      EXTRA_LDFLAGS=""
      ;;
  esac

  #PREFIX="$DEST/one/$version" && mkdir -p $PREFIX
  PREFIX=$DEST/neon && mkdir -p $PREFIX
  FFMPEG_FLAGS="$FFMPEG_FLAGS --prefix=$PREFIX"

###gcc configration for cross-compile ADD BY ME
GCC_FLAG="-DHAVE_ISNAN -DHAVE_ISIN"

  ./configure $FFMPEG_FLAGS --extra-cflags="$CFLAGS" --extra-ldflags="$LDFLAGS $GCC_FLAG" | tee $PREFIX/configuration.txt
  cp config.* $PREFIX
  [ $PIPESTATUS == 0 ] || exit 1

  make clean
  find . -name "*.o" -type f -delete
  make -j4 || exit 1
  make install || exit 1

  #$AR rcs $PREFIX/libffmpeg.a libavutil/*.o libavcodec/*.o libavformat/*.o libswresample/*.o libswscale/*.o  
  #arm-linux-androideabi-strip --strip-unneeded $PREFIX/libffmpeg.a
  
  ##not work
  #$OBJ_FILES
  
mkdir -p $PREFIX/m_hard && cd $PREFIX/m_hard
$AR x $SYSROOT/usr/lib/libm_hard.a && cd -
$CC -o $PREFIX/libffmpeg.so -shared $CFLAGS $LDFLAGS $PREFIX/m_hard/*.o \
  libavutil/*.o libavutil/arm/*.o libavcodec/*.o libavcodec/arm/*.o libavformat/*.o \
  libavfilter/*.o libswscale/*.o  libswresample/*.o libswresample/arm/*.o compat/*.o
###libswscale/arm/*.o

  
  cp $PREFIX/libffmpeg.so $PREFIX/libffmpeg-debug.so
  $STRIP  --strip-unneeded $PREFIX/libffmpeg.so
done
