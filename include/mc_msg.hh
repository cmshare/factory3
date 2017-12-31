//---------------------------------------------------------------------------
#ifndef _MC_MSGID_H
#define _MC_MSGID_H
//--------------------------------------------------------------------------- 
//����˵��
//U:�û����ֻ�APP�û���
//V:�û�����ָAPP�ο��û���
//D:�豸������ͷ�豸��
//T:�նˣ������豸�ն�DT���û��ն�UT
//R:request; A: Acknowledge
//DSR��ʾRequest of Device to Server
//UDA��ʾAcknowledge of User to Device

#define MSG_ACK_MASK              0x80000000    //���룺����Ӧ��ָ������λ��1
#define MSG_STA_GENERAL           MSG_ACK_MASK    //������ͨ��Ӧ��

#define MSG_BSR_NOTIFY            0x00008001    //WEB����ƽ̨(�����)֪ͨ��������̨����

#define MSG_USR_LOGIN             0x00000012    //�ֻ��û���¼����
#define MSG_SUA_LOGIN             (MSG_USR_LOGIN|MSG_STA_GENERAL)  //�û���¼��Ӧ

#define MSG_USR_LOGIN2              0x0000001F  //�ֻ��û���¼����(MD5���룩
#define MSG_SUA_LOGIN2              (MSG_USR_LOGIN2|MSG_STA_GENERAL)

#define MSG_SUR_KICKOFF           0x00000031  //���������û�������
#define MSG_USA_KICKOFF           (MSG_SUR_KICKOFF|MSG_STA_GENERAL)
                                  
#define MSG_USR_LOGOUT            0x00000013   //�ֻ�ע�����󣬷��ط�����ͨ��Ӧ��
                                  
#define MSG_TSR_HEARTBEAT         0x00000030    //�ֻ����ն���������
                                  
#define MSG_USR_VERIFYCODE        0x00000010  //�ֻ�������֤������
#define MSG_SUA_VERIFYCODE        (MSG_USR_VERIFYCODE|MSG_STA_GENERAL) //�ֻ�������֤����Ӧ
                                  
#define MSG_USR_REGIST            0x00000011   //�ֻ�ע���˺�����
#define MSG_SUA_REGIST            (MSG_USR_REGIST|MSG_STA_GENERAL)  //�ֻ�ע���˺���Ӧ

#define MSG_DSR_LOGIN             0x00000014    //�ն��豸��¼����
#define MSG_SDA_LOGIN             (MSG_DSR_LOGIN|MSG_STA_GENERAL)  //�ն˵�¼��Ӧ
                                  
#define MSG_DSR_APPLYFORUID       0x00000063   //�豸����UID����
#define MSG_SDA_APPLYFORUID       (MSG_DSR_APPLYFORUID|MSG_STA_GENERAL) 

#define MSG_USR_CHANGENICK        0x00000016   //�ֻ��޸��ǳ����󣬷��ط�����ͨ��Ӧ��
                                  
#define MSG_USR_CHANGEPSW         0x00000017   //�ֻ��޸���������
#define MSG_SUA_CHANGEPSW         (MSG_USR_CHANGEPSW|MSG_STA_GENERAL) //�ֻ��޸�������Ӧ
                                  
#define MSG_USR_GETUSERINFO       0x00000018   //�ֻ���ȡ�û���������
#define MSG_SUA_GETUSERINFO       (MSG_USR_GETUSERINFO|MSG_STA_GENERAL) //�ֻ���ȡ�û�������Ӧ

#define MSG_USR_CHANGEHEAD        0x00000015   //�ֻ��޸�ͷ�����󣬷��ط�����ͨ��Ӧ��
                                  
#define MSG_USR_GETUSERHEAD       0x00000019   //�ֻ���ȡ�û�ͷ����Ϣ����
#define MSG_SUA_GETUSERHEAD       (MSG_USR_GETUSERHEAD|MSG_STA_GENERAL)//�ֻ���ȡ�û�ͷ����Ϣ��Ӧ
                                  
#define MSG_USR_CONFIGS           0x0000001D   //�ֻ���ȡϵͳ����
#define MSG_SUA_CONFIGS           (MSG_USR_CONFIGS|MSG_STA_GENERAL) 

#define MSG_USR_GETBINDLIST       0x00000023   //�ֻ���ѯ���б�����
#define MSG_SUA_GETBINDLIST       (MSG_USR_GETBINDLIST|MSG_STA_GENERAL)    //�ֻ���ѯ���б���Ӧ
//---------------------------------------------------------------------------
#pragma pack (push,1)
//---------------------------------------------------------------------------
enum {ERR_UNKNOWN=-1,ERR_NONE=0,ERR_OVERFREQUENCY,ERR_WRONG,ERR_TIMEOUT,ERR_OVERRUN};
enum {ACTION_NONE=0,ACTION_BDR_DEVSTATECHANGED=1,ACTION_BUR_USERSTATECHANGED=2,ACTION_BUR_GROUPMSG=3,ACTION_BUR_GROUPSMS=4,ACTION_BDR_FORCESLEEP=5,ACTION_BDR_COMMJSON=6};
enum {DEVSTATE_OFFLINE=0,DEVSTATE_SLEEP=1,DEVSTATE_WAKEUP=2,DEVSTATE_ONLINE=3};

//1:��Ԥ��; 2:ȱ��Ԥ�� 3:�����Ѿ��ľ����棨ֻ����һ�Σ� 4:���������ľ�Ԥ�������ڷ�ֵÿ������һ�Σ� 5:������������Ԥ��(���5��ÿ������һ��) 6:��ץ���ϴ���� 10��Ⱥ��-�����࣬11��Ⱥ��-�ƹ��࣬12��Ⱥ��-������
enum {WARNINGMSG_VIBRATE=1,WARNINGMSG_LOWPOWER=2,WARNINGMSG_FLOWDEPLETE=3,WARNINGMSG_LOWFLOW=4,WARNINGMSG_FLOWTOEXPIRE=5,WARNINGMSG_SNAPSHOT_1=6};

typedef struct
{ char name[SIZE_SN_DEVICE+1],psw[MAXLEN_PASSWORD+1];
  U8 type; //0��һ���豸; 1���������ӣ�2����������ͷ
}TMSG_DSR_LOGIN,TMSG_TSR_SPYLOGIN;

typedef struct
{ char name[SIZE_MOBILE_PHONE+1],psw[MAXLEN_PASSWORD+1];
}TMSG_USR_LOGIN;

typedef struct
{ char name[SIZE_MOBILE_PHONE+1],psw_md5[SIZE_MD5+1];
}TMSG_USR_LOGIN2;

typedef struct
{ U32 ack_msgid;
  U8  error;
}TMSG_STA_GENERAL;

typedef struct
{ U32 result;
}TMSG_ACK_GENERAL;//,TMSG_SVA_LIVE_RET,TMSG_SUA_VERIFYCODE,TMSG_SUA_REGIST,TMSG_SUA_CHANGEPSW,TMSG_SDA_NOTIFY_STATE,TMSG_USA_NOTIFY_STATE,TMSG_SDA_NOTIFY_STRIKE,TMSG_USA_NOTIFY_STRIKE,TMSG_SUA_WAKEUP,TMSG_DSA_WAKEUP,TMSG_USA_LIVE,TMSG_VSA_LIVE,TMSG_SDA_SYNC,TMSG_USA_LIVE_RET,TMSG_SDA_UPLOAD_GPS,TMSG_SDA_UPLOAD_BEHAVIOR,TMSG_SDA_UPLOAD_IMSI/*,TMSG_SDA_UPLOAD_ICCID*/;

typedef struct
{ U32 ack_synid;
  U8  error,state;
}TMSG_SUA_QUERY_STATE,TMSG_DSA_QUERY_STATE ;

typedef struct
{ U8  value;
}TMSG_SSR_SMS,TMSG_STR_SPYNOTIFY,TMSG_VSR_LIVE_RET,TMSG_DSR_NOTIFY_STATE,TMSG_DSR_NOTIFY_STRIKE,TMSG_SDR_WAKEUP,TMSG_USR_CHANGESEX,TMSG_USR_ACCEPTMSGPUSH,TMSG_USR_ACCEPTLIVEPUSH;

typedef struct
{ U32 ack_synid;
  U8  error;
  U32 session;
}TMSG_SDA_LOGIN,TMSG_SUA_LOGIN;

typedef struct
{ char phone[SIZE_MOBILE_PHONE+1];
  U32  groupid;
}TMSG_USR_VERIFYCODE;

typedef struct
{ char phone[SIZE_MOBILE_PHONE+1];
}TMSG_VSR_LIVE,TMSG_USR_GETUSERHEAD;

typedef struct
{ char phone[SIZE_MOBILE_PHONE+1];
  char psw[MAXLEN_PASSWORD+1];
  char nick[MAXLEN_NICKNAME+1];   //UTF-8����
  char verifycode[SIZE_VERIFY_CODE+1];
  U32  groupid;  //�û�����(��ǰ�̶�Ϊ����2)
}TMSG_USR_REGIST;

typedef struct
{ U32 timestamp; //����ʱ�䣨unixʱ�����
  U8  type;  //1:��Ԥ��; 2:ȱ��Ԥ�� 3:�����Ѿ��ľ����棨ֻ����һ�Σ� 4:���������ľ�Ԥ�������ڷ�ֵÿ������һ�Σ� 5:������������Ԥ��(���5��ÿ������һ��) 10��Ⱥ��-�����࣬11��Ⱥ��-�ƹ��࣬12��Ⱥ��-������
  char content[0];//utf-8,�����Ⱥ������Ϣ��type>=10)����Ϣ���ݰ�����Ϣ�������Ϣ���ݶ�Ӧ����ҳ��ַ�����֣�����ͨ���س����ָ���
}TMSG_SUR_NOTIFY_MSGBOX;

typedef struct
{ U32 datalen;
  U8  data[0];  
}TMSG_USR_CHANGEHEAD,T_VARDATA;

typedef struct
{ U32 ack_synid;
  char nickname[MAXLEN_NICKNAME+1];   //UTF-8����
  U8   sex; //0:����;1:��;2:Ů
  U8   msg_push_acceptable; //�Ƿ���ܷ�������Ϣ֪ͨ����
  U8   live_push_acceptable;//�Ƿ���������ֻ���ֱ������
  U32  score;//���֣����ң�
}TMSG_SUA_GETUSERINFO;

typedef struct
{ U32  ack_synid;
  T_VARDATA json;
}TMSG_SUA_CONFIGS,TMSG_SUA_QUERY_VERSION,TMSG_SUA_GETBINDLIST;


typedef struct
{ char nick[MAXLEN_NICKNAME+1];   //UTF-8����
}TMSG_USR_CHANGENICK;

typedef struct
{ U8   check_mode;//0��ʹ����֤�루�������ֶ���Ч��1��ʹ�þ�������֤����֤���ֶ���Ч��
  char mobilephone[SIZE_MOBILE_PHONE+1];
  char verifycode[SIZE_VERIFY_CODE+1];
  char old_psw[MAXLEN_PASSWORD+1];
  char new_psw[MAXLEN_PASSWORD+1];
}TMSG_USR_CHANGEPSW;

typedef struct
{ U32 ack_synid;
  U32 data_size;
  U8  data[0]; 
}TMSG_SUA_GETUSERHEAD;

typedef struct
{ char device_sn[SIZE_SN_DEVICE+1];
  U8   action;
}TMSG_USR_BIND,TMSG_USR_WAKEUP;


typedef struct
{ char sn[SIZE_SN_DEVICE+1];
}TMSG_VSR_GETBINDUSER,TMSG_USR_QUERY_FLOWPACKAGE,TMSG_USR_QUERY_GPS,TMSG_USR_QUERY_STATE,TMSG_SDR_QUERY_STATE;

typedef struct
{ U32 ack_synid;
  U8 binded;
  char phone[SIZE_MOBILE_PHONE+1];
}TMSG_SVA_GETBINDUSER;

typedef struct
{  char device_sn[SIZE_SN_DEVICE+1];
   U32 state_type;//0:����״̬;1:UID�Ѿ����(stateֵ��Ч)
   U8 state_value;
}TMSG_SUR_NOTIFY_STATE;

typedef struct
{ U32 local_version;
}TMSG_DSR_MCU_CHECKVERSION;

typedef struct
{  U32 ack_synid;
   U8  error; // 0:������1:������
   S32 fw_size; //��������С;
   U32 fw_version;//�������汾;
}TMSG_SDA_MCU_CHECKVERSION;

typedef struct
{ U32 fw_version;//���������İ汾��
  U32 fw_offset; //����������ƫ����
  U32 fw_packet_size;//��������ݰ���С
}TMSG_DSR_MCU_UPGRADE;

typedef struct
{  U32 ack_synid;
   U8  error; // 0:������1:������
   U32 packet_size; //���ݰ���С;
   char packet_data[0];
}TMSG_SDA_MCU_UPGRADE;
//---------------------------------------------------------------------------
#pragma pack (pop) 
//---------------------------------------------------------------------------
#endif
