/*************************************************************************
	> File Name: fixff.h
	> Author: 
	> Mail: 
	> Created Time: 2015年11月20日 星期五 19时15分37秒
 ************************************************************************/
 //refer to http://stackoverflow.com/questions/9475987/error-codec-type-audio-undeclared-when-make-m3u8-segmenter


#ifndef _FIXFF_H
#define _FIXFF_H

/* Support older versions of ffmpeg and libav */
//#if LIBAVCODEC_VERSION_INT < AV_VERSION_INT(52, 64, 0)
  #define  CODEC_TYPE_AUDIO AVMEDIA_TYPE_AUDIO
  #define  CODEC_TYPE_VIDEO AVMEDIA_TYPE_VIDEO
   #define  PKT_FLAG_KEY AV_PKT_FLAG_KEY
//      #endif

#endif
