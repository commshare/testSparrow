#include<stdio.h>
#include<stdlib.h>
#include <string.h>


#include"test_cirbuf.h"
#include"sc_log.h"


#define MIN(X,Y) ((X)>(Y)?(Y):(X))

/*ϣ���û����ݵ���һ����ַ*/
int cirbuf_init(cirbuf_t **buf,int max){
     /*�ı��ַָ������ݵ�ֵ*/
     *buf=(cirbuf_t *)malloc(sizeof(cirbuf_t));
	 if(*buf==NULL)
	 {
	 	LOGD("malloc fail");
		return -1;
	 }
	 memset(*buf,0,sizeof(cirbuf_t));
	 (*buf)->size=max;
     (*buf)->data=(uint8_t*)malloc(max*sizeof(uint8_t));
	 memset((*buf)->data,0,max*sizeof(uint8_t));
	 (*buf)->wptr=(*buf)->rptr=(*buf)->data;
	 return 0;

}
/*����ǻ������ݣ������������ݵ�ֵ�������ָ�룬��������ʵ�ε�ֵ��ֵ��һ����ַ����*/
void cirbuf_unint(cirbuf_t *buf){
    free(buf->data);
	buf->data=buf->rptr=buf->wptr=NULL;
    /*�ͷ������ַָ����ڴ�*/
    free(buf);
	/*��ı�����βΰɣ�û��������*/
   //    buf=NULL;
}

int cirbuf_empty(cirbuf_t *buf)
{
	if((buf->data==buf->rptr && buf->data==buf->wptr))
	{
		LOGD("cirbuf is empty 1");
		return 1;
	}
	if(buf->rptr+1 == buf->wptr)
	{
		LOGD("cirbuf is empty 2");
		return 1;
	}
	return 0;
}
int cirbuf_full(cirbuf_t *buf){
	if(buf->wptr + 1 == buf->rptr){
		LOGD("cirbuf is full");
		return 1;
	}
	return 0;

}
int cirbuf_put (cirbuf_t *buf,void *input,int put_size){
	LOGD("cirbuf_put [%d]",put_size);
	if(cirbuf_full(buf)){
		LOGD("full cirbuf ,put fail");
		return -1;
	}
	int rest=-1;
	int capability=-1;
	if(buf->data==buf->wptr && buf->data==buf->rptr){
		LOGD("buf->data == W == R ");
		rest=buf->size;
		capability=MIN(rest,put_size);
        memcpy(buf->wptr,(uint8_t*)input,put_size);
        buf->wptr=buf->data+capability;
		LOGD("after move capacity[%d] step: *(buf->wptr)[%c] *(buf->wptr-1)[%c]",capability,*(buf->wptr),*(buf->wptr-1));
        if(buf->wptr== (buf->data+buf->size)){/*��ʾ�Ѿ�д��*/
			LOGD("W at the end,from the begin 1");
			buf->wptr=buf->data;/*��0��ʼ����*/
		}
		return capability;
	}

	if(buf->wptr < buf->rptr){
		LOGD("W<R");
		//int III=buf->rptr-buf->wptr;
		/*rΪdata+7,wΪdata+4,��ʾ4 5  6 ��д�룬��7�ɶ�ȡ*/
		rest=buf->rptr-buf->wptr;//-1;
		capability=MIN(rest,put_size);
		memcpy(buf->wptr,(uint8_t*)input,capability);
		buf->wptr+=capability;
		return capability;
	}

    if(buf->wptr > buf->rptr){
		LOGD("W>R");
		/*����ֵʱ��ָ�븳��������δ������ת�� [Ĭ������]*/
		/*
		��һ��ָ��ת��Ϊ��С��ͬ������ [-Wpointer-to-int-cast]
		*/
        #if 0 /*�����ҵ�bug*/
		rest=(int)(buf->data+buf->size-1)-(buf->wptr-buf->rptr);
		#else
        rest=(buf->size)-(buf->wptr-buf->rptr);
		#endif
		LOGD("*(buf->wptr-1)[%c]",*(buf->wptr-1));
		int j=0;
		#if 0
		for(j=0;j<buf->size;j++){
			printf("####  *buf->data[%d][%c]\n",j,*(buf->data[j]));
		}
		#else
		  LOGD("-1-$$$$buf->data[%s]",buf->data);
		#endif
		capability=MIN(rest,put_size);
		int I=(buf->data+buf->size) - buf->wptr;
		int II=0;
		LOGD("rest[%d] capablity[%d] I[%d]",rest,capability,I);
		int check=capability-I;
		/*��Ϊ����Ҫ������wptr�������жϺͲ��������Ⱥ�˳��ģ��Ȳ���I���ƶ�wptr��
		��ʱ��I�����ã�ʹ���ƶ����wptr����II���ٴ��ƶ�wptr��׼���´β���*/
		if(check==0 || check<0)/*I����*/
		{
			LOGD("put capacity[%d] to I [%d] *(buf->wptr)[%c] ",capability,I,*(buf->wptr) );
			memcpy(buf->wptr,input,capability);
			buf->wptr+=capability;
			LOGD("buf->wptr[%c] buf->wptr+1 [%c]",*buf->wptr,*(buf->wptr+1));
			if(buf->wptr== (buf->data+buf->size)){/*��ʾ�Ѿ�д��*/
				LOGD("W at the end,from the begin 2");
			    buf->wptr=buf->data;/*��0��ʼ����*/
		    }
		}else
		/*I������*/
		{
			LOGD("put II [%d]",II);
			II=capability-I;
			memcpy(buf->data,input-I,II);
			buf->wptr=buf->data+II;
		}
		return capability;//(I+II);
	}
}

void main(){
	int i;
	cirbuf_t *buf;
    cirbuf_init(&buf,10);
	uint8_t *array="1234567890";
	#if 0
	/*д��1��0����10��*/
	cirbuf_put(buf,array,10);
	/*uint8_t ��Ȼֻռ��һ���ֽڣ���charһ��*/
	LOGD("sizeof(int)[%d] sizeof(uint8_t)[%d] sizeof(char)[%d]",sizeof(int),sizeof(uint8_t),sizeof(char));
	LOGD("----1---AFTER PUT");

	for(i=0;i<10;i++)
		printf("buf->data[%c] \n",*(buf->data+i));
	#endif
	uint8_t *array2="abcdefghik";
	/*��ʱ��Ӧ����array2ȫ�������˵�ǰ���壬�����a��k*/
	cirbuf_put(buf,array2,10);
	LOGD("----2---AFTER PUT");
	for(i=0;i<10;i++)
		printf("buf->data[%c] \n",*(buf->data+i));

	/*д��4�����֣�ǰ���ĸ���1234��������efghik*/
	cirbuf_put(buf,array,4);
	LOGD("----3---AFTER PUT");
	for(i=0;i<10;i++)
		printf("buf->data[%c] \n",*(buf->data+i));


	/*��д��4����ĸ��Ӧ����1234abcdik*/
	cirbuf_put(buf,array2,4);
	LOGD("----4---AFTER PUT");
	for(i=0;i<10;i++)
		printf("buf->data[%c] \n",*(buf->data+i));
#if 0
    /*�ռ䲻�㣬д����������12��1234abcd12*/
	cirbuf_put(buf,array,7);
	LOGD("----5---AFTER PUT");
	for(i=0;i<10;i++)
		printf("buf->data[%c] \n",*(buf->data+i));

    /*abcdefgd12*/
	cirbuf_put(buf,array2,7);
	LOGD("----6---AFTER PUT");
	for(i=0;i<10;i++)
		printf("buf->data[%c] \n",*(buf->data+i));
#endif
	cirbuf_unint(buf);

}
