#include "mc_routine.h"
//---------------------------------------------------------------------------
void spy_notify(U8 value, TNetAddr *spyAddr)
{ TMcMsg *msg=msg_alloc(MSG_STR_SPYNOTIFY,sizeof(TMSG_STR_SPYNOTIFY));
	int  msgLen=MC_MSG_SIZE(msg);
	msg->encrypt=ENCRYPTION_NONE;
	((TMSG_STR_SPYNOTIFY *)msg->body)->value=value;
	msg_UpdateChecksum(msg,msgLen);  //����У��λ 
	hsk_sendData(msg,msgLen,spyAddr);//������Ϣ��
}

void spy_forwardLoginRequest(TMcMsg *msg,TNetAddr *spyAddr)
{ int msgLen;
	U8 encrypt=msg->encrypt;
	if(encrypt)
	{	msg->encrypt=0;
 		msg->bodylen=(msg->msgid==MSG_USR_LOGIN)?sizeof(TMSG_USR_LOGIN):sizeof(TMSG_DSR_LOGIN);
 		msgLen=MC_MSG_SIZE(msg);
 		msg_UpdateChecksum(msg,msgLen); 
  }
  else
  { msgLen=MC_MSG_SIZE(msg);
  }	 	
 	hsk_sendData(msg,msgLen,spyAddr);
  if(encrypt)msg->encrypt=encrypt;
}

//---------------------------------------------------------------------------
void Handle_MSG_TSR_SPYLOGIN(TMcPacket *packet)
{ TMSG_TSR_SPYLOGIN *req=(TMSG_TSR_SPYLOGIN *)packet->msg.body;
	U32 sessionid=0;
	U8 login_error=-1;
	if(MobilePhone_check(req->name))
	{ if(!Password_check(req->psw))login_error=SPY_IDDENTIFY_ERR;//�û������������ 
		else
    {	MYSQL_RES *res=db_queryf("select session from `mc_users` where username='%s' and password=md5('%s')",req->name,req->psw);
    	if(res)
    	{	MYSQL_ROW row=mysql_fetch_row(res);
       if(row)sessionid=atoi(row[0]);
       else login_error=SPY_IDDENTIFY_ERR;	//�û������������
       	mysql_free_result(res);
      } 	
	  }
	}    
	else if(req->name[0] && db_checkSQL(req->name))
	{ if(!Password_check(req->psw))login_error=SPY_IDDENTIFY_ERR;//�û������������ 
		else
    { MYSQL_RES *res=db_queryf("select session from `mc_devices` where sn='%s' and password=md5('%s')",req->name,req->psw);
    	if(res)
    	{	MYSQL_ROW row=mysql_fetch_row(res);
        if(row)sessionid=atoi(row[0]);
        else login_error=SPY_IDDENTIFY_ERR;	//�û������������	
        mysql_free_result(res);
      }  	
    }  
	}
	else if(!req->name[0] && !req->psw[0]) //�û������붼Ϊ�ռ�Ϊע������
	{ //�Ƴ�ָ����ַ�ļ�ض�
		TTerminal *terminal=dtmr_find2(terminalLinks,0,0,&packet->peerAddr,sizeof(TNetAddr),T_NODE_OFFSET(TTerminal,spyAddr),0);
    if(terminal)
    {	terminal->spyAddr.ip=0;
  	  login_error=SPY_LOGOFF_SUCCEED;
    }
	}	
	else login_error=SPY_IDDENTIFY_ERR;//�û������������	
	if(sessionid)
	{	TTerminal *terminal=(TTerminal *)dtmr_find(terminalLinks,sessionid,0,NULL,0); 
		if(terminal)
		{ //ԭ��ض˷�����Ϣ��֪ͨ��ر���ռ��
			if(terminal->spyAddr.ip && (terminal->spyAddr.ip!=packet->peerAddr.ip || terminal->spyAddr.port!=packet->peerAddr.port)!=0)
			{ spy_notify(SPY_TERMINAL_PREEMPT, &terminal->spyAddr);
		  }	
		  //�����µļ�ض˵�ַ
			terminal->spyAddr=packet->peerAddr;
			login_error=SPY_LOGIN_SUCCEED;//��ض˵�¼�ɹ�
	  }	
	  else login_error=SPY_TERMINAL_OFFLINE;//�ն˲�����
  }
  else if(sessionid>0)
  { login_error=SPY_TERMINAL_OFFLINE;//�ն˲�����
  }	
  spy_notify(login_error, &packet->peerAddr);
}

