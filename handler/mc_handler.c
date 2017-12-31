#include "mc_routine.h"

void Handle_MSG_DSR_LOGIN(TMcPacket *packet){
/*  if(packet->peerAddr.socket!=hsk_getUdpSocket()){
    msg_ack_general(packet,-1);
    return 0;//登录方式必须要求UDP
  }*/
  enum{DEV_LOGIN_ORIGIN=0,DEV_LOGIN_BOX=1,DEV_LOGIN_CAMERA=2};
  TTerminal *terminal=NULL;
  U32  deviceID=0,sessionid=0,loginTime,deviceGroup,deviceState;
  TMSG_DSR_LOGIN *content=(TMSG_DSR_LOGIN *)packet->msg.body;
  U8 error_code=1;
  if(content->name[0] && db_checkSQL(content->name) && Password_check(content->psw))
  {  MYSQL_RES *res;
     res=db_queryf("select id,session,groupid,logintime,state from `mc_devices` where sn='%s' and password=md5('%s')",content->name,content->psw); 
     if(res)
     { MYSQL_ROW row=mysql_fetch_row(res);
       if(row)
       {  deviceID=atoi(row[0]);
          sessionid=atoi(row[1]);
          deviceGroup=atoi(row[2]);
          loginTime=atoi(row[3]);
          deviceState=atoi(row[4]);
          if(sessionid)
          { terminal=(TTerminal *)dtmr_find(terminalLinks,sessionid,0,0,HEARTBEAT_OVERTIME_S); 
            if(terminal && terminal->id!=deviceID)
            { //上一次使用的session已经被其他设备占用（所查到的terminal是其他设备）。
              sessionid=0;
              terminal=NULL;
            }
          } 
       }
       mysql_free_result(res);   
     }  
  }

  if(deviceID)
  { extern U32 SessionID_new(void);
    if(!sessionid)sessionid=SessionID_new(); 
    if(!terminal) {
      int termsize=sizeof(TTermDevice);
      terminal=(TTerminal *)dtmr_add(terminalLinks,sessionid,0,0,NULL,termsize,HEARTBEAT_OVERTIME_S);
      memset(terminal,0,termsize);
      loginTime=time(NULL);
    }
    terminal->term_type=TT_DEVICE;
    terminal->id=deviceID;
    terminal->loginAddr=packet->peerAddr;//登录方式必须要求UDP
    terminal->session=sessionid;
    terminal->group=deviceGroup;
    terminal->encrypt=packet->msg.encrypt;//消息体默认加密方式
    terminal->term_state=deviceState; //the login action do not change device state
    strncpy(terminal->name,content->name,SIZE_SN_DEVICE+1);
    error_code=0;
    db_queryf("update `mc_devices` set session=%u,ip=%u,port=%u,logintime=%u where id=%u",sessionid,packet->peerAddr.ip,packet->peerAddr.port,loginTime,deviceID);
  }
  
  packet->terminal=terminal;
  TMcMsg *msg=msg_alloc(MSG_SDA_LOGIN,sizeof(TMSG_SDA_LOGIN));
  TMSG_SDA_LOGIN *ackBody=(TMSG_SDA_LOGIN *)msg->body;
  ackBody->ack_synid=packet->msg.synid;
  ackBody->session=sessionid;
  ackBody->error=error_code;
  msg_send(msg,packet,NULL);

  if(error_code==0){ 
    DBLog_AppendData("\xFF\xFF\xFF\xFF\x00",5,terminal); //登录日志
  }
}
        

void Handle_MSG_USR_LOGIN(TMcPacket *packet){
/*  if(packet->peerAddr.socket!=hsk_getUdpSocket()){
    msg_ack_general(packet,-1);
    return 0;//登录方式必须要求UDP
  }*/
  TTerminal *terminal=NULL,*terminalKickOff=NULL;
  U32 userid=0,sessionid=0,userGroup;
  U8 sex_type,msgpush,livepush,error_code=0;
  char *username,pwd_pattern[SIZE_MD5+3];
  if(packet->msg.msgid==MSG_USR_LOGIN){
     TMSG_USR_LOGIN *content=(TMSG_USR_LOGIN *)packet->msg.body;
     username=content->name;
     if(content->psw[0]=='\0' || Password_check(content->psw))sprintf(pwd_pattern,"md5('%s')",content->psw);
     else pwd_pattern[0]='\0';//Invalid password!
  }
  else{
     TMSG_USR_LOGIN2 *content=(TMSG_USR_LOGIN2 *)packet->msg.body;
     username=content->name;
     if(strlen(content->psw_md5)==SIZE_MD5 && Password_check(content->psw_md5)) sprintf(pwd_pattern,"'%s'",content->psw_md5);
     else pwd_pattern[0]='\0'; //Invalid password!
  }

  if(db_checkSQL(username) && pwd_pattern[0]!='\0'){ 
     MYSQL_RES *res=db_queryf("select id,session,groupid,sex,msgpush,livepush from `mc_users` where username='%s' and password=%s",username,pwd_pattern);
     if(res){ 
        MYSQL_ROW row=mysql_fetch_row(res);
        if(row)
        { userid=atoi(row[0]);
          sessionid=atoi(row[1]);
          userGroup=atoi(row[2]);
          sex_type=atoi(row[3]);
          msgpush=atoi(row[4]);
          livepush=atoi(row[5]);
          if(sessionid)
          { terminal=(TTerminal *)dtmr_find(terminalLinks,sessionid,0,0,HEARTBEAT_OVERTIME_S);
            if(terminal)
            {  if(terminal->id!=userid)
               { //上一次使用的session已经被其他用户占用（所查到的terminal是其他用户）。
                 sessionid=0;
                 terminal=NULL;
               }
               else if(memcmp(&terminal->loginAddr,&packet->peerAddr,sizeof(TNetAddr))!=0)
               { //同一用户名多处登录的状况
                 //将原先登录的用户踢下线
                 TMcMsg *reqmsg=msg_alloc(MSG_SUR_KICKOFF,0);
                 msg_request(reqmsg,terminal,NULL,0);
                 //安全删除原先登录的用户节点
                 dtmr_remove(terminal);//删除后的节点无法被查找，但会保留足够长一段时间
                 terminalKickOff=terminal;
                 terminal=NULL;
                 //sessionid=0; //sessionID可继续使用
               }
            }
          }
        }
        mysql_free_result(res);  
    }
  }
  if(!userid)
  { error_code=1;//用户名或密码错误。
  }
  else
  { extern U32 SessionID_new(void);
    if(!sessionid)sessionid=SessionID_new(); 
    if(!terminal)
    { terminal=(TTerminal *)dtmr_add(terminalLinks,sessionid,0,0,NULL,sizeof(TTermUser),HEARTBEAT_OVERTIME_S);
      memset(terminal,0,sizeof(TTermUser));
    }
    terminal->term_type=TT_USER;
    terminal->id=userid;
    terminal->loginAddr=packet->peerAddr;//登录方式必须要求UDP
    terminal->session=sessionid;
    terminal->group=userGroup;
    terminal->sex_type=sex_type;
    terminal->msg_push_acceptable=msgpush;
    terminal->encrypt=packet->msg.encrypt;//消息体默认加密方式
    strncpy(terminal->name,username,SIZE_MOBILE_PHONE+1);
    db_queryf("update `mc_users` set session=%u,ip=%u,port=%u,logintime=unix_timestamp() where id=%u",sessionid,packet->peerAddr.ip,packet->peerAddr.port,userid);
  }
  packet->terminal=terminal;
  
  TMcMsg *msg=msg_alloc(packet->msg.msgid|MSG_STA_GENERAL,sizeof(TMSG_SUA_LOGIN));
  TMSG_SUA_LOGIN *ackBody=(TMSG_SUA_LOGIN *)msg->body;
  ackBody->ack_synid=packet->msg.synid;
  ackBody->error=error_code;
  ackBody->session=sessionid;
  msg_send(msg,packet,NULL);
  if(error_code==0)DBLog_AppendData("\xFF\xFF\xFF\xFF\x00",5,terminal); //登录日志
}


void Handle_MSG_USR_LOGIN2(TMcPacket *packet)
{  Handle_MSG_USR_LOGIN(packet);
}

void Handle_MSG_USR_LOGOUT(TMcPacket *packet)
{ db_queryf("update `mc_users` set session=0,logouttime=unix_timestamp() where id=%u",packet->terminal->id);
  dtmr_remove(packet->terminal);
  msg_ack_general(packet,0);
}

void Handle_MSG_USR_REGIST(TMcPacket *packet)
{ TMSG_USR_REGIST *req=(TMSG_USR_REGIST *)packet->msg.body;
  U8 ret_error=-1;
  if(MobilePhone_check(req->phone) && Password_check(req->psw))
  { int vcode_err=vcode_apply(req->verifycode,req->phone);
    if(vcode_err==0)
    { db_lock(TRUE);
      MYSQL_RES *res=db_queryf("select id from `mc_users` where username='%s'",req->phone);
      if(res)
      { MYSQL_ROW row=mysql_fetch_row(res);
        if(row && atoi(row[0]))ret_error=1; //用户名已经存在
        else
        { char sqlText[256];
          req->nick[MAXLEN_NICKNAME]='\0';
          sprintf(sqlText,"`mc_users` set username='%s',password=md5('%s'),groupid=%u,score=0,sex=0,nickname='%s',session=0,ip=%u,port=%u,logintime=0,logouttime=0,regtime=unix_timestamp()",req->phone,req->psw,req->groupid,db_filterSQL(req->nick),packet->peerAddr.ip,packet->peerAddr.port);
          if(db_queryf("update %s where groupid=0 order by logintime asc limit 1",sqlText))ret_error=0;//herelong
          else
          { db_queryf("insert into %s",sqlText);
            ret_error=0;
          } 
        }
        mysql_free_result(res);
      }  
      db_lock(FALSE);
    }
    else if(vcode_err==ERR_TIMEOUT)ret_error=3; //验证码已经失效
  	else ret_error=2; //验证码错误
  }
  msg_ack_general(packet,ret_error);
}


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

void Handle_MSG_USR_CHANGEPSW(TMcPacket *packet)
{ TMSG_USR_CHANGEPSW *req=(TMSG_USR_CHANGEPSW *)packet->msg.body;
  U8 ret_error=-1;//unknown error
  if(req->check_mode==0)//使用验证码（旧密码字段无效）
  { //用户在未登录状态，使用验证码修改密码
    if(MobilePhone_check(req->mobilephone) && Password_check(req->new_psw))
    { int vcode_err=vcode_apply(req->verifycode,req->mobilephone);
      if(vcode_err==0)
    	{ db_queryf("update `mc_users` set password=md5('%s') where username='%s'",req->new_psw,req->mobilephone);
        ret_error=0;
    	}
    	else if(vcode_err==ERR_TIMEOUT)ret_error=3; //验证码已经失效
  	else ret_error=2; //验证码错误
    }   
  } 
  else if(req->check_mode==1)//使用旧密码验证（验证码字段无效）
  { //用户在已登录状态，使用旧修改密码（要判断是否登录）
    if(Password_check(req->old_psw) && Password_check(req->new_psw) && packet->terminal && packet->terminal->id>0)
    {  MYSQL_RES *res=db_queryf("select id from `mc_users` where id=%u and password=md5('%s')",packet->terminal->id,req->old_psw);
    	 if(res){
           MYSQL_ROW row=mysql_fetch_row(res);
           if(row && row[0]){
             db_queryf("update `mc_users` set password=md5('%s') where id=%u",req->new_psw,packet->terminal->id);
             ret_error=0;
           }
           else ret_error=1; //凭证不正确（这里是密码错误）  
           mysql_free_result(res);	
         }  	
    }
  }
  msg_ack_general(packet,ret_error);  
}


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

