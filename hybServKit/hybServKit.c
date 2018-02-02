#include "cmbase.h"
//---------------------------------------------------------------------------
#define DISCRETETRANSFER_OVERTIME_MS     2000
#define DiscreteTransferIDL(addr)      ((addr)->socket|((addr)->port<<16))
#define TCP_CONNECTION_TIMEOUT_S          180    //unit:second                                                                         |typedef struct
#define SOCKET_MAX_DGRAMSIZE              1500  
#define SOCKET_MAX_LISTEN                 FD_SETSIZE  //服务器最大并发连接数（不超过FD_SETSIZE,修改FD_SETSIZE在系统头文件中。）
//---------------------------------------------------------------------------
#define s_maxTcpSliceSize 1500
static int hskUdpSocket=-1,hskTcpSocket=-1,s_maxUdpDatagramSize;
static int *hskAppendUdpSocketArry=NULL,hskAppendUdpSocketCount=0;
static HAND socket_queue_mutex,socket_read_sem;
static HAND socket_inqueue=NULL;
static HAND discreteTransferLinks=NULL;
static TNetAddr	ServerUdpAddr;
//---------------------------------------------------------------------------
static HAND udp_queue_mutex,udp_read_sem;
static HAND udp_recv_queue=NULL;
//---------------------------------------------------------------------------
#pragma pack (push,1)
//---------------------------------------------------------------------------
typedef struct
{ void *owner;
	TNetAddr addr;
	char data[0];
}THskPacket;

typedef struct
{ TNetAddr addr;
  U32   datasize;  
  char  data[0];
}TcmPacketHead;

typedef struct
{ U32 recvbytes,totalbytes;
  THskPacket packet;
}TDiscreteTransfer;

typedef struct t_tcp_client_socket
{ struct t_tcp_client_socket *next,*prev;
  TNetAddr addr;
  U32 timeout;
}TTcpClients;

typedef struct
{	HAND close_mutex;
	TTcpClients actives,frees,clients[SOCKET_MAX_LISTEN];
}TSVRConnections;

static TSVRConnections tcpConnections;
//---------------------------------------------------------------------------
#pragma pack (pop)  
//---------------------------------------------------------------------------
static int UDP_Init(int localUdpPort){
  int udpSocket=socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
  if(udpSocket>0){
    int values=1;
    setsockopt(udpSocket,SOL_SOCKET,SO_REUSEADDR,&values,sizeof(int));
    struct sockaddr_in s_addr;	
    s_addr.sin_family = AF_INET;
    s_addr.sin_addr.s_addr =0;
    s_addr.sin_port = htons(localUdpPort);
    bzero(&s_addr.sin_zero,8);
    if(bind(udpSocket,(const struct sockaddr *)&s_addr, sizeof(s_addr) )!=-1){
       return udpSocket;
    }	
  }
  if(udpSocket>0)close(udpSocket);
  return 0;  
}

static int TCP_Init(int localTcpPort){
  int tcpSocket=socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
  if(tcpSocket>0){
    int values=1;
    setsockopt(tcpSocket,SOL_SOCKET,SO_REUSEADDR,&values,sizeof(int));
    struct sockaddr_in s_addr;	
    s_addr.sin_family = AF_INET;
    s_addr.sin_addr.s_addr =0;
    s_addr.sin_port = htons(localTcpPort);
    bzero(&s_addr.sin_zero,8);
    if(bind(tcpSocket,(const struct sockaddr *)&s_addr, sizeof(s_addr) )!=-1){
      if(listen(tcpSocket,SOCKET_MAX_LISTEN)!=-1){
        int i;
    	TTcpClients *freelist=&tcpConnections.frees;
        BINODE_ISOLATE(freelist,prev,next);
    	BINODE_ISOLATE(&tcpConnections.actives,prev,next);
    	for(i=0;i<SOCKET_MAX_LISTEN;i++){
          TTcpClients *nextnode=tcpConnections.clients+i;
    	  BINODE_INSERT(nextnode,freelist,next,prev)
    	}
    	os_createSemphore(&tcpConnections.close_mutex,1);
    	return tcpSocket;
      }	
    }
  }
  if(tcpSocket>0)close(tcpSocket);  
  return 0;    	
}

//发送数据给自己
static int sendloop(void *data,int datalen)
{ int send_size=sizeof(TcmPacketHead)+datalen;
  os_obtainSemphore(socket_queue_mutex);
  while(1)
  { int block_size1,block_size2;
    TcmPacketHead *packet=(TcmPacketHead *)qb_freeSpace(socket_inqueue,&block_size1,&block_size2);
    if(block_size1>=send_size)
    { packet->addr=ServerUdpAddr;
      packet->datasize=datalen;
      memcpy(packet->data,data,datalen);
      qb_write(socket_inqueue,NULL,send_size);
      break;
    }
    else if(block_size2>=send_size)
    { //如果队列尾部没有足够的连续空间，则处理掉尾部空间，从队列头部再行分配。
      if(block_size1>=sizeof(TcmPacketHead))
      { packet->addr.socket=0;
        packet->datasize=block_size1-sizeof(TcmPacketHead);
      }
      qb_write(socket_inqueue,NULL,block_size1);
    }
    else
    { puts("####Socket inqueue is out of space!");
      os_releaseSemphore(socket_queue_mutex);	
      return 0; //队列耗尽，没有足够的空间。
    }
  }
  os_releaseSemphore(socket_queue_mutex);	
  os_releaseSemphore(socket_read_sem);
  return datalen; 
}
      
static int net_read_data(TNetAddr *peer)
{ int total_received=0;
  os_obtainSemphore(socket_queue_mutex);
  while(1)
  { int block_size1,block_size2;
    int maxDatagramSize=(peer)?s_maxTcpSliceSize:s_maxUdpDatagramSize;
    int maxDatagramWithHeader=maxDatagramSize+sizeof(TcmPacketHead);
    TcmPacketHead *packet=(TcmPacketHead *)qb_freeSpace(socket_inqueue,&block_size1,&block_size2);
    if(block_size1>=maxDatagramWithHeader)
    { int received_bytes;
      if(!peer)
      { struct sockaddr peerAddr;
      	int addrLen=sizeof(peerAddr);
      	 //对于connectionless sockets连接，只能通过recvfrom来取得PeerAddress
     	received_bytes=recvfrom( hskUdpSocket, packet->data, maxDatagramSize, MSG_DONTWAIT ,&peerAddr, (os_socklen_t *)&addrLen);
        if(received_bytes>0){
     	    packet->addr.socket=hskUdpSocket;
     	    packet->addr.ip=((struct sockaddr_in *)&peerAddr)->sin_addr.s_addr;
            packet->addr.port=ntohs(((struct sockaddr_in *)&peerAddr)->sin_port);
        }else break;
      }
      else
      {	//对于connection-oriented的sockets连接，只能通过getpeername来取得PeerAddress
     	  received_bytes=recv(peer->socket,packet->data,maxDatagramSize,MSG_DONTWAIT);
          if(received_bytes>0){
     	    packet->addr=*peer;
     	    #if 0
     	    { extern int recv_counter;
     	      if(received_bytes>0)recv_counter+=received_bytes;
     	    }
     	    #endif
          }else break;
      }		
      packet->datasize=received_bytes;
      qb_write(socket_inqueue,NULL,received_bytes+sizeof(TcmPacketHead));
      #if 0
         packet->data[received_bytes]='\0';
         Log_AppendText("###receive from socket(%d):\r\n",iSocket);
         puts(packet->data);
      #endif
      total_received+=received_bytes;
      os_releaseSemphore(socket_read_sem);	 
      if(peer && received_bytes<SOCKET_MAX_DGRAMSIZE)break;
    }
    else if(block_size2>=maxDatagramWithHeader)
    { //如果队列尾部没有足够的连续空间，则处理掉尾部空间，从队列头部再行分配。
    	if(block_size1>=sizeof(TcmPacketHead))
    	{ packet->addr.socket=0;
    	  packet->datasize=block_size1-sizeof(TcmPacketHead);
    	}
      qb_write(socket_inqueue,NULL,block_size1);
    }
    else
    { puts("##################Socket inqueue is out of space,data is abandoned!!!");
    	#define SIZE_TCP_CLRBUF    256
    	#define SIZE_UDP_CLRBUF    1
    	if(peer)
    	{ char socket_data[SIZE_TCP_CLRBUF];
    		while(recv(peer->socket,socket_data,SIZE_TCP_CLRBUF,MSG_DONTWAIT)==SIZE_TCP_CLRBUF);//清除TCP数据流
    	}
    	else
    	{ char socket_data[SIZE_UDP_CLRBUF];
    		while(recv(hskUdpSocket,&socket_data,SIZE_UDP_CLRBUF,MSG_DONTWAIT)==SIZE_UDP_CLRBUF);//如果UDP报文实际长度超过接收缓冲的长度时余下部分会被丢弃。
    	}		
    	break;//队列耗尽，没有足够的空间。
    }
  }
  os_releaseSemphore(socket_queue_mutex);	
  return total_received;
}

static void *_socket_recv_proc(void *param){
  int maxfd;
  fd_set fdset_connects;  
  struct timeval waitimeout={5,0};//5sec
  TTcpClients *activeList=&tcpConnections.actives;
  TTcpClients *freeList=&tcpConnections.frees;
  FD_ZERO(&fdset_connects);
  FD_SET(hskUdpSocket,&fdset_connects);
  FD_SET(hskTcpSocket,&fdset_connects);
  maxfd=(hskUdpSocket>hskTcpSocket)?hskUdpSocket:hskTcpSocket;
  while(1){
    struct timeval wt=waitimeout;
    fd_set read_flags=fdset_connects;
    int stat=select(maxfd+1, &read_flags,NULL,NULL,&wt);//wt会被改变
    
    if(stat>0){
      if(FD_ISSET(hskUdpSocket,&read_flags)){
        net_read_data(NULL);
       	//printf("net_read_data:%d\r\n",i);
      }
      U32 now_time=(U32)time(NULL);
      TTcpClients *peer=activeList->next;
      while(peer!=activeList){
        int sfd=peer->addr.socket;
      	if(now_time>peer->timeout){
          printf("client[%d] timeout\r\n", sfd);
      	  goto LABEL_CloseSocket;
      	}
      	else if(FD_ISSET(sfd,&read_flags)){
          if(net_read_data(&peer->addr)>0){
            peer->timeout=now_time+TCP_CONNECTION_TIMEOUT_S;
    	  }
    	  else{
   	   LABEL_CloseSocket:
           FD_CLR(sfd,&fdset_connects);
           if(maxfd==sfd)maxfd=0;
    	   if(os_obtainSemphore(tcpConnections.close_mutex)){
              TTcpClients *nextnode=peer->next;
              BINODE_REMOVE(peer,prev,next);
              BINODE_INSERT(peer,freeList,prev,next);
              peer=nextnode;
              close(sfd);
              os_releaseSemphore(tcpConnections.close_mutex);
              printf("###TCP peer client closed\r\n");
            }  
            else{
              Log_AppendText("##sem_wait(&tcpConnections.close_mutex) error!");
              exit(0);
            }
            //Log_AppendText("<<##client[%d] close\r\n", sfd);
            continue;	
     	  }
        }
        peer=peer->next;
      }
      if(maxfd==0){//need to update maxfd
        maxfd=(hskUdpSocket>hskTcpSocket)?hskUdpSocket:hskTcpSocket;
        peer=activeList->next;
        while(peer!=activeList){
          if(peer->addr.socket>maxfd)maxfd=peer->addr.socket;
          peer=peer->next;
        }	
      }	
      if(FD_ISSET(hskTcpSocket, &read_flags)){
        struct sockaddr_in clientAddr;
        int sClient,clientAddrLen=sizeof(clientAddr);	
        //Log_AppendText("listen");
        sClient=accept(hskTcpSocket,(struct sockaddr *)&clientAddr,(os_socklen_t *)&clientAddrLen);
        if (sClient <= 0)continue;
        else if(sClient<SOCKET_MAX_LISTEN && freeList!=freeList->next){
          TTcpClients *newpeer=freeList->next;
      	  BINODE_REMOVE(newpeer,prev,next);
      	  BINODE_INSERT(newpeer,activeList,next,prev);
      	  newpeer->addr.socket=sClient;
      	  newpeer->addr.ip=clientAddr.sin_addr.s_addr;
      	  newpeer->addr.port=ntohs(clientAddr.sin_port);
      	  newpeer->timeout=now_time+TCP_CONNECTION_TIMEOUT_S;
      	  FD_SET(sClient,&fdset_connects);
    	  if(sClient>maxfd)maxfd=sClient;
      	  //Log_AppendText("new connection client[%d] %s:%d\r\n", sClient,inet_ntoa(newpeer->addr.ip),newpeer->addr.port);
      	  Log_AppendText("new connection client[%d] %d\r\n", sClient,newpeer->addr.port);
        }
        else{
          Log_AppendText("max connections arrive!\n");
          send(sClient, "bye", 4, 0);
          close(sClient);
          continue;
        }
        //Log_AppendText(">>##client[%d] connected!",sClient);
      }
    }	
  }
  return NULL;
}

static int udp_read_data(int udpSocket){
  int total_received=0;
  os_obtainSemphore(udp_queue_mutex);
  while(1){
    int block_size1,block_size2;
    TcmPacketHead *packet=(TcmPacketHead *)qb_freeSpace(udp_recv_queue,&block_size1,&block_size2);
    if(block_size1>=s_maxUdpDatagramSize+sizeof(TcmPacketHead)){
       struct sockaddr peerAddr;
       int addrLen=sizeof(peerAddr);
       //对于connectionless sockets连接，只能通过recvfrom来取得PeerAddress
       int received_bytes=recvfrom(udpSocket, packet->data, s_maxUdpDatagramSize, MSG_DONTWAIT ,&peerAddr, (os_socklen_t *)&addrLen);
       if(received_bytes>0){
     	 packet->addr.socket=udpSocket;
     	 packet->addr.ip=((struct sockaddr_in *)&peerAddr)->sin_addr.s_addr;
         packet->addr.port=ntohs(((struct sockaddr_in *)&peerAddr)->sin_port);
       }else break;
       packet->datasize=received_bytes;
       qb_write(udp_recv_queue,NULL,received_bytes+sizeof(TcmPacketHead));
       #if 0
         packet->data[received_bytes]='\0';
         Log_AppendText("###receive from socket(%d):\r\n",iSocket);
         puts(packet->data);
       #endif
       total_received+=received_bytes;
       os_releaseSemphore(udp_read_sem);	 
    }
    else if(block_size2>=s_maxUdpDatagramSize+sizeof(TcmPacketHead)){
      //如果队列尾部没有足够的连续空间，则处理掉尾部空间，从队列头部再行分配。
      if(block_size1>=sizeof(TcmPacketHead)){
        packet->addr.socket=0;
        packet->datasize=block_size1-sizeof(TcmPacketHead);
      }
      qb_write(udp_recv_queue,NULL,block_size1);
    }
    else
    { puts("##################Socket inqueue is out of space,data is abandoned!!!");
    	#define SIZE_UDP_CLRBUF    1
        char socket_data[SIZE_UDP_CLRBUF];
    	while(recv(udpSocket,&socket_data,SIZE_UDP_CLRBUF,MSG_DONTWAIT)==SIZE_UDP_CLRBUF);//如果UDP报文实际长度超过接收缓冲的长度时余下部分会被丢弃。
    	break;//队列耗尽，没有足够的空间。
    }
  }
  os_releaseSemphore(udp_queue_mutex);	
  return total_received;
}

static void *_udp_socket_recv_proc(void *param){
  struct timeval waitimeout={15,0};//15sec
  int udpSocket=*(int *)param;
  int maxfd=udpSocket+1;
  fd_set read_flags;
  FD_ZERO(&read_flags);
  while(1){
    struct timeval wt=waitimeout;
    FD_SET(udpSocket,&read_flags);
    if(select(maxfd, &read_flags,NULL,NULL,&wt)>0){//wt会被改变
      if(FD_ISSET(udpSocket,&read_flags)) udp_read_data(udpSocket);
    }	
  }
  return NULL;
}
//---------------------------------------------------------------------------
int hsk_readDatagram(void *recvBuf, int bufSize,TNetAddr *peerAddr){
  int readbytes=0;
  os_obtainSemphore(udp_read_sem);
  os_obtainSemphore(udp_queue_mutex);
  while(1){
    TcmPacketHead packet;
    if(qb_blockRead(udp_recv_queue,&packet,sizeof(TcmPacketHead))){
      if(packet.datasize>0){
        if(packet.addr.socket>0){
          readbytes=(packet.datasize<=bufSize)?packet.datasize:bufSize;
	  if(qb_read(udp_recv_queue,recvBuf,readbytes)==readbytes){
            if(readbytes<packet.datasize)qb_read(udp_recv_queue,NULL,packet.datasize-readbytes);
	    if(peerAddr)*peerAddr=packet.addr;
	    break;
	  }
          else readbytes=0;  	
        }
        else{
          qb_read(udp_recv_queue,NULL,packet.datasize);
        }	 
      }	
    }
    else break;	
  }
  os_releaseSemphore(udp_queue_mutex);	  	
  return readbytes;
}
//---------------------------------------------------------------------------
void hsk_shuttownTcpClient(TNetAddr *peerAddr){
  if(peerAddr){
    int peerSocket=peerAddr->socket; 
    if(peerSocket && peerSocket!=hskUdpSocket && os_obtainSemphore(tcpConnections.close_mutex)){
      TTcpClients *activeList=&tcpConnections.actives;
      TTcpClients *node=activeList->next;
      while(node!=activeList){
        if(node->addr.socket==peerSocket){
          if(node->addr.ip==peerAddr->ip && node->addr.port==peerAddr->port){
            //tcp server执行shutdown是通知peersocket关闭，等收到peersocket关闭的消息后再调用closesocket关闭localsocket。
      	   shutdown(peerSocket,2);
            //Log_AppendText("shutdown socket:%d",peerAddr->socket);
          }
          break;
        }
        else node=node->next;
      }  
      os_releaseSemphore(tcpConnections.close_mutex);
      //if(node==activeList)Log_AppendText("client socket already closed by peer!");
    }
  }  
}

static void OnDiscreteTransferTimeout(HAND ttsk,void *node,U32 *taskid,char *taskName){
  U32 isShortConnect= (dtmr_getSize(node)==sizeof(BOOL))?*(BOOL *)node:FALSE; //默认为长连接
  if(isShortConnect){
     hsk_shuttownTcpClient((TNetAddr *)taskid);
     Log_AppendText("discrete overtime and close tcp short connection!");
  }
  else{
     Log_AppendText("discrete overtime and keep tcp long connection");
  } 
}

int hsk_httpPost(char *URL,void *formData,int formSize,char *responseBuffer,int buffersize,int sTimeout)
{ return SOCKET_HttpPost(URL,formData,formSize,responseBuffer,buffersize,sTimeout);
}

int hsk_httpGet(char *URL,char *responseBuffer,int buffersize,int sTimeout)
{ return SOCKET_HttpPost(URL,NULL,0,responseBuffer,buffersize,sTimeout);
}


int hsk_readData(void *recvBuf, int bufSize,TNetAddr *peerAddr){
  int readbytes=0;
  os_obtainSemphore(socket_read_sem);
  os_obtainSemphore(socket_queue_mutex);
  while(1){
    TcmPacketHead packet;
    if(qb_blockRead(socket_inqueue,&packet,sizeof(TcmPacketHead))){
      if(packet.datasize>0){
        if(packet.addr.socket>0){
          readbytes=(packet.datasize<=bufSize)?packet.datasize:bufSize;
	  if(qb_read(socket_inqueue,recvBuf,readbytes)==readbytes){
            if(readbytes<packet.datasize)qb_read(socket_inqueue,NULL,packet.datasize-readbytes);
	    if(peerAddr)*peerAddr=packet.addr;
	    break;
	  }
          else readbytes=0;  	
	}
	else{
          qb_read(socket_inqueue,NULL,packet.datasize);
	}	 
     }	
   }
   else break;	
 }
 os_releaseSemphore(socket_queue_mutex);	  	
 return readbytes;
}
//---------------------------------------------------------------------------
THskPacket *hsk_assemble(TNetAddr *peerAddr,void *sliceData,int sliceLen,int predictPacketSize){
   #define KEEP_ENOUGH_TIME    1000*60*60    
   TDiscreteTransfer *node=(TDiscreteTransfer *)dtmr_find(discreteTransferLinks,DiscreteTransferIDL(peerAddr),peerAddr->ip,NULL,DTMR_LOCK);
   LABEL_ASSEMBLE_BEGIN:
   if(node){
      if(dtmr_getSize(node)>sizeof(TDiscreteTransfer)){
         if(node->recvbytes<node->totalbytes){
            char *p_recv=(char *)node->packet.data+node->recvbytes;
   	    node->recvbytes+=sliceLen;
   	    if(node->recvbytes<node->totalbytes){
               memcpy(p_recv,sliceData,sliceLen);
   	    }	
   	    else{
              if(node->recvbytes>node->totalbytes){
                sliceLen=node->totalbytes-(node->recvbytes-sliceLen);
                node->recvbytes=node->totalbytes;
   	      }
   	      memcpy(p_recv,sliceData,sliceLen);
   	      dtmr_unlock(node,KEEP_ENOUGH_TIME);//Long enough time
     	      return &node->packet; 
   	    }		
   	  }
       }
       else{
         dtmr_unlock(node,0);
    	 node=NULL;
    	 goto LABEL_ASSEMBLE_BEGIN;
       }	
       dtmr_unlock(node,0);
   }
   else if(predictPacketSize>0 && predictPacketSize<=s_maxTcpSliceSize){
      const U32 dtmrOptions=DTMR_LOCK|DTMR_TIMEOUT_DELETE|DTMR_OVERRIDE;
      node=(TDiscreteTransfer *)dtmr_add(discreteTransferLinks,DiscreteTransferIDL(peerAddr),peerAddr->ip,NULL,NULL,sizeof(TDiscreteTransfer)+predictPacketSize,DISCRETETRANSFER_OVERTIME_MS,&dtmrOptions);
      if(node){
         node->packet.addr=*peerAddr;
    	 node->totalbytes=predictPacketSize;
         if(sliceLen<predictPacketSize){
           node->recvbytes=sliceLen;
           memcpy(node->packet.data,sliceData,sliceLen);
           dtmr_unlock(node,0);
         }
         else{
           node->recvbytes=predictPacketSize;
       	   memcpy(node->packet.data,sliceData,predictPacketSize);
           dtmr_unlock(node,KEEP_ENOUGH_TIME);
     	   return &node->packet; 
         }  
      }	
   }
   return NULL;	
}

//---------------------------------------------------------------------------
int hsk_getUdpSocket(void)
{ return hskUdpSocket;
}

int hsk_getTcpSocket(void)
{ return hskTcpSocket;
}

BOOL hsk_isTcpAddr(TNetAddr *addr){
  return (addr->socket && addr->socket!=hskUdpSocket);
}

#if 1
void hsk_sendData(void *data,int datalen,TNetAddr *peerAddr){
  if(peerAddr){
    int peerSocket=peerAddr->socket;
    if(peerSocket==hskUdpSocket){
      label_udp_send:{
        struct sockaddr_in m_sockaddr;
        m_sockaddr.sin_family = AF_INET;
        m_sockaddr.sin_addr.s_addr = peerAddr->ip;
        m_sockaddr.sin_port = htons(peerAddr->port);
        bzero(&m_sockaddr.sin_zero,8);
        sendto(peerSocket,(char *)data,datalen,0,(const struct sockaddr *)&m_sockaddr, sizeof(m_sockaddr));
        //printf("[UDP SEND %dBytes to %s:%d @%u]\r\n",datalen,inet_ntoa(*((struct in_addr *)&peerAddr->ip)),peerAddr->port,time(NULL));
      }
    }
    else if(peerSocket>0){  
      int i,*udpSocket=hskAppendUdpSocketArry;
      //通过查询的方式判断socket类型，总体效率比通过getsockopt方式判断高效一点
      for(i=hskAppendUdpSocketCount-1;i>=0;i--){
        if(udpSocket[i]==peerSocket) goto label_udp_send;
      }
      send(peerSocket, (char *)data, datalen, 0);// tcp connection
      //printf("[TCP SEND %dBytes to %s:%d @%u]\r\n",datalen,inet_ntoa(*((struct in_addr *)&peerAddr->ip)),peerAddr->port,time(NULL));
    }   
    else{
      peerSocket=hskUdpSocket;
      goto label_udp_send;
    }
  }
  else {
    sendloop(data,datalen);
  }
}
#else
void hsk_sendData(void *data,int datalen,TNetAddr *peerAddr){
  if(peerAddr){
    int peerSocket=peerAddr->socket;
    if(peerSocket==hskUdpSocket){
      label_udp_send:{
        struct sockaddr_in m_sockaddr;
        m_sockaddr.sin_family = AF_INET;
        m_sockaddr.sin_addr.s_addr = peerAddr->ip;
        m_sockaddr.sin_port = htons(peerAddr->port);
        bzero(&m_sockaddr.sin_zero,8);
        sendto(peerSocket,(char *)data,datalen,0,(const struct sockaddr *)&m_sockaddr, sizeof(m_sockaddr));
        //printf("[UDP SEND %dBytes to %s:%d @%u]\r\n",datalen,inet_ntoa(*((struct in_addr *)&peerAddr->ip)),peerAddr->port,time(NULL));
      }
    }
    else if(peerSocket>0){ // tcp connection 
      int sockType = 0;
      int optlen = 4;
      //检测socket类型,这种方式比较通用，但是系统调用的开销相对大一点
      if(getsockopt(peerSocket,SOL_SOCKET,SO_TYPE,&sockType,(socklen_t *)&optlen)==0){
        if(sockType==1){//TCP
          send(peerSocket, (char *)data, datalen, 0);
        }
        else if(sockType==2){//UDP
          goto label_udp_send;
        }
      }
      //printf("[TCP SEND %dBytes to %s:%d @%u]\r\n",datalen,inet_ntoa(*((struct in_addr *)&peerAddr->ip)),peerAddr->port,time(NULL));
    }   
  }
  else {
    sendloop(data,datalen);
  }
}
#endif


void hsk_releaseTcpPacket(THskPacket *packet,BOOL isHeapMsg,BOOL isShortConnect){
		/*两个作用(巧妙):
		 (1) 对动态分配的且锁定的消息报文进行释放；
		 (2) 通过节点超时回调延迟关闭Tcp客户端连接；
		*/
		//puts("##NO.17 hsk_releasePacket");
  if(isShortConnect||isHeapMsg){
      const U32 closeWaitTime_ms=1000;	//通过超时关闭socket
      U32 dtmrOptions=DTMR_ENABLE|DTMR_TIMEOUT_DELETE|DTMR_OVERRIDE;
      dtmr_add(discreteTransferLinks,DiscreteTransferIDL(&packet->addr),packet->addr.ip,NULL,&isShortConnect,sizeof(isShortConnect),closeWaitTime_ms,&dtmrOptions);
  }
}


int hsk_init(int localTcpPort,int localUdpPort,int maxUdpDatagramSize,int inQueueSize){
  HAND socket_recv_thread;
  hskUdpSocket=UDP_Init(localUdpPort);
  hskTcpSocket=TCP_Init(localTcpPort);
  if(hskUdpSocket>0 && hskTcpSocket>0){
    discreteTransferLinks=dtmr_create(0,0,OnDiscreteTransferTimeout,"tcp_discreteLinks");
    os_createSemphore(&socket_queue_mutex,1);
    os_createSemphore(&socket_read_sem,0);
    socket_inqueue=qb_create(inQueueSize);
    if(os_createThread(&socket_recv_thread,_socket_recv_proc, NULL)){
      s_maxUdpDatagramSize=maxUdpDatagramSize;
      return TRUE; 
    }
  }
  return FALSE; 
}

int hsk_append_udp(int udpPortArray[],int udpCount,int inQueueSize){
  int i,ret=0;
  if(udpCount>0 && hskAppendUdpSocketCount==0){
    int *udpSocket=(int *)malloc(udpCount*sizeof(int));
    if(!udp_recv_queue){
      udp_recv_queue=qb_create(inQueueSize);
      os_createSemphore(&udp_queue_mutex,1);
      os_createSemphore(&udp_read_sem,0);
    }
    for(i=0;i<udpCount;i++){
      udpSocket[i]=UDP_Init(udpPortArray[i]);
      if(udpSocket>0){
        HAND udp_recv_thread;
        if(os_createThread(&udp_recv_thread,_udp_socket_recv_proc, udpSocket+i))ret++;
      }
    }
    hskAppendUdpSocketArry=udpSocket;
    hskAppendUdpSocketCount=udpCount;
  }
  return ret; 
}
