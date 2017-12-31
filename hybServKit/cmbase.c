/******************************************************************************\
* Copyright (c)  2005  cmShare, All rights Reserved                           *
* FILE NAME:		       cmBase.c                                               *
* PROGRAMMER:	   	     ming.chan                                              *
* Date of Creation:		 2005/03/12                                             *
* Date of Last Modify: 2005/05/11                                             *
* DESCRIPTION: 			in common use method lib                                  *
\******************************************************************************/
#include "cmbase.h"
//------------------------------------------------------------------------------
// 字符串处理
//---------------------------------------------------------------------------
BOOL str_isNumeric(char *data)
{ if(!data || *data==0)return FALSE;
  while(*data)
  { if(*data<'0'||*data>'9')return(FALSE);
    else data++;
  }
  return(TRUE);
}

//字符串转整数，可用系统函数atoi取代。
// 区别于atoi，如果atoi传入指针NULL会抛出异常。
int str_atoi(char *data)
{ int ret=0;
  if(data)
  { while(*data>='0' && *data<='9')
    {  ret=ret*10+ *data-'0';
       data++;
    }
  }
  return ret;
}

int str_htoi(char *strHex)
{ if(strHex)
  { int data=0;
    while(*strHex)
	{ unsigned char hex=*strHex++;
      if(hex>='0' && hex<='9') hex-='0';
      else if(hex>='a' && hex<='f') hex-=('a'-0x0a);
      else if(hex>='A' && hex<='F') hex-=('A'-0x0a);
	  else return -1;
	  data=(data<<4)|hex;
	}
    return data;
  }
  return -1;
}

double str_atof(char *p)
{ if(p)
  { double ret;
    unsigned data_i=0,negtive;
    while(*p && (*p<'0' || *p>'9') && *p!='.' && *p!='-')p++;
    if(*p=='-'){negtive=1;p++;}else negtive=0;
    while(*p>='0' && *p<='9')
    { data_i=data_i*10+(*p-'0');
      p++;
    }
    ret=data_i;
    if(*p=='.')
    { double xs=0.1;
      p++;
      while(*p>='0' && *p<='9')
      { ret=ret+xs*(*p-'0');
        xs/=10;
        p++;
      }
    }
    return (negtive)?-ret:ret;
  }else return 0;
}

int str_ftoa(float fdata,int digit,char *strfloat)
{ long d1=(long)fdata;
  int i=str_itoa(d1,strfloat);
	fdata=fdata-d1;
	if(fdata<0)fdata=-fdata;
	strfloat[i++]='.';
	while(digit-->0)
	{ fdata=fdata*10;
	  strfloat[i++]='0'+((long)fdata)%10;
	}
	strfloat[i]='\0';
  return i; //返回转换后字符串长度
}

int  str_itoa(int value,char *desStr)
{ int i=0;
  if(value>=-9 && value<=9)
  { if(value<0)
    { desStr[i++]='-';
      desStr[i++]='0'-value;
    }
    else
    { desStr[i++]='0'+value;
    }
  }
  else
  { char dtempbuf[16];
    int dLen=0;
    if(value<0){desStr[i++]='-';value=-value;}
    while(value)
    { dtempbuf[dLen++]='0'+(value%10);
      value/=10;
    }
    while(--dLen>=0)
    { desStr[i++]=dtempbuf[dLen];
    }
  }
  desStr[i]='\0';
  return i; //返回转换后字符串长度
}

#define HEX_CHAR(bytedata) (((bytedata)<10)?(bytedata)+'0':(bytedata)-10+'A')
int str_bytesToHex(void *srcData,int dataLen,char *hexBuf,int bufSize,char splitter)
{  int i,space;
   if(!srcData || !hexBuf)return 0;
   //一个字节数据由2个HexChar及1个分隔符，共3个字符组成。
   if(splitter)space=bufSize/3;
   else space=bufSize>>1;
   if(dataLen>space)dataLen=space;
   for(i=space=0;i<dataLen;i++)
   { hexBuf[space++]=HEX_CHAR(((unsigned char *)srcData)[i]>>4);
     hexBuf[space++]=HEX_CHAR(((unsigned char *)srcData)[i]&0x0F);
     if(splitter)hexBuf[space++]=splitter;
   }
   if(space<bufSize)hexBuf[space]=0;
   return space;
}

int str_hexToBytes(char *strHex,void *dataBuf,int bufSize)
{ if(!strHex || !dataBuf || bufSize<=0)return 0;
  else {
   unsigned char ch;
   int i,one_byte,index=-1;
   BOOL  gethigh=FALSE;
   for(i=0;(ch=strHex[i])!=0;i++)
   { if(ch>='0'&&ch<='9') one_byte=ch-'0';
     else if(ch>='a'&&ch<='f')one_byte=ch-'a'+10;
     else if(ch>='A'&&ch<='F')one_byte=ch-'A'+10;
     else
     { gethigh=FALSE;
       continue;
     }
     if(gethigh) ((char *)dataBuf)[index]=(((char *)dataBuf)[index] << 4) | (one_byte & 0x0f);
     else if(++index<bufSize)((char *)dataBuf)[index]=one_byte;
     else return index;
     gethigh=!gethigh;
   }
   return index+1;
 }
}

int str_hexToBin(char* strHex,char *binBuf,int bufSize)
{ BOOL spaced=TRUE;
  unsigned char ch;
  int i,j,k;
  if(!strHex || !binBuf || bufSize<=0)return 0;
  for(i=j=0;(ch=strHex[i])!=0;i++)
  { if(ch>='0' && ch<='9') ch=ch-'0';
    else if(ch>='A' && ch<='F') ch=ch-'A'+10;
    else if(ch>='a' && ch<='f') ch=ch-'a'+10;
    else
    { if(!spaced)
      { if(j<bufSize)binBuf[j++]=32;
        else return j;
        spaced=TRUE;
      }
      continue;
    }
    spaced=FALSE;
    for(k=0;k<4;k++)
    {  if(j<bufSize)binBuf[j++]=((ch>>(3-k))&0x01)+'0';
       else return j;
    }
  }
  if(j<bufSize)binBuf[j]=0;
  return j;
}

int str_hexToDec(char* strHex,char *decBuf,int bufSize)
{ BOOL spaced=TRUE;
  int i,j,k;
  unsigned long decdata=0;
  unsigned char ch,datatemp[12];
  if(!strHex || !decBuf || bufSize<=0)return 0;
  for(i=j=k=0;(ch=strHex[i])!=0;i++)
  { if(ch>='0' && ch<='9')  ch=ch-'0';
    else if(ch>='A' && ch<='F')  ch=ch-'A'+10;
    else if(ch>='a' && ch<='f')  ch=ch-'a'+10;
    else
    { if(!spaced)
      { label_save_data:
        spaced=TRUE;
        do
        { datatemp[k++]= (unsigned char)(decdata % 10)+'0';
          decdata=decdata /10;
        }while(decdata>0);
        while(k)
        { if(j<bufSize)decBuf[j++]=datatemp[--k];
          else return j;
        }
        if(ch)
        { if(j<bufSize)decBuf[j++]=32;
          else return j;
        }
        else break;
      }
      if(ch=='\0')break;
      else continue;
    }
    spaced=FALSE;
    decdata=(decdata<<4)|ch;
  }
  if(!spaced) goto label_save_data;
  if(j<bufSize)decBuf[j]=0;
  return j;
}

char *stristr(char *src,char *obj)
{ if(src && obj)
  { int state=0;
    while(*src && obj[state])
    { if((*src|0x20)==(obj[state]|0x20))
      { state++;
      }
      else if(state)
      { src-=state;
        state=0;
      }
      src++;
    }
    return (obj[state])?NULL:src-state;
  }else return NULL;
}

//字符串替换（区分大小写)：将pText字符串中包含的src子串全部替换成des字符串
//返回替换的次数
int str_replace(char *pText,char *src,char *des)
{ int replacecount=0;
  pText=strstr(pText,src);
  if(pText)
  { int srcLen=strlen(src);
    int desLen=strlen(des);
    if(desLen==srcLen)
    { while(pText)
      { replacecount++;
        memcpy(pText,des,desLen);
        pText=strstr(pText+desLen,src);
      }
    }
    else
    { char *temstr=(char *)malloc(strlen(pText)+1);
      int blocklen;
      char *p1,*p2;
      strcpy(temstr,pText);
      p1=temstr;
      while(p1)
      { replacecount++;
        memcpy(pText,des,desLen);
        pText+=desLen;
        p1+=srcLen;
        p2=strstr(p1,src);
        blocklen=(p2)?(int)(p2-p1):strlen(p1);
        if(blocklen)
        { memcpy(pText,p1,blocklen);
          pText+=blocklen;
        }
        p1=p2;
      }
      *pText='\0';
      free(temstr);
    }
  }
  return replacecount; //返回替换的次数
}

char  *str_keySeek(char *keyList,char *key,char splitter)
{ if(keyList && key && splitter)
  { int i=0,state=0;
    while(keyList[i])
    { if(keyList[i]==key[state])state++;
      else if(state)
      { if(key[state]=='\0' && keyList[i]==splitter)return keyList+i;
        else
        { i-=state;
          state=0;
        }
      }
      i++;
    }
    if(state && key[state]=='\0') return keyList+i;
  }
  return NULL;
}

/*从内存中查找一个数组，并返回首指针，若没有找到则返回NULL*/
void *mem_search(void *srcMem,int memSize,void *desData,int dataSize)
{ int i,state;
  unsigned char *pmem=(unsigned char *)srcMem;
  unsigned char *psearch=(unsigned char *)desData;
  if(!srcMem || !desData || !memSize || !dataSize)return NULL;
  for(i=0,state=0;i<memSize;i++)
  { if(pmem[i]==psearch[state])
    { state++;
      if(state>=dataSize)return pmem+i+1-dataSize;
    }
    else if(state)
    { i-=state;
      state=0;
    }
  }
  return NULL;
}

void mem_reverse(void *p_buf,int dLen)
{ unsigned char temp,*p=(unsigned char *)p_buf;
  int i,the_pos,steps=dLen >>1;
  for(i=0;i<steps;i++)
  { the_pos=dLen-i-1;
    temp=p[i];
    p[i]=p[the_pos];
    p[the_pos]=temp;
  }
}
//---------------------------------------------------------------------------
//utf8字符长度1-6，可以根据每个字符第一个字节判断整个字符长度
//0xxxxxxx
//110xxxxx 10xxxxxx
//1110xxxx 10xxxxxx 10xxxxxx
//11110xxx 10xxxxxx 10xxxxxx 10xxxxxx
//111110xx 10xxxxxx 10xxxxxx 10xxxxxx 10xxxxxx
//1111110x 10xxxxxx 10xxxxxx 10xxxxxx 10xxxxxx 10xxxxxx
int str_lenOfUTF8(char *str)//计算str字符数目
{ //定义查找表，长度256，表中的数值表示以此为起始字节的utf8字符长度
	static unsigned char utf8_lookup_table[] ={1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3,4, 4, 4, 4, 4, 4, 4, 4, 5, 5, 5, 5, 6, 6, 1, 1};
	int len=0,size=0;
  while(*str)
  { if(size){str++;size--;}else{size=utf8_lookup_table[(unsigned char)*str];len++;}
  }
  return len;
}

char *str_xmlSeek(char *xmlbuf,char *key,int *len)
{  if(xmlbuf && key && *key)
   { int keyLen=strlen(key);
     char *match_first=NULL;
     while((xmlbuf=stristr(xmlbuf,key))!=NULL)
     { if(match_first)
       { if(*(xmlbuf-1)=='/' && *(xmlbuf-2)=='<' && *(xmlbuf+keyLen)=='>')
       	 { *len=xmlbuf-match_first-2;
       	   return match_first;
	 }else xmlbuf+=keyLen;
       }
       else
       { if(*(xmlbuf-1)=='<')
         { xmlbuf+=keyLen;
	   if(*xmlbuf=='>')match_first=xmlbuf+1;
	 }else xmlbuf+=keyLen;
       }
     }
   }
   return NULL;
}
//---------------------------------------------------------------------------
// Unix时间戳相关
//---------------------------------------------------------------------------
int str_fromTime(char *strTime,char *format,time_t timestamp)
{ const int strsize=32;
  return strftime(strTime,strsize,(format)?format:"%Y-%m-%d %H:%M:%S",localtime((const time_t *)&timestamp));
}

time_t str_toTime(char *strTime,char *format)
{ if(strTime && *strTime)
  { struct tm tm_time;
    strptime(strTime,(format)?format:"%Y-%m-%d %H:%M:%S", &tm_time);//这个函数仅带日期会出错误结果
    return mktime(&tm_time);
  }
  else return (time_t)0;
}

//get local hour from unix timestamp;
int tm_getLocalHour(time_t timestamp)
{ struct tm *p=localtime(&timestamp); //时间戳转换为本地时间（hour读数与时区相关）
  return p->tm_hour;
}
//---------------------------------------------------------------------------
// Queue buffer api
//---------------------------------------------------------------------------
typedef struct{int size, readpos, writepos;char buffer[0];}TQueueBuffer;
/*理解：
qb->redapos指向将要读而尚未读的位置；
qb->redapos指向将要写而尚未写的位置；
qb->size是指队列的存储空间，但是可用空间只有qb->size-1个，
因此，队列空时，可用空间是qb->size-1；队列满时，写入数据总量也是qb->size-1；
如果强行写入qb->size个数据，必然导致队列溢出。
*/
#define _QB_WP(hQb)        ((TQueueBuffer *)hQb)->writepos
#define _QB_RP(hQb)        ((TQueueBuffer *)hQb)->readpos
#define _QB_BUF(hQb)       ((TQueueBuffer *)hQb)->buffer
#define _QB_SIZE(hQb)      ((TQueueBuffer *)hQb)->size
#define _QB_USED_SIZE(hQb) ((_QB_RP(hQb)>_QB_WP(hQb))?(_QB_SIZE(hQb)+_QB_WP(hQb)-_QB_RP(hQb)):(_QB_WP(hQb)-_QB_RP(hQb)))
#define _QB_FREE_SIZE(hQb)  ((_QB_RP(hQb)>_QB_WP(hQb))?(_QB_RP(hQb)-_QB_WP(hQb)-1):(_QB_SIZE(hQb)+_QB_RP(hQb)-_QB_WP(hQb)-1))

HAND qb_create(int size)
{ HAND qb=(HAND)malloc(sizeof(TQueueBuffer)+size+1);//多分配1个结束符
  if(qb)
  { _QB_WP(qb)=0;
    _QB_RP(qb)=0;
    _QB_SIZE(qb)=size;
  }
  return qb;
}

void  qb_destroy(HAND qb)
{ if(qb)free(qb);
}

int  qb_size(HAND qb)
{ return (qb)?_QB_SIZE(qb):0;
}

int  qb_isEmpty(HAND qb)
{ return qb && _QB_RP(qb)==_QB_WP(qb);
}

void qb_clear(HAND qb)
{ if(qb){_QB_RP(qb) = _QB_WP(qb) = 0;}
}

int qb_usedSize(HAND qb)
{ if(qb)
  { return _QB_USED_SIZE(qb);
  }else return 0;
}

int qb_freeSize(HAND qb)
{ if(qb)
  { return  _QB_FREE_SIZE(qb);
  }else return 0;
}

void *qb_usedSpace(HAND qb,int *size1, int *size2)
{ if(_QB_WP(qb)>=_QB_RP(qb))
  { if(size1)*size1=_QB_WP(qb)-_QB_RP(qb);
                if(size2)*size2=0;
  }
  else
  { if(size1)*size1=_QB_SIZE(qb)-_QB_RP(qb);
                if(size2)*size1=_QB_WP(qb);
  }
  return _QB_BUF(qb)+_QB_RP(qb);
}

void *qb_freeSpace(HAND qb,int *size1,int *size2)
{ if(_QB_WP(qb)<_QB_RP(qb))
  { if(size1)*size1=_QB_RP(qb)-_QB_WP(qb)-1;
                if(size2)*size2=0;
  }
  else if(_QB_RP(qb)==0)
  { if(size1)*size1=_QB_SIZE(qb)-_QB_WP(qb)-1;
                if(size2)*size2=0;
        }       
  else
        {       if(size1)*size1=_QB_SIZE(qb)-_QB_WP(qb);
                if(size2)*size2=_QB_RP(qb)-1;
        }
  return _QB_BUF(qb)+_QB_WP(qb);
}

int qb_write(HAND qb,void *buf, int len)
{ if(qb && len>0)
  { int BytesTail,BytesAhead;
    int remaims=_QB_FREE_SIZE(qb);
    if(len>remaims)
    { if(remaims<=0)return 0;
      else len=remaims;
    }
    BytesAhead=_QB_WP(qb)+len-_QB_SIZE(qb);
    if(BytesAhead<0)
    { if(buf)memcpy(_QB_BUF(qb)+_QB_WP(qb), buf, len);
      _QB_WP(qb) += len;
    }
    else if(BytesAhead==0)
    { if(buf)memcpy(_QB_BUF(qb)+_QB_WP(qb), buf, len);
      _QB_WP(qb) = 0;
    }
    else
    { BytesTail=len-BytesAhead;
      if(buf)
      { memcpy(_QB_BUF(qb)+_QB_WP(qb), buf , BytesTail);
        memcpy(_QB_BUF(qb),(char *)buf+BytesTail, BytesAhead );
      }
      _QB_WP(qb) = BytesAhead;
    }
    return len;  /*返回实际写入的字节数*/
  }else return 0;
}

int qb_read(HAND qb,void *buf, int len)
{ //充许buf为NULL，当buf为NULL时相当于从队列中清除指定数目个字节.
  if(qb && _QB_WP(qb)!=_QB_RP(qb) && len>0)
  { if(len==1)
    { if(buf) *(char *)buf=_QB_BUF(qb)[_QB_RP(qb)];
      if(++_QB_RP(qb)>=_QB_SIZE(qb))_QB_RP(qb)=0;
    }
    else
    { int BytesTail,BytesAhead;
      int QueueBytes=_QB_USED_SIZE(qb);
      if(len>QueueBytes)len=QueueBytes;
      BytesAhead=_QB_RP(qb)+len-_QB_SIZE(qb);
      if(BytesAhead<0)
      { if(buf)memcpy(buf, _QB_BUF(qb)+_QB_RP(qb), len);
        _QB_RP(qb)+=len;
      }
      else if(BytesAhead==0)
      { if(buf)memcpy(buf, _QB_BUF(qb)+_QB_RP(qb), len);
        _QB_RP(qb)=0;
      }
      else
      { BytesTail=len-BytesAhead;
        if(buf)
        { memcpy(buf, _QB_BUF(qb)+_QB_RP(qb), BytesTail );
          memcpy((char *)buf+BytesTail, _QB_BUF(qb), BytesAhead);
        }
        _QB_RP(qb)=BytesAhead;
      }
    }
    return len;
  }else return 0;
}

int qb_peek(HAND qb,void *buf,int len)
{ if(qb && _QB_WP(qb)!=_QB_RP(qb) && len>0  && buf)
  { if(len==1)
    { *(char *)buf=_QB_BUF(qb)[_QB_RP(qb)];
    }
    else
    { int BytesTail,BytesAhead;
      int QueueBytes=_QB_USED_SIZE(qb);
      if(len>QueueBytes)len=QueueBytes;
      BytesAhead=_QB_RP(qb)+len-_QB_SIZE(qb);
      if(BytesAhead<=0)
      { memcpy(buf, _QB_BUF(qb)+_QB_RP(qb), len);
      }
      else
      { BytesTail=len-BytesAhead;
        memcpy(buf, _QB_BUF(qb)+_QB_RP(qb), BytesTail );
        memcpy((char *)buf+BytesTail, _QB_BUF(qb), BytesAhead);
      }
    }
    return len;
  }else return 0;
}

#if 0
int qb_blockWritePrefetch(HAND qb,void **pbuf,int minimum)
{ //从队列中读出地址连续的空闲块的指针以及的空闲块大小，空闲块地址通过pbuf参数返回,空闲块大小通过函数返回。
  //如果minimum参数大于0且连续空闲块的长度小于minimum时，则函数返回值将作如下两种情形的调整：
  //::一种是队列总的可写空间不够（返回0），第二种是连续写空间不够（返回取到的连续空间大小）。
  if(qb)
  { if(pbuf)*pbuf=_QB_BUF(qb)+_QB_WP(qb);
    if(_QB_WP(qb)<_QB_RP(qb))
    { int remains=_QB_RP(qb)-_QB_WP(qb)-1;
      return (remains<minimum)?0:remains;
    }
    else
    { if(_QB_RP(qb)==0)
      { int remains=_QB_SIZE(qb)-_QB_WP(qb)-1;
        return (remains<minimum)?0:remains;
      }else return _QB_SIZE(qb)-_QB_WP(qb);
    }
  }else return 0;
}

int qb_blockReadPrefetch(HAND qb,void **pbuf,int minimum)
{ //从队列中读出地址连续的数据块的指针以及的数据块大小，数据块地址通过pbuf参数返回,数据块大小通过函数返回。
  //如果minimum参数大于0且连续数据块的长度小于minimum时，则函数返回值将作如下两种情形的调整：
  //::一种是队列总的可读内容不够（返回0），第二种是连续读空间不够（返回取到的连续空间大小）。
  if(qb)
  { if(pbuf)*pbuf=_QB_BUF(qb)+_QB_RP(qb);
    if(_QB_WP(qb)>=_QB_RP(qb))
    { int remains=_QB_WP(qb)-_QB_RP(qb);
      return (remains<minimum)?0:remains;
    }
    else
    { return _QB_SIZE(qb)-_QB_RP(qb);
    }
  }
  return 0;
}
#endif

int qb_blockRead(HAND qb,void *buf,int len)
{ //从队列中读出指定长度且地址连续的内容。队列尾部不足的空间将被跳过。
  //返回实际读取的长度，要么等于len，要么为0;
  //允许buf为空，表示只改变读指针；
  if(qb && _QB_WP(qb)!=_QB_RP(qb) && len>0 )
  { int read_pos=_QB_RP(qb);
    if(_QB_WP(qb)<read_pos)
    { int tailspace=_QB_SIZE(qb)-read_pos;
      if(len<=tailspace)
      { if(buf)memcpy(buf,_QB_BUF(qb)+read_pos,len);
      	_QB_RP(qb)=(tailspace==len)?0:read_pos+len;
      	return len;
      }
      else
      { read_pos=0;
      }
    }
    if(len<=_QB_WP(qb)-read_pos)
    { if(buf)memcpy(buf,_QB_BUF(qb)+read_pos,len);
      _QB_RP(qb)=read_pos+len;
      return len;
    }
  }
  return 0;
}

int qb_blockWrite(HAND qb,void *buf,int len)
{ //向队列中写入指定长度且地址连续的内容。队列尾部不足的空间将被跳过。
  //返回实际写入的长度，要么等于len，要么为0;
  //允许buf为空，表示只改变写指针；
  if(qb && len>0)
  { int write_pos=_QB_WP(qb);
    if(write_pos>=_QB_RP(qb))
    { int tailspace=_QB_SIZE(qb)-write_pos;
      if(len<tailspace)
      { if(buf)memcpy(_QB_BUF(qb)+write_pos,buf,len);
      	_QB_WP(qb)=write_pos+len;
      	return len;
      }
      else
      { if(_QB_RP(qb)==0)return 0;
        else if(len==tailspace)
        { if(buf)memcpy(_QB_BUF(qb)+write_pos,buf,len);
          _QB_WP(qb)=0;
      	  return len;
        }
        else write_pos=0;
      }
    }
    if(len<=_QB_RP(qb)-write_pos-1)
    { if(buf)memcpy(_QB_BUF(qb)+write_pos,buf,len);
      _QB_WP(qb)=write_pos+len;
      return len;
    }
  }
  return 0;
}
//---------------------------------------------------------------------------
//TMailBox
//---------------------------------------------------------------------------
typedef struct{HAND qb,_msg_mutex,_msg_sem;}TcmMailBox;
typedef struct{int msgLen;char msgData[0];}TcmMailMsg;
static TcmMailBox  *g_mailBox=NULL;//目前总共只设置了一个邮箱

void mb_create(int queue_size)
{ if(!g_mailBox)
  { g_mailBox=(TcmMailBox *)malloc(sizeof(TcmMailBox));
    g_mailBox->qb=qb_create(queue_size);
    os_createSemphore(&g_mailBox->_msg_mutex,1);/*信号量初值为1，作互斥量*/
    os_createSemphore(&g_mailBox->_msg_sem,0);  /*信号量初值为0，表示消息数*/
  }
}

void mb_destroy(void)
{ if(g_mailBox)
  { qb_destroy(g_mailBox->qb);
    os_destroySemphore(g_mailBox->_msg_mutex);
    os_destroySemphore(g_mailBox->_msg_sem);
    free(g_mailBox);
    g_mailBox=NULL;
  }
}

int  mb_post(void *msgData,int dataLen)//发送一条邮箱消息
{ if(msgData && dataLen)
  { int packet_size=sizeof(TcmMailMsg)+dataLen;
    os_obtainSemphore(g_mailBox->_msg_mutex);
    if(qb_freeSize(g_mailBox->qb)>=packet_size)
    { TcmMailMsg mb_msg;
      mb_msg.msgLen=dataLen;
      qb_write(g_mailBox->qb,&mb_msg,sizeof(TcmMailMsg));
      qb_write(g_mailBox->qb,msgData,dataLen);
      os_releaseSemphore(g_mailBox->_msg_mutex);
      os_releaseSemphore(g_mailBox->_msg_sem);
      return dataLen;
    }
    else
    { os_releaseSemphore(g_mailBox->_msg_mutex);
    }
  }
  return 0;
}

int  mb_receive(void *msgBuf,int bufSize)//读取一条邮箱消息
{ int got_msg_size=0;
  os_obtainSemphore(g_mailBox->_msg_sem);
  os_obtainSemphore(g_mailBox->_msg_mutex);
  if(qb_freeSize(g_mailBox->qb)>=sizeof(TcmMailMsg))
  { TcmMailMsg mb_msg;
    if(qb_read(g_mailBox->qb,&mb_msg,sizeof(TcmMailMsg))==sizeof(TcmMailMsg))
    { got_msg_size=(mb_msg.msgLen<=bufSize)?mb_msg.msgLen:bufSize;
      if(qb_read(g_mailBox->qb,msgBuf,got_msg_size)==got_msg_size)
      { if(got_msg_size<mb_msg.msgLen)qb_read(g_mailBox->qb,NULL,mb_msg.msgLen-got_msg_size);
      }
      else got_msg_size=0;
    }
  }
  os_releaseSemphore(g_mailBox->_msg_mutex);
  return got_msg_size;
}
//---------------------------------------------------------------------------
// Other api
//---------------------------------------------------------------------------
// BKDR Hash Function
U32 BKDRHash(char *str)
{ U32 hash = 0,seed = 131; // 31 131 1313 13131 131313 etc..
  while (*str)
  { hash = hash * seed + (*str++);
  }
  return hash;
}
//---------------------------------------------------------------------------
// TDateTimer
//---------------------------------------------------------------------------
#define DTMR_MAGIC_NUMBER      0x44544D52U//DTMR
#define DTMR_LIFE_MASK         0x0FFFFFFFU
typedef struct t_datatimer_node  TDateTimerNode;
typedef struct
{ DTMR_TimeoutEvent OnTimeout;
  TDateTimerNode *_tsklist;//dummy
  TBinodeLink *_hashMapTable;
  U32  hashMapLength,dataProtectTime,magicNumber;
  HAND _timer_check_thread,_task_mutex,_sem_timer;
}TDateTimer;

struct t_datatimer_node
{ TDateTimerNode *up,*down,*prev,*next;//顺序不要变
  TDateTimer *dtimer;
  U32 states,overrideCount,hashIndex,msTimeOut,extraSize,extraAlloc,nodeID[2];
  U8 ExtraData[0];//&ExtraData[extraSize]后面是nodeName字符串，但其长度不计在extraSize中。
};

static void _DTMR_UpdateTimeout(TDateTimer *ttasks,TDateTimerNode *node,U32 sLifeTime)
{ TDateTimerNode *listhead=ttasks->_tsklist;
  U32 msTimeOut=(sLifeTime)?(os_msRunTime()+sLifeTime*1000):0;
  node->msTimeOut=msTimeOut;
  if(node==listhead->next && (node->next==listhead || msTimeOut<=node->next->msTimeOut))
  { //本身就在表头，且时间排序不用调整。
     os_releaseSemphore(ttasks->_sem_timer);//头部节点的时间调整后要发信号量
  }
  else
  { //从时间轴链表中删除
    BINODE_REMOVE(node,prev,next);

    //重新插入横向环形双向链表（按时间排列）
    if(msTimeOut<=listhead->next->msTimeOut)
    { BINODE_INSERT(node,listhead,next,prev);
      os_releaseSemphore(ttasks->_sem_timer);//头部节点的时间调整后要发信号量
    }
    else
    { TDateTimerNode *lastNode=listhead->prev;
      while(msTimeOut<lastNode->msTimeOut)
      { lastNode=lastNode->prev;
      }
      BINODE_INSERT(node,lastNode,next,prev);
    }
  }
}

static void _DTMR_DeleteTask(TDateTimerNode *tskNode)
{  tskNode->nodeID[0]=0;
   tskNode->nodeID[1]=0;//taskID及nodeName删除后不会触发超时回调函数
   tskNode->ExtraData[tskNode->extraSize]='\0';
   tskNode->states=0;//清楚状态、解除锁定
   _DTMR_UpdateTimeout(tskNode->dtimer,tskNode,tskNode->dtimer->dataProtectTime);
}

void *dtmr_add(HAND dtimer,U32 nodeIDL,U32 nodeIDH,char *nodeName,void *nodeData,U32 nodeSize,U32 sLifeTime)
{ if(dtimer && ((TDateTimer *)dtimer)->magicNumber==DTMR_MAGIC_NUMBER && (nodeIDL||nodeIDH||nodeName) && os_obtainSemphore(((TDateTimer *)dtimer)->_task_mutex))
  { int nameSize,hashIndex;
    TDateTimerNode *header,*node;
    //当同时指定有效的taskID与nodeName时，优先通过taskID建立哈希查找表;
    if(nodeIDL||nodeIDH)
    { hashIndex=(nodeIDL+nodeIDH)%((TDateTimer *)dtimer)->hashMapLength;
      header=(TDateTimerNode *)(((TDateTimer *)dtimer)->_hashMapTable+hashIndex);
      for(node=header->down;node!=header;node=node->down)
      { if(node->nodeID[0]==nodeIDL && node->nodeID[1]==nodeIDH &&(!nodeName||strcmp(nodeName,(char *)node->ExtraData+node->extraSize)==0))break;
      }
    }
    else
    { hashIndex=BKDRHash(nodeName)%((TDateTimer *)dtimer)->hashMapLength;
      header=(TDateTimerNode *)(((TDateTimer *)dtimer)->_hashMapTable+hashIndex);
      for(node=header->down;node!=header;node=node->down)
      { if(strcmp((char *)node->ExtraData+node->extraSize,nodeName)==0)break;
      }
    }
    if(node==header)//没有找到同名节点则重新创建节点
    {  label_create_new_node:
       nameSize=(nodeName)?strlen(nodeName)+1:1;//含字符串结束符号
       node=(TDateTimerNode *)malloc(sizeof(TDateTimerNode)+nodeSize+nameSize);
       if(!node)
       { puts("[ERROR:dtmr_add]#######################################malloc fail!");
       	 exit(0);
       }
       if(nodeName)memcpy(&node->ExtraData[nodeSize],nodeName,nameSize);
       else node->ExtraData[nodeSize]='\0';
       node->nodeID[0]=nodeIDL;
       node->nodeID[1]=nodeIDH;
       node->overrideCount=0;	//表示第一次新建立的节点
       node->hashIndex=hashIndex;
       node->extraSize=node->extraAlloc=nodeSize;
       node->dtimer=(TDateTimer *)dtimer;
       BINODE_INSERT(node,header,down,up);//插入hash链表头部（若有重名则优先搜索匹配最新节点）。
       BINODE_INSERT(node,((TDateTimer *)dtimer)->_tsklist,next,prev);//插入时间轴链表头部
    }
    else //原来就存在的节点。
    { if((sLifeTime&DTMR_NOVERRIDE))//不允许覆盖同名节点
      { os_releaseSemphore(((TDateTimer *)dtimer)->_task_mutex);
        return NULL;
      }
      else //allowOverride:找到同名节点则重置原节点
      {  if(node->extraAlloc<nodeSize)
         {  _DTMR_DeleteTask(node);
            goto label_create_new_node;
         }
         else
         {  node->extraSize=nodeSize;
         	  node->overrideCount++; //覆盖计数
         }	
      }
    }
    if(node->overrideCount==0 || !(sLifeTime&DTMR_KEEPLIFE))
    { node->states=sLifeTime;
      _DTMR_UpdateTimeout((TDateTimer *)dtimer,node,sLifeTime&DTMR_LIFE_MASK);//更新节点的寿命及时间轴位置
    }
    else if((sLifeTime&DTMR_LOCK))
    { node->states|=DTMR_LOCK;
    }
    if(nodeData && nodeSize)memcpy(node->ExtraData,nodeData,nodeSize);
    if(!(sLifeTime&DTMR_LOCK))os_releaseSemphore(((TDateTimer *)dtimer)->_task_mutex);
    return node->ExtraData;
  }else return NULL;
}

void *dtmr_find(HAND dtimer,U32 nodeIDL,U32 nodeIDH,char *nodeName,U32 sUpdateLifeTime)
{ //msUpdateLifeTime为0表示不更新节点寿命，为-1表示锁定（在到期时自动按秒续寿），其它表示更新为新的寿命。
  if(dtimer && ((TDateTimer *)dtimer)->magicNumber==DTMR_MAGIC_NUMBER && os_obtainSemphore(((TDateTimer *)dtimer)->_task_mutex))
  { TDateTimerNode *header,*node;
    if(nodeIDL || nodeIDH)
    { int mIndex=(nodeIDL+nodeIDH) % ((TDateTimer *)dtimer)->hashMapLength;
      header=(TDateTimerNode *)(((TDateTimer *)dtimer)->_hashMapTable+mIndex);
      for(node=header->down;node!=header;node=node->down)
      { if(node->nodeID[0]==nodeIDL && node->nodeID[1]==nodeIDH &&(!nodeName||strcmp(nodeName,(char *)node->ExtraData+node->extraSize)==0))goto label_DoUpdateTime;
      }
    }
    else if(nodeName)
    { int mIndex=BKDRHash(nodeName)%((TDateTimer *)dtimer)->hashMapLength;
      header=(TDateTimerNode *)(((TDateTimer *)dtimer)->_hashMapTable+mIndex);
      for(node=header->down;node!=header;node=node->down)
      { if(strcmp((char *)node->ExtraData+node->extraSize,nodeName)==0)
      	{ label_DoUpdateTime:
    	    if(sUpdateLifeTime)
    	    { U32 sNewLifeTime=sUpdateLifeTime&DTMR_LIFE_MASK;
    	      if(sNewLifeTime)
    	      {	_DTMR_UpdateTimeout((TDateTimer *)dtimer,node,sNewLifeTime);
    	    	  node->states=(node->states&~DTMR_LIFE_MASK)|sNewLifeTime;
    	      }
            if((sUpdateLifeTime&DTMR_FOREVER))node->states|=DTMR_FOREVER;
            if((sUpdateLifeTime&DTMR_LOCK))node->states|=DTMR_LOCK;
            else os_releaseSemphore(((TDateTimer *)dtimer)->_task_mutex);
      	  }
          else os_releaseSemphore(((TDateTimer *)dtimer)->_task_mutex);
      	  return node->ExtraData;
      	}
      }
    }
    os_releaseSemphore(((TDateTimer *)dtimer)->_task_mutex);
  }
  return NULL;
}

void *dtmr_find2(HAND dtimer,U32 nodeIDL,U32 nodeIDH,void *nodeData,U32 nodeSize,int extraOffset,U32 sUpdateLifeTime)
{  if(dtimer && ((TDateTimer *)dtimer)->magicNumber==DTMR_MAGIC_NUMBER && os_obtainSemphore(((TDateTimer *)dtimer)->_task_mutex))
   { TDateTimerNode *header,*node;
     if(nodeIDL || nodeIDH)
     { int mIndex=(nodeIDL+nodeIDH) % ((TDateTimer *)dtimer)->hashMapLength;
       header=(TDateTimerNode *)(((TDateTimer *)dtimer)->_hashMapTable+mIndex);
       for(node=header->down;node!=header;node=node->down)
       { if(node->nodeID[0]==nodeIDL && node->nodeID[1]==nodeIDH &&(!nodeData||memcmp(nodeData,(char *)node->ExtraData+extraOffset,nodeSize)==0)) goto label_DoUpdateTime;
       }
     }
     else if(nodeData && nodeSize)
     { header=((TDateTimer *)dtimer)->_tsklist;
       for(node=header->next;node!=header;node=node->next)
       { if(memcmp(nodeData,(char *)node->ExtraData+extraOffset,nodeSize)==0 && (node->nodeID[0] || node->nodeID[1] || node->ExtraData[node->extraSize]))
         { label_DoUpdateTime:
           if(sUpdateLifeTime)
    	   {   U32 sNewLifeTime=sUpdateLifeTime&DTMR_LIFE_MASK;
    	       if(sNewLifeTime)
    	       { _DTMR_UpdateTimeout((TDateTimer *)dtimer,node,sNewLifeTime);
    	         node->states=(node->states&~DTMR_LIFE_MASK)|sNewLifeTime;
    	       }
               if((sUpdateLifeTime&DTMR_FOREVER))node->states|=DTMR_FOREVER;
               if((sUpdateLifeTime&DTMR_LOCK))node->states|=DTMR_LOCK;
               else os_releaseSemphore(((TDateTimer *)dtimer)->_task_mutex);
      	   }
           else os_releaseSemphore(((TDateTimer *)dtimer)->_task_mutex);
           return node->ExtraData;
         }
       }
     }
     os_releaseSemphore(((TDateTimer *)dtimer)->_task_mutex);
   }
   return NULL;
}

void  dtmr_unlock(void *dnode,U32 sUpdateLifeTime)
{ if(dnode)
  { TDateTimerNode *tskNode=T_PARENT_NODE(TDateTimerNode,ExtraData,dnode);
    if((tskNode->states&DTMR_LOCK))
    { TDateTimer *dtimer=tskNode->dtimer;
      if(dtimer && dtimer->magicNumber==DTMR_MAGIC_NUMBER)
      {  if(sUpdateLifeTime)
         { U32 sNewLifeTime=sUpdateLifeTime&DTMR_LIFE_MASK;
           if(sNewLifeTime)
           { _DTMR_UpdateTimeout((TDateTimer *)dtimer,tskNode,sNewLifeTime);
             tskNode->states=(tskNode->states&~DTMR_LIFE_MASK)|sNewLifeTime;
           }
           if((sUpdateLifeTime&DTMR_FOREVER))tskNode->states|=DTMR_FOREVER;
         }
         tskNode->states&=~DTMR_LOCK;
         os_releaseSemphore(dtimer->_task_mutex);
      }
    }
  }
}

void  dtmr_update(void *dnode,U32 sUpdateLifeTime)
{ if(dnode && sUpdateLifeTime)
  { TDateTimerNode *tsknode=T_PARENT_NODE(TDateTimerNode,ExtraData,dnode);
    TDateTimer *dtimer=tsknode->dtimer;
    if(dtimer && dtimer->magicNumber==DTMR_MAGIC_NUMBER && os_obtainSemphore(dtimer->_task_mutex))
    { U32 sNewLifeTime=sUpdateLifeTime&DTMR_LIFE_MASK;
      if(sNewLifeTime)
      {	_DTMR_UpdateTimeout((TDateTimer *)dtimer,tsknode,sNewLifeTime);
       	tsknode->states=(tsknode->states&~DTMR_LIFE_MASK)|sNewLifeTime;
      }
      if((sUpdateLifeTime&DTMR_FOREVER))tsknode->states|=DTMR_FOREVER;
      else if((tsknode->states&DTMR_FOREVER))tsknode->states&=~DTMR_FOREVER;;
      if(!(sUpdateLifeTime&DTMR_LOCK))os_releaseSemphore(dtimer->_task_mutex);
      else tsknode->states|=DTMR_LOCK;
    }
  }
}

int  dtmr_getSize(void *dnode)
{ if(dnode)
  { TDateTimerNode *tsknode=T_PARENT_NODE(TDateTimerNode,ExtraData,dnode);
    return tsknode->extraSize;
  }else return 0;	
}
  	
//根据TaskCode地址删除节点 （这里只是设置为超时，后面由定时器来删除）
void  dtmr_remove(void *dnode)
{ if(dnode)
  { TDateTimerNode *tskNode=T_PARENT_NODE(TDateTimerNode,ExtraData,dnode);
    TDateTimer *dtimer=tskNode->dtimer;
    if(dtimer && dtimer->magicNumber==DTMR_MAGIC_NUMBER && os_obtainSemphore(dtimer->_task_mutex))
    {  _DTMR_DeleteTask(tskNode);
       os_releaseSemphore(dtimer->_task_mutex);
    }
  }
}

static void *_DTMR_timer_check_proc(void *param)
{ TDateTimer *dtimer=(TDateTimer *)param;
  TDateTimerNode *timerlist=dtimer->_tsklist;
  while(dtimer->_timer_check_thread)
  { TDateTimerNode *dueNode;
    U32 now_time_ms,msWaitTime;
    if(!os_obtainSemphore(dtimer->_task_mutex))break;
    now_time_ms=os_msRunTime();
    while((dueNode=timerlist->next)!=timerlist && now_time_ms>=dueNode->msTimeOut)
    { char *nodeName=(char *)dueNode->ExtraData+dueNode->extraSize;
      if(dueNode->nodeID[0] || dueNode->nodeID[1] || nodeName[0])
      { if((dueNode->states&DTMR_FOREVER))
        { //若加锁标记则继续延长寿命
           _DTMR_UpdateTimeout(dtimer,dueNode,dueNode->states&DTMR_LIFE_MASK);
       	   //Log_AppendText("Locked_%d node_%x_%x timeout!",dueNode->locked,dueNode->nodeID[0],dueNode->nodeID[1]);
           continue;
        }
      	else if(dtimer->OnTimeout)
        {  U32 sUpdateLifeTime=0;
           os_releaseSemphore(dtimer->_task_mutex);
           dtimer->OnTimeout(dtimer, dueNode->ExtraData,dueNode->nodeID,nodeName,&sUpdateLifeTime);
           os_obtainSemphore(dtimer->_task_mutex);
           if(sUpdateLifeTime)_DTMR_UpdateTimeout(dtimer,dueNode,sUpdateLifeTime&DTMR_LIFE_MASK);
           if(timerlist->next!=dueNode || now_time_ms<dueNode->msTimeOut)continue;
        }
        if(dtimer->dataProtectTime)
        {  dueNode->nodeID[0]=0;
           dueNode->nodeID[1]=0;
           nodeName[0]=0;
           _DTMR_UpdateTimeout(dtimer,dueNode,dtimer->dataProtectTime);
           continue;
        }
      }

      if(dueNode->next)
      { BINODE_REMOVE(dueNode,prev,next);//移出时间轴链表
        BINODE_REMOVE(dueNode,up,down);//从哈希链表中删除节点
        dueNode->next=NULL; //add only for debug
      }else puts("###################### Free fail, node alread destroyed！");
      //printf("free node:%lu\r\n",(long)dueNode);
      free(dueNode);
    }
    msWaitTime=(dueNode==timerlist)?30000:(dueNode->msTimeOut-now_time_ms);
    os_releaseSemphore(dtimer->_task_mutex);
    os_waitSemphore(dtimer->_sem_timer,msWaitTime);
  }
  os_destroySemphore(dtimer->_task_mutex);
  os_destroySemphore(dtimer->_sem_timer);
  free(dtimer);
  return NULL;
}

HAND dtmr_create(int hashLen,U32 sHoldTime,DTMR_TimeoutEvent OnTimeout)
{ int c_hashMapLength=(hashLen>0)?hashLen:256;
  int ttask_size=(sizeof(TDateTimer)&0x3)?(sizeof(TDateTimer)|0x03)+1:sizeof(TDateTimer);//address alignment
  int list_binode_size=sizeof(TBinodeLink)*2;
  int hash_binode_size=c_hashMapLength*sizeof(TBinodeLink);
  TDateTimer *dtimer=(TDateTimer *)malloc(ttask_size+list_binode_size+hash_binode_size);
  if(dtimer)
  { int i;
    dtimer->magicNumber=DTMR_MAGIC_NUMBER;
    dtimer->OnTimeout=OnTimeout;
    dtimer->dataProtectTime=(sHoldTime)?sHoldTime:3;
    dtimer->hashMapLength=c_hashMapLength;
    dtimer->_tsklist=(TDateTimerNode *)((char *)dtimer+ttask_size);
    BINODE_ISOLATE(dtimer->_tsklist,prev,next);
    dtimer->_hashMapTable=(TBinodeLink *)((char *)dtimer+ttask_size+list_binode_size);
    for(i=0;i<c_hashMapLength;i++)
    { TBinodeLink *node=dtimer->_hashMapTable+i;
      BINODE_ISOLATE(node,prev,next);
    }
    os_createSemphore(&dtimer->_task_mutex, 1);/*信号量初值为1，作互斥量*/
    os_createSemphore(&dtimer->_sem_timer,0);
    os_createThread(&dtimer->_timer_check_thread,_DTMR_timer_check_proc, dtimer);
  }
  return (HAND)dtimer;
}

char *dtmr_getName(void *dnode)
{  if(dnode)
   { TDateTimerNode *tskNode=T_PARENT_NODE(TDateTimerNode,ExtraData,dnode);
     return (char *)tskNode->ExtraData+tskNode->extraSize;
   }else return NULL;
}

int dtmr_getOverrideCount(void *dnode)
{  if(dnode)
   { TDateTimerNode *tskNode=T_PARENT_NODE(TDateTimerNode,ExtraData,dnode);
     return (int)tskNode->overrideCount;
   }else return 0;
}

void dtmr_destroy(HAND dtimer)
{ if(dtimer && ((TDateTimer *)dtimer)->magicNumber==DTMR_MAGIC_NUMBER && ((TDateTimer *)dtimer)->_timer_check_thread && os_obtainSemphore(((TDateTimer *)dtimer)->_task_mutex))
  { TDateTimerNode *listhead=((TDateTimer *)dtimer)->_tsklist;
    TDateTimerNode *node=listhead->next;
    while(node!=listhead)
    { TDateTimerNode *nextNode=node->next;
      node->states=0;
      node->dtimer=NULL;
      free(node);
      node=nextNode;
    }
    BINODE_ISOLATE(listhead,prev,next);
    ((TDateTimer *)dtimer)->magicNumber=0;
    ((TDateTimer *)dtimer)->_timer_check_thread=0;
    os_releaseSemphore(((TDateTimer *)dtimer)->_task_mutex);
    os_releaseSemphore(((TDateTimer *)dtimer)->_sem_timer);
  }
}
//---------------------------------------------------------------------------
//TcmSocket
//---------------------------------------------------------------------------
#define SOCKET_INBUFFER_SIZE_DEFAULT      655360U  //should be far larger than SOCKET_MAX_DGRAMSIZE

typedef struct t_tcp_client_socket
{ struct t_tcp_client_socket *next,*prev;
  TNetAddr addr;
  U32 timeout;
}TTcpClients;

typedef struct
{ HAND _client_close_mutex;
  TTcpClients actives,frees,clients[SOCKET_MAX_LISTEN];
}TServerConnections;

typedef struct _t_cmsocket
{ TNetAddr _localAddr,_remoteAddr;//必须放在开头（默认从结构体开头能索引到地址）
  TServerConnections *_connections;
  HAND _inqueue,_tag;
  U8  _zeroLocalIP,_zeroLocalPort;
  TSocketProtocol _protocol;
  HAND _socket_recv_thread,_thread_closed_sem;
  HAND _inqueue_read_mutex,_inqueue_read_sem,_socket_op_mutex;
  int  (*OnRecv)(HAND,TNetAddr *);
  void (*OnClientChange)(HAND,int,BOOL);
  void (*OnClose)(HAND);
}TcmSocket;

typedef struct
{ TNetAddr addr;
  int   datasize;  
  char  data[0];
}TcmPacketHead;

static int _SOCKET_ReceiveToQueue(HAND hSocket,TNetAddr *addr)
{ int total_received=0,iSocket=addr->socket;
  os_obtainSemphore(((TcmSocket *)hSocket)->_inqueue_read_mutex);
  while(1)//靠靠socket靠靠靠靠靠靠靠靠靠select靠靠�
  { int block_size1,block_size2;
    TcmPacketHead *packet=(TcmPacketHead *)qb_freeSpace(((TcmSocket *)hSocket)->_inqueue,&block_size1,&block_size2);
    if(block_size1>=SOCKET_MAX_DGRAMSIZE+sizeof(TcmPacketHead))
    { int received_bytes;
      #if MSG_DONTWAIT
      int nonblock_flag=MSG_DONTWAIT;
      #else
      int nonblock_flag=0;
      #endif
      if(((TcmSocket *)hSocket)->_protocol==spUDP)
      { struct sockaddr peerAddr;
        int addrLen=sizeof(peerAddr);
        //靠connectionless sockets靠靠靠縭ecvfrom靠縋eerAddress
          received_bytes=recvfrom(iSocket, packet->data, SOCKET_MAX_DGRAMSIZE, nonblock_flag ,&peerAddr, (os_socklen_t *)&addrLen);
          packet->addr.socket=iSocket;
          packet->addr.ip=((struct sockaddr_in *)&peerAddr)->sin_addr.s_addr;
        packet->addr.port=ntohs(((struct sockaddr_in *)&peerAddr)->sin_port);
      }
      else
      { //靠connection-oriented縮ockets靠靠靠縢etpeername靠縋eerAddress
          received_bytes=recv(iSocket,packet->data,SOCKET_MAX_DGRAMSIZE,nonblock_flag);
          packet->addr=*addr;
      }
      if(received_bytes>0)
      { packet->datasize=received_bytes;
        qb_write(((TcmSocket *)hSocket)->_inqueue,NULL,received_bytes+sizeof(TcmPacketHead));
        #if 0
         packet->data[received_bytes]='\0';
         printf("###receive from socket(%d):\r\n",iSocket);
         puts(packet->data);
        #endif
        total_received+=received_bytes;
        os_releaseSemphore(((TcmSocket *)hSocket)->_inqueue_read_sem);
        if(!nonblock_flag ||(((TcmSocket *)hSocket)->_protocol!=spUDP && received_bytes<SOCKET_MAX_DGRAMSIZE))break;
      }else break;
    }
    else if(block_size2>=SOCKET_MAX_DGRAMSIZE+sizeof(TcmPacketHead))
    { //靠靠靠靠靠靠靠靠靠靠靠靠靠靠靠靠靠靠�
      if(block_size1>=sizeof(TcmPacketHead))
      { packet->addr.socket=0;
        packet->datasize=block_size1-sizeof(TcmPacketHead);
      }
      qb_write(((TcmSocket *)hSocket)->_inqueue,NULL,block_size1);
    }
    else
    { puts("####Socket inqueue is out of space!");
      break;//靠靠靠靠靠靠�
    }
  }
  os_releaseSemphore(((TcmSocket *)hSocket)->_inqueue_read_mutex);
  return total_received;
}

void *_SOCKET_RecvProc(void *param)
{ TcmSocket *cmSocket=(TcmSocket *)param;
  int maxfd=cmSocket->_localAddr.socket;
  struct timeval waitimeout={15,0};//1seconds
  if(cmSocket->_protocol==spTcpServer)
  { TTcpClients *activeList=&cmSocket->_connections->actives;
    TTcpClients *freeList=&cmSocket->_connections->frees;
    TTcpClients *peer;
    fd_set fdset_connects;
    FD_ZERO(&fdset_connects);
    FD_SET(maxfd,&fdset_connects);
    while(cmSocket->_socket_recv_thread)
    { struct timeval wt=waitimeout;
      fd_set read_flags=fdset_connects;
      //select的socket文件集合中，不管是UDP或是TCP，只有一个socket的数据没读完就会直接返回。
      //select返回时（不管是因为超时还是有读写事件发生，fdset中没有发生事件的位都会被清零，传入的计时变量timeval也会被改变，因此每次重新调用select时都要重新赋值fdset及timeval)
      int stat=select(maxfd+1, &read_flags,NULL,NULL,&wt);//wt会被改变
      if(stat>0)
      { U32 now_time=(U32)time(NULL);
      	peer=activeList->next;
      	while(peer!=activeList)
      	{ int sfd=peer->addr.socket;
      	  if(now_time>peer->timeout)
      	  { printf("client[%d] timeout\r\n", sfd);
      	    goto LABEL_CloseSocket;
      	  }
      	  else if(FD_ISSET(sfd,&read_flags))
    	    { if(cmSocket->OnRecv((HAND)cmSocket,&peer->addr)>0)
    	      { peer->timeout=now_time+TCP_CONNECTION_TIMEOUT_S;
    	      }
    	      else
    	      { LABEL_CloseSocket:
    	        { HAND _client_close_mutex=cmSocket->_connections->_client_close_mutex;
    	      	  if(os_obtainSemphore(_client_close_mutex))
    	       	  { TTcpClients *nextnode=peer->next;
                  BINODE_REMOVE(peer,prev,next);
                  BINODE_INSERT(peer,freeList,prev,next);
                  peer=nextnode;
                  os_closesocket(sfd);
                  os_releaseSemphore(_client_close_mutex);
                }
                if(cmSocket->OnClientChange)cmSocket->OnClientChange(cmSocket,sfd,FALSE);
                FD_CLR((unsigned int)sfd, &fdset_connects);
                if(maxfd==sfd)maxfd=0;

                printf("client[%d] close\r\n", sfd);
              }
              continue;
    	      }
    	    }
      	  peer=peer->next;
      	}
      	if(maxfd==0)//need to update maxfd
      	{ maxfd=cmSocket->_localAddr.socket;
      	  peer=activeList->next;
      	  while(peer!=activeList)
      	  { if(peer->addr.socket>maxfd)maxfd=peer->addr.socket;
      	    peer=peer->next;
      	  }
      	}
    	if(FD_ISSET(cmSocket->_localAddr.socket, &read_flags))
   	  { struct sockaddr_in clientAddr;
    	  int sClient,clientAddrLen=sizeof(clientAddr);
    	  sClient=accept(cmSocket->_localAddr.socket,(struct sockaddr *)&clientAddr,(os_socklen_t *)&clientAddrLen);
    	  if (sClient <= 0)continue;
       	else if(freeList!=freeList->next)
      	{ TTcpClients *newpeer=freeList->next;
      	  BINODE_REMOVE(newpeer,prev,next);
      	  BINODE_INSERT(newpeer,activeList,next,prev);
      	  newpeer->addr.socket=sClient;
      	  newpeer->addr.ip=clientAddr.sin_addr.s_addr;
      	  newpeer->addr.port=ntohs(clientAddr.sin_port);
      	  newpeer->timeout=now_time+TCP_CONNECTION_TIMEOUT_S;
      	  FD_SET(sClient,&fdset_connects);
    	    if(sClient>maxfd)maxfd=sClient;
    	    printf("new connection client[%d] %s:%d\r\n", sClient,inet_ntoa(clientAddr.sin_addr),newpeer->addr.port);
       	  if(cmSocket->OnClientChange)cmSocket->OnClientChange(cmSocket,sClient,TRUE);
      	}
      	else
    	  { printf("max connections arrive!\n");
          send(sClient, "bye", 4, 0);
          os_closesocket(sClient);
          continue;
    	  }
    	  printf("accepted=%d\r\n",sClient);
        }
      }
      else if(stat==0)
      { printf("select timeout!\r\n");
      	continue;
      }
      else
      {	printf("error:select fail!\r\n");
      	break;
      }
    }
    peer=activeList->next;
    while(peer!=activeList)
    { TTcpClients *delNode=peer;
      os_closesocket(peer->addr.socket);
      peer=peer->next;
      BINODE_REMOVE(delNode,next,prev)
      BINODE_INSERT(delNode,freeList,next,prev)
    }
    BINODE_ISOLATE(activeList,prev,next);
  }
  else
  { while(cmSocket->_socket_recv_thread)
    { int stat;
      struct timeval wt=waitimeout;
      fd_set read_flags;
      FD_ZERO(&read_flags);
      FD_SET(maxfd,&read_flags);
      stat=select(maxfd+1, &read_flags,NULL,NULL,&wt);//waitd会被改变
      if(stat>0)
      { if(FD_ISSET(maxfd, &read_flags))
    	  { if(cmSocket->OnRecv((HAND)cmSocket,&cmSocket->_localAddr)<=0)
   	      { printf("client[%d] close\r\n", maxfd);
            break;
    	    }
    	  }
      }
      else if(stat==0)
      { //printf("select timeout!\r\n");
      	continue;
      }
      else
      {	printf("error:select fail!\r\n");
      	break;
      }
    }
  }
  if(cmSocket->_socket_recv_thread) //判断是否是己方主动关闭的
  { //非主动关闭情况（被对方或其他情况关闭的）
    os_closesocket(cmSocket->_localAddr.socket);
    cmSocket->_localAddr.socket=0;
  }
  else //己方通过SOCKET_Close()主动关闭的情况
  { cmSocket->_localAddr.socket=0;
    os_releaseSemphore(cmSocket->_thread_closed_sem);
  }
  if(cmSocket->OnClose)cmSocket->OnClose(cmSocket);

  return NULL;
}

static BOOL _SOCKET_Switch(TcmSocket *cmSocket,BOOL OnOff)
{ if(!OnOff)
  { if(cmSocket->_localAddr.socket>0 && cmSocket->_socket_recv_thread)
    { cmSocket->_socket_recv_thread=0;//主动关闭标志
      os_closesocket(cmSocket->_localAddr.socket);
      os_obtainSemphore(cmSocket->_thread_closed_sem);//等待接收线程关闭
    }
    return TRUE;
  }
  if(cmSocket->_localAddr.socket>0)return TRUE;
  else
  { BOOL _opened=FALSE;
    int newSocket,socket_type,protocol_value;
    if(cmSocket->_protocol==spTcpServer||cmSocket->_protocol==spTcpClient){protocol_value=IPPROTO_TCP;socket_type=SOCK_STREAM;}
    else if(cmSocket->_protocol==spUDP){protocol_value=IPPROTO_UDP;socket_type=SOCK_DGRAM;}
    else
    { socket_type=SOCK_RAW;
      if(cmSocket->_protocol==spICMP)protocol_value=IPPROTO_ICMP;
      else if(cmSocket->_protocol==spIGMP)protocol_value==IPPROTO_IGMP;
      else protocol_value=IPPROTO_RAW;
    }
    newSocket=socket(AF_INET, socket_type, protocol_value);
    if(newSocket>0)
    { struct sockaddr_in saddr;
      /*if (!cmSocket->_zeroLocalPort)
      {	//SO_REUSEADDR BOOL 允许套接口和一个已在使用中的地址捆绑。
        int values=1;
        setsockopt(newSocket,SOL_SOCKET,SO_REUSEADDR,(const char *)&values,sizeof(int));
      }*/
      saddr.sin_family = AF_INET;
      saddr.sin_addr.s_addr =(cmSocket->_zeroLocalIP)?SOCKET_IPatoi(NULL):cmSocket->_localAddr.ip;
      saddr.sin_port = (cmSocket->_zeroLocalPort)?0:htons(cmSocket->_localAddr.port);
      memset(&saddr.sin_zero,0,8);

      if(bind(newSocket,(const struct sockaddr *)&saddr, sizeof(saddr) )!=-1)
      { if(cmSocket->_protocol==spTcpServer)
      	{ if(listen(newSocket,SOCKET_MAX_LISTEN)!=-1)_opened=TRUE;
      	}
      	else if(cmSocket->_protocol==spTcpClient || (cmSocket->_protocol==spUDP && cmSocket->_remoteAddr.ip &&  cmSocket->_remoteAddr.port))
      	{ //在UDP是无连接协议，在UDP协议下调用connect只是设定目标目机地址，此处无阻塞与阻塞区别。
          saddr.sin_family=AF_INET;
          saddr.sin_port=htons(cmSocket->_remoteAddr.port);
          saddr.sin_addr.s_addr=cmSocket->_remoteAddr.ip;
          memset(&saddr.sin_zero,0,8);
          _opened=(connect(newSocket,(const struct sockaddr *)&saddr, sizeof(saddr))==0);
        }else _opened=TRUE;
      }
      if(_opened)
      { void *_SOCKET_RecvProc(void *);
      	if(cmSocket->_zeroLocalIP || cmSocket->_zeroLocalPort)
      	{ int rt=sizeof(saddr);
     	  if(getsockname(newSocket, (struct sockaddr *)&saddr, (os_socklen_t *)&rt)!=-1)
          { if(cmSocket->_zeroLocalPort)cmSocket->_localAddr.port=ntohs(saddr.sin_port);
            if(cmSocket->_zeroLocalIP)cmSocket->_localAddr.ip=saddr.sin_addr.s_addr;
          }
        }
        os_createThread(&cmSocket->_socket_recv_thread,_SOCKET_RecvProc,cmSocket);
      }
      else
      { os_closesocket(newSocket);
        newSocket=0;
      }
    }
    cmSocket->_localAddr.socket=newSocket;
    return _opened;
  }
}

BOOL SOCKET_Open(HAND hSocket)
{ if(hSocket && os_obtainSemphore(((TcmSocket *)hSocket)->_socket_op_mutex))
  {  BOOL ret=_SOCKET_Switch((TcmSocket *)hSocket,TRUE);
     os_releaseSemphore(((TcmSocket *)hSocket)->_socket_op_mutex);
     return ret;
  }else return FALSE;
}

void SOCKET_Close(HAND hSocket)
{ if(hSocket && os_obtainSemphore(((TcmSocket *)hSocket)->_socket_op_mutex))
  {  _SOCKET_Switch((TcmSocket *)hSocket,FALSE);
     os_releaseSemphore(((TcmSocket *)hSocket)->_socket_op_mutex);
  }
}

void SOCKET_Bind(HAND hSocket,U32 localIP,U16 localPort,U32 remoteIP,U16 remotePort)
{ if(hSocket && os_obtainSemphore(((TcmSocket *)hSocket)->_socket_op_mutex))
  { BOOL _active=(((TcmSocket *)hSocket)->_localAddr.socket>0);
    if(_active)_SOCKET_Switch((TcmSocket *)hSocket,FALSE); //修改绑定地址前临时先关闭连接随后恢复
    ((TcmSocket *)hSocket)->_zeroLocalIP=(localIP==0);
    ((TcmSocket *)hSocket)->_zeroLocalPort=(localPort==0);
    ((TcmSocket *)hSocket)->_localAddr.ip=localIP;
    ((TcmSocket *)hSocket)->_localAddr.port=localPort;
    ((TcmSocket *)hSocket)->_remoteAddr.ip=remoteIP;
    ((TcmSocket *)hSocket)->_remoteAddr.port=remotePort;
    if(_active)_SOCKET_Switch((TcmSocket *)hSocket,TRUE);
    os_releaseSemphore(((TcmSocket *)hSocket)->_socket_op_mutex);
  }
}

HAND SOCKET_Create(TSocketProtocol protocol,int inBufSzie)
{ TcmSocket *newsocket=(TcmSocket *)malloc(sizeof(TcmSocket));
  if(newsocket)
  { if(protocol==spTcpServer)
    { TServerConnections *peers=(TServerConnections *)malloc(sizeof(TServerConnections));
    	newsocket->_connections=peers;
    	if(peers)
    	{ int i;
    	  TTcpClients *freelist=&peers->frees;
    	  BINODE_ISOLATE(freelist,prev,next);
    	  BINODE_ISOLATE(&peers->actives,prev,next);
    	  for(i=0;i<SOCKET_MAX_LISTEN;i++)
    	  { TTcpClients *nextnode=&peers->clients[i];
    	    BINODE_INSERT(nextnode,freelist,next,prev)
    	  }
    	  os_createSemphore(&peers->_client_close_mutex,1);/*信号量初值为1，作互斥量*/
      }else{free(newsocket);return NULL;}
    }else newsocket->_connections=NULL;
    newsocket->_inqueue=(inBufSzie)?qb_create((inBufSzie<SOCKET_INBUFFER_SIZE_DEFAULT)?SOCKET_INBUFFER_SIZE_DEFAULT:inBufSzie):NULL;
    newsocket->_protocol=protocol;
    memset(&newsocket->_localAddr,0,sizeof(TNetAddr));
    memset(&newsocket->_remoteAddr,0,sizeof(TNetAddr));
    newsocket->_tag=NULL;
    newsocket->_zeroLocalIP=0;
    newsocket->_zeroLocalPort=0;
    newsocket->_socket_recv_thread=0;    
    newsocket->OnClose=NULL;
    newsocket->OnClientChange=NULL;
    newsocket->OnRecv=_SOCKET_ReceiveToQueue;
    os_createSemphore(&newsocket->_inqueue_read_sem,0);
    os_createSemphore(&newsocket->_inqueue_read_mutex,1);
    os_createSemphore(&newsocket->_socket_op_mutex,1);
    os_createSemphore(&newsocket->_thread_closed_sem,0);
  }
  return newsocket;
}

void SOCKET_CloseTcpClient(HAND hSocket,TNetAddr *peerAddr)
{ if(hSocket && peerAddr)
  { TServerConnections *connections=((TcmSocket *)hSocket)->_connections;
    if(connections && os_obtainSemphore(connections->_client_close_mutex))
    { TTcpClients *activeList=&connections->actives;
      TTcpClients *node=activeList->next;
      while(node!=activeList)
      { if(node->addr.socket==peerAddr->socket)
        { if(node->addr.ip==peerAddr->ip && node->addr.port==peerAddr->port)
          { //tcp server执行shutdown是通知peersocket关闭，等收到peersocket关闭的消息后再调用closesocket关闭localsocket。
            //usleep(100000);
            shutdown(peerAddr->socket,2);
          }
          break;
        }else node=node->next;
      }
      os_releaseSemphore(connections->_client_close_mutex);
    }
  }
}

void SOCKET_Destroy(HAND hSocket)
{ if(hSocket)
  { TcmSocket *cmSocket=(TcmSocket *)hSocket;
    SOCKET_Close(hSocket);
    if(cmSocket->_connections)
    { os_destroySemphore(cmSocket->_connections->_client_close_mutex);
      free(cmSocket->_connections);
    }
    free(cmSocket);
    if(cmSocket->_inqueue)qb_destroy(cmSocket->_inqueue);
    os_destroySemphore(cmSocket->_inqueue_read_mutex);
    os_destroySemphore(cmSocket->_inqueue_read_sem);
    os_destroySemphore(cmSocket->_thread_closed_sem);
    os_destroySemphore(cmSocket->_socket_op_mutex);
  }
}

int SOCKET_Send(HAND hSocket,void *pData, U32 dataLen)
{ //执行send函数只是将数据放入协议栈的发送缓冲区，不会真正执行发送动作。
  //如果调用send发送送据的长度(dataLen)小于等于发送缓冲区的剩余空间大(freeLen),则返回实际放入发送缓和冲区的字节数(dataLen)，此时不涉及阻塞或非阻塞模式。
  //如果调用send发送送据的长度(dataLen)大于发送缓冲区的剩余空间大(freeLen),在非阻塞模式下，则返回实际放入发送缓和冲区的字节数(freeLen)；在阻塞模式下会自动分批将数据放入发送缓冲区，最终返回dateLen。
  //如果发送缓冲区已满，在socket阻塞模式下会等待所有数据放入发送缓冲区，而在非阻塞模式下则立即返回-1。
  //阻塞模式是流式发送的，不管多大长度的数据都能发，阻塞等待所有数据发送完才返回。
  //CentOS协议栈默认的TCP发送缓冲约为20KB，UDP发送缓冲约为64KB，可设置。
  if(hSocket)
  { int localSocket=((TcmSocket *)hSocket)->_localAddr.socket;
    if(localSocket>0)return send(localSocket,(const char *)pData,dataLen,0);
  }
  return -1;
}

BOOL SOCKET_SetSocketBuffer(HAND hSocket,int recvBufsize,int sendBufsize)
{ /* 当设置TCP套接口接收缓冲区的大小时，函数调用顺序是很重要的，因为TCP的窗口规模选项是在建立连接时用SYN与对方互换得到的。对于客户，SO_RCVBUF选项必须在connect之前设置；对于服务器，SO_RCVBUF选项必须在listen前对监听的socket进行设置。
     如果成功设置，实际设置的缓冲区大小会是设定值的2倍，可以通过相应的getsockopt来读取检验。
     若要查看socket缓冲区的默认配置，可以通过查看以下系统文件获知：
    /proc/sys/net/core/rmem_default	 socket创建时默认的读（接收)缓冲大小；
    /proc/sys/net/core/wmem_default      socket创建时默认的写（发送)缓冲大小；
    /proc/sys/net/core/rmem_max	         socket允许通过setsockopt设置的最大接收缓冲大小；
    /proc/sys/net/core/wmem_max	         socket允许通过setsockopt设置的最大发送缓冲大小；
    若要永久修改以上配置，可以编辑/etc/sysctl.conf文件，添加以下需要修改的行(修改完保存文件后通过命令/sbin/sysctl -p使之立即生效)：
    net.core.rmem_default=124928
    net.core.rmem_max=124928
    net.core.wmem_default=124928
    net.core.wmem_max=124928
  */
  if(hSocket)
  { int localSocket=((TcmSocket *)hSocket)->_localAddr.socket;
    if(localSocket>0)
    { if(recvBufsize && setsockopt(localSocket,SOL_SOCKET,SO_RCVBUF,(char*)&recvBufsize,sizeof(int))!=0) return FALSE;
      if(sendBufsize && setsockopt(localSocket,SOL_SOCKET,SO_SNDBUF,(char*)&sendBufsize,sizeof(int))!=0) return FALSE;
      return TRUE;
    }
  }
  return FALSE;
}

BOOL SOCKET_GetSocketBuffer(HAND hSocket,int *recvBufsize,int *sendBufsize)
{ if(hSocket)
  { int localSocket=((TcmSocket *)hSocket)->_localAddr.socket;
    if(localSocket>0)
    { int len=sizeof(int);
      if(recvBufsize && getsockopt(localSocket,SOL_SOCKET, SO_RCVBUF, (os_sockopt_t *)recvBufsize, (os_socklen_t *)&len)!=0)return FALSE;
      if(sendBufsize && getsockopt(localSocket,SOL_SOCKET, SO_SNDBUF, (os_sockopt_t *)sendBufsize, (os_socklen_t *)&len)!=0)return FALSE;
      return TRUE;
    }
  }
  return FALSE;
}

//参数peerAddr可以是目标主机的IP或socket，如果是IP则peerPort为非零，否则为零。
int SOCKET_SendTo(HAND hSocket,void *pData, U32 dataLen,U32 peerAddr,U16 peerPort)
{ if(hSocket)
  { int localSocket=((TcmSocket *)hSocket)->_localAddr.socket;
    if(localSocket>0 && pData && dataLen)
    { if(peerPort)
      { struct sockaddr_in saddr;
        saddr.sin_family=AF_INET;
        saddr.sin_port=htons(peerPort);
        saddr.sin_addr.s_addr=peerAddr;
        memset(&saddr.sin_zero,0,8);
        return sendto(localSocket,(const char *)pData,dataLen,0,(const struct sockaddr *)&saddr, sizeof(saddr));
      }else return send(peerAddr,(const char *)pData,dataLen,0);
    }
  }
  return -1;
}

int SOCKET_Receive(HAND hSocket,char *recvBuf, int bufSize,TNetAddr *peerAddr)
{ TcmSocket *cmSocket=(TcmSocket *)hSocket;
  if(cmSocket && cmSocket->_localAddr.socket>0)
  { int readbytes=0;
    os_obtainSemphore(cmSocket->_inqueue_read_sem);
    os_obtainSemphore(cmSocket->_inqueue_read_mutex);
    while(1)
    { TcmPacketHead packet;
      if(qb_blockRead(cmSocket->_inqueue,&packet,sizeof(TcmPacketHead)))
      { if(packet.datasize>0)
        { if(packet.addr.socket>0)
          { readbytes=(packet.datasize<=bufSize)?packet.datasize:bufSize;
            if(qb_read(cmSocket->_inqueue,recvBuf,readbytes)==readbytes)
	    { if(readbytes<packet.datasize)qb_read(cmSocket->_inqueue,NULL,packet.datasize-readbytes);
	      if(peerAddr)*peerAddr=packet.addr;
	      break;
	    }else readbytes=0;
	  }
          else
          { qb_read(cmSocket->_inqueue,NULL,packet.datasize);
          }
        }
      }else break;
    }
    os_releaseSemphore(cmSocket->_inqueue_read_mutex);
    return readbytes;
  }else return 0;  
}

void SOCKET_SetTag(HAND hSocket,HAND tag)
{ if(hSocket)((TcmSocket *)hSocket)->_tag=tag;
}

HAND SOCKET_GetTag(HAND hSocket)
{ return (hSocket)?((TcmSocket *)hSocket)->_tag:NULL;
}

void SOCKET_SetEvents(HAND hSocket,TSocketRecvEvent onReceive,TSocketCloseEvent onClose,TSocketClientEvent onClientChange)
{ if(hSocket)
  { ((TcmSocket *)hSocket)->OnRecv=(onReceive)?onReceive:_SOCKET_ReceiveToQueue;
    ((TcmSocket *)hSocket)->OnClientChange=onClientChange;
    ((TcmSocket *)hSocket)->OnClose=onClose;
  }
}

BOOL SOCKET_CheckIPStr(char *ipstr)
{ if(ipstr && *ipstr)
  { if(inet_addr(ipstr)!=(U32)-1)return TRUE;
  }
  return FALSE;
}

BOOL SOCKET_IPitoa(U32 ip_data,char *ipstr)
{ if(!ip_data)ip_data=SOCKET_IPatoi(NULL);
  strcpy(ipstr,inet_ntoa(*((struct in_addr *)&ip_data)));
  //在64位Linux环境下必须加入头文件<arpa/inet.h>，否则调用inet_ntoa会产生Segmentation fault错误。
  return TRUE;
}

// this routine simply converts the address into an internet ip
U32 SOCKET_IPatoi(char *host_name)
{ U32 ip_data=-1;
  if(host_name && *host_name)
  { ip_data=inet_addr(host_name);
    if(ip_data==(U32)-1)
    { struct hostent *host_ent=gethostbyname(host_name);
      if(host_ent) ip_data=*(U32 *)host_ent->h_addr;
      #if __BORLANDC__
      else if(WSAGetLastError()==WSANOTINITIALISED)MessageBox(NULL,"Either the application has not called WSAStartup, or WSAStartup failed!","warning!",MB_OK);
      #endif
    }
  }
  else
  { char localhostbuf[64];
    if(gethostname(localhostbuf,64)==0)
    { struct hostent *host_ent=gethostbyname(localhostbuf);
      if(host_ent) ip_data=*(U32 *)host_ent->h_addr;
    }
  }
  return ip_data;
}
static char *SOCKET_UrlSplit(char *url,int *port,char *hostnamebuf,int bufsize)
{  *port=80;//default port
   while(*url && *url!=':')url++;
   if(url[0]==':' && url[1]=='/' && url[2]=='/')
   { int i=0;
     url+=3;
     while(i<bufsize)
     { if(*url==':')
       { url++;
         *port=0;
	 while(*url>='0' && *url<='9'){*port=*port*10+(int)(*url-'0');url++;}
       }
       else if(*url=='/' || *url=='\0') {hostnamebuf[i]='\0';return url;}
       else hostnamebuf[i++]=*url++;
     }
   }
   return NULL;
}

static int SOCKET_ParseHttpHeap(char *header,int *pHeaderLength,int *pContentLength)
{ //返回httpcode,若httpcode返会0表示httpheader尚未接收完。
  //仅在httpcode返回200时，headerLength和contentLength方有效。
  if(header[0]=='H' && header[1]=='T' && header[2]=='T' && header[3]=='P')
  { int httpcode=0;
    char *p=header+4;
    while(*p && *p!=32)p++;
    while(*p==32)p++;
    while(*p>='0'&&*p<='9')httpcode=httpcode*10+(int)(*p++-'0');
    if(httpcode==200)
    { int contentLength;
      char *q=stristr(p,"Content-Length:");
      if(q)
      { contentLength=0; 
        q+=15;
      	while(*q==32)q++;
      	while(*q>='0'&&*q<='9')contentLength=contentLength*10+(int)(*q++-'0');
        p=strstr(q,"\r\n\r\n");
      }
      else
      { contentLength=-1;//suspend
        p=strstr(p,"\r\n\r\n");
      }
      if(p)
      { *pContentLength=contentLength;
        *pHeaderLength=(int)(p-header+4);
        return httpcode;
      }
    }
    else if(httpcode==302 || httpcode==301)
    { p=stristr(p,"location:");
      if(p)
      { p+=9;
      	while(*p==32)p++;
      	*pHeaderLength=p-header; //url偏移量
        p=strstr(p,"\r\n");
        if(p)
        { *pContentLength=p-header-*pHeaderLength;//url长度
          return httpcode;
    	}
      }
    }
    else return -1;
  }else return -1;
  return 0;	//header不完整，还待进一步下载。
}

static void socket_nonblock(int socket,BOOL bNonblock)
{ if(socket>0){
#if __BORLANDC__
    unsigned long blockMode=bNonblock;
    ioctlsocket(socket, FIONBIO, &blockMode);
#else
    U32 flags=fcntl(socket,F_GETFL,0); // Get socket flags
    if(flags&O_NONBLOCK)
    { if(bNonblock)return;
      else flags&=~O_NONBLOCK;
    }
    else
    { if(bNonblock)flags|=O_NONBLOCK;
      else return;
    }
    fcntl(socket,F_SETFL,flags);
#endif
  }
}

//POST表单的最后一个字段值会附加\r\n符，需要接收方注意。
int SOCKET_HttpPost(char *URL,void *formData,int formSize,char *responseBuffer,int buffersize,int sTimeout)
{ struct sockaddr_in servaddr;
  #define HTTP_BUFFER_SIZE 1024
  #define MAXLEN_HOST_NAME   64
  U32 start_time=time(NULL);
  int sockfd,ret,svrport,readbytes,httpcode,remains,jmp=0;
  char httpbuf[HTTP_BUFFER_SIZE+1],strhostname[MAXLEN_HOST_NAME],*pbuf,*scriptName;
  label_start_post:
  readbytes=0;
  httpcode=0;
  remains=HTTP_BUFFER_SIZE;
  pbuf=httpbuf;
  if((scriptName=SOCKET_UrlSplit(URL,&svrport,strhostname,MAXLEN_HOST_NAME))!=NULL)
  { if((servaddr.sin_addr.s_addr=SOCKET_IPatoi(strhostname))!=(U32)-1)
  	{ servaddr.sin_family = AF_INET;
      servaddr.sin_port = htons(svrport);
      memset(&servaddr.sin_zero,0,8);
    }else return -1;
  }else return -1;
  if((sockfd = socket(AF_INET, SOCK_STREAM, 0))<=0)return -1;
  else socket_nonblock(sockfd,TRUE); //非阻塞连接

  if (connect(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) ==-1)
  { struct timeval tm;
    fd_set set;
    int error=-1, len = sizeof(int);
    tm.tv_sec = sTimeout;
    tm.tv_usec = 0;
    FD_ZERO(&set);
    FD_SET(sockfd, &set);
    if( select(sockfd+1, NULL, &set, NULL, &tm) > 0)
    { getsockopt(sockfd, SOL_SOCKET, SO_ERROR, (os_sockopt_t *)&error, (os_socklen_t *)&len);
      if(error) goto label_connect_error;
    }
    else
    { label_connect_error:
      printf("connect error!\n");
      os_closesocket(sockfd);
      return -1;
    }
  }
  socket_nonblock(sockfd,FALSE);  //设置回阻塞模式
  sTimeout-=(time(NULL)-start_time);//根据连接消耗的时间来计算剩余时间。
  if(sTimeout<=0)return -1;
  
  if(!formData) //http get
  { //http的请求头或响应头都是以/r/n/r/n为结束标志；
    //后面Keep-Alive改成close，先验证通过contentLength完成结束，再验证通过Keep-Alive改成close完成结束;
    ret=sprintf(httpbuf,"GET %s HTTP/1.0\r\nAccept: */*\r\nAccept-Language: cn\r\nUser-Agent: Mozilla/4.0\r\nHost: %s\r\nConnection: close\r\nCache-Control: no-cache\r\n\r\n",(scriptName[0])?scriptName:"/",strhostname);
    ret=send(sockfd,httpbuf,ret,0);
  }
  else  //http post
  { if(formSize<=0)formSize=strlen((char *)formData);
    ret=sprintf(httpbuf,"POST %s HTTP/1.0\r\nHost: %s\r\nAccept-Encoding: gzip,deflate\r\nAccept-Language: cn\r\nAccept: */*\r\nContent-Type: application/x-www-form-urlencoded\r\nContent-Length: %d\n\n",(scriptName[0])?scriptName:"/",strhostname,formSize);
    if(ret+formSize+10<HTTP_BUFFER_SIZE)
    {  memcpy(httpbuf+ret,formData,formSize);
       ret+=formSize;
       ret+=sprintf(httpbuf+ret,"\r\n\r\n");
       ret = send(sockfd,httpbuf,ret,0);
    }
    else
    {  ret = send(sockfd,httpbuf,ret,0);
       if(ret>0)
       { ret=send(sockfd,(const char *)formData,formSize,0);
         if(ret>0)ret=send(sockfd,"\r\n\r\n",4,0);
       }
    }
  }
  if (ret<0)
  { printf("send error %d，Error message'%s'\n",errno, strerror(errno));
    os_closesocket(sockfd);
    return -1;
  }
  while(1)
  { fd_set sockfd_set;
    struct timeval tv_timeout;
    tv_timeout.tv_sec=sTimeout;
    tv_timeout.tv_usec=0;
    FD_ZERO(&sockfd_set);
    FD_SET(sockfd, &sockfd_set);
    ret=select(sockfd+1, &sockfd_set, NULL, NULL, &tv_timeout);
    if(ret>0)
    { ret=recv(sockfd, pbuf+readbytes,remains,0);
      if(ret>0)
      { readbytes+=ret;
      	remains-=ret;
      	if(!httpcode)
      	{ int contentLength,headerLength;
      	  pbuf[readbytes]='\0';
      	  httpcode=SOCKET_ParseHttpHeap(pbuf,&headerLength,&contentLength);
      	  if(httpcode==200)
      	  { readbytes-=headerLength;
      	    if(readbytes>buffersize)readbytes=buffersize;
      	    if(responseBuffer) memcpy(responseBuffer,pbuf+headerLength,readbytes);
      	    else
      	    { os_closesocket(sockfd);
      	    	return contentLength;
      	    }
     	      if(contentLength>0 && contentLength<buffersize)remains=contentLength-readbytes;
     	      else remains=buffersize-readbytes;
      	    pbuf=responseBuffer;
      	  }
      	  else if( (httpcode==301 || httpcode==302) && ++jmp<3 )
      	  { //支持301/302任意转向重定位，转向时执行http-get。
      	    URL=pbuf+headerLength;
      	    URL[contentLength]='\0';
      	    os_closesocket(sockfd);
      	    formData=NULL;//去除formData参数
      	    goto label_start_post;//to perform http-get
      	  }
      	  else if(httpcode!=0)break;
      	}
      	if(remains<=0)break;
      }else break;
    }else break;
  }
  os_closesocket(sockfd);
  if(httpcode==200)
  { if(readbytes<buffersize)responseBuffer[readbytes]='\0';
    return readbytes;
  }
  else return	0;
}

int SOCKET_HttpGet(char *URL,char *responseBuffer,int buffersize,int sTimeout)
{ return SOCKET_HttpPost(URL,NULL,0,responseBuffer,buffersize,sTimeout);
}

#if (__linux__)
U32   os_msRunTime(void)
{ struct timeval tv;
  gettimeofday(&tv,NULL);
  return ((U32)tv.tv_usec/1000)+((U32)tv.tv_sec*1000);
}
#endif

#ifdef _mysql_h
//---------------------------------------------------------------------------
//注意：同一个连接(conn)下的多个查询是线程不安全的，多线程查询时要注意加锁。
static pthread_mutex_t db_mutex_lock; 
#define SIZE_SQL_FORMAT_BUFFER     640
static MYSQL *_dbconn=NULL;
//---------------------------------------------------------------------------
static void db_exception(void)
{ int _errno=mysql_errno(_dbconn);
  if (_errno)printf("Connection error %d: %s", _errno, mysql_error(_dbconn));
  abort();//force core dump 
}

void db_open(char *dbhost,char *dbname,char *dbuser,char *password)
{ _dbconn=mysql_init(NULL);
  if(mysql_real_connect(_dbconn,dbhost,dbuser,password,dbname,0,NULL,0))
  { mysql_query(_dbconn,"set names utf8");
   	pthread_mutexattr_t attr; 
  	pthread_mutexattr_init(&attr); 
    pthread_mutexattr_settype(&attr,PTHREAD_MUTEX_RECURSIVE_NP); 
  	pthread_mutex_init(&db_mutex_lock,&attr);
  	pthread_mutexattr_destroy(&attr); 
  }	
  else
  {	printf("Error:can not connect to mysql!\r\n");
  	db_exception();
  }
}

MYSQL *db_conn(void)
{ return _dbconn;
}

void db_close(void)
{ if(_dbconn){mysql_close(_dbconn);_dbconn=NULL;}
}

void db_lock(BOOL lock)
{ if(lock)pthread_mutex_lock(&db_mutex_lock);else pthread_mutex_unlock(&db_mutex_lock);
}

BOOL db_checkSQL(char *sql)
{ if(sql){while(*sql){if(*sql=='\'') return FALSE;else sql++;}}
	return TRUE;
}

char *db_filterSQL(char *text)
{ if(text){{char *p=text;while(*p){if(*p=='\'') *p='`';else p++;}}}
	return text;
}

MYSQL_RES *db_query(char *sql)
{ pthread_mutex_lock(&db_mutex_lock);
  if(mysql_query(_dbconn,sql))
  { db_exception();
  	return NULL;
  }
  else
  {	MYSQL_RES *ret=((sql[0]|0x20)=='s')?mysql_store_result(_dbconn):(MYSQL_RES *)mysql_affected_rows(_dbconn);
  	pthread_mutex_unlock(&db_mutex_lock);
  	return ret;
  }
}

MYSQL_RES *db_queryf(const char *format, ...)
{	char sql_buf[SIZE_SQL_FORMAT_BUFFER+1];
	va_list arg_ptr;
  va_start(arg_ptr, format);
  if(vsprintf(sql_buf, format, arg_ptr)>SIZE_SQL_FORMAT_BUFFER)
  { puts("[ERROR]#################db_queryf###############out of range!");
  	puts(sql_buf);
       abort();//force core dump  
  }
  va_end(arg_ptr);
  return db_query(sql_buf);
}
//---------------------------------------------------------------------------
#endif //_mysql_h
