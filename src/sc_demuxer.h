#ifndef  __SC_DEMUXER_H_
#define __SC_DEMUXER_H_

#include"sc_thread_common.h"
#include"sc_av.h"
#include"sc_ffmpeg_common.h"
#include"sc_queue.h"

#include"sc_main.h"

typedef enum
{
    ADEMUX_STATUS_IDLE,
    ADEMUX_STATUS_RUNNING,
    ADEMUX_STATUS_PAUSED,
    ADEMUX_STATUS_EXIT
} audio_demux_status_t;


typedef struct audio_stream_info{
	int index;
    int id;
    int bit_rate;
    int sample_rate;
    int channels;
    int bps;
    int64_t duration;
    dtratio time_base;
    int extradata_size;
    uint8_t *extradata;
  //  dtaudio_format_t format;
    void *codec_priv;
}ademx_streaminfo_t;

typedef struct audio_demuxer
{
	ademx_streaminfo_t *astream_info;
   // dtaudio_para_t aparam;
   // ad_wrapper_t *wrapper;
   AVFormatContext *fmt_ctx;
    pthread_t audio_demuxer_pid;
   	char *filename;
	PacketQueue *ademx_queue;

	int ff_audio_stream_index;
	int status; //audio_demux_status_t
    int eof_flag; ////int exit_demuxer;
   #if 0
    adec_status_t status;
    int decode_err_cnt;
    int decode_offset;

	////pts
    int64_t pts_current;
    int64_t pts_first;
    int64_t pts_last_valid;
    int pts_buffer_size;
    int pts_cache_size;

     adec_ctrl_t info;
    //dt_buffer_t *buf_out;
	#endif
    void *parent;
    void *priv;         //point to avcodeccontext

	#if 0
	//存储格式、codec等信息。
	audio_param_t param;
	//struct AudioParams audio_tgt;
    struct SwrContext *swr_ctx;
    #endif
}audio_demuxer_t;
int createDemuxer( char *filename,ffmpeg_info_t *ff);

#endif


