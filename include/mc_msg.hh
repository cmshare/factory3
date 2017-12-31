//---------------------------------------------------------------------------
#ifndef _MC_MSGID_H
#define _MC_MSGID_H
//--------------------------------------------------------------------------- 
//符号说明
//U:用户（手机APP用户）
//V:用户（特指APP游客用户）
//D:设备（摄像头设备）
//T:终端，包括设备终端DT与用户终端UT
//R:request; A: Acknowledge
//DSR表示Request of Device to Server
//UDA表示Acknowledge of User to Device

#define MSG_ACK_MASK              0x80000000    //掩码：用于应答指令的最高位置1
#define MSG_STA_GENERAL           MSG_ACK_MASK    //服务器通用应答

#define MSG_BSR_NOTIFY            0x00008001    //WEB管理平台(浏览器)通知服务器后台服务

#define MSG_USR_LOGIN             0x00000012    //手机用户登录请求
#define MSG_SUA_LOGIN             (MSG_USR_LOGIN|MSG_STA_GENERAL)  //用户登录响应

#define MSG_USR_LOGIN2              0x0000001F  //手机用户登录请求(MD5密码）
#define MSG_SUA_LOGIN2              (MSG_USR_LOGIN2|MSG_STA_GENERAL)

#define MSG_SUR_KICKOFF           0x00000031  //服务器将用户踢下线
#define MSG_USA_KICKOFF           (MSG_SUR_KICKOFF|MSG_STA_GENERAL)
                                  
#define MSG_USR_LOGOUT            0x00000013   //手机注销请求，返回服务器通用应答。
                                  
#define MSG_TSR_HEARTBEAT         0x00000030    //手机、终端心跳请求
                                  
#define MSG_USR_VERIFYCODE        0x00000010  //手机申请验证码请求
#define MSG_SUA_VERIFYCODE        (MSG_USR_VERIFYCODE|MSG_STA_GENERAL) //手机申请验证码响应
                                  
#define MSG_USR_REGIST            0x00000011   //手机注册账号请求
#define MSG_SUA_REGIST            (MSG_USR_REGIST|MSG_STA_GENERAL)  //手机注册账号响应

#define MSG_DSR_LOGIN             0x00000014    //终端设备登录请求
#define MSG_SDA_LOGIN             (MSG_DSR_LOGIN|MSG_STA_GENERAL)  //终端登录响应
                                  
#define MSG_DSR_APPLYFORUID       0x00000063   //设备申请UID请求
#define MSG_SDA_APPLYFORUID       (MSG_DSR_APPLYFORUID|MSG_STA_GENERAL) 

#define MSG_USR_CHANGENICK        0x00000016   //手机修改昵称请求，返回服务器通用应答。
                                  
#define MSG_USR_CHANGEPSW         0x00000017   //手机修改密码请求
#define MSG_SUA_CHANGEPSW         (MSG_USR_CHANGEPSW|MSG_STA_GENERAL) //手机修改密码响应
                                  
#define MSG_USR_GETUSERINFO       0x00000018   //手机获取用户资料请求
#define MSG_SUA_GETUSERINFO       (MSG_USR_GETUSERINFO|MSG_STA_GENERAL) //手机获取用户资料响应

#define MSG_USR_CHANGEHEAD        0x00000015   //手机修改头像请求，返回服务器通用应答。
                                  
#define MSG_USR_GETUSERHEAD       0x00000019   //手机获取用户头像信息请求
#define MSG_SUA_GETUSERHEAD       (MSG_USR_GETUSERHEAD|MSG_STA_GENERAL)//手机获取用户头像信息响应
                                  
#define MSG_USR_CONFIGS           0x0000001D   //手机获取系统配置
#define MSG_SUA_CONFIGS           (MSG_USR_CONFIGS|MSG_STA_GENERAL) 

#define MSG_USR_GETBINDLIST       0x00000023   //手机查询绑定列表请求
#define MSG_SUA_GETBINDLIST       (MSG_USR_GETBINDLIST|MSG_STA_GENERAL)    //手机查询绑定列表响应
//---------------------------------------------------------------------------
#pragma pack (push,1)
//---------------------------------------------------------------------------
enum {ERR_UNKNOWN=-1,ERR_NONE=0,ERR_OVERFREQUENCY,ERR_WRONG,ERR_TIMEOUT,ERR_OVERRUN};
enum {ACTION_NONE=0,ACTION_BDR_DEVSTATECHANGED=1,ACTION_BUR_USERSTATECHANGED=2,ACTION_BUR_GROUPMSG=3,ACTION_BUR_GROUPSMS=4,ACTION_BDR_FORCESLEEP=5,ACTION_BDR_COMMJSON=6};
enum {DEVSTATE_OFFLINE=0,DEVSTATE_SLEEP=1,DEVSTATE_WAKEUP=2,DEVSTATE_ONLINE=3};

//1:震动预警; 2:缺电预警 3:流量已经耗尽警告（只提醒一次） 4:流量即将耗尽预警（低于阀值每天提醒一次） 5:流量即将到期预警(最后5天每天提醒一次) 6:震动抓拍上传完毕 10：群发-其他类，11：群发-推广类，12：群发-宣传类
enum {WARNINGMSG_VIBRATE=1,WARNINGMSG_LOWPOWER=2,WARNINGMSG_FLOWDEPLETE=3,WARNINGMSG_LOWFLOW=4,WARNINGMSG_FLOWTOEXPIRE=5,WARNINGMSG_SNAPSHOT_1=6};

typedef struct
{ char name[SIZE_SN_DEVICE+1],psw[MAXLEN_PASSWORD+1];
  U8 type; //0：一代设备; 1：二代盒子；2：二代摄像头
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
  char nick[MAXLEN_NICKNAME+1];   //UTF-8编码
  char verifycode[SIZE_VERIFY_CODE+1];
  U32  groupid;  //用户分组(当前固定为常数2)
}TMSG_USR_REGIST;

typedef struct
{ U32 timestamp; //发生时间（unix时间戳）
  U8  type;  //1:震动预警; 2:缺电预警 3:流量已经耗尽警告（只提醒一次） 4:流量即将耗尽预警（低于阀值每天提醒一次） 5:流量即将到期预警(最后5天每天提醒一次) 10：群发-其他类，11：群发-推广类，12：群发-宣传类
  char content[0];//utf-8,如果是群发的消息（type>=10)，消息内容包含消息标题和消息内容对应的网页地址两部分，两者通过回车符分隔。
}TMSG_SUR_NOTIFY_MSGBOX;

typedef struct
{ U32 datalen;
  U8  data[0];  
}TMSG_USR_CHANGEHEAD,T_VARDATA;

typedef struct
{ U32 ack_synid;
  char nickname[MAXLEN_NICKNAME+1];   //UTF-8编码
  U8   sex; //0:保密;1:男;2:女
  U8   msg_push_acceptable; //是否接受服务器消息通知推送
  U8   live_push_acceptable;//是否接受其他手机的直播推送
  U32  score;//积分（喵币）
}TMSG_SUA_GETUSERINFO;

typedef struct
{ U32  ack_synid;
  T_VARDATA json;
}TMSG_SUA_CONFIGS,TMSG_SUA_QUERY_VERSION,TMSG_SUA_GETBINDLIST;


typedef struct
{ char nick[MAXLEN_NICKNAME+1];   //UTF-8编码
}TMSG_USR_CHANGENICK;

typedef struct
{ U8   check_mode;//0：使用验证码（旧密码字段无效）1：使用旧密码验证（验证码字段无效）
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
   U32 state_type;//0:工作状态;1:UID已经变更(state值无效)
   U8 state_value;
}TMSG_SUR_NOTIFY_STATE;

typedef struct
{ U32 local_version;
}TMSG_DSR_MCU_CHECKVERSION;

typedef struct
{  U32 ack_synid;
   U8  error; // 0:升级；1:不升级
   S32 fw_size; //升级包大小;
   U32 fw_version;//升级包版本;
}TMSG_SDA_MCU_CHECKVERSION;

typedef struct
{ U32 fw_version;//请求升级的版本号
  U32 fw_offset; //升级包数据偏移量
  U32 fw_packet_size;//请求的数据包大小
}TMSG_DSR_MCU_UPGRADE;

typedef struct
{  U32 ack_synid;
   U8  error; // 0:升级；1:不升级
   U32 packet_size; //数据包大小;
   char packet_data[0];
}TMSG_SDA_MCU_UPGRADE;
//---------------------------------------------------------------------------
#pragma pack (pop) 
//---------------------------------------------------------------------------
#endif
