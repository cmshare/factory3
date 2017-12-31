#include "mc_routine.h" 
//---------------------------------------------------------------------------
static int svrUdpSocket=-1,svrTcpSocket=-1;
//---------------------------------------------------------------------------
static void SetSocketBuffer(int sockd,int recvBufsize,int sendBufsize)
{	if(recvBufsize)
	{ if(setsockopt(sockd,SOL_SOCKET,SO_RCVBUF,(char*)&recvBufsize,sizeof(int))==0)
		{ int len=sizeof(int),result;
	  	if(getsockopt(sockd,SOL_SOCKET, SO_RCVBUF, (char*)&result, (socklen_t *)&len)==0)
	  	{ result>>=1;//���õ�ֵ��ʵ�ʶ�����ֵ��������ϵ
	  		if(result!=recvBufsize)printf("SET SO_RCVBUF to %d,but limited to %d,pelease check config[/proc/sys/net/core/rmem_max]\r\n",recvBufsize,result);
	  	}
		}else printf("SET SO_RCVBUF failed\r\n");
  }	
  if(sendBufsize)
	{	if(setsockopt(sockd,SOL_SOCKET,SO_SNDBUF,(char*)&sendBufsize,sizeof(int))==0)
		{ int len=sizeof(int),result;
	  	if(getsockopt(sockd,SOL_SOCKET, SO_SNDBUF, (char*)&result, (socklen_t *)&len)==0)
	  	{ result>>=1;
	  		if(result!=sendBufsize)printf("SET SO_SNDBUF to %d,but limited to %d,pelease check config[/proc/sys/net/core/wmem_max]\r\n",sendBufsize,result);
	  	}	
		}
  }else printf("SET SO_SNDBUF failed\r\n");	
}
//---------------------------------------------------------------------------
static BOOL packet_checksum_and_decrypt(TMcPacket *packet){
  TMcMsg *msg=&packet->msg;
  int msgLen=MC_MSG_SIZE(msg);
  if(msg_ValidChecksum(msg,msgLen)){
    TTerminal *terminal=(msg->sessionid)?(TTerminal *)dtmr_find(terminalLinks,msg->sessionid,0,NULL,0):NULL;
    if(terminal){
      //�����sessionid��IP��ַ�ǰ󶨵Ĺ�ϵ���������Ӧ��ʾsessionid�Ѿ�ʧЧ��
      //TCP��UDP�в�ͬ�Ķ˿ڣ���������ֻ��֤UDP�˿ڵİ󶨹�ϵ��
      if(packet->peerAddr.ip!=terminal->loginAddr.ip){
        terminal=NULL; //����豸��δ��¼�����ߵ�¼��ʱ��
      }
      else if(packet->peerAddr.port!=terminal->loginAddr.port){
         if((packet->peerAddr.socket==svrUdpSocket)==(terminal->loginAddr.socket==svrUdpSocket)) terminal=NULL;
      }
    }
    packet->terminal=terminal;
    if(msg->bodylen>0 && msg->encrypt>ENCRYPTION_NONE) msg_decrypt(msg);
    return TRUE;		
  }
  return FALSE;
}
//---------------------------------------------------------------------------
static TMcPacket *NET_RecvPacket(void *dgramBuffer,int bufferSize){
   TMcPacket *packet=(TMcPacket *)dgramBuffer;
   TMcMsg *msg=&packet->msg;
   int msgsize,sliceLen=hsk_readData(msg,bufferSize-sizeof(TMcPacket),&packet->peerAddr);

   #ifdef DEBUG_MODE
    Log_AppendData(msg,sliceLen,&packet->peerAddr,FALSE);
   #endif

   if(sliceLen>sizeof(TMcMsg)){
      msgsize=MC_MSG_SIZE(msg);
      if(sliceLen==msgsize && packet_checksum_and_decrypt(packet))return packet; 
      else if(msgsize>MAXLEN_MSG_PACKET)msgsize=0;
   }
   else{
     if(!sliceLen)return NULL;
     msgsize=0;
   }
 	 
   if(packet->peerAddr.socket!=svrUdpSocket){ //���TCP�������ݽ�����װ 	
     packet=(TMcPacket *)hsk_assemble(&packet->peerAddr,msg,sliceLen,msgsize);
     if(packet){
        if(packet_checksum_and_decrypt(packet))return packet;
        else hsk_releaseTcpPacket((THskPacket *)packet,TRUE,FALSE); 
     }
   }
 /*  else{
    //����У������UDP����(����ת������ض� ,���ڸ��ӶȽ�����������TCP����) TTerminal *terminal=dtmr_find2(terminalLinks,0,0,&packet->peerAddr,sizeof(TNetAddr),T_NODE_OFFSET(TTerminal,loginAddr),0);
     if(terminal && terminal->spyAddr.ip)hsk_sendData(msg,sliceLen,&terminal->spyAddr);	
   }*/
   return NULL;  	
}
//---------------------------------------------------------------------------
static void *mc_dispatch_proc(void *param){
  extern void mc_dispatchmsg(TMcPacket *);
  int bufferSize=sizeof(TMcPacket)+MAXLEN_IP_FRAGMENT;
  void *pbuf=malloc(bufferSize);
  while(1)//process user request packet  
  { TMcPacket *packet=NET_RecvPacket(pbuf,bufferSize);
    if(packet)
    {  if(packet->terminal)
    	{ DBLog_AppendMsg(&packet->msg,packet->terminal,FALSE);
    	}
        mc_dispatchmsg(packet);
  	//((void (*)(TMcPacket *))param)(packet);
        if(packet->peerAddr.socket!=svrUdpSocket){
    	   hsk_releaseTcpPacket((THskPacket *)packet,(packet!=pbuf),packet->msg.tcp_short_connection);
        }
    }	
  }
  return NULL;
}
//---------------------------------------------------------------------------
static void *uwb_location_proc(void *param){
  extern void udp_process_packet(void *,int,TNetAddr *);
  unsigned char recvBuf[MAXLEN_UDP_DATAGRAM];
  while(1){  
    TNetAddr peerAddr;
    int dataLen=hsk_readDatagram(recvBuf,sizeof(recvBuf),&peerAddr);
    udp_process_packet(recvBuf,dataLen,&peerAddr);
  }
  return NULL;
}
//---------------------------------------------------------------------------
void Handle_MSG_TSR_HEARTBEAT(TMcPacket *packet){
  printf("heatbeat\n");
  dtmr_update(packet->terminal,HEARTBEAT_OVERTIME_S);
  msg_ack_general(packet,0);
}
//---------------------------------------------------------------------------
void mc_schedule(void){
  int i;
  pthread_t _thread=0;
  for(i=0;i<NUM_DISPATCH_THREAD;i++){
    pthread_create(&_thread, NULL,mc_dispatch_proc,NULL);
    //printf("Create dispatch-thread-%02d...%s!\r\n",i+1,(_thread)?"OK":"failed");	
  }
  for(i=0;i<NUM_UWB_PROCESS_THREAD;i++){
    pthread_create(&_thread, NULL,uwb_location_proc,NULL);
  }
  puts("UWB daemon service Ver "MEOW_SERVICE_VERSION" @"WEB_SERVER_HOST",running...");
}
//---------------------------------------------------------------------------
void net_init(void){
   if(hsk_init(SERVICE_PORT_TCP,SERVICE_PORT_UDP,MAXLEN_UDP_DATAGRAM,SIZE_NET_RECV_BUFFER)){
       int udp_port_array[]={SERVICE_UWB_PORT1,SERVICE_UWB_PORT2,SERVICE_UWB_PORT3};
       svrUdpSocket=hsk_getUdpSocket();
       svrTcpSocket=hsk_getTcpSocket();
       printf("svrUdpSocket=%d,svrTcpSocket=%d\r\n",svrUdpSocket,svrTcpSocket);
       SetSocketBuffer(svrUdpSocket,MAX_SOCKET_RECV_MEM,MAX_SOCKET_SEND_MEM);
       SetSocketBuffer(svrTcpSocket,MAX_SOCKET_RECV_MEM,MAX_SOCKET_SEND_MEM);
       hsk_append_udp(udp_port_array,3,SIZE_UWB_RECV_BUFFER);
   }
   else{
      Log_AppendText("Net init error!");
      exit(0);//should not be here
   }		
}
//-----------------------------------------------------------------------------
//
