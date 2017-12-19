#include "mc_routine.h"
//---------------------------------------------------------------------------
/*
UID���е�UID�����ݷ���״�������ֻ���״̬��
(1). δ����UID���Ѿ��ͷŻ�δʹ�õĿ���UID��
(2). �ѷ���UID���Ѱ󶨵�ĳ���ն˵�UID����
�����ѷ���UID���������豸����״̬�ַ����֣�
(1) ����UID ���豸����)
(2) ����UID ���豸����)
UID������ƣ�
 ��1���û��������UIDʱ�������ǰUIDû�б��ͷţ����Ծɷ��䵱ǰUID��
 ��2���û��������UIDʱ�������ǰUID�Ѿ����ͷţ������������һ����ֱ�Ӵ�UID���з���һ�����е�UID����һ���ǣ�ǿ���ͷ�һ������ʱ�����UID��      �����ǰUID���п���UID���㣬����õ�һ�֣�������õڶ��֡����ٽ���������UID��<uid_reserved_num)��
*/
//---------------------------------------------------------------------------
void Handle_MSG_DSR_APPLYFORUID(TMcPacket *packet){
  TMcMsg *ackmsg=msg_alloc(MSG_SDA_APPLYFORUID,sizeof(TMSG_SDA_APPLYFORUID));
  TMSG_SDA_APPLYFORUID *ackbody=(TMSG_SDA_APPLYFORUID *)ackmsg->body;
  char bind_phone[SIZE_MOBILE_PHONE+1];
  BOOL uid_changed=FALSE;
  ackbody->error=1;
  bind_phone[0]='\0';

  db_lock(TRUE);
  //get bindedUser and origin UID of the device
  //the return UID should be null where the origin UID of the device is not in the uidpool
  //MYSQL_RES *rs=db_queryf("select `mc_devices`.uid,`mc_devices`.username,`mc_uidpool`.uid from `mc_devices` left join `mc_uidpool` on `mc_devices`.sn=`mc_uidpool`.sn where `mc_devices`.id=%u",packet->terminal->id);
  MYSQL_RES *rs=db_queryf("select `mc_devices`.username,`mc_uidpool`.uid,`mc_uidpool`.id from `mc_devices` left join `mc_uidpool` on `mc_devices`.sn=`mc_uidpool`.sn where `mc_devices`.id=%u",packet->terminal->id);
  if(rs)
  { MYSQL_ROW row=mysql_fetch_row(rs);
    if(row){
      if(row[1] && row[1][0])
      { //ԭ����UID��δ�����գ����Է���ʹ��ԭ����UID
        strncpy(ackbody->uid,row[1],MAXLEN_UID+1);
        ackbody->error=0;
      }
      else
      { if(row[0])strncpy(bind_phone,row[0],SIZE_MOBILE_PHONE+1);	
        if(row[2])db_queryf("update `mc_uidpool` set sn=null where id=%s",row[2]); //this case may not happen.
      }
    }	
    mysql_free_result(rs);  	
  }
  
  if(ackbody->error){
    //��ȡһ�����е�UID
    if((rs=db_query("select uid from `mc_uidpool` where sn is null and uid is not null order by id asc limit 1"))){
      MYSQL_ROW row=mysql_fetch_row(rs);
      if(row && row[0]){
         strncpy(ackbody->uid,row[0],MAXLEN_UID+1);
         ackbody->error=0;
         uid_changed=TRUE;
      }
      mysql_free_result(rs);  
    }
    if(!uid_changed){//�ҵ�һ������ʱ�����UID�����ƻ�����������·���
      if((rs=db_query("select `mc_uidpool`.uid from `mc_devices` inner join `mc_uidpool` on `mc_devices`.sn=`mc_uidpool`.sn where `mc_devices`.session=0 and `mc_devices`.sn is not null and `mc_uidpool`.uid is not null order by `mc_devices`.logouttime asc limit 1"))){
       MYSQL_ROW row=mysql_fetch_row(rs);
       if(row && row[0]){
         strncpy(ackbody->uid,row[0],MAXLEN_UID+1);
    	 ackbody->error=0;
    	 uid_changed=TRUE;
        }
        mysql_free_result(rs);  
      }
    }		
  }
  ackbody->ack_synid=packet->msg.synid;
  msg_send(ackmsg,packet,NULL); 	
  
  //���UID�������ʱ������UID�󶨹�ϵ����֪ͨ���ֻ�
  if(!ackbody->error && uid_changed){
    db_queryf("update `mc_uidpool` set sn='%s',updatetime=unix_timestamp() where uid='%s'",packet->terminal->name,ackbody->uid);
    if(bind_phone[0]){
      if((rs=db_queryf("select session from `mc_users` where username='%s'",bind_phone))){
        MYSQL_ROW row=mysql_fetch_row(rs);
        if(row && row[0]){
          U32 binded_usersession=atoi(row[0]);
          if(binded_usersession){
            TTerminal * usrnode=(TTerminal *)dtmr_find(terminalLinks,binded_usersession,0,NULL,0);
            if(usrnode){//if binded user is online
              TMcMsg *reqmsg=msg_alloc(MSG_SUR_NOTIFY_STATE,sizeof(TMSG_SUR_NOTIFY_STATE));
              TMSG_SUR_NOTIFY_STATE *ackbody=(TMSG_SUR_NOTIFY_STATE *)reqmsg->body;
              strncpy(ackbody->device_sn,packet->terminal->name,SIZE_SN_DEVICE+1);
              ackbody->state_type=1;//UID�Ѿ����(stateֵ��Ч)
              ackbody->state_value=0;//UID�Ѿ����(stateֵ��Ч)
              msg_request(reqmsg,usrnode,NULL,0);
            }
          }    
        }
        mysql_free_result(rs);
      }
    }   
  }
  db_lock(FALSE);
}
