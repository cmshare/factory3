#include "mc_routine.h"
//---------------------------------------------------------------------------
#define GENERAL_RESPONSE(MSGNAME)  case MSGNAME:{extern void GeneralResponse_##MSGNAME(TMcPacket *,void *);GeneralResponse_##MSGNAME(response,extraData);break;}
//---------------------------------------------------------------------------
void Response_MSG_TIMEOUT(TMcPacket *request,void *extraData){
  switch(request->msg.msgid){/*
      case MSG_SUR_NOTIFY_MSGBOX:{
             extern void push_device_msg_timeout(TMcPacket *);
             push_device_msg_timeout(request);
           }
           break;*/
  }
}

void Response_MSG_ACK_GENERAL(TMcPacket *response,void *extraData){
  switch(((TMSG_ACK_GENERAL *)response->msg.body)->ack_msgid){
    GENERAL_RESPONSE(MSG_SDR_CALIBRATION_RANGING)
    GENERAL_RESPONSE(MSG_SUR_CALIBRATION_PROGRESS)
    GENERAL_RESPONSE(MSG_SUR_CALIBRATION_RESULT)
    default: puts("######Response_MSG_ACK_GENERAL fail to match request!");
  }
}

void Response_MSG_DSA_WAKEUP(TMcPacket *response,void *extraData)
{ TMcPacket *sus_packet=(TMcPacket *)extraData;
  U8 ret_error=((TMSG_ACK_GENERAL *)response->msg.body)->error;
  msg_ack_general(sus_packet,ret_error);
  
 // printf("####respose wakeup =%d\r\n",ret_error);
/* 终端设备响应成功只表示响应操作，不表示已经成功执行唤醒或者休眠。
   当终端设备执行完唤醒或者休眠后（状态发生变更时），会通过MSG_DSR_NOTIFY_STATE指令来通知服务器。*/
}

void Response_MSG_USA_NOTIFY_MSGBOX(TMcPacket *response,void *extraData){
}

void Response_MSG_USA_NOTIFY_STATE(TMcPacket *response, void *extraData){
  /*TMSG_ACK_GENERAL *ackBody=(TMSG_ACK_GENERAL *)response->msg.body;
  if(ackBody->error==0)Log_AppendText("terminal state notify succeed!");
  */
}

