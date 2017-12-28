#include "mc_routine.h"
//---------------------------------------------------------------------------
#define AES128_KEY_CODE          "www.mplanet.cn  "
#define MSG_SNDQUEUE_SIZE        SERVER_BAND_WIDTH*1024*1024*4U            //��������32��������෢�͵�����������λByte)
static  U32 msg_sndbuf_pos=0;
static  char *msg_sndbuf=NULL;
static sem_t msg_alloc_lock;
static HAND suspendRequestLinks=NULL;
//---------------------------------------------------------------------------
extern void AES128_setkey(void *aeskey);
extern void AES128_encrypt(void *data,int dataLen);
extern void AES128_decrypt(void *data,int dataLen);

//---------------------------------------------------------------------------
#define MSG_ALLOC_MEM(pMsg,msgLen)\
{ sem_wait(&msg_alloc_lock);\
	pMsg=(TMcMsg *)&msg_sndbuf[msg_sndbuf_pos];\
	msg_sndbuf_pos+=(msgLen);\
	if(msg_sndbuf_pos>=MSG_SNDQUEUE_SIZE)\
	{	msg_sndbuf_pos=0;\
		pMsg=(TMcMsg *)msg_sndbuf;\
  }\
  else if(msg_sndbuf_pos&0x03)\
  { msg_sndbuf_pos=(msg_sndbuf_pos|0x03)+1;/*��ַ����*/ \
  }\
	sem_post(&msg_alloc_lock);\
}
//---------------------------------------------------------------------------
TMcMsg *msg_alloc(U32 msgID,U32 bodyLen){
  TMcMsg *msg;
  MSG_ALLOC_MEM(msg,sizeof(TMcMsg)+bodyLen+1);
  msg->msgid=msgID;
  msg->bodylen=bodyLen;
  msg->encrypt=ENCRYPTION_NONE;
  msg->sessionid=SERVER_DYNAMIC_SESSION(msg); //������֤�Ƿ��Ƿ�����������ָ��
  return msg;
}
//---------------------------------------------------------------------------
void msg_UpdateChecksum(void *data,int datalen)
{ unsigned char *pd=(unsigned char *)data;
  unsigned char checkbyte=0;
  int i; 
  for(i=1;i<datalen;i++)
  {  checkbyte^=*pd++;
  }
  *pd=checkbyte;
}
//---------------------------------------------------------------------------
BOOL msg_ValidChecksum(void *data,int datalen)
{ int i;
  unsigned char *pd=(unsigned char *)data;
  unsigned char  checkbyte=0;
  for(i=1;i<datalen;i++)
  { checkbyte^=*pd++;
  }
  return (*pd==checkbyte);
}
//---------------------------------------------------------------------------
TMcMsg *msg_encrypt(TMcMsg *msg)
{  if(msg->encrypt==ENCRYPTION_AES128)
   {  TMcMsg *encryptMsg;
      int new_bodylen=(msg->bodylen&0x0F)?(msg->bodylen|0x0F)+1:msg->bodylen;//21��?16��??��??��y��?3��?��;
      int new_msgLen=sizeof(TMcMsg)+new_bodylen+1;
      int old_msgDatalen=sizeof(TMcMsg)+msg->bodylen;
      MSG_ALLOC_MEM(encryptMsg,new_msgLen);
      memcpy(encryptMsg,msg,old_msgDatalen);
      if(new_bodylen>msg->bodylen)memset((char *)encryptMsg+old_msgDatalen,0,new_bodylen-msg->bodylen);
      AES128_encrypt(encryptMsg->body,new_bodylen);//������Ϣ��
      encryptMsg->bodylen=new_bodylen;//������Ϣ�峤��
      return encryptMsg;//���ؼ��ܺ����Ϣ�����·���ռ䣩
   }
   return NULL;//û�м��ܻ����ʧ��
}
//---------------------------------------------------------------------------
BOOL msg_decrypt(TMcMsg *msg)
{ if(msg->encrypt==ENCRYPTION_AES128)
  { if((msg->bodylen&0xf)==0)
  	{ AES128_decrypt(msg->body,msg->bodylen);
  		return TRUE;//�ɹ�����
    }	
  }
  else if(msg->encrypt==ENCRYPTION_RAS1024)
  {
  }
  return FALSE;//δ���ܻ����ʧ��
}
//---------------------------------------------------------------------------
BOOL msg_response_dispatch(TMcPacket *packet,void msgHandle(TMcPacket *,void *)){
    //TMSG_ACK_GENERAL *ack=(TMSG_ACK_GENERAL *)packet->msg.body;
    BOOL ret=FALSE;
    TSuspendRequest *susRequest=(TSuspendRequest *)dtmr_find(suspendRequestLinks,packet->msg.synid,0,NULL,DTMR_FOREVER);
    if(susRequest && (packet->msg.msgid==(susRequest->reqPacket.msg.msgid|MSG_ACK_MASK)||packet->msg.msgid==MSG_ACK_MASK)){
      U32 _ackSession=(susRequest->reqPacket.terminal)?susRequest->reqPacket.terminal->session:SERVER_DYNAMIC_SESSION(&packet->msg);
      if(_ackSession==packet->msg.sessionid /*&& susRequest->ack_msg==packet->msg.msgid*/){
         // RESPONSE_APPENDIX(packet)=susRequest->extraData;
          msgHandle(packet,susRequest->extraData);
	  ret=TRUE;
      }	
      dtmr_remove(susRequest);
  }
  return ret;
}
//---------------------------------------------------------------------------
static void msg_request_timeout(HAND ttasks,void *taskCode,U32 *taskID,char *taskName,U32 *sUpdateLifeTime){
  TSuspendRequest * node=(TSuspendRequest *)taskCode;
  TTerminal *terminal=node->reqPacket.terminal;
  if(terminal){//terminalΪNULL��ʾ�����������Լ������������ʱֱ�Ӷ�����
    if(node->retry_counter<REQUEST_RETRY_COUNT){
      msg_send(&node->reqPacket.msg,NULL,terminal);//��ʱ�ط���Ϣ��(��ˮ�Ų���,�յ��ط�)
      node->retry_counter++;
      *sUpdateLifeTime=REQMSG_RETRY_INTERVAL_S;
    }
    else{ //��ʱ�ص�
      extern void Response_MSG_TIMEOUT(TMcPacket *requestPacket,void *extraData);
      Response_MSG_TIMEOUT(&node->reqPacket,node->extraData);
    }
  }	
}  
//---------------------------------------------------------------------------
void msg_sendto(TMcMsg *msg,TNetAddr *peerAddr){
  int  msgLen=MC_MSG_SIZE(msg);
  //��Ϣ�����
   if(msg->bodylen>0 && msg->encrypt>ENCRYPTION_NONE){
      //Ϊ��Ϣ�������·����ַ�ռ䣬�Ա���ԭʼ��Ϣ��������
      TMcMsg *encryptMsg=msg_encrypt(msg);
      if(encryptMsg){
        msg=encryptMsg;
        msgLen=MC_MSG_SIZE(msg);
      }	
      else msg->encrypt=ENCRYPTION_NONE;	
   }
   //����У��λ 
   msg_UpdateChecksum(msg,msgLen); 
   //������Ϣ��
   hsk_sendData(msg,msgLen,peerAddr);
	 	
   #ifdef DEBUG_MODE
   Log_AppendData(msg,msgLen,peerAddr,TRUE);
   #endif
}

void msg_send(TMcMsg *msg,TMcPacket *packet,TTerminal *terminal){
  static U32 msg_synid=0;
  TNetAddr *peerAddr;
  if(packet){
    peerAddr=&packet->peerAddr;
    msg->encrypt=packet->msg.encrypt;
    if((msg->msgid&MSG_ACK_MASK)) msg->synid=packet->msg.synid;
    else msg->synid=++msg_synid;
    DBLog_AppendMsg(msg,packet->terminal,TRUE);
  }
  else if(terminal){
    peerAddr=&terminal->loginAddr;
    msg->encrypt=terminal->encrypt;
    msg->synid=++msg_synid;
    DBLog_AppendMsg(msg,terminal,TRUE);
  }
  else{
    peerAddr=NULL;
    msg->encrypt=ENCRYPTION_NONE;
    msg->synid=++msg_synid;
  }
  msg_sendto(msg,peerAddr);
}
//---------------------------------------------------------------------------
void msg_request(TMcMsg *reqMsg,TTerminal *terminal,/*U32 ackMsgID,*/void *extraData,U32 extraSize)
{ msg_send(reqMsg,NULL,terminal);//�������������������ض���UDPЭ�顣
  //printf("[request send to %s:%d @%u]\r\n",inet_ntoa(*((struct in_addr *)&terminal->loginAddr.ip)),terminal->loginAddr.port,time(NULL));
  //�Ƚ��յ�B�������Ӧ���ٴ�������A�����packet��
  //ע�⣺msgLenStoredΪ������Ϣ��ĳ��ȣ���Ϊ0��ֻ������Ϣͷ��
  int req_packet_size=sizeof(TSuspendRequest);
  if(terminal)req_packet_size+=(reqMsg->bodylen+1); //terminalΪNULL��ʾ�����������Լ������������ʱֱ�Ӷ����������ط���
  if(req_packet_size&0x03)req_packet_size=(req_packet_size|0x03)+1;	
  if(extraSize){
    if(extraData)req_packet_size+=extraSize;
    else extraSize=0;    
  }
  TSuspendRequest *node=(TSuspendRequest *)dtmr_add(suspendRequestLinks,reqMsg->synid,0,NULL,NULL,req_packet_size,REQMSG_RETRY_INTERVAL_S|DTMR_LOCK);
  if(node)
  { //node->ack_msg=ackMsgID;
    node->retry_counter=1;
    node->reqPacket.terminal=terminal;
    if(terminal){
      node->reqPacket.peerAddr=terminal->loginAddr;
      memcpy(&node->reqPacket.msg,reqMsg,MC_MSG_SIZE(reqMsg));
    }
    if(extraSize)
    { node->extraData=(char *)node+(req_packet_size-extraSize);
      memcpy(node->extraData,extraData,extraSize);
    }else node->extraData=NULL;	
    dtmr_unlock(node,0);
  }  
}

//---------------------------------------------------------------------------
void msg_ack(TMcPacket *srcPacket,void *msgData,int dataLen){
    TMcMsg *msg=msg_alloc(srcPacket->msg.msgid|MSG_ACK_MASK,dataLen);
    if(dataLen==1)*(U8*)msg->body=*(U8 *)msgData;
    else memcpy(msg->body,msgData,dataLen);
    msg_send(msg,srcPacket,NULL);
}

void msg_ack_general(TMcPacket *srcPacket,U8 errCode){
    TMcMsg *msg=msg_alloc(MSG_STA_GENERAL,sizeof(TMSG_STA_GENERAL));
    TMSG_STA_GENERAL *ackBody=(TMSG_STA_GENERAL *)msg->body;
    ackBody->ack_msgid=srcPacket->msg.msgid;
    ackBody->error=errCode;
    msg_send(msg,srcPacket,NULL);
}
//---------------------------------------------------------------------------
void msg_init(void)
{ sem_init(&msg_alloc_lock, 0, 1);
  msg_sndbuf=(char *)malloc(MSG_SNDQUEUE_SIZE+4);
  AES128_setkey(AES128_KEY_CODE);
  suspendRequestLinks=dtmr_create(0,0,msg_request_timeout); 
}
