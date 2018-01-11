#include "mc_routine.h"

void Handle_MSG_USR_GETUSERHEAD(TMcPacket *packet)
{ TMSG_USR_GETUSERHEAD *req=(TMSG_USR_GETUSERHEAD *)packet->msg.body;
  U32 userid=0;
  if(MobilePhone_check(req->phone))
  {  MYSQL_RES *res=db_queryf("select id from `mc_users` where username='%s'",req->phone);
     if(res)
     { MYSQL_ROW row=mysql_fetch_row(res);
       if(row && row[0])userid=atoi(row[0]);
        mysql_free_result(res);
     }  
  }
  if(userid>0)
  { extern U32  Userlogo_Load(U32 userid,void *dataBuffer,int bufszie);
    int maxImageDataLen=MAXLEN_MSG_PACKET-(sizeof(TMcMsg)+sizeof(TMSG_SUA_GETUSERHEAD)+1);
    TMcMsg *ackmsg=msg_alloc(MSG_SUA_GETUSERHEAD,maxImageDataLen);
    TMSG_SUA_GETUSERHEAD *ackbody=(TMSG_SUA_GETUSERHEAD *)ackmsg->body;
    ackbody->ack_synid=packet->msg.synid;
    ackbody->data_size=Userlogo_Load(userid,ackbody->data,maxImageDataLen);      
    ackmsg->bodylen=sizeof(TMSG_SUA_GETUSERHEAD)+ackbody->data_size;
    packet->terminal->encrypt=ENCRYPTION_NONE;//头像不加密
    msg_send(ackmsg,packet,NULL); 
    //hsk_closeTcpClient(&packet->peerAddr);
  }  
}

void Handle_MSG_USR_CHANGEHEAD(TMcPacket *packet)
{ extern BOOL Userlogo_Save(U32 userid,void *data,int datalen);
  TMSG_USR_CHANGEHEAD *req=(TMSG_USR_CHANGEHEAD *)packet->msg.body;
  U8 ret_error=(Userlogo_Save(packet->terminal->id,req->data,req->datalen))?0:1;
  msg_ack_general(packet,ret_error);
  //hsk_closeTcpClient(&packet->peerAddr);
}

void Handle_MSG_USR_GETUSERINFO(TMcPacket *packet)
{ TMcMsg *ackmsg=msg_alloc(MSG_SUA_GETUSERINFO,sizeof(TMSG_SUA_GETUSERINFO));
  TMSG_SUA_GETUSERINFO *ackBody=(TMSG_SUA_GETUSERINFO *)ackmsg->body;
  U8 ret_error=-1;
  MYSQL_RES *res=db_queryf("select nickname,score from `mc_users` where id=%u",packet->terminal->id);
  if(res)
  { MYSQL_ROW row=mysql_fetch_row(res);
    if(row)
    { ret_error=0;
      if(row[0])strncpy(ackBody->nickname,row[0],MAXLEN_NICKNAME+1);
      else ackBody->nickname[0]='\0';
      ackBody->score=atoi(row[1]);
      ackBody->sex=packet->terminal->sex_type;
      ackBody->msg_push_acceptable=packet->terminal->msg_push_acceptable;
    }
    mysql_free_result(res);
  }   
  if(!ret_error)
  { ackBody->ack_synid=packet->msg.synid;
    msg_send(ackmsg,packet,NULL);
  }  
}

void Handle_MSG_USR_CHANGENICK(TMcPacket *packet)
{ TMSG_USR_CHANGENICK *req=(TMSG_USR_CHANGENICK *)packet->msg.body;
  db_queryf("update `mc_users` set nickname='%s' where id=%u",db_filterSQL(req->nick),packet->terminal->id);
  msg_ack_general(packet,0);
}

/*
void Handle_MSG_USR_GETBINDLIST(TMcPacket *packet){
  const int MAX_ITEM_SIZE=128;
  TMcMsg *ackmsg=msg_alloc(MSG_SUA_GETBINDLIST,sizeof(TMSG_SUA_GETBINDLIST)+MAX_BINDED_NUM*MAX_ITEM_SIZE);
  TMSG_SUA_GETBINDLIST *ackBody=(TMSG_SUA_GETBINDLIST *)ackmsg->body;
  MYSQL_RES *res=db_queryf("select `mc_devices`.sn,`mc_devices`.name,`mc_devices`.ssid,`mc_devices`.imsi,`mc_uidpool`.uid,`mc_devices`.state from `mc_devices` left join `mc_uidpool` on `mc_devices`.sn=`mc_uidpool`.sn where `mc_devices`.username='%s' limit %d",packet->terminal->name,MAX_BINDED_NUM);
  char *json=(char *)ackBody->json.data;
  int jslen=sprintf(json,"{\"items\":[");	
  if(res){
    MYSQL_ROW row;
    int bind_count=0;
    while((row = mysql_fetch_row(res))){ 
      if(bind_count++>0) json[jslen++]=',';	
      jslen+=sprintf(json+jslen,"{\"sn\":\"%s\",\"name\":\"%s\",\"ssid\":\"%s\",\"imsi\":\"%s\",\"uid\":\"%s\",\"state\":\"%s\"}",row[0],row[1],row[2],row[3],row[4],row[5]);
    }
    mysql_free_result(res);
  }  
  jslen+=sprintf(json+jslen,"]}");
  jslen++;//保留null-terminate符位置

  ackBody->ack_synid=packet->msg.synid;
  ackBody->json.datalen=jslen;
  ackmsg->bodylen=sizeof(TMSG_SUA_GETBINDLIST)+jslen;//重新计算实际消息体长度。
  msg_send(ackmsg,packet,NULL);
}
*/

void Handle_MSG_USR_CONFIGS(TMcPacket *packet)
{/////////////////////////////////////////////
  #define config_key1 "pay_url"
  #define config_key2 "trackmap_url"
  #define config_key3 "video_share_url"
  #define config_key4 "appshow_url"
  MYSQL_RES *res=db_query("select "config_key1","config_key2","config_key3","config_key4" from `mc_configs`");
  if(res)
  { MYSQL_ROW row=mysql_fetch_row(res);
    if(row)
    { TMcMsg *ackmsg=msg_alloc(MSG_SUA_CONFIGS,sizeof(TMSG_SUA_CONFIGS)+1024);
      TMSG_SUA_CONFIGS *ackBody=(TMSG_SUA_CONFIGS *)ackmsg->body;
      ackBody->ack_synid=packet->msg.synid;
      ackBody->json.datalen=1+sprintf((char *)ackBody->json.data,"{\""config_key1"\":\"%s\",\""config_key2"\":\"%s\",\""config_key3"\":\"%s\",\""config_key4"\":\"%s\"}",(row[0])?row[0]:"",(row[1])?row[1]:"",(row[2])?row[2]:"",(row[3])?row[3]:"");
      ackmsg->bodylen=sizeof(TMSG_SUA_CONFIGS)+ackBody->json.datalen;
      msg_send(ackmsg,packet,NULL);
    }
    mysql_free_result(res);  
  }
}

