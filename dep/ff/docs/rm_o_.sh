#!/bin/sh
####build for libffmpeg.so with all .a 
OUT=/home/zhangbin/nuf/sc/sparrow/dep/ff/build/ffmpeg/armv7/lib

arm-linux-androideabi-ar d $OUT/libavcodec.a log2_tab.o
arm-linux-androideabi-ar d $OUT/libavcodec.a golomb.o
arm-linux-androideabi-ar d $OUT/libavformat.a golomb.o
arm-linux-androideabi-ar d $OUT/libavformat.a log2_tab.o
arm-linux-androideabi-ar d $OUT/libavcodec.a reverse.o
arm-linux-androideabi-ar d $OUT/libavutil.a log2_tab.o
arm-linux-androideabi-ar d $OUT/libswscale.a log2_tab.o
arm-linux-androideabi-ar d $OUT/libswresample.a log2_tab.o


