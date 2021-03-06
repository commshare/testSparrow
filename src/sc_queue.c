#include"sc_queue.h"

#include"sc_log.h"

void packet_queue_init(PacketQueue *q)
{
    memset(q, 0, sizeof(PacketQueue));
}

 int packet_queue_get(PacketQueue *q, AVPacket *pkt, int block)
{
    AVPacketList *pkt1;
    int ret=-1; //added by me 默认设置为-1吧。
        if(q->quit)
        {
			slogd("!!!ask for quit");
            ret = -1;
          return -1;
        }
		if(q->nb_packets==0){
			slogd("!!! no packet in queue ");
			return -1;
		}

        pkt1 = q->first_pkt;
        if (pkt1)
        {
            q->first_pkt = pkt1->next;
            if (!q->first_pkt)
                q->last_pkt = NULL;
            q->nb_packets--;
            q->size -= pkt1->pkt.size;
            *pkt = pkt1->pkt;
			//找不到了
           // av_free(pkt1);
           //用这个吧
           free(pkt1);
            ret = 1;//拿到了就返回1
            return ret;
        } else if (!block) {
            ret = 0; //阻塞就是0
            return ret;
        }
    return ret; //错误就是-1
}

int packet_queue_put(PacketQueue *q, AVPacket *pkt)
{

    AVPacketList *pkt1;
	//复制一份的�
    if(av_dup_packet(pkt) < 0)
        return -1;
	//新malloc了一个内存，存储pkt的数据。实际上packetqueue里头的数据类型是AVPacketList
    pkt1 = av_malloc(sizeof(AVPacketList));
    if (!pkt1)
        return -1;
    pkt1->pkt = *pkt;
    pkt1->next = NULL;



    if (!q->last_pkt)
        q->first_pkt = pkt1;
    else
        q->last_pkt->next = pkt1;
    q->last_pkt = pkt1;
    q->nb_packets++;
    q->size += pkt1->pkt.size;

    return 0;
}