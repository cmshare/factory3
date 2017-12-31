#include "cmbase.h"
//---------------------------------------------------------------------------
static int s_maxUdpPacketSize=SOCKET_MAX_DGRAMSIZE;
static HAND udp_recv_thread;
static HAND udp_queue_mutex,udp_read_sem;
static HAND udp_recv_queue=NULL;
//---------------------------------------------------------------------------
#pragma pack (push,1)
//---------------------------------------------------------------------------
typedef struct
{ TNetAddr addr;
  U32   datasize;  
  char  data[0];
}TcmPacketHead;
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

      
static int udp_read_data(int udpSocket){
  int total_received=0;
  os_obtainSemphore(udp_queue_mutex);
  while(1){
    int block_size1,block_size2;
    TcmPacketHead *packet=(TcmPacketHead *)qb_freeSpace(udp_recv_queue,&block_size1,&block_size2);
    if(block_size1>=s_maxUdpPacketSize+sizeof(TcmPacketHead)){
       struct sockaddr peerAddr;
       int addrLen=sizeof(peerAddr);
       //对于connectionless sockets连接，只能通过recvfrom来取得PeerAddress
       int received_bytes=recvfrom(udpSocket, packet->data, s_maxUdpPacketSize, MSG_DONTWAIT ,&peerAddr, (os_socklen_t *)&addrLen);
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
    else if(block_size2>=SOCKET_MAX_DGRAMSIZE+sizeof(TcmPacketHead)){
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
    { if(FD_ISSET(udpSocket,&read_flags)) udp_read_data(udpSocket);
    }	
  }
}


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
int hsk_init_udps(int udpPortArray[],int udpCount,int maxUdpPacketSize,int inQueueSize){
  int ret=0;
  if(!udp_recv_queue){
    udp_recv_queue=qb_create(inQueueSize);
    s_maxUdpPacketSize=maxUdpPacketSize;
    os_createSemphore(&udp_queue_mutex,1);
    os_createSemphore(&udp_read_sem,0);
    for(int i=0;i<udpCount;i++){
      int udpSocket=UDP_Init(udpPortArray[i]);
      if(udpSocket>0){
        if(os_createThread(&udp_recv_thread,_udp_socket_recv_proc, &udpSocket))ret++;
      }
    }
  }
  return ret; 
}
