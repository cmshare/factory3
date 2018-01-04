#include "mc_routine.h"
//---------------------------------------------------------------------------
extern void msg_init(void);
extern void db_init(void);
extern void net_init(void);
extern void terminal_init(void);
extern void upload_init(void);
extern void mcufw_init(void);
extern void routine_init(void);
extern void dblog_init(void);
extern void mc_schedule(void);
extern void UWB_system_init(void);
extern void session_init(void);
//---------------------------------------------------------------------------
int main(int argc, char* argv[]){
  //printf("SOCKET_MAX_LISTEN=%d\r\n",SOCKET_MAX_LISTEN);
  srand((int)time(NULL));

  session_init();

  db_init();//database initialize	

  dblog_init();

  msg_init(); //消息队列初始化

  net_init();//initialize tcp and udp socket  
	
  terminal_init();//load and initialize devices and users;

  UWB_system_init();
	
  upload_init();
	 

  routine_init();

  mc_schedule();

  pthread_exit(0);//通过pthread_exit实现退出当前线程（主线程）而不结束进程。
  return 0;
}

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
