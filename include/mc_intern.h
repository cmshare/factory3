//---------------------------------------------------------------------------
#ifndef _MC_INTERN_H
#define _MC_INTERN_H
//---------------------------------------------------------------------------
enum{TT_USER=0,TT_DEVICE=1,TT_BOX=2};
enum{UDP=0,TCP=1};
//enum{SESSION_NULL=0,SESSION_OFFLINE=1,SESSION_RESEARVED1,SESSION_RESEARVED2,MIN_SESSIONID=10};
//����û���sessionidΪ0��ʾ���û��Ѿ�ɾ��/�����ڣ�Ϊ1��ʾ����;���ڵ���MIN_SESSIONID��ʾ���ߡ�
enum{ENCRYPTION_NONE=0,ENCRYPTION_AES128=1,ENCRYPTION_RAS1024=2};
enum{STAT_LIVE_CLOSE=0,STAT_LIVE_OPEN=1};
enum{SPY_NONE_ERR=0,SPY_IDDENTIFY_ERR,SPY_LOGOFF_SUCCEED,SPY_LOGIN_SUCCEED,SPY_TERMINAL_OFFLINE,SPY_TERMINAL_PREEMPT};
enum{DEV_STATE_OFFLINE=0,DEV_STATE_SLEEP,DEV_STATE_WAKEUP,DEV_STATE_ONLINE,DEV_STATE_GOTOSLEEP,DEV_STATE_STARTING,DEV_STATE_CRUISE,DEV_EVENT_ENGINEOFF};
//---------------------------------------------------------------------------
#define SERVER_DYNAMIC_SESSION(msg)      ~((msg)->msgid+(msg)->synid)
#define MC_MSG_SIZE(msg)                  (sizeof(TMcMsg)+(msg)->bodylen+1)
#define MC_PACKET_SIZE(packet)            (sizeof(TMcPacket)+(packet)->msg.bodylen+1)
//#define RESPONSE_APPENDIX(packet)        *((void **)&packet->msg)
//---------------------------------------------------------------------------
#pragma pack (push,1)
//---------------------------------------------------------------------------
typedef struct
{ U32  id,session;
  U32  live_user; //������������ֱ��·�� bind_user -> device -> visiter
  TNetAddr loginAddr,spyAddr;//udp address
 
  //����λ��ṹ���붨����޷������ͣ�����һλ��1�ᱻ���������ͳ�-1�� 
  U32 sex_type:2;//0:����;1:��;2:Ů
  U32 term_type:2;//0��ʾdevice, 1��ʾuser
  U32 term_state:4; //for terminnal 0:����;1:����;2����;3:����
  U32 encrypt:8;//�ն���Ϣ��Ĭ�ϼ��ܷ�ʽ
  U32 group:8;//���豸����	
  U32 live_state:1;//ֱ��״̬��
  U32 msg_push_acceptable:1; //�Ƿ���ܷ�������Ϣ֪ͨ����
  U32 live_push_acceptable:1;//�Ƿ���������ֻ���ֱ������
  char name[0];
}TTerminal;

typedef struct
{ TTerminal terminal;
  char username[SIZE_MOBILE_PHONE+1];
}TTermUser;

typedef struct
{ TTerminal terminal;
  char sn[SIZE_SN_DEVICE+1];
  U32 boxid;
  U32 onlinetime;//���ߵ�ʱ��
}TTermDevice;

typedef struct
{ TTerminal terminal;
  char sn[SIZE_SN_BOX+1];
}TTermBox;

typedef struct
{ U32 msgid;
  U32 sessionid; //�ỰID����¼���������Ϊ�����һ��Ψһ��session��ID��
  U32 synid;     //��ˮ�ţ�������˳��� 0 ��ʼѭ���ۼ�
  U32 bodylen;    //��Ϣ�峤��
  U8  encrypt;   //��Ϣ����ܷ�ʽ(0�������� 1��AES)
  U8  body[0];   //��Ϣ������
}TMcMsg;

typedef struct
{ TTerminal *terminal;
  TNetAddr peerAddr;
  TMcMsg msg;
}TMcPacket;

typedef struct
{ U16 year;
  U8 month,day,hour,minute,second,reserved;
}TMcTime;

typedef struct
{ U32 /*ack_msg,*/retry_counter;
  void *extraData;
  TMcPacket reqPacket;
}TSuspendRequest;

/*
typedef struct
{ U32 ack_msg,retry_counter;
	TTerminal *terminal;
	TMcMsg    *reqMsg;
        //TMcPacket *srcPacket;
	void      *extraData;
}TSuspendRequest;
*/

//---------------------------------------------------------------------------
#pragma pack (pop)  
//---------------------------------------------------------------------------

#endif
