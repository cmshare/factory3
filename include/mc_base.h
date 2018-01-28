//---------------------------------------------------------------------------
#ifndef _MC_BASE_H
#define _MC_BASE_H
//---------------------------------------------------------------------------
#include <sys/time.h>
#include <pthread.h>
#include <semaphore.h>
#include <signal.h>
#include <errno.h>     //错误号定义
//---------------------------------------------------------------------------
#include <netdb.h> 
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>  
#include <fcntl.h>     //文件控制定义
#include <unistd.h>    //Unix标准函数定义
//---------------------------------------------------------------------------
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
//---------------------------------------------------------------------------
typedef void *            HAND;
typedef long              S64;
typedef unsigned long     U64;
typedef int               S32,BOOL;
typedef unsigned int      U32;
typedef unsigned short    U16;
typedef unsigned char     U8;
typedef short             S16;
typedef struct{S16 socket;U16 port;U32 ip;}TNetAddr;
typedef struct t_binode_link{struct t_binode_link *prev,*next;}TBinodeLink;
#define T_NODE_OFFSET(TStruct,NodeName)  ((char *)(&((TStruct *)0)->NodeName)-(char *)0)
#define T_PARENT_NODE(TParent,ChildName,IChild) (TParent *)((char *)(IChild)-(char *)(&((TParent *)0)->ChildName))
#define TRUE             -1
#define FALSE             0
//---------------------------------------------------------------------------
#define os_createThread(pTaskHandle,thread_proc,param)  (pthread_create((pthread_t *)pTaskHandle,NULL,thread_proc,param)==0)
#define os_joinThread(pTaskHandle)                      pthread_join((pthread_t *)pTaskHandle, NULL)
#define os_createSemphore(pSemHandle,initialValue)      do{*(pSemHandle)=malloc(sizeof(sem_t));sem_init((sem_t *)(*(pSemHandle)),0,initialValue);}while(0)
#define os_obtainSemphore(semHandle)                    (sem_wait((sem_t *)semHandle)==0)
#define os_tryObtainSemphore(semHandle)                 (sem_trywait((sem_t *)semHandle)==0)
#define os_waitSemphore(semHandle,msWaitTime)           do{struct timeval _waitimeout;U32 part_msec=msWaitTime%1000;gettimeofday(&_waitimeout,NULL);_waitimeout.tv_sec+=(msWaitTime/1000);if(part_msec){_waitimeout.tv_usec+=(part_msec*1000);if(_waitimeout.tv_usec>=1000000){_waitimeout.tv_usec-=1000000;_waitimeout.tv_sec++;}} sem_timedwait((sem_t *)semHandle,(const struct timespec *)&_waitimeout);}while(0)
#define os_releaseSemphore(semHandle)                   sem_post((sem_t *)semHandle)
#define os_destroySemphore(semHandle)                   do{sem_destroy((sem_t *)semHandle);free(semHandle);}while(0)
#define os_closeSocket(s)                               close(s)
#define os_sockopt_t                                    void
#define os_socklen_t                                    socklen_t
#define os_sSleep                                       sleep                 //单位秒
#define os_msSleep(ms)                                  usleep((ms)*1000)
#define os_usSleep                                      usleep                //单位微妙秒
//---------------------------------------------------------------------------
#endif
