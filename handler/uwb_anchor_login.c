#include "mc_routine.h"
#include "uwb_model.h"


void Handle_MSG_DSR_LOGIN(TMcPacket *packet){
  TMSG_DSR_LOGIN *content=(TMSG_DSR_LOGIN *)packet->msg.body;
  U32 anchorID=content->anchorid;
  int uwbLabID=0,devGroupID;
  U8 error_code=1;
  TUWBAnchor *anchor=NULL;
  session_lock(TRUE);
  if(anchorID)
  {  MYSQL_RES *res;
     res=db_queryf("select sessionid,labid,groupid from `uwb_anchor` where id=%u",anchorID);
     if(res)
     { MYSQL_ROW row=mysql_fetch_row(res);
       if(row)
       {  U32 origin_sessionid=atoi(row[0]);
          uwbLabID=atoi(row[1]);
          devGroupID=atoi(row[2]);
          if(origin_sessionid) {
            anchor=(TUWBAnchor *)dtmr_findById(dtmr_termLinks,origin_sessionid,TRUE);//HEARTBEAT_OVERTIME_MS);
            if(anchor && anchor->terminal.id!=anchorID)
            { //上一次使用的session已经被其他设备占用（所查到的terminal是其他设备）。
              dtmr_unlock(anchor,0);
              anchor=NULL;
            }
          }

       }
       mysql_free_result(res);
     }
  }

  if(uwbLabID>0){
    if(!anchor){
      anchor=UWB_anchor_load(anchorID,uwbLabID);
      if(anchor){
        anchor->terminal.term_type=TT_DEVICE;
        anchor->terminal.group=devGroupID;
        anchor->terminal.encrypt=packet->msg.encrypt;//消息体默认加密方式
      }
    }
    if(anchor){
      anchor->terminal.loginAddr=packet->peerAddr;//登录方式必须要求UDP
      anchor->terminal.term_state=DEV_STATE_ONLINE;
      error_code=0;
      dtmr_unlock(anchor,HEARTBEAT_OVERTIME_MS);
      db_queryf("update `uwb_anchor` set sessionid=%u,state=%d,ip=%u,port=%u,logintime=%u where id=%u",anchor->terminal.sessionid,DEV_STATE_ONLINE,packet->peerAddr.ip,packet->peerAddr.port,(U32)time(NULL),anchorID);
    }
  }

  packet->terminal=(TTerminal *)anchor;
  TMcMsg *msg=msg_alloc(MSG_SDA_LOGIN,sizeof(TMSG_SDA_LOGIN));
  TMSG_SDA_LOGIN *ackBody=(TMSG_SDA_LOGIN *)msg->body;
  ackBody->session=(error_code==0)?anchor->terminal.sessionid:0;
  //ackBody->error=error_code;
  msg_send(msg,packet,NULL);

  if(error_code==0){
    DBLog_AppendData("\xFF\xFF\xFF\xFF\x00",5,anchor); //登录日志
  }
  session_lock(FALSE);
  printf("anchor#%d login \n",anchorID);
}



BOOL load_lab_configs(int labID,TUWBLocalAreaBlock *configs,U32 *anchorIDs,int *anchorModes,int maxAnchorCount){
  configs->anchorCount=0;
  MYSQL_RES *res=db_queryf("select algorithm,abline,acline,bcline,daline,dbline,dcline from `uwb_lab` where id=%u",labID); 
  if(res){
    MYSQL_ROW row=mysql_fetch_row(res);
    if(row){
      configs->algorithm=atoi(row[0]);
      configs->ab_mm=round(atof(row[1])*1000);
      configs->ac_mm=round(atof(row[2])*1000);
      configs->bc_mm=round(atof(row[3])*1000);
      configs->da_mm=round(atof(row[4])*1000);
      configs->db_mm=round(atof(row[5])*1000);
      configs->dc_mm=round(atof(row[6])*1000);
      mysql_free_result(res);   
      res=db_queryf("select id,mode from uwb_anchor where labid=%u and number>0 order by number asc",labID); 
      if(res){
        int anchorCount=0;
        while((row=mysql_fetch_row(res)) && anchorCount<maxAnchorCount){
          anchorIDs[anchorCount]=atoi(row[0]);
          anchorModes[anchorCount]=atoi(row[1]);
          anchorCount++;
        } 
        configs->anchorCount=anchorCount;
      }
    }
  } 
  if(res)mysql_free_result(res);   
  //printf("anchor login: anchor count=%d\r\n",configs->anchorCount);
  return configs->anchorCount;
}


