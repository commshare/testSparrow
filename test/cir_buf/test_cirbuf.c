#include<stdio.h>
#include<stdlib.h>
#include <string.h>


#include"test_cirbuf.h"
#include"sc_log.h"


#define MIN(X,Y) ((X)>(Y)?(Y):(X))


/*
ÔÚputÖĞ£¬I II III ÊÇ¿ÉÒÔputµÄÎ»ÖÃ¡£

data      r      w    data+size
-----------------------
     II             I

data      w      r     data+size
-----------------------
     V       III    IV
*/


/*Ï£ÍûÓÃ»§´«µİµÄÊÇÒ»¸öµØÖ·*/
int cirbuf_init(cirbuf_t **buf,int max){
     /*¸Ä±äµØÖ·Ö¸ÏòµÄÄÚÈİµÄÖµ*/
     *buf=(cirbuf_t *)malloc(sizeof(cirbuf_t));
	 if(*buf==NULL)
	 {
	 	LOGD("malloc fail");
		return -1;
	 }
	 (*buf)->level=0;
	 memset(*buf,0,sizeof(cirbuf_t));
	 (*buf)->size=max;
     (*buf)->data=(uint8_t*)malloc(max*sizeof(uint8_t));
	 memset((*buf)->data,0,max*sizeof(uint8_t));
	 (*buf)->wptr=(*buf)->rptr=(*buf)->data;
	 return 0;

}
/*Èç¹ûÊÇ»ù±¾Êı¾İ£¬¿½±´µÄÊÇÊı¾İµÄÖµ£¬Èç¹ûÊÇÖ¸Õë£¬¿½±´µÄÊÇÊµ²ÎµÄÖµ£¨ÖµÊÇÒ»¸öµØÖ·£©¡£*/
void cirbuf_unint(cirbuf_t *buf){
    if(buf->data)
	    free(buf->data);
	buf->data=buf->rptr=buf->wptr=NULL;
    /*ÊÍ·ÅÕâ¸öµØÖ·Ö¸ÏòµÄÄÚ´æ*/
    free(buf);
	/*Õâ¸Ä±äµÄÊÇĞÎ²Î°É£¬Ã»ÆäËû×÷ÓÃ*/
   //    buf=NULL;
}

int cirbuf_empty(cirbuf_t *buf)
{
	LOGD("buf->level [%d]",buf->level);
	if(buf->level==0)
		return 1;
//	return buf->level;
	return 0;
}
/*ÊÇ²»ÊÇÑ­»·»º³å²»´æÔÚÂúµÄÇé¿ö£¬¿ÉÒÔÖ±½Ó¸²¸ÇµÄå*/
int cirbuf_full(cirbuf_t *buf){

	if(buf->level==buf->size)
	{
		LOGD("buff is full ");
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
	if(buf->wptr < buf->rptr){
		LOGD("W<R");
		rest=buf->rptr-buf->wptr;
		capability=MIN(rest,put_size);
		memcpy(buf->wptr,(uint8_t*)input,capability);
		buf->wptr+=capability;
	}else
     {
		LOGD("W>R or W==R");
        rest=(buf->size)-(buf->wptr-buf->rptr);
		capability=MIN(rest,put_size);
		int I=(buf->data+buf->size) - buf->wptr;
		int II=0;
		LOGD("rest[%d] capablity[%d] I[%d]",rest,capability,I);
		int check=capability-I;
		memcpy(buf->wptr,input,capability);
		buf->wptr+=capability;
		if(check==0)
			buf->wptr=buf->data;
		else
		if(check>0){/*Ç°¶Ë²åÈë*/
			II=capability-I;
			memcpy(buf->data,input-I,II);
			buf->wptr=buf->data+II;
		}
	}
OK:
	    buf->level+=capability;
		return capability;
}

int cirbuf_get(cirbuf_t *buf,void *output,int get_size){
  if(cirbuf_empty(buf)==1)
  {
  	  LOGD("cirbuf is empty,could not get");
	  return -1;
  }
  int availability=-1;
  int stored=-1;
  LOGD("do get[%d]",get_size);
  if(buf->wptr > buf->rptr){
  	LOGD("W>R");
	stored=buf->wptr-buf->rptr;
	goto OKOK;
  }else
  {
  	LOGD("W < R or W==R");
  	stored  =buf->size-(buf->rptr-buf->wptr);
	availability=MIN(stored,get_size);
    int IV=buf->data+buf->size - buf->rptr;
	int check=IV-availability;
	memcpy(output,buf->rptr,availability);
	buf->rptr+=availability;
	if(check<0){
		LOGD("output from V");
		/*ÒÑ¾­Ğ´ÈëÁËIV´óĞ¡*/
		memcpy(output+IV,buf->data,(availability-IV));
	}
    #if 0
	else{
		if(check==0){/*´æÔÚÕâÃ´Ò»ÖÖ¸ÕºÃ¹»ÓÃµÄÇé¿ö*/
	   		LOGD("rptr move to beginning");
			buf->rptr=buf->data;
	   }
	}
	#endif
	goto OK;

  }
OKOK:
	availability=MIN(stored,get_size);
	memcpy(output,buf->rptr,availability);
	buf->rptr+=availability;
 OK:
	buf->level-=availability;
	if(buf->rptr == (buf->data+buf->size)){
		LOGD("read to end ,from the begin  ");
		buf->rptr=buf->data;
	}
	return availability;
}

#define TEST_PUT 0
#define TEST_OUT 1

int main(){
	int i;
	cirbuf_t *buf;
    cirbuf_init(&buf,10);
	uint8_t *array="1234567890";
    uint8_t *array2="abcdefghik";
	/*Ğ´Èë1µ½0£¬¹²10¸ö*/
	cirbuf_put(buf,array,10);
	/*uint8_t ¾ÓÈ»Ö»Õ¼ÁËÒ»¸ö×Ö½Ú£¬ºÍcharÒ»Ñù*/
	LOGD("sizeof(int)[%d] sizeof(uint8_t)[%d] sizeof(char)[%d]",sizeof(int),sizeof(uint8_t),sizeof(char));
	LOGD("----1---AFTER PUT");

	for(i=0;i<10;i++)
		printf("buf->data[%c] \n",*(buf->data+i));
#if TEST_PUT

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

#if TEST_OUT
    uint8_t * out=malloc(10*sizeof(uint8_t));
    if(out==NULL){
		LOGD("malloc fail");
		return -1;
	}
    memset(out,0,10*sizeof(uint8_t));
    cirbuf_get(buf, out, 4);
	LOGD("out[%s]",out);
	cirbuf_get(buf, out, 4);
	LOGD("out[%s]",out);
	cirbuf_get(buf, out, 2);
	LOGD("out[%s]",out);
	cirbuf_get(buf, out, 4);
	LOGD("out[%s]",out);
	cirbuf_put(buf,array2,4);
	LOGD("buf->data[%s]",buf->data);
	cirbuf_put(buf,array,4);
	LOGD("buf->data[%s]",buf->data);
	cirbuf_put(buf,array2,5);
	LOGD("buf->data[%s]",buf->data);
	cirbuf_put(buf,array,3);
	LOGD("buf->data[%s]",buf->data);
	cirbuf_get(buf, out, 8);
	LOGD("out[%s]",out);
	LOGD("get from buf->data[%s]",buf->data);
	cirbuf_get(buf, out, 7);
	LOGD("then out[%s]",out);
	LOGD("get from buf->data[%s]",buf->data);
	cirbuf_get(buf, out, 7);
	LOGD("then out[%s]",out);
	free(out);
#endif

	cirbuf_unint(buf);
    return 0;
}
