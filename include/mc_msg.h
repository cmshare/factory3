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

#define MSG_ACK_MASK                0x80000000    //服务器通用应答
#define MSG_ACK_GENERAL             MSG_ACK_MASK     //服务器通用应答

#define MSG_TSR_HEARTBEAT           0x00000001    //手机、终端心跳请求，返回服务器通用应答。

//#define MSG_SUR_NOTIFY_MSGBOX     0x00000001    //服务器事件通知请求
//#define MSG_USA_NOTIFY_MSGBOX     (MSG_SUR_NOTIFY_MSGBOX|MSG_ACK_GENERAL) 

#define MSG_DSR_LOGIN                0x00000002    //终端设备登录请求
#define MSG_SDA_LOGIN                (MSG_DSR_LOGIN|MSG_ACK_GENERAL)  //终端登录响应

#define MSG_STR_KICKOFF              0x00000004  //服务器将基站踢下线或者通知sessionid失效

#define MSG_SDR_CALIBRATION_RANGING  0x000000C   //请求标定测距
#define MSG_DSA_CALIBRATION_RANGING  (MSG_SDR_CALIBRATION_RANGING|MSG_ACK_GENERAL)

#define MSG_DSR_CALIBRATION_RESULT   0x000000D   

#define MSG_SDR_CALIBRATE            0x000000E
#define MSG_DSA_CALIBRATE            (MSG_SDR_CALIBRATE|MSG_ACK_GENERAL)

#define MSG_USR_TEST                 0x0000F000   //返回通用应答

#define MSG_USR_REGIST               0x0000F001   //手机注册账号请求
#define MSG_SUA_REGIST               (MSG_USR_REGIST|MSG_ACK_GENERAL)  //手机注册账号响应

#define MSG_USR_LOGIN                0x0000F002    //手机用户登录请求
#define MSG_SUA_LOGIN                (MSG_USR_LOGIN|MSG_ACK_GENERAL)  //用户登录响应

#define MSG_USR_LOGOUT               0x0000F003   //手机注销请求，返回服务器通用应答。

#define MSG_USR_SWITCHLAB            0x0000F007   //切换Lab监听

#define MSG_SUR_POSTDATA             0x0000F008    //发送定位数据包（不要求应答）

#define MSG_USR_GETBINDLIST          0x0000F009   //读取绑定lab列表
#define MSG_SUA_GETBINDLIST          (MSG_USR_GETBINDLIST|MSG_ACK_GENERAL)

#define MSG_USR_GETCONFIG            0x0000F00A   //读取一个指定lab的配置参数
#define MSG_SUA_GETCONFIG            (MSG_USR_GETCONFIG|MSG_ACK_GENERAL)

#define MSG_USR_SETCONFIG            0x0000F00B   //设置lab配置参数（手动标定）

#define MSG_USR_CALIBRATION_START    0x0000F00C   //请求自动标定(返回通用应答)

#define MSG_SUR_CALIBRATION_RESULT   0x0000F00D   //自动标定结果(返回通用应答)

#define MSG_SUR_CALIBRATION_PROGRESS 0x0000F00E   //自动标定状态通知



#define MSG_BSR_NOTIFY             0x00008001    //WEB管理平台(浏览器)通知服务器后台服务

#define MSG_SDR_COMMJSON           0x00000100    //终端JSON信息请求（应答按通用应答）

#define MSG_DSR_COMMJSON           0x00000101    //终端JSON信息上报（应答按通用应答）
                                  
#define MSG_USR_VERIFYCODE         0x00000010  //手机申请验证码请求
#define MSG_SUA_VERIFYCODE         (MSG_USR_VERIFYCODE|MSG_ACK_GENERAL) //手机申请验证码响应
                                  
#define MSG_DSR_APPLYFORUID        0x00000063   //设备申请UID请求
#define MSG_SDA_APPLYFORUID        (MSG_DSR_APPLYFORUID|MSG_ACK_GENERAL) 

#define MSG_USR_CHANGENICK         0x00000016   //手机修改昵称请求，返回服务器通用应答。
                                  
#define MSG_USR_CHANGEPSW          0x00000017   //手机修改密码请求
#define MSG_SUA_CHANGEPSW          (MSG_USR_CHANGEPSW|MSG_ACK_GENERAL) //手机修改密码响应
                                  
#define MSG_USR_GETUSERINFO        0x00000018   //手机获取用户资料请求
#define MSG_SUA_GETUSERINFO        (MSG_USR_GETUSERINFO|MSG_ACK_GENERAL) //手机获取用户资料响应

#define MSG_USR_CHANGEHEAD         0x00000015   //手机修改头像请求，返回服务器通用应答。
                                  
#define MSG_USR_GETUSERHEAD        0x00000019   //手机获取用户头像信息请求
#define MSG_SUA_GETUSERHEAD        (MSG_USR_GETUSERHEAD|MSG_ACK_GENERAL)//手机获取用户头像信息响应
                                  
#define MSG_USR_CHANGESEX          0x0000001A   //手机修改用户性别请求

#define MSG_USR_ACCEPTMSGPUSH      0x0000001B   //手机接受服务器的消息推送:使能请求

#define MSG_USR_ACCEPTLIVEPUSH     0x0000001C   //手机接受其他手机的直播推送:使能请求

#define MSG_USR_CONFIGS            0x0000001D   //手机获取系统配置
#define MSG_SUA_CONFIGS            (MSG_USR_CONFIGS|MSG_ACK_GENERAL) 

#define MSG_USR_BIND               0x00000020   //手机绑定/解除绑定终端请求
#define MSG_SUA_BIND               (MSG_USR_BIND|MSG_ACK_GENERAL)  //手机绑定/解除绑定终端响应

#define MSG_VSR_GETBINDUSER        0x00000022   //手机查询终端绑定情况请求
#define MSG_SVA_GETBINDUSER        (MSG_VSR_GETBINDUSER|MSG_ACK_GENERAL)    //手机查询终端绑定情况响应

 
#define MSG_USR_QUERY_GPS          0x00000049 //手机查询终端final location  (手机-->服务器)
#define MSG_SUA_QUERY_GPS          (MSG_USR_QUERY_GPS|MSG_ACK_GENERAL) //响应
                                 
#define MSG_DSR_NOTIFY_STATE       0x00000050 //终端状态通知请求   (终端-->服务器)
#define MSG_SDA_NOTIFY_STATE       (MSG_DSR_NOTIFY_STATE|MSG_ACK_GENERAL) //终端状态通知响应

#define MSG_SUR_NOTIFY_STATE       0x00000032 //终端状态通知请求 (服务器--手机>)
#define MSG_USA_NOTIFY_STATE       (MSG_SUR_NOTIFY_STATE|MSG_ACK_GENERAL) //终端状态通知响应
                                 
#define MSG_DSR_NOTIFY_STRIKE       0x00000060  //终端碰撞通知请求
#define MSG_SDA_NOTIFY_STRIKE      (MSG_DSR_NOTIFY_STRIKE|MSG_ACK_GENERAL) //终端碰撞通知响应

#define MSG_USR_QUERY_VERSION       0x00000061   //手机查询版本信息
#define MSG_SUA_QUERY_VERSION      (MSG_USR_QUERY_VERSION|MSG_ACK_GENERAL)  

#define MSG_USR_VERSION_DEPRECATED  0x00000070 //版本信息同步请求 
#define MSG_SUA_VERSION_DEPRECATED  (MSG_USR_VERSION_DEPRECATED|MSG_ACK_GENERAL)  //版本信息同步响应
                                 
#define MSG_DSR_NOTIFY_LOWPOWER   0x00000064   //终端电瓶缺电通知请求

#define MSG_DSR_SYNC               0x00000062 //终端信息同步请求 (终端-->服务器)
#define MSG_SDA_SYNC               (MSG_DSR_SYNC|MSG_ACK_GENERAL)  //终端信息同步响应
                                 
#define MSG_USR_WAKEUP             0x00000051  //手机唤醒/休眠终端请求 （手机-->服务器）
#define MSG_SUA_WAKEUP             (MSG_USR_WAKEUP|MSG_ACK_GENERAL)//手机唤醒/休眠终端响应
                                 
#define MSG_SDR_WAKEUP             0x00000052  //服务器唤醒/休眠终端请求（服务器-->终端）
#define MSG_DSA_WAKEUP             (MSG_SDR_WAKEUP|MSG_ACK_GENERAL)  //服务器唤醒/休眠终端响应
                                 
#define MSG_USR_LIVE               0x00000040  //手机直播/取消直播请求  （绑定手机-->服务器）
#define MSG_SUA_LIVE               (MSG_USR_LIVE|MSG_ACK_GENERAL)  //手机直播/取消直播响应
                                 
#define MSG_SVR_LIVE               0x00000041  //服务器直播/取消直播请求  (服务器-->被请求的手机)
#define MSG_VSA_LIVE               (MSG_SVR_LIVE|MSG_ACK_GENERAL) //服务器直播/取消直播响应
                                 
#define MSG_VSR_LIVE_RET           0x00000044 //手机直播结果通知请求（观看手机-->服务器）
#define MSG_SVA_LIVE_RET           (MSG_VSR_LIVE_RET|MSG_ACK_GENERAL) //手机直播结果通知响应（服务器-->观看手机）
                                 
#define MSG_SUR_LIVE_RET           0x00000045 //服务器直播结果通知请求（服务器-->绑定手机）
#define MSG_USA_LIVE_RET           (MSG_SUR_LIVE_RET|MSG_ACK_GENERAL) //服务器直播结果通知响应
                                 
#define MSG_USR_LIVE_STOP          0x00000046 //绑定手机取消直播请求（绑定手机-->服务器）
#define MSG_SUA_LIVE_STOP           (MSG_USR_LIVE_STOP|MSG_ACK_GENERAL) //绑定手机取消直播响应
                                 
#define MSG_VSR_LIVE               0x00000042  //手机申请直播/取消观看请求  （游客手机-->服务器）
#define MSG_SVA_LIVE               (MSG_VSR_LIVE|MSG_ACK_GENERAL)  //手机申请直播/取消观看响应
                                 
#define MSG_SUR_LIVE               0x00000043  //服务器申请直播/取消观看请求  (服务器-->被申请的手机)
#define MSG_USA_LIVE               (MSG_SUR_LIVE|MSG_ACK_GENERAL) //服务器申请直播/取消观看响应

#define MSG_USR_POST_ADVICE        0x00000047  //提交投诉建议请求

#define MSG_USR_QUERY_FLOWPACKAGE  0x00000048  //套餐流量使用查询
#define MSG_SUA_QUERY_FLOWPACKAGE  (MSG_USR_QUERY_FLOWPACKAGE|MSG_ACK_GENERAL)
 
#define MSG_DSR_MCU_CHECKVERSION   0x00000071  //MCU固件版本号请求
#define MSG_SDA_MCU_CHECKVERSION   (MSG_DSR_MCU_CHECKVERSION|MSG_ACK_GENERAL)

#define MSG_DSR_MCU_UPGRADE        0x00000072  //MCU固件升级包请求
#define MSG_SDA_MCU_UPGRADE        (MSG_DSR_MCU_UPGRADE|MSG_ACK_GENERAL)

#define MSG_DSR_UPLOAD_GPS         0x00000073  //终端上报位置点信息请求
#define MSG_SDA_UPLOAD_GPS         (MSG_DSR_UPLOAD_GPS|MSG_ACK_GENERAL)

#define MSG_DSR_UPLOAD_BEHAVIOR    0x00000074  //终端上报驾驶行为请求
#define MSG_SDA_UPLOAD_BEHAVIOR     (MSG_DSR_UPLOAD_BEHAVIOR|MSG_ACK_GENERAL)

#define MSG_DSR_UPLOAD_ICCID       0x00000075  //终端上报SIM卡ICCID
#define MSG_SDA_UPLOAD_ICCID      (MSG_DSR_UPLOAD_ICCID|MSG_ACK_GENERAL)

#define MSG_DSR_UPLOAD_IMSI        0x00000076  //终端上报SIM卡IMSI码
#define MSG_SDA_UPLOAD_IMSI        (MSG_DSR_UPLOAD_IMSI|MSG_ACK_GENERAL)

#define MSG_DSR_QUERY_SN808         0x00000065  //终端获取电信808设备ID请求
#define MSG_SDA_QUERY_SN808         (MSG_DSR_QUERY_SN808|MSG_ACK_GENERAL)

#define MSG_USR_QUERY_SN808         0x00000066  //用户查询电信808设备ID请求
#define MSG_SUA_QUERY_SN808         (MSG_USR_QUERY_SN808|MSG_ACK_GENERAL)

#define MSG_DSR_QUERY_SNQQ          0x00000077  //终端获取QQ物联设备ID&license请求
#define MSG_SDA_QUERY_SNQQ          (MSG_DSR_QUERY_SNQQ|MSG_ACK_GENERAL)

#define MSG_USR_QUERY_ACCESSNO      0x00000067  //终端查询流量卡接入码
#define MSG_SUA_QUERY_ACCESSNO      (MSG_USR_QUERY_ACCESSNO|MSG_ACK_GENERAL)

#define MSG_USR_QUERY_SN_FROM808    0x00000068  //电信用户通过808设备号查询喵星设备号
#define MSG_SUA_QUERY_SN_FROM808    (MSG_USR_QUERY_SN_FROM808|MSG_ACK_GENERAL)

#define MSG_SDR_FORCE_SLEEP         0x00000078  //强制设备休眠（应答方式为通用应答）

#define MSG_DSR_UPDATE_BOXSN        0x00000080  //终端上报所连接的盒子的设备号（应答方式为通用应答）

#define MSG_DSR_NOTIFY_SNAPSHOT     0x00000081  //抓拍上传完成通知（应答方式为通用应答）

#define MSG_USR_QUERY_STATE         0x00000053  //手机向服务器查询终端状态
#define MSG_SUA_QUERY_STATE         (MSG_USR_QUERY_STATE|MSG_ACK_GENERAL)

#define MSG_SDR_QUERY_STATE         0x00000054  //服务器向终端查询状态
#define MSG_DSA_QUERY_STATE         (MSG_SDR_QUERY_STATE|MSG_ACK_GENERAL)

//---------------------------------------------------------------------------
#pragma pack (push,1)
//---------------------------------------------------------------------------
enum {ERR_UNKNOWN=-1,ERR_NONE=0,ERR_OVERFREQUENCY,ERR_WRONG,ERR_TIMEOUT,ERR_OVERRUN};
enum {ACTION_NONE=0,ACTION_BDR_DEVSTATECHANGED=1,ACTION_BUR_USERSTATECHANGED=2,ACTION_BUR_GROUPMSG=3,ACTION_BUR_GROUPSMS=4,ACTION_BDR_FORCESLEEP=5,ACTION_BDR_COMMJSON=6};
enum {DEVSTATE_OFFLINE=0,DEVSTATE_SLEEP=1,DEVSTATE_WAKEUP=2,DEVSTATE_ONLINE=3};

//1:震动预警; 2:缺电预警 3:流量已经耗尽警告（只提醒一次） 4:流量即将耗尽预警（低于阀值每天提醒一次） 5:流量即将到期预警(最后5天每天提醒一次) 6:震动抓拍上传完毕 10：群发-其他类，11：群发-推广类，12：群发-宣传类
enum {WARNINGMSG_VIBRATE=1,WARNINGMSG_LOWPOWER=2,WARNINGMSG_FLOWDEPLETE=3,WARNINGMSG_LOWFLOW=4,WARNINGMSG_FLOWTOEXPIRE=5,WARNINGMSG_SNAPSHOT_1=6};

typedef struct
{ U32 anchorid;  //基站ID
  U32 version;   //基站当前版本（目前固定为0）
}TMSG_DSR_LOGIN;

typedef struct
{ char name[MAXLEN_USERNAME+1],passwd[SIZE_MD5+1];
}TMSG_USR_LOGIN;

typedef struct{
 U32 ack_msgid;
 U8  error;
}TMSG_ACK_GENERAL;//,TMSG_SVA_LIVE_RET,TMSG_SUA_VERIFYCODE,TMSG_SUA_REGIST,TMSG_SUA_CHANGEPSW,TMSG_SDA_NOTIFY_STATE,TMSG_USA_NOTIFY_STATE,TMSG_SDA_NOTIFY_STRIKE,TMSG_USA_NOTIFY_STRIKE,TMSG_SUA_WAKEUP,TMSG_DSA_WAKEUP,TMSG_USA_LIVE,TMSG_VSA_LIVE,TMSG_SDA_SYNC,TMSG_USA_LIVE_RET,TMSG_SDA_UPLOAD_GPS,TMSG_SDA_UPLOAD_BEHAVIOR,TMSG_SDA_UPLOAD_IMSI/*,TMSG_SDA_UPLOAD_ICCID*/;

typedef struct
{ U32 ack_synid;
  U8  error,state;
}TMSG_SUA_QUERY_STATE,TMSG_DSA_QUERY_STATE ;

typedef struct
{ U8  value;
}TMSG_SSR_SMS,TMSG_STR_SPYNOTIFY,TMSG_VSR_LIVE_RET,TMSG_DSR_NOTIFY_STATE,TMSG_DSR_NOTIFY_STRIKE,TMSG_SDR_WAKEUP,TMSG_USR_CHANGESEX,TMSG_USR_ACCEPTMSGPUSH,TMSG_USR_ACCEPTLIVEPUSH;

typedef struct{
  U32 session;
  U32 flags;
}TMSG_SDA_LOGIN,TMSG_SUA_LOGIN;

typedef struct
{ char phone[SIZE_PHONE_MOBILE+1];
  U32  groupid;
}TMSG_USR_VERIFYCODE;

typedef struct
{ char phone[SIZE_PHONE_MOBILE+1];
}TMSG_VSR_LIVE,TMSG_USR_GETUSERHEAD;

typedef struct
{ char username[MAXLEN_USERNAME+1];
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
{ U32 ack_synid;
  U32 msg_count;
  TMSG_SUR_NOTIFY_MSGBOX msg_list[0]; //消息列表
}TMSG_SUA_READ_OFFLINEMSG;


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
{ U32 usrgroup;
}TMSG_USR_QUERY_VERSION;

typedef struct
{ U32  ack_synid;
  T_VARDATA json;
}TMSG_SUA_CONFIGS,TMSG_SUA_QUERY_VERSION;


typedef struct
{ char nick[MAXLEN_NICKNAME+1];   //UTF-8编码
}TMSG_USR_CHANGENICK;

typedef struct
{ U8   check_mode;//0：使用验证码（旧密码字段无效）1：使用旧密码验证（验证码字段无效）
  char username[MAXLEN_USERNAME+1];
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
  char username[MAXLEN_USERNAME+1];
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

typedef struct
{ S32 latitude;//纬度(以度为单位的纬度值乘以10的6次方，精确到百万分之一度，正数为北纬，负数为南纬)
  S32 longitude;//经度(以度为单位的经度值乘以10的6次方，精确到百万分之一度，正数为东经，负数为西经)
  S16 altitude;//海拔高度，in meters according to WGS84 ellipsoid。
  U16 speed;//地面速度（over ground in m/s）
  U16 azimuth;//水平面方位角,正北为0，顺时针（in degree 0 ~ 359)
}TGPSLocation;
 

typedef struct
{ U8 type; //1:投诉;2:建议
  char username[MAXLEN_USERNAME+1];//投诉人手机号
  char advice[512];//utf-8编码
}TMSG_USR_POST_ADVICE;

/*
typedef struct
{ char iccid[SIZE_ICCID+1];//投诉人手机号
}TMSG_DSR_UPLOAD_ICCID;
*/

typedef struct
{ char name[MAXLEN_PACKAGENAME];//流量包名称
  U32  total_flow,used_flow;//总流量及已使用流量(单位MB)
  U32  start_time,end_time;//起始日期（unix时间戳格式）
}TFlowPackageItem;

typedef struct
{ U32 ack_synid;
  U8  error;//0:没错误；-1:未知错误;1:卡号无效或不存在
  U32 number;//当前流量包数量
  TFlowPackageItem items[0];
}TMSG_SUA_QUERY_FLOWPACKAGE;

typedef struct
{ U32 action; //1:设备信息改变；2:用户信息改变;3:推送群组消息
  U32 param_size;
  char param_data[0]; 
}TMSG_SSR_NOTIFY;

//---------------------------------------------------------------------------
#pragma pack (pop) 
//---------------------------------------------------------------------------
#endif
