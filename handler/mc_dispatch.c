#include "mc_routine.h"
#include "mc_dispatch.h"
//---------------------------------------------------------------------------
void mc_dispatchmsg(TMcPacket *packet){
//to map user message to handler 
  BEGIN_MESSAGE_MAP_ONLINE
     MESSAGE_HANDLER(MSG_TSR_HEARTBEAT)
     MESSAGE_HANDLER(MSG_USR_GETUSERHEAD)
     MESSAGE_HANDLER(MSG_USR_CHANGEHEAD)
     MESSAGE_HANDLER(MSG_USR_GETUSERINFO)
     MESSAGE_HANDLER(MSG_USR_GETBINDLIST)
     MESSAGE_HANDLER(MSG_USR_CHANGENICK)
     MESSAGE_HANDLER(MSG_USR_LOGOUT)
     MESSAGE_HANDLER(MSG_USR_SWITCHLAB)
     MESSAGE_HANDLER(MSG_USR_SETCONFIG)
  BEGIN_MESSAGE_MAP_ANYWAY
     MESSAGE_HANDLER(MSG_USA_KICKOFF)
     MESSAGE_HANDLER(MSG_DSR_LOGIN) 
     MESSAGE_HANDLER(MSG_USR_LOGIN)
     MESSAGE_HANDLER(MSG_USR_CONFIGS)
  END_MESSAGE_MAP
}
//---------------------------------------------------------------------------
/*todolist
##################todo_first_level:
◆hsk_init -增加包检验回调函数;hsk_readPacket取代hsk_readData;
◆短信限制（Vericode合并）
◆推广消息web端发送页面
√多线程流量监测预警
√FindTask Lock与解锁机制（保护TaskData)
##################todo_second_level:  
◆ 除了登录及心跳包，其他协议使用TCP/UDP均可。
◆ 登录必须用UDP包，以便记录客服端UDP地址，心跳包也必须用UDP包，以便维持客户端NAT映射表。保证服务器随时可以通过UDP地址找到客户端。
◆现在MySQL连接超时没用心跳查询，而是设置MySQL加长超时时间为1年。
◆目前网络数据接收采用独立线程，而数据发送不采用独立线程，而是通过适当增加socket的发送缓冲大小，来避免发送大数据时的线程阻塞。
##################rules:  
◆ 1. 终端/设备的状态是指摄像头的状态，MiFI盒子的状态不记录
  2. 摄像头通过盒子账号上报状态时，要更新到摄像头在内存以及数据库中的状态
  3. 收到APP唤醒请求指令时，绑定了盒子则去唤醒盒子，否则直接唤醒绑定设备
  5. 盒子心跳超时，更新数据库状态
  6. 增加盒子专用登录指令
  7. 盒子登录与摄像头登录互斥
  8. 新增接口：摄像头上报盒子绑带关系
◆  。
*/ 
