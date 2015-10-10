#ifndef __SC_AV_H_
#define __SC_AV_H_
// raw data
typedef struct sc_pkt
{
    uint8_t *data;
    int size;
    int64_t pts;
    int64_t dts;
    int duration;
    int key_frame; // flag from ffmpeg
//    dt_media_type_t type;
} sc_pkt_t;

typedef struct
{
    int num;
    int den;
} dtratio;
#endif
