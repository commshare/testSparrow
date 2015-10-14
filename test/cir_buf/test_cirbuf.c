#include<stdio.h>
#include<stdlib.h>
#include <string.h>


#include"test_cirbuf.h"
#include"sc_log.h"


#define MIN(X,Y) ((X)>(Y)?(Y):(X))

/*Ï£ÍûÓÃ»§´«µİµÄÊÇÒ»¸öµØÖ·*/
int cirbuf_init(cirbuf_t **buf,int max){
     /*¸Ä±äµØÖ·Ö¸ÏòµÄÄÚÈİµÄÖµ*/
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
/*Èç¹ûÊÇ»ù±¾Êı¾İ£¬¿½±´µÄÊÇÊı¾İµÄÖµ£¬Èç¹ûÊÇÖ¸Õë£¬¿½±´µÄÊÇÊµ²ÎµÄÖµ£¨ÖµÊÇÒ»¸öµØÖ·£©¡£*/
void cirbuf_unint(cirbuf_t *buf){
    free(buf->data);
	buf->data=buf->rptr=buf->wptr=NULL;
    /*ÊÍ·ÅÕâ¸öµØÖ·Ö¸ÏòµÄÄÚ´æ*/
    free(buf);
	/*Õâ¸Ä±äµÄÊÇĞÎ²Î°É£¬Ã»ÆäËû×÷ÓÃ*/
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
/*ÊÇ²»ÊÇÑ­»·»º³å²»´æÔÚÂúµÄÇé¿ö£¬¿ÉÒÔÖ±½Ó¸²¸ÇµÄå*/
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
        if(buf->wptr== (buf->data+buf->size)){/*±íÊ¾ÒÑ¾­Ğ´Âú*/
			LOGD("W at the end,from the begin 1");
			buf->wptr=buf->data;/*´Ó0¿ªÊ¼À´ÁË*/
		}
		return capability;
	}

	if(buf->wptr < buf->rptr){
		LOGD("W<R");
		//int III=buf->rptr-buf->wptr;
		/*rÎªdata+7,wÎªdata+4,±íÊ¾4 5  6 ¿ÉĞ´Èë£¬´Ó7¿É¶ÁÈ¡*/
		rest=buf->rptr-buf->wptr;//-1;
		capability=MIN(rest,put_size);
		memcpy(buf->wptr,(uint8_t*)input,capability);
		buf->wptr+=capability;
		return capability;
	}

    if(buf->wptr > buf->rptr){
		LOGD("W>R");
		/*£º¸³ÖµÊ±½«Ö¸Õë¸³¸øÕûÊı£¬Î´×÷ÀàĞÍ×ª»» [Ä¬ÈÏÆôÓÃ]*/
		/*
		½«Ò»¸öÖ¸Õë×ª»»Îª´óĞ¡²»Í¬µÄÕûÊı [-Wpointer-to-int-cast]
		*/
        #if 0 /*ÕâÊÇÎÒµÄbug*/
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
		/*ÒòÎª²åÈëÒªÒÀÀµÓÚwptr£¬ËùÒÔÅĞ¶ÏºÍ²åÈëÊÇÓĞÏÈºóË³ĞòµÄ£¬ÏÈ²åÈëI£¬ÒÆ¶¯wptr£¬
		´ËÊ±£¬I²»¹»ÓÃ£¬Ê¹ÓÃÒÆ¶¯ºóµÄwptr²åÈëII£¬ÔÙ´ÎÒÆ¶¯wptr£¬×¼±¸ÏÂ´Î²åÈë*/
		LOGD("put capacity[%d] to I [%d] *(buf->wptr)[%c] ",capability,I,*(buf->wptr) );
		memcpy(buf->wptr,input,capability);
		buf->wptr+=capability;
		LOGD("buf->wptr[%c] buf->wptr+1 [%c]",*buf->wptr,*(buf->wptr+1));
		//if(buf->wptr== (buf->data+buf->size)){/*±íÊ¾ÒÑ¾­Ğ´Âú*/
		//	LOGD("W at the end,from the begin 2");
		//   buf->wptr=buf->data;/*´Ó0¿ªÊ¼À´ÁË*/
		//}
		if(check==0 || check<0)/*I¹»ÓÃ*/
		{
           LOGD("only I is enough");
		   if(check==0)
		   	 buf->wptr=buf->data;
		   goto OK;

		}else
		/*I²»¹»ÓÃ*/
		{
			LOGD("put II [%d]",II);
			II=capability-I;
			memcpy(buf->data,input-I,II);
			buf->wptr=buf->data+II;
		}
OK:
		return capability;//(I+II);
	}
}

int cirbuf_get(cirbuf_t *buf,void *output,int get_size){
  if(cirbuf_empty(buf)==1)
  {
  	  LOGD("cirbuf is empty,could not get");
	  return -1;
  }
  int availability=-1;
  int stored=-1;
  if(buf->wptr > buf->rptr){
	stored=buf->wptr-buf->rptr;
	availability=MIN(stored,get_size);
	memcpy(output,buf->rptr,availability);
	buf->rptr+=availability;
	if(buf->rptr == buf->data+buf->size){
		LOGD("read to end ,from the begin");
		buf->rptr=buf->data;
	}
	return availability;
  }
  if(buf->wptr < buf->rptr){
  	stored=(buf->data+buf->size-buf->rptr) + (buf->wptr-buf->data);
	availability=MIN(stored,get_size);
    int IV=buf->data+buf->size - buf->rptr;
	int check=IV-availability;
	if(check>0 || check==0){
       memcpy(output,buf->rptr,availability);
	   buf->rptr+=availability;
	   if(check=0){
	   		LOGD("rtpr move to beginning");
			buf->rptr=buf->data;
	   }
	}else /*IV²»¹»*/
	{
		/*ÒÑ¾­Ğ´ÈëÁËIV´óĞ¡*/
		//memcpy(output+IV,buf->);
	}

  }
}

void main(){
	int i;
	cirbuf_t *buf;
    cirbuf_init(&buf,10);
	uint8_t *array="1234567890";
	#if 1
	/*Ğ´Èë1µ½0£¬¹²10¸ö*/
	cirbuf_put(buf,array,10);
	/*uint8_t ¾ÓÈ»Ö»Õ¼ÁËÒ»¸ö×Ö½Ú£¬ºÍcharÒ»Ñù*/
	LOGD("sizeof(int)[%d] sizeof(uint8_t)[%d] sizeof(char)[%d]",sizeof(int),sizeof(uint8_t),sizeof(char));
	LOGD("----1---AFTER PUT");

	for(i=0;i<10;i++)
		printf("buf->data[%c] \n",*(buf->data+i));
	#endif
	uint8_t *array2="abcdefghik";
	/*´ËÊ±£¬Ó¦¸ÃÓÃarray2È«²¿¸²¸ÇÁËµ±Ç°»º³å£¬¼´Êä³öaµ½k*/
	cirbuf_put(buf,array2,10);
	LOGD("----2---AFTER PUT");
	for(i=0;i<10;i++)
		printf("buf->data[%c] \n",*(buf->data+i));

	/*Ğ´Èë4¸öÊı×Ö£¬Ç°ÃæËÄ¸öÊÇ1234£¬ºóÃæÊÇefghik*/
	cirbuf_put(buf,array,4);
	LOGD("----3---AFTER PUT");
	for(i=0;i<10;i++)
		printf("buf->data[%c] \n",*(buf->data+i));


	/*ÔÙĞ´Èë4¸ö×ÖÄ¸£¬Ó¦¸ÃÊÇ1234abcdik*/
	cirbuf_put(buf,array2,4);
	LOGD("----4---AFTER PUT");
	for(i=0;i<10;i++)
		printf("buf->data[%c] \n",*(buf->data+i));
#if 1
    /*¿Õ¼ä²»×ã£¬Ğ´ÈëÁ½¸öÊı×Ö12£¬1234abcd12*/
	cirbuf_put(buf,array,7);
	LOGD("----5---AFTER PUT");
	for(i=0;i<10;i++)
		printf("buf->data[%c] \n",*(buf->data+i));

    /*abcdefgd12*/
	cirbuf_put(buf,array2,7);
	LOGD("----6---AFTER PUT");
	for(i=0;i<10;i++)
		printf("buf->data[%c] \n",*(buf->data+i));

	/*abcdefg123*/
	cirbuf_put(buf,"#",1);
	LOGD("----7---AFTER PUT");
	printf("buf->data[%s] \n",(buf->data));
#endif

	cirbuf_unint(buf);

}
