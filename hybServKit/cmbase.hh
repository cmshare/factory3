#ifndef _CM_BASE_H
#define _CM_BASE_H
//---------------------------------------------------------------------------
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <time.h>
//---------------------------------------------------------------------------
typedef int               BOOL,S32;
typedef short             S16;
typedef unsigned int      U32;
typedef unsigned short    U16;
typedef unsigned char     U8;
typedef void *            HAND;
#if (_MSC_VER  || __BORLANDC__)     /*detect operation system type*/
//---------------------------------------------------------------------------
#include <winsock.h>
typedef __int64            INT64;
typedef unsigned __int64   UINT64;
#define wsa_startup()                                   do{WSADATA wsaD;WORD SOCKET_VERSION=MAKEWORD(2,2);WSAStartup(SOCKET_VERSION,&wsaD);}while(0)
#define os_createThread(pTaskHandle,thread_proc,param)  (*(pTaskHandle)=CreateThread(0,0,(DWORD WINAPI (*)(LPVOID))thread_proc,param,0,NULL))
#define os_createSemphore(pSemHandle,initialValue)      *(pSemHandle)=CreateSemaphore(NULL,initialValue,65535,NULL)
#define os_obtainSemphore(semHandle)                    (WaitForSingleObject((HANDLE)semHandle,INFINITE)==WAIT_OBJECT_0)
#define os_tryObtainSemphore(semHandle)                 (WaitForSingleObject((HANDLE)semHandle,0)==WAIT_OBJECT_0)
#define os_waitSemphore(semHandle,msWaitTime)           WaitForSingleObject((HANDLE)semHandle,msWaitTime)
#define os_releaseSemphore(semHandle)                   ReleaseSemaphore((HANDLE)semHandle,1,NULL)
#define os_destroySemphore(semHandle)                   CloseHandle((HANDLE)semHandle)
#define os_closesocket(s)                               closesocket(s)
#define os_sockopt_t                                    char
#define os_socklen_t                                    int
#define os_msRunTime                                   GetTickCount
//---------------------------------------------------------------------------
#elif (__linux__)// && __x86_64__)
//---------------------------------------------------------------------------
#include <sys/time.h>
#include <pthread.h>
#include <semaphore.h>
#include <signal.h>
#include <errno.h>      //错误号定义
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <fcntl.h>      //文件控制定义
#include <unistd.h>     //Unix标准函数定义
#include <mysql/mysql.h>//mysql database will introduce macro _mysql_h
typedef long          INT64;
typedef unsigned long UINT64;
U32     os_msRunTime(void);
#define os_createThread(pTaskHandle,thread_proc,param)  (pthread_create((pthread_t *)pTaskHandle,NULL,thread_proc,param)==0)
#define os_createSemphore(pSemHandle,initialValue)      do{*(pSemHandle)=malloc(sizeof(sem_t));sem_init((sem_t *)(*(pSemHandle)),0,initialValue);}while(0)
#define os_obtainSemphore(semHandle)                    (sem_wait((sem_t *)semHandle)==0)
#define os_tryObtainSemphore(semHandle)                 (sem_trywait((sem_t *)semHandle)==0)
#define os_waitSemphore(semHandle,msWaitTime)           do{struct timeval _waitimeout;U32 part_msec=msWaitTime%1000;gettimeofday(&_waitimeout,NULL);_waitimeout.tv_sec+=(msWaitTime/1000);if(part_msec){_waitimeout.tv_usec+=(part_msec*1000);if(_waitimeout.tv_usec>=1000000){_waitimeout.tv_usec-=1000000;_waitimeout.tv_sec++;}} sem_timedwait((sem_t *)semHandle,(const struct timespec *)&_waitimeout);}while(0)
#define os_releaseSemphore(semHandle)                   sem_post((sem_t *)semHandle)
#define os_destroySemphore(semHandle)                   do{sem_destroy((sem_t *)semHandle);free(semHandle);}while(0)
#define os_closesocket(s)                               close(s)
#define os_sockopt_t                                    void
#define os_socklen_t                                    socklen_t
//---------------------------------------------------------------------------
#endif
//------------------------------------------------------------------------------------------
#define FORCE_WORD(variable) (*(unsigned short *)&(variable))
#define FORCE_DWORD(variable) (*(unsigned long *)&(variable))
#define GET_BIT(data,index)   ((unsigned char)((data)>>(index))&0x1)
#define SET_BIT(data,index,state)  {if(((unsigned char)((data)>>(index))&0x1)!=(state)){if(state)data |= (1<<(index));else data &= (~(1<<(index)));}}
#define T_PARENT_NODE(TParent,ChildName,IChild) (TParent *)((char *)(IChild)-(char *)(&((TParent *)0)->ChildName))
#define T_NODE_OFFSET(TStruct,NodeName)  ((char *)(&((TStruct *)0)->NodeName)-(char *)0)

#ifndef TRUE
  #define TRUE             -1
#endif
#ifndef FALSE
  #define FALSE             0
#endif

#ifndef MAX
  #define MAX(a,b)            (((a) > (b)) ? (a) : (b))
#endif

#ifndef MIN
  #define MIN(a,b)            (((a) < (b)) ? (a) : (b))
#endif

#ifndef SWAP
  #define SWAP(a,b)            {a^=b;b^=a;a^=b;}
#endif

#ifndef ABS
  #define ABS(a)               (((a)>=0)?(a):-(a))
#endif

#ifndef MAKEWORD
  #define MAKEWORD(a, b)      ((WORD)(((BYTE)(a)) | ((WORD)((BYTE)(b))) << 8))
#endif

#ifndef MAKELONG
  #define MAKELONG(a, b)      ((long)(((WORD)(a)) | ((DWORD)((WORD)(b))) << 16))
#endif

#ifndef LOWORD
  #define LOWORD(L)           ((WORD)(L))
#endif

#ifndef HIWORD
  #define HIWORD(L)           ((WORD)((DWORD)(L)>>16))
#endif

#ifndef LOBYTE
  #define LOBYTE(w)           ((BYTE)(w))
#endif

#ifndef HIBYTE
  #define HIBYTE(w)           ((BYTE)((w)>>8))
#endif
//---------------------------------------------------------------------------
#define BITS_MASK(bitwidth)   ((1L<<(bitwidth))-1)
//---------------------------------------------------------------------------
//p_reve在前就是插在linklist的p_prev方向，否则插到linklist的p_next方向
#define BINODE_INSERT(node,linklist,p_prev,p_next)  {(node)->p_prev=(linklist)->p_prev;(node)->p_prev->p_next=node;(node)->p_next=linklist;(linklist)->p_prev=node;}
#define BINODE_ISOLATE(node,p_prev,p_next) {(node)->p_next=node;(node)->p_prev=node;}
#define BINODE_REMOVE(node,p_prev,p_next) {(node)->p_next->p_prev=(node)->p_prev;(node)->p_prev->p_next=(node)->p_next;}
//---------------------------------------------------------------------------
typedef struct{S16 socket;U16 port;U32 ip;}TNetAddr;
typedef struct t_binode_link{struct t_binode_link *prev,*next;}TBinodeLink;
//---------------------------------------------------------------------------
// String & memory buffer
//---------------------------------------------------------------
BOOL   str_isNumeric(char *data);
int    str_atoi(char *value);
int    str_htoi(char *value);
int    str_itoa(int value, char *str);
double str_atof(char *p);
int    str_ftoa(float fdata,int digit,char *strfloat);//float_to_str
int    str_bytesToHex(void *srcData,int dataLen,char *hexBuf,int bufSize,char splitter);
int    str_hexToBytes(char *strHex,void *dataBuf,int bufSize);
int    str_hexToBin(char* strHex,char *binBuf,int bufSize);
int    str_hexToDec(char* strHex,char *decBuf,int bufSize);
int    str_replace(char *pText,char *src,char *des);
char  *stristr(char *src,char *obj); //字符串搜索（忽略大小写）
int    str_lenOfUTF8(char *str);//计算str字符数目
char  *str_xmlSeek(char *xmlbuf,char *key,int *len);
void  *mem_search(void *srcMem,int memSize,void *desData,int dataSize);
void   mem_reverse(void *p_buf,int dLen);
int    str_fromTime(char *strTime,char *format,time_t timestamp);
time_t str_toTime(char *strTime,char *format);
int    tm_getLocalHour(time_t timestamp);//get local hour from unix timestamp;
//---------------------------------------------------------------------------
// QueueBuffer
//---------------------------------------------------------------------------
HAND qb_create(int size);
void   qb_destroy(HAND qb);
void   qb_clear(HAND qb);
int    qb_size(HAND qb);
int    qb_isEmpty(HAND qb);
int    qb_usedSize(HAND qb);
int    qb_freeSize(HAND qb);
void  *qb_usedSpace(HAND qb,int *size1, int *size2);
void  *qb_freeSpace(HAND qb,int *size1,int *size2);
int    qb_write(HAND qb,void *buf, int len);
int    qb_read(HAND qb,void *buf, int len);
int    qb_peek(HAND qb,void *buf,int len);
int    qb_blockRead(HAND qb,void *buf,int len);
int    qb_blockWrite(HAND qb,void *buf,int len);
//int    qb_blockReadPrefetch(HAND qb,void **pbuf,int minimum);
//int    qb_blockWritePrefetch(HAND qb,void **pbuf,int minimum);
//---------------------------------------------------------------------------
//TMailBox
//---------------------------------------------------------------------------
void   mb_create(int queue_size);
void   mb_destroy(void);
int    mb_post(void *msgData,int msgLen);
int    mb_receive(void *msgBuf,int bufSize);
//---------------------------------------------------------------------------
// TDateTimer
//---------------------------------------------------------------------------
enum  {DTMR_LOCK=0x80000000U,DTMR_NOVERRIDE=0x40000000U,DTMR_FOREVER=0x20000000U,DTMR_KEEPLIFE=0x10000000U};
typedef void (*DTMR_TimeoutEvent)(HAND,void *,U32 *,char *,U32 *);
HAND  dtmr_create(int hashLen,U32 sHoldTime,DTMR_TimeoutEvent OnTimeout);
void  dtmr_destroy(HAND dtimer);
void *dtmr_add(HAND dtimer,U32 nodeIDL,U32 nodeIDH,char *nodeName,void *nodeData,U32 nodeSize,U32 sLifeTime);
void *dtmr_find(HAND dtimer,U32 nodeIDL,U32 nodeIDH,char *nodeName,U32 sUpdateLifeTime);
void *dtmr_find2(HAND dtimer,U32 nodeIDL,U32 nodeIDH,void *nodeData,U32 nodeSize,int extraOffset,U32 sUpdateLifeTime);
void  dtmr_update(void *dnode,U32 sUpdateLifeTime);
int   dtmr_getOverrideCount(void *dnode);
char *dtmr_getName(void *dnode);
int   dtmr_getSize(void *dnode);
void  dtmr_unlock(void *dnode,U32 sUpdateLifeTime);
void  dtmr_remove(void *dnode);
//---------------------------------------------------------------------------
// TcmSocket
//---------------------------------------------------------------------------
#define TCP_CONNECTION_TIMEOUT_S          100    //unit:second
#define SOCKET_MAX_DGRAMSIZE              1500
#define SOCKET_MAX_LISTEN                 FD_SETSIZE  //服务器最大并发连接数（不超过FD_SETSIZE,修改FD_SETSIZE在系统头文件中。）
typedef enum{spICMP,spIGMP,spUDP,spRawIP,spTcpServer,spTcpClient}TSocketProtocol;
typedef int  (*TSocketRecvEvent)(HAND,TNetAddr *);
typedef void (*TSocketCloseEvent)(HAND);
typedef void (*TSocketClientEvent)(HAND,int,BOOL);
HAND SOCKET_Create(TSocketProtocol protocol,int inBufSzie);
void SOCKET_Bind(HAND cmSocket,U32 localIP,U16 localPort,U32 remoteIP,U16 remotePort);
BOOL SOCKET_Open(HAND cmSocket);
void SOCKET_Close(HAND cmSocket);
void SOCKET_Destroy(HAND cmSocket);
void SOCKET_CloseTcpClient(HAND cmSocket,TNetAddr *peerAddr);
int  SOCKET_Send(HAND cmSocket,void *pData, U32 dataLen);
int  SOCKET_SendTo(HAND cmSocket,void *pData, U32 dataLen,U32 peerAddr,U16 peerPort);
int  SOCKET_Receive(HAND cmSocket,char *recvBuf, int bufSize,TNetAddr *peerAddr);
BOOL SOCKET_SetSocketBuffer(HAND cmSocket,int recvBufsize,int sendBufsize);
BOOL SOCKET_GetSocketBuffer(HAND cmSocket,int *recvBufsize,int *sendBufsize);
void SOCKET_SetEvents(HAND hSocket,TSocketRecvEvent onReceive,TSocketCloseEvent onClose,TSocketClientEvent onClientChange);
void SOCKET_SetTag(HAND hSocket,HAND tag);
HAND SOCKET_GetTag(HAND hSocket);
//------------------------------------------------------------------------------------------
U32  SOCKET_IPatoi(char *host_name);
BOOL SOCKET_IPitoa(U32 ip_data,char *ipstr);
BOOL SOCKET_CheckIPStr(char *ipstr);
int  SOCKET_HttpGet(char *URL,char *responseBuffer,int buffersize,int sTimeout);
int  SOCKET_HttpPost(char *URL,void *formData,int formSize,char *responseBuffer,int buffersize,int sTimeout);
//------------------------------------------------------------------------------------------
#endif
