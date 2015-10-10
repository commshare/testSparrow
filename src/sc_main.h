#ifndef _SC_MAIN_H_
#define _SC_MAIN_H_

#include"common_internal.h" //for ffmpeg
#include"sc_log.h"

#include"sc_queue.h"
//#include"sc_demuxer.h"
//#include"sc_audio_out.h"
//#include"sc_audio_decoder.h"

//用于保存ffmpeg变的信息
typedef struct ffmpeg_info{
	 AVFormatContext	 *fmt_ctx;
	 AVCodecContext		 *dec_ctx;
	 int 	audio_stream_index ;
	 int last_audio_stream;
	 AVCodec *dec;
	 PacketQueue *pPktQueue;
    // sc_buf_t *adec_out_buf; //PASS FROM DECODER TO AOUT

	 //we use these
	// audio_demuxer_t
	void *demxuer;
	// audio_decoder_t
	void *decoder;
	// audio_out_t
	void *aout;

	//like ffplay 20141211
	int  abort_request;
}ffmpeg_info_t;
#endif
