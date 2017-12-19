#include "mc_routine.h"
#include "mc_dispatch.h"
//---------------------------------------------------------------------------
void mc_dispatchmsg(TMcPacket *packet){
//to map user message to handler 
  BEGIN_MESSAGE_MAP_ONLINE
     MESSAGE_HANDLER(MSG_DSR_COMMJSON)  
     MESSAGE_HANDLER(MSG_VSA_LIVE)  
     MESSAGE_HANDLER(MSG_USA_LIVE)
     MESSAGE_HANDLER(MSG_USA_LIVE_RET)
     MESSAGE_HANDLER(MSG_DSA_WAKEUP)
     MESSAGE_HANDLER(MSG_USA_NOTIFY_STATE)
     MESSAGE_HANDLER(MSG_USA_NOTIFY_MSGBOX);
     MESSAGE_HANDLER(MSG_DSR_MCU_CHECKVERSION)
     MESSAGE_HANDLER(MSG_DSR_MCU_UPGRADE)
     MESSAGE_HANDLER(MSG_TSR_HEARTBEAT)
     MESSAGE_HANDLER(MSG_DSR_APPLYFORUID)
     MESSAGE_HANDLER(MSG_DSR_SYNC)
     MESSAGE_HANDLER(MSG_USR_GETUSERHEAD)
     MESSAGE_HANDLER(MSG_USR_CHANGEHEAD)
     MESSAGE_HANDLER(MSG_USR_GETUSERINFO)
     MESSAGE_HANDLER(MSG_USR_CHANGENICK)
     MESSAGE_HANDLER(MSG_USR_CHANGESEX)
     MESSAGE_HANDLER(MSG_USR_ACCEPTMSGPUSH)
     MESSAGE_HANDLER(MSG_USR_ACCEPTLIVEPUSH)
     MESSAGE_HANDLER(MSG_USR_LOGOUT)
     MESSAGE_HANDLER(MSG_USR_BIND)
     MESSAGE_HANDLER(MSG_USR_GETBINDLIST)
     MESSAGE_HANDLER(MSG_VSR_GETBINDUSER)
     MESSAGE_HANDLER(MSG_DSR_NOTIFY_STATE)
     MESSAGE_HANDLER(MSG_DSR_NOTIFY_STRIKE)
     MESSAGE_HANDLER(MSG_DSR_NOTIFY_LOWPOWER)
     MESSAGE_HANDLER(MSG_USR_WAKEUP)
     MESSAGE_HANDLER(MSG_USR_LIVE)
     MESSAGE_HANDLER(MSG_VSR_LIVE)
     MESSAGE_HANDLER(MSG_VSR_LIVE_RET)
     MESSAGE_HANDLER(MSG_USR_LIVE_STOP)
     MESSAGE_HANDLER(MSG_DSR_UPLOAD_GPS)
     MESSAGE_HANDLER(MSG_DSR_UPLOAD_BEHAVIOR)
     MESSAGE_HANDLER(MSG_DSR_UPLOAD_IMSI)
     MESSAGE_HANDLER(MSG_USR_QUERY_FLOWPACKAGE)
     MESSAGE_HANDLER(MSG_USR_READ_OFFLINEMSG)
     MESSAGE_HANDLER(MSG_USR_DELETE_OFFLINEMSG);
     MESSAGE_HANDLER(MSG_DSR_QUERY_SN808);
     MESSAGE_HANDLER(MSG_DSR_QUERY_SNQQ);
     MESSAGE_HANDLER(MSG_USR_QUERY_SN_FROM808);
     MESSAGE_HANDLER(MSG_USR_QUERY_GPS);   	    	 
     MESSAGE_HANDLER(MSG_DSR_UPDATE_BOXSN);
     MESSAGE_HANDLER(MSG_DSR_NOTIFY_SNAPSHOT);
     MESSAGE_HANDLER(MSG_USR_QUERY_STATE);                  
     MESSAGE_HANDLER(MSG_DSA_QUERY_STATE);
  BEGIN_MESSAGE_MAP_ANYWAY
     MESSAGE_HANDLER(MSG_USR_QUERY_SN808);
     MESSAGE_HANDLER(MSG_USR_QUERY_ACCESSNO);
     MESSAGE_HANDLER(MSG_USA_KICKOFF)
     MESSAGE_HANDLER(MSG_DSR_LOGIN) 
     MESSAGE_HANDLER(MSG_USR_LOGIN)
     MESSAGE_HANDLER(MSG_USR_LOGIN2) 
     MESSAGE_HANDLER(MSG_TSR_SPYLOGIN)
     MESSAGE_HANDLER(MSG_USR_QUERY_VERSION)
     MESSAGE_HANDLER(MSG_USR_REGIST)
     MESSAGE_HANDLER(MSG_USR_CHANGEPSW)  //�޸��û����루��¼״̬ͨ��ԭ�����޸ģ�����ʱͨ����֤���޸ģ�
     MESSAGE_HANDLER(MSG_USR_POST_ADVICE)
     MESSAGE_HANDLER(MSG_USR_VERIFYCODE)
     MESSAGE_HANDLER(MSG_USR_CONFIGS)
     MESSAGE_HANDLER(MSG_BSR_NOTIFY)
  END_MESSAGE_MAP
}
//---------------------------------------------------------------------------
/*todolist
##################todo_first_level:
��hsk_init -���Ӱ�����ص�����;hsk_readPacketȡ��hsk_readData;
���������ƣ�Vericode�ϲ���
���ƹ���Ϣweb�˷���ҳ��
�̶��߳��������Ԥ��
��FindTask Lock��������ƣ�����TaskData)
##################todo_second_level:  
�� ���˵�¼��������������Э��ʹ��TCP/UDP���ɡ�
�� ��¼������UDP�����Ա��¼�ͷ���UDP��ַ��������Ҳ������UDP�����Ա�ά�ֿͻ���NATӳ�����֤��������ʱ����ͨ��UDP��ַ�ҵ��ͻ��ˡ�
������MySQL���ӳ�ʱû��������ѯ����������MySQL�ӳ���ʱʱ��Ϊ1�ꡣ
��Ŀǰ�������ݽ��ղ��ö����̣߳������ݷ��Ͳ����ö����̣߳�����ͨ���ʵ�����socket�ķ��ͻ����С�������ⷢ�ʹ�����ʱ���߳�������
##################rules:  
�� 1. �ն�/�豸��״̬��ָ����ͷ��״̬��MiFI���ӵ�״̬����¼
  2. ����ͷͨ�������˺��ϱ�״̬ʱ��Ҫ���µ�����ͷ���ڴ��Լ����ݿ��е�״̬
  3. �յ�APP��������ָ��ʱ�����˺�����ȥ���Ѻ��ӣ�����ֱ�ӻ��Ѱ��豸
  5. ����������ʱ���������ݿ�״̬
  6. ���Ӻ���ר�õ�¼ָ��
  7. ���ӵ�¼������ͷ��¼����
  8. �����ӿڣ�����ͷ�ϱ����Ӱ����ϵ
��  ��
*/ 
