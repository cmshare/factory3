//
//  hybServKit.h
//  
//  This file is in the public domain.
//  Originally created by jojo.luo in Q3 2011.
//
//
//---------------------------------------------------------------------------
#ifndef HYBRID_SERVER_KIT_H
#define HYBRID_SERVER_KIT_H
//---------------------------------------------------------------------------
typedef struct{void *owner;TNetAddr addr;char data[0];}THskPacket;
#define BINODE_INSERT(node,linklist,p_prev,p_next)  {(node)->p_prev=(linklist)->p_prev;(node)->p_prev->p_next=node;(node)->p_next=linklist;(linklist)->p_prev=node;}
#define BINODE_ISOLATE(node,p_prev,p_next) {(node)->p_next=node;(node)->p_prev=node;}
#define BINODE_REMOVE(node,p_prev,p_next) {(node)->p_next->p_prev=(node)->p_prev;(node)->p_prev->p_next=(node)->p_next;}
//---------------------------------------------------------------------------
int    hsk_init(int localTcpPort,int localUdpPort,int maxUdpDatagramSize,int inQueueSize);
int    hsk_append_udp(int udpPortArray[],int udpCount,int inQueueSize);
int    hsk_getUdpSocket(void);
int    hsk_getTcpSocket(void);
void   hsk_sendData(void *data,int datalen,TNetAddr *peerAddr);
int    hsk_readData(void *recvBuf, int bufSize,TNetAddr *peerAddr);
int    hsk_readDatagram(void *recvBuf, int bufSize,TNetAddr *peerAddr);
void   hsk_releaseTcpPacket(THskPacket *packet,BOOL isHeapMsg,BOOL isShotConnect);
THskPacket *hsk_assemble(TNetAddr *peerAddr,void *sliceData,int sliceLen,int predictPacketSize);
int    hsk_httpPost(char *URL,void *formData,int formSize,char *responseBuffer,int buffersize,int sTimeout);
int    hsk_httpGet(char *URL,char *responseBuffer,int buffersize,int sTimeout);
BOOL   hsk_isTcpAddr(TNetAddr *addr);
void   hsk_shuttownTcpClient(TNetAddr *peerAddr);
char  *str_xmlSeek(char *xmlbuf,char *key,int *len);
char  *str_itemSeek(char *itemlist,char *item,char splitter);
int    str_lenOfUTF8(char *str);
int    str_fromTime(char *strTime,char *format,time_t timestamp);
time_t str_toTime(char *strTime,char *format);
int    str_bytesToHex(void *srcData,int dataLen,char *hexBuf,int bufSize,char splitter);
char  *str_keySeek(char *keyList,char *key,char splitter);
int    tm_getLocalHour(time_t timestamp);//get local hour from unix timestamp;
//---------------------------------------------------------------------------
enum  {DTMR_LOCK=0x80000000U,DTMR_ENABLE=0x40000000U,DTMR_CYCLE=0x20000000U,DTMR_TIMEOUT_DELETE=0x10000000U,DTMR_OVERRIDE=0x08000000U,DTMR_EXIST=0x00000001U,DTMR_TIMEOUT_STOP=0,DTMR_DISABLE=0,DTMR_NOVERRIDE=0,DTMR_UNLOCK_DELETE=-1,DTMR_UNLOCK_DISABLE=-2};
typedef void (*DTMR_TimeoutEvent)(HAND,void *,U32 *,char *);
HAND  dtmr_create(int hashLen,U32 msHoldTime,DTMR_TimeoutEvent OnTimeout,char *taskName);
void  dtmr_destroy(HAND dtimer);
void *dtmr_add(HAND dtimer,U32 nodeIDL,U32 nodeIDH,char *nodeName,void *nodeData,U32 dataSize,U32 msLifeTime,U32 *options);
void *dtmr_find(HAND dtimer,U32 nodeIDL,U32 nodeIDH,char *nodeName,BOOL addLock);
void *dtmr_findByID(HAND dtimer,U32 nodeIDL,U32 nodeIDH,BOOL addLock);
void *dtmr_findById(HAND dtimer,U32 nodeIDL,BOOL addLock);
void *dtmr_findByName(HAND dtimer,char *nodeName,BOOL addLock);
void *dtmr_findByData(HAND dtimer,U32 nodeIDL,U32 nodeIDH,void *nodeData,U32 dataSize,int dataOffset,BOOL addLock);
void  dtmr_reload(void *dnode);
BOOL  dtmr_update(void *dnode,U32 msUpdateLifeTime,U32 options);
BOOL  dtmr_lock(void *dnode);
void  dtmr_unlock(void *dnode,U32 msUpdateLifeTime);
char *dtmr_getName(void *dnode);
int   dtmr_getSize(void *dnode);
U32   dtmr_getMode(void *dnode);
void  dtmr_delete(void *dnode);
void  dtmr_test(HAND dtmr);
//---------------------------------------------------------------------------	
MYSQL *db_conn(void);
void   db_open(char *dbhost,char *dbname,char *dbuser,char *password);
void   db_close(void);
void   db_lock(BOOL lock);
BOOL   db_checkSQL(char *sql);
char  *db_filterSQL(char *text);
MYSQL_RES *db_query(char *sql);
MYSQL_RES *db_queryf(const char *format, ...);
//---------------------------------------------------------------------------
//TMailBox
//---------------------------------------------------------------------------
void   mb_create(int queue_size);
void   mb_destroy(void);
int    mb_post(void *msgData,int msgLen);
int    mb_receive(void *msgBuf,int bufSize);
//---------------------------------------------------------------------------		
//Others
//---------------------------------------------------------------------------		
void    async_free(void *mem,int msDelay);
void    async_exec(void (*asyncTask)(void *),void *param,int msDelay);
//---------------------------------------------------------------------------		
#endif
