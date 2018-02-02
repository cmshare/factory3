#include "mc_routine.h"
//---------------------------------------------------------------------------
#define AES128_KEY_CODE          "hello my darling"
#define MSG_SNDQUEUE_SIZE        SERVER_BAND_WIDTH*1024*1024*4U            //服务器在32秒钟内最多发送的数据量（单位Byte)
static  U32 msg_sndbuf_pos=0,msg_syncid=0;
static  char *msg_sndbuf=NULL;
static sem_t msg_alloc_lock;
static HAND dtmr_suspendRequestLinks=NULL;
//---------------------------------------------------------------------------
extern void AES128_setkey(void *aeskey);
extern void AES128_encrypt(void *data,int dataLen);
extern void AES128_decrypt(void *data,int dataLen);

//---------------------------------------------------------------------------
#define MSG_ALLOC_MEM(pMsg,msgLen,allocSyncID)\
{ sem_wait(&msg_alloc_lock);\
	pMsg=(TMcMsg *)&msg_sndbuf[msg_sndbuf_pos];\
	msg_sndbuf_pos+=(msgLen);\
	if(msg_sndbuf_pos>=MSG_SNDQUEUE_SIZE)\
	{	msg_sndbuf_pos=0;\
		pMsg=(TMcMsg *)msg_sndbuf;\
  }\
  else if(msg_sndbuf_pos&0x03)\
  { msg_sndbuf_pos=(msg_sndbuf_pos|0x03)+1;/*地址对齐*/ \
  }\
  if(allocSyncID)pMsg->synid=++msg_syncid;\
  sem_post(&msg_alloc_lock);\
}
//---------------------------------------------------------------------------
void msg_updateSyncID(TMcMsg *msg){
  sem_wait(&msg_alloc_lock);
  msg->synid=++msg_syncid;
  sem_post(&msg_alloc_lock);
}
//---------------------------------------------------------------------------
TMcMsg *msg_alloc(U32 msgID,U32 bodyLen){
  TMcMsg *msg;
  MSG_ALLOC_MEM(msg,sizeof(TMcMsg)+bodyLen+1,!(msgID&MSG_ACK_MASK));
  msg->msgid=msgID;
  msg->bodylen=bodyLen;
  msg->encrypt=ENCRYPTION_NONE;
  msg->sessionid=SERVER_DYNAMIC_SESSION(msg); //用于验证是否是服务器发出的指令
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
      int new_bodylen=(msg->bodylen&0x0F)?(msg->bodylen|0x0F)+1:msg->bodylen;//21×?16×??ú??êy±?3¤?è;
      int new_msgLen=sizeof(TMcMsg)+new_bodylen+1;
      int old_msgDatalen=sizeof(TMcMsg)+msg->bodylen;
      MSG_ALLOC_MEM(encryptMsg,new_msgLen,FALSE);
      memcpy(encryptMsg,msg,old_msgDatalen);
      if(new_bodylen>msg->bodylen)memset((char *)encryptMsg+old_msgDatalen,0,new_bodylen-msg->bodylen);
      AES128_encrypt(encryptMsg->body,new_bodylen);//加密消息体
      encryptMsg->bodylen=new_bodylen;//更新消息体长度
      return encryptMsg;//返回加密后的消息（重新分配空间）
   }
   return NULL;//没有加密或加密失败
}
//---------------------------------------------------------------------------
BOOL msg_decrypt(TMcMsg *msg)
{ if(msg->encrypt==ENCRYPTION_AES128)
  { if((msg->bodylen&0xf)==0)
  	{ AES128_decrypt(msg->body,msg->bodylen);
  		return TRUE;//成功解密
    }	
  }
  else if(msg->encrypt==ENCRYPTION_RAS1024)
  {
  }
  return FALSE;//未解密或解密失败
}
//---------------------------------------------------------------------------
BOOL msg_response_dispatch(TMcPacket *packet,void msgHandle(TMcPacket *,void *)){
    //TMSG_ACK_GENERAL *ack=(TMSG_ACK_GENERAL *)packet->msg.body;
  TSuspendRequest *susRequest=(TSuspendRequest *)dtmr_findById(dtmr_suspendRequestLinks,packet->msg.synid,TRUE);
  if(susRequest){
    U32 ackMsgID=packet->msg.msgid;
    U32 reqMsgID=susRequest->reqPacket.msg.msgid;
    if(ackMsgID==(reqMsgID|MSG_ACK_MASK)||(ackMsgID==MSG_ACK_MASK && ((TMSG_ACK_GENERAL *)packet->msg.body)->ack_msgid==reqMsgID)){
      U32 _ackSession=(susRequest->reqPacket.terminal)?susRequest->reqPacket.terminal->sessionid:SERVER_DYNAMIC_SESSION(&packet->msg);
      if(_ackSession==packet->msg.sessionid /*&& susRequest->ack_msg==packet->msg.msgid*/){
        // RESPONSE_APPENDIX(packet)=susRequest->extraData;
        msgHandle(packet,susRequest->extraData);
        dtmr_unlock(susRequest,DTMR_UNLOCK_DELETE);
        return TRUE;
      }
      //else printf("#session match fail:%x:%x\n",_ackSession,packet->msg.sessionid);
    }	
    dtmr_unlock(susRequest,0);
  }
  return FALSE;
}
//---------------------------------------------------------------------------
static void msg_request_timeout(HAND ttasks,void *taskNode,U32 *taskID,char *taskName){
  TSuspendRequest * node=(TSuspendRequest *)taskNode;
  TTerminal *terminal=node->reqPacket.terminal;
  if(terminal){//terminal为NULL表示服务器发给自己的请求包（超时直接丢弃）
    if(node->retry_counter<REQUEST_RETRY_COUNT){
      msg_send(&node->reqPacket.msg,NULL,terminal);//超时重发消息包(流水号不变,照单重发)
      node->retry_counter++;
      return;
    }
    else{ //超时回调
      extern void Response_MSG_TIMEOUT(TMcPacket *requestPacket,void *extraData);
      Response_MSG_TIMEOUT(&node->reqPacket,node->extraData);
    }
  }	
  dtmr_delete(taskNode);
}  
//---------------------------------------------------------------------------
void msg_sendto(TMcMsg *msg,TNetAddr *peerAddr){
  int  msgLen=MC_MSG_SIZE(msg);
  //消息体加密
   if(msg->bodylen>0 && msg->encrypt>ENCRYPTION_NONE){
      //为消息密文重新分配地址空间，以保留原始消息明文数据
      TMcMsg *encryptMsg=msg_encrypt(msg);
      if(encryptMsg){
        msg=encryptMsg;
        msgLen=MC_MSG_SIZE(msg);
      }	
      else msg->encrypt=ENCRYPTION_NONE;	
   }
   //更新校验位 
   msg_UpdateChecksum(msg,msgLen); 
   //发送消息包
   hsk_sendData(msg,msgLen,peerAddr);
	 	
   #ifdef DEBUG_MODE
   Log_AppendData(msg,msgLen,peerAddr,TRUE);
   #endif
}

void msg_send(TMcMsg *msg,TMcPacket *packet,TTerminal *terminal){
  TNetAddr *peerAddr;
  if(packet){
    peerAddr=&packet->peerAddr;
    msg->encrypt=packet->msg.encrypt;
    if((msg->msgid&MSG_ACK_MASK)) msg->synid=packet->msg.synid;
    DBLog_AppendMsg(msg,packet->terminal,TRUE);
  }
  else if(terminal){
    peerAddr=&terminal->loginAddr;
    msg->encrypt=terminal->encrypt;
    DBLog_AppendMsg(msg,terminal,TRUE);
  }
  else{
    peerAddr=NULL;
    msg->encrypt=ENCRYPTION_NONE;
  }
  msg_sendto(msg,peerAddr);
}
//---------------------------------------------------------------------------
void msg_request(TMcMsg *reqMsg,TTerminal *terminal,/*U32 ackMsgID,*/void *extraData,U32 extraSize)
{ msg_send(reqMsg,NULL,terminal);
  //printf("[request send to %s:%d @%u]\r\n",inet_ntoa(*((struct in_addr *)&terminal->loginAddr.ip)),terminal->loginAddr.port,time(NULL));
  //等接收到B对象的响应后，再处理挂起的A对象的packet。
  //注意：msgLenStored为保存消息体的长度，若为0则只保存消息头。
  int req_packet_size=sizeof(TSuspendRequest);
  if(terminal)req_packet_size+=(reqMsg->bodylen+1); //terminal为NULL表示服务器发给自己的请求包（超时直接丢弃，不需重发）
  if(req_packet_size&0x03)req_packet_size=(req_packet_size|0x03)+1;	
  if(extraSize){
    if(extraData)req_packet_size+=extraSize;
    else extraSize=0;    
  }
  U32 dtmrOptions=DTMR_LOCK|DTMR_ENABLE|DTMR_CYCLE|DTMR_TIMEOUT_DELETE|DTMR_OVERRIDE;
  TSuspendRequest *node=(TSuspendRequest *)dtmr_add(dtmr_suspendRequestLinks,reqMsg->synid,0,NULL,NULL,req_packet_size,REQMSG_RETRY_INTERVAL_MS,&dtmrOptions);
  if(node){
    //node->ack_msg=ackMsgID;
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
    TMcMsg *msg=msg_alloc(MSG_ACK_GENERAL,sizeof(TMSG_ACK_GENERAL));
    TMSG_ACK_GENERAL *ackBody=(TMSG_ACK_GENERAL *)msg->body;
    ackBody->ack_msgid=srcPacket->msg.msgid;
    ackBody->error=errCode;
    msg_send(msg,srcPacket,NULL);
}

//---------------------------------------------------------------------------
void msg_init(void)
{ sem_init(&msg_alloc_lock, 0, 1);
  msg_sndbuf=(char *)malloc(MSG_SNDQUEUE_SIZE+4);
  AES128_setkey(AES128_KEY_CODE);
  dtmr_suspendRequestLinks=dtmr_create(0,0,msg_request_timeout,"dtmr_suspendRequestLinks"); 
}
