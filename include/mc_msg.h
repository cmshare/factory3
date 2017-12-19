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

#define MSG_STA_GENERAL           0x80000000    //������ͨ��Ӧ��
#define MSG_TSR_SPYLOGIN          0x0000800A    //��ؿͻ��˵�¼����
#define MSG_STR_SPYNOTIFY         0x0000800B    //�������Լ�ض˵���Ϣ֪ͨ

#define MSG_BSR_NOTIFY            0x00008001    //WEB����ƽ̨(�����)֪ͨ��������̨����

#define MSG_SDR_COMMJSON          0x00000100    //�ն�JSON��Ϣ����Ӧ��ͨ��Ӧ��

#define MSG_DSR_COMMJSON          0x00000101    //�ն�JSON��Ϣ�ϱ���Ӧ��ͨ��Ӧ��

#define MSG_SUR_NOTIFY_MSGBOX    0x00000001    //�������¼�֪ͨ����
#define MSG_USA_NOTIFY_MSGBOX    (MSG_SUR_NOTIFY_MSGBOX|MSG_STA_GENERAL) 

#define MSG_USR_READ_OFFLINEMSG   0x00000004    //��ȡ������Ϣ
#define MSG_SUA_READ_OFFLINEMSG   (MSG_USR_READ_OFFLINEMSG|MSG_STA_GENERAL) 

#define MSG_USR_DELETE_OFFLINEMSG 0x00000005    //ɾ��������Ϣ
#define MSG_SUA_DELETE_OFFLINEMSG (MSG_USR_DELETE_OFFLINEMSG|MSG_STA_GENERAL) 

#define MSG_DSR_LOGIN             0x00000014    //�ն��豸��¼����
#define MSG_SDA_LOGIN             (MSG_DSR_LOGIN|MSG_STA_GENERAL)  //�ն˵�¼��Ӧ
                                  
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
                                  
#define MSG_USR_CHANGESEX         0x0000001A   //�ֻ��޸��û��Ա�����

#define MSG_USR_ACCEPTMSGPUSH     0x0000001B   //�ֻ����ܷ���������Ϣ����:ʹ������

#define MSG_USR_ACCEPTLIVEPUSH    0x0000001C   //�ֻ����������ֻ���ֱ������:ʹ������

#define MSG_USR_CONFIGS           0x0000001D   //�ֻ���ȡϵͳ����
#define MSG_SUA_CONFIGS           (MSG_USR_CONFIGS|MSG_STA_GENERAL) 

#define MSG_USR_BIND              0x00000020   //�ֻ���/������ն�����
#define MSG_SUA_BIND              (MSG_USR_BIND|MSG_STA_GENERAL)  //�ֻ���/������ն���Ӧ

#define MSG_USR_GETBINDLIST_DEPRECATED  0x00000021   //�ֻ���ѯ���б�����
#define MSG_SUA_GETBINDLIST_DEPRECATED  (MSG_USR_GETBINDLIST_DEPRECATED|MSG_STA_GENERAL)    //�ֻ���ѯ���б���Ӧ
                                 
#define MSG_VSR_GETBINDUSER       0x00000022   //�ֻ���ѯ�ն˰��������
#define MSG_SVA_GETBINDUSER       (MSG_VSR_GETBINDUSER|MSG_STA_GENERAL)    //�ֻ���ѯ�ն˰������Ӧ

#define MSG_USR_GETBINDLIST       0x00000023   //�ֻ���ѯ���б�����
#define MSG_SUA_GETBINDLIST       (MSG_USR_GETBINDLIST|MSG_STA_GENERAL)    //�ֻ���ѯ���б���Ӧ
 
#define MSG_USR_QUERY_GPS         0x00000049 //�ֻ���ѯ�ն�final location  (�ֻ�-->������)
#define MSG_SUA_QUERY_GPS         (MSG_USR_QUERY_GPS|MSG_STA_GENERAL) //��Ӧ
                                 
#define MSG_DSR_NOTIFY_STATE       0x00000050 //�ն�״̬֪ͨ����   (�ն�-->������)
#define MSG_SDA_NOTIFY_STATE       (MSG_DSR_NOTIFY_STATE|MSG_STA_GENERAL) //�ն�״̬֪ͨ��Ӧ

#define MSG_SUR_NOTIFY_STATE       0x00000002 //�ն�״̬֪ͨ���� (������--�ֻ�>)
#define MSG_USA_NOTIFY_STATE       (MSG_SUR_NOTIFY_STATE|MSG_STA_GENERAL) //�ն�״̬֪ͨ��Ӧ
                                 
#define MSG_DSR_NOTIFY_STRIKE       0x00000060  //�ն���ײ֪ͨ����
#define MSG_SDA_NOTIFY_STRIKE      (MSG_DSR_NOTIFY_STRIKE|MSG_STA_GENERAL) //�ն���ײ֪ͨ��Ӧ

#define MSG_USR_QUERY_VERSION       0x00000061   //�ֻ���ѯ�汾��Ϣ
#define MSG_SUA_QUERY_VERSION      (MSG_USR_QUERY_VERSION|MSG_STA_GENERAL)  

#define MSG_USR_VERSION_DEPRECATED  0x00000070 //�汾��Ϣͬ������ 
#define MSG_SUA_VERSION_DEPRECATED  (MSG_USR_VERSION_DEPRECATED|MSG_STA_GENERAL)  //�汾��Ϣͬ����Ӧ
                                 
#define MSG_DSR_NOTIFY_LOWPOWER   0x00000064   //�ն˵�ƿȱ��֪ͨ����

#define MSG_DSR_SYNC               0x00000062 //�ն���Ϣͬ������ (�ն�-->������)
#define MSG_SDA_SYNC               (MSG_DSR_SYNC|MSG_STA_GENERAL)  //�ն���Ϣͬ����Ӧ
                                 
#define MSG_USR_WAKEUP             0x00000051  //�ֻ�����/�����ն����� ���ֻ�-->��������
#define MSG_SUA_WAKEUP             (MSG_USR_WAKEUP|MSG_STA_GENERAL)//�ֻ�����/�����ն���Ӧ
                                 
#define MSG_SDR_WAKEUP             0x00000052  //����������/�����ն����󣨷�����-->�նˣ�
#define MSG_DSA_WAKEUP             (MSG_SDR_WAKEUP|MSG_STA_GENERAL)  //����������/�����ն���Ӧ
                                 
#define MSG_USR_LIVE               0x00000040  //�ֻ�ֱ��/ȡ��ֱ������  �����ֻ�-->��������
#define MSG_SUA_LIVE               (MSG_USR_LIVE|MSG_STA_GENERAL)  //�ֻ�ֱ��/ȡ��ֱ����Ӧ
                                 
#define MSG_SVR_LIVE               0x00000041  //������ֱ��/ȡ��ֱ������  (������-->��������ֻ�)
#define MSG_VSA_LIVE               (MSG_SVR_LIVE|MSG_STA_GENERAL) //������ֱ��/ȡ��ֱ����Ӧ
                                 
#define MSG_VSR_LIVE_RET           0x00000044 //�ֻ�ֱ�����֪ͨ���󣨹ۿ��ֻ�-->��������
#define MSG_SVA_LIVE_RET           (MSG_VSR_LIVE_RET|MSG_STA_GENERAL) //�ֻ�ֱ�����֪ͨ��Ӧ��������-->�ۿ��ֻ���
                                 
#define MSG_SUR_LIVE_RET           0x00000045 //������ֱ�����֪ͨ���󣨷�����-->���ֻ���
#define MSG_USA_LIVE_RET           (MSG_SUR_LIVE_RET|MSG_STA_GENERAL) //������ֱ�����֪ͨ��Ӧ
                                 
#define MSG_USR_LIVE_STOP          0x00000046 //���ֻ�ȡ��ֱ�����󣨰��ֻ�-->��������
#define MSG_SUA_LIVE_STOP           (MSG_USR_LIVE_STOP|MSG_STA_GENERAL) //���ֻ�ȡ��ֱ����Ӧ
                                 
#define MSG_VSR_LIVE               0x00000042  //�ֻ�����ֱ��/ȡ���ۿ�����  ���ο��ֻ�-->��������
#define MSG_SVA_LIVE               (MSG_VSR_LIVE|MSG_STA_GENERAL)  //�ֻ�����ֱ��/ȡ���ۿ���Ӧ
                                 
#define MSG_SUR_LIVE               0x00000043  //����������ֱ��/ȡ���ۿ�����  (������-->��������ֻ�)
#define MSG_USA_LIVE               (MSG_SUR_LIVE|MSG_STA_GENERAL) //����������ֱ��/ȡ���ۿ���Ӧ

#define MSG_USR_POST_ADVICE        0x00000047  //�ύͶ�߽�������

#define MSG_USR_QUERY_FLOWPACKAGE  0x00000048  //�ײ�����ʹ�ò�ѯ
#define MSG_SUA_QUERY_FLOWPACKAGE  (MSG_USR_QUERY_FLOWPACKAGE|MSG_STA_GENERAL)
 
#define MSG_DSR_MCU_CHECKVERSION   0x00000071  //MCU�̼��汾������
#define MSG_SDA_MCU_CHECKVERSION   (MSG_DSR_MCU_CHECKVERSION|MSG_STA_GENERAL)

#define MSG_DSR_MCU_UPGRADE        0x00000072  //MCU�̼�����������
#define MSG_SDA_MCU_UPGRADE        (MSG_DSR_MCU_UPGRADE|MSG_STA_GENERAL)

#define MSG_DSR_UPLOAD_GPS         0x00000073  //�ն��ϱ�λ�õ���Ϣ����
#define MSG_SDA_UPLOAD_GPS         (MSG_DSR_UPLOAD_GPS|MSG_STA_GENERAL)

#define MSG_DSR_UPLOAD_BEHAVIOR    0x00000074  //�ն��ϱ���ʻ��Ϊ����
#define MSG_SDA_UPLOAD_BEHAVIOR     (MSG_DSR_UPLOAD_BEHAVIOR|MSG_STA_GENERAL)

#define MSG_DSR_UPLOAD_ICCID       0x00000075  //�ն��ϱ�SIM��ICCID
#define MSG_SDA_UPLOAD_ICCID      (MSG_DSR_UPLOAD_ICCID|MSG_STA_GENERAL)

#define MSG_DSR_UPLOAD_IMSI        0x00000076  //�ն��ϱ�SIM��IMSI��
#define MSG_SDA_UPLOAD_IMSI        (MSG_DSR_UPLOAD_IMSI|MSG_STA_GENERAL)

#define MSG_DSR_QUERY_SN808         0x00000065  //�ն˻�ȡ����808�豸ID����
#define MSG_SDA_QUERY_SN808         (MSG_DSR_QUERY_SN808|MSG_STA_GENERAL)

#define MSG_USR_QUERY_SN808         0x00000066  //�û���ѯ����808�豸ID����
#define MSG_SUA_QUERY_SN808         (MSG_USR_QUERY_SN808|MSG_STA_GENERAL)

#define MSG_DSR_QUERY_SNQQ          0x00000077  //�ն˻�ȡQQ�����豸ID&license����
#define MSG_SDA_QUERY_SNQQ          (MSG_DSR_QUERY_SNQQ|MSG_STA_GENERAL)

#define MSG_USR_QUERY_ACCESSNO      0x00000067  //�ն˲�ѯ������������
#define MSG_SUA_QUERY_ACCESSNO      (MSG_USR_QUERY_ACCESSNO|MSG_STA_GENERAL)

#define MSG_USR_QUERY_SN_FROM808    0x00000068  //�����û�ͨ��808�豸�Ų�ѯ�����豸��
#define MSG_SUA_QUERY_SN_FROM808    (MSG_USR_QUERY_SN_FROM808|MSG_STA_GENERAL)

#define MSG_SDR_FORCE_SLEEP         0x00000078  //ǿ���豸���ߣ�Ӧ��ʽΪͨ��Ӧ��

#define MSG_DSR_UPDATE_BOXSN        0x00000080  //�ն��ϱ������ӵĺ��ӵ��豸�ţ�Ӧ��ʽΪͨ��Ӧ��

#define MSG_DSR_NOTIFY_SNAPSHOT     0x00000081  //ץ���ϴ����֪ͨ��Ӧ��ʽΪͨ��Ӧ��

#define MSG_USR_QUERY_STATE         0x00000053  //�ֻ����������ѯ�ն�״̬
#define MSG_SUA_QUERY_STATE         (MSG_USR_QUERY_STATE|MSG_STA_GENERAL)

#define MSG_SDR_QUERY_STATE         0x00000054  //���������ն˲�ѯ״̬
#define MSG_DSA_QUERY_STATE         (MSG_SDR_QUERY_STATE|MSG_STA_GENERAL)

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
{ U32 ack_synid,ack_msgid;
  U8  error;
}TMSG_STA_GENERAL;

typedef struct
{ U32 ack_synid;
  U8  error;
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
{ U32 ack_synid;
  U32 msg_count;
  TMSG_SUR_NOTIFY_MSGBOX msg_list[0]; //��Ϣ�б�
}TMSG_SUA_READ_OFFLINEMSG;


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
{ U32 usrgroup;
}TMSG_USR_QUERY_VERSION;

typedef struct
{ U32  ack_synid;
  T_VARDATA json;
}TMSG_SUA_CONFIGS,TMSG_SUA_QUERY_VERSION,TMSG_SUA_GETBINDLIST;

typedef struct
{ U32 ack_synid;
  U8  error;
  char uid[MAXLEN_UID+1];
}TMSG_SDA_APPLYFORUID;

typedef struct
{ char ssid[MAXLEN_SSID+1];
}TMSG_DSR_SYNC;

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
{ U32 ack_synid;
  U8  error;
  char uid[MAXLEN_UID+1];
}TMSG_SUA_BIND;

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

//typedef struct
//{ char device_sn[SIZE_SN_DEVICE+1];
//  U8  strike_level;   //��ײ����0����΢ 1��ǿ�� 2�����ң�
//}TMSG_SUR_NOTIFY_STRIKE;

typedef struct
{ U8 action;   //����(0������ֱ�� 1���ܾ�ֱ�������ھܾ������û�������ۿ�ֱ��)
  char visitor_phone[SIZE_MOBILE_PHONE+1];//Ŀ���ο͵��ֻ��û���
  char uid[MAXLEN_UID+1];//ֱ���ն�UID,20λtutk uid
  U8 audio;//����ѡ�0:��������1:����������
}TMSG_USR_LIVE;

typedef struct
{ U32 ack_synid;
  U8  error;
  char nickname[MAXLEN_NICKNAME+1];   //�Կ����ǳƣ�UTF-8���룩
}TMSG_SUA_LIVE,TMSG_SVA_LIVE;

typedef struct
{ U8 action;   //����(0������ֱ�� 1���ܾ�ֱ��)
  char usr_phone[SIZE_MOBILE_PHONE+1];//���������뷽���ֻ��û���
  char usr_nick[MAXLEN_NICKNAME+1];   //�����ǳƣ�UTF-8���룩
  char uid[MAXLEN_UID+1];//ֱ���ն�UID,20λtutk uid
  U8 audio;//����ѡ�0:��������1:����������
}TMSG_SVR_LIVE;

typedef struct
{ char visitor_phone[SIZE_MOBILE_PHONE+1];//Ŀ���ο͵��ֻ��û���
  U8  error;
}TMSG_SUR_LIVE_RET;

typedef struct
{ char visitor_phone[SIZE_MOBILE_PHONE+1];
  char visitor_nick[MAXLEN_NICKNAME+1];   //UTF-8����
}TMSG_SUR_LIVE;

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

typedef struct
{ S32 latitude;//γ��(�Զ�Ϊ��λ��γ��ֵ����10��6�η�����ȷ�������֮һ�ȣ�����Ϊ��γ������Ϊ��γ)
  S32 longitude;//����(�Զ�Ϊ��λ�ľ���ֵ����10��6�η�����ȷ�������֮һ�ȣ�����Ϊ����������Ϊ����)
  S16 altitude;//���θ߶ȣ�in meters according to WGS84 ellipsoid��
  U16 speed;//�����ٶȣ�over ground in m/s��
  U16 azimuth;//ˮƽ�淽λ��,����Ϊ0��˳ʱ�루in degree 0 ~ 359)
}TGPSLocation;
 
typedef struct
{ TMcTime time;
  S32 count;
  TGPSLocation gpsItems[0];
}TMSG_DSR_UPLOAD_GPS;

typedef struct
{ U32 ack_synid;
  U8  error;
  TGPSLocation location;
  U32 time;
}TMSG_SUA_QUERY_GPS;

typedef struct
{ TMcTime time;
  U8 behavior;//0�������٣�1�������٣�2����ת�䣬3������
}TMSG_DSR_UPLOAD_BEHAVIOR;

typedef struct
{ U8 type; //1:Ͷ��;2:����
  char mobile[SIZE_MOBILE_PHONE+1];//Ͷ�����ֻ���
  char advice[512];//utf-8����
}TMSG_USR_POST_ADVICE;

/*
typedef struct
{ char iccid[SIZE_ICCID+1];//Ͷ�����ֻ���
}TMSG_DSR_UPLOAD_ICCID;
*/

typedef struct
{ char imsi[SIZE_IMSI+1];//Ͷ�����ֻ���
}TMSG_DSR_UPLOAD_IMSI;

typedef struct
{ char boxsn[SIZE_SN_BOX+1];//�����豸��
}TMSG_DSR_UPDATE_BOXSN;

typedef struct
{ U32 timestamp;//ץ��ʱ���
  U32 reserved;
}TMSG_DSR_NOTIFY_SNAPSHOT;

typedef struct
{ char name[MAXLEN_PACKAGENAME];//����������
  U32  total_flow,used_flow;//����������ʹ������(��λMB)
  U32  start_time,end_time;//��ʼ���ڣ�unixʱ�����ʽ��
}TFlowPackageItem;

typedef struct
{ U32 ack_synid;
  U8  error;//0:û����-1:δ֪����;1:������Ч�򲻴���
  U32 number;//��ǰ����������
  TFlowPackageItem items[0];
}TMSG_SUA_QUERY_FLOWPACKAGE;

typedef struct
{ U32 action; //1:�豸��Ϣ�ı䣻2:�û���Ϣ�ı�;3:����Ⱥ����Ϣ
  U32 param_size;
  char param_data[0]; 
}TMSG_SSR_NOTIFY;

typedef struct
{ U32  ack_synid;
  U8   error;
  char sn808[SIZE_SN_808+1];
}TMSG_SDA_QUERY_SN808;

typedef struct
{  char device_sn[SIZE_SN_DEVICE+1];
}TMSG_USR_QUERY_SN808,TMSG_USR_QUERY_ACCESSNO;

typedef struct
{ U32  ack_synid;
  U8   error;
  char sn808[SIZE_SN_808+1];//�����ն�808�豸�ţ����ڷǵ����豸�����㡣
}TMSG_SUA_QUERY_SN808;

typedef struct
{ U32  ack_synid;
  U8   error;
  char snqq[SIZE_SN_QQ+1];
  char license[256];//Ŀǰ�144�ֽ�
}TMSG_SDA_QUERY_SNQQ;

typedef struct
{ U32  ack_synid;
  U8   error;
  char accessno[SIZE_ACCESSNO+1];
}TMSG_SUA_QUERY_ACCESSNO;

typedef struct
{ char sn808[SIZE_SN_808+1];
}TMSG_USR_QUERY_SN_FROM808;

typedef struct
{ U32  ack_synid;
  U8   error;
  char device_sn[SIZE_SN_DEVICE+1];
}TMSG_SUA_QUERY_SN_FROM808;
//---------------------------------------------------------------------------
#pragma pack (pop) 
//---------------------------------------------------------------------------
#endif
