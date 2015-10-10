#ifndef __SC_QUEUE_H_
#define __SC_QUEUE_H_



#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>

typedef struct PacketQueue
{
    AVPacketList * first_pkt, *last_pkt;
    int nb_packets;
    int size;
	int quit;//added by me 20141105
}PacketQueue;

void packet_queue_init(PacketQueue *q);
 int packet_queue_get(PacketQueue *q, AVPacket *pkt, int block);
int packet_queue_put(PacketQueue *q, AVPacket *pkt);
#endif
