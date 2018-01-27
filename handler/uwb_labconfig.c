#include "mc_routine.h"
#include "uwb_model.h"

//---------------------------------------------------------------------------
#pragma pack (push,1)
//---------------------------------------------------------------------------
typedef struct{
  U32  labID;
}TMSG_USR_GETCONFIG;


typedef struct{
  char username[MAXLEN_USERNAME+1];
}TMSG_USR_GETBINDLIST;

typedef struct{
  U32 labCount;
  TUWBLabConfig labConfigs[0];
}TMSG_SUA_GETBINDLIST;

typedef struct{
  U32  labID;//为０表示退出监听（一个用户只能同时监听一个Lab，如果要同时监听多个就要建立虚拟的shadow子账户）
}TMSG_USR_SWITCHLAB;//返回结果表示当前JoinedLab集合N

//---------------------------------------------------------------------------
#pragma pack (pop) 
//---------------------------------------------------------------------------

void Handle_MSG_USR_GETCONFIG(TMcPacket *packet){
  TMSG_USR_GETCONFIG *reqBody=(TMSG_USR_GETCONFIG *)packet->msg.body;
  TMcMsg *msg=msg_alloc(MSG_SUA_GETCONFIG,sizeof(TUWBLabConfig));
  TUWBLabConfig *ackBody=(TUWBLabConfig *)msg->body;
  U8 error_code=-1;
  U32 labID=reqBody->labID;
  if(labID){
    MYSQL_RES *res=db_queryf("select algorithm,abline,acline,bcline,daline,dbline,dcline,count(uwb_anchor.id) from uwb_lab,uwb_anchor where uwb_anchor.labid=uwb_lab.id and uwb_lab.id=%u",labID); 
    if(res){
      MYSQL_ROW row=mysql_fetch_row(res);
      if(row){
        ackBody->labID=labID;
        ackBody->algorithm=atoi(row[0]);
        ackBody->ab_mm=round(atof(row[1])*1000);
        ackBody->ac_mm=round(atof(row[2])*1000);
        ackBody->bc_mm=round(atof(row[3])*1000);
        ackBody->da_mm=round(atof(row[4])*1000);
        ackBody->db_mm=round(atof(row[5])*1000);
        ackBody->dc_mm=round(atof(row[6])*1000);
        ackBody->anchorCount=atoi(row[7]);
        mysql_free_result(res);   
        error_code=0;
      }
      mysql_free_result(res);   
    } 
  }
  if(error_code==0) msg_send(msg,packet,NULL);
  else msg_ack_general(packet,error_code);
}

void Handle_MSG_USR_SETCONFIG(TMcPacket *packet){
  TUWBLabConfig *reqBody=(TUWBLabConfig *)packet->msg.body;
  U8 errCode=-1;
  if(reqBody->labID>0){
    if(db_queryf("update uwb_lab set abline=%f,bcline=%f,acline=%f,daline=%f,dbline=%f,dcline=%f where id=%u",reqBody->ab_mm/1000.0,reqBody->bc_mm/1000.0,reqBody->ac_mm/1000.0,reqBody->da_mm/1000.0,reqBody->db_mm/1000.0,reqBody->dc_mm/1000.0,reqBody->labID)){
      UWB_update_labconfig(reqBody);//更新在线设备正在使用的配置
      errCode=0;
    } 
  }
  msg_ack_general(packet,errCode);
}


void Handle_MSG_USR_GETBINDLIST(TMcPacket *packet){//有于现在尚未建立绑定逻辑（这里返回所有可用热的lab列表）
  TMSG_USR_GETBINDLIST *reqBody=(TMSG_USR_GETBINDLIST*)packet->msg.body;
  TMcMsg *msg=msg_alloc(MSG_SUA_GETBINDLIST,sizeof(TMSG_SUA_GETBINDLIST)+MAX_BINDED_NUM*sizeof(TUWBLabConfig));
  TMSG_SUA_GETBINDLIST *ackBody=(TMSG_SUA_GETBINDLIST *)msg->body;
  U8 error_code=-1;
  int labCount=0;
  if(reqBody->username[0]){
    char *bindLabList=((TTermUser *)packet->terminal)->bindedLabIDs;
    if(bindLabList[0]){
      MYSQL_RES *res=db_queryf("select uwb_lab.id,algorithm,abline,acline,bcline,daline,dbline,dcline,count(uwb_anchor.id) from uwb_lab,uwb_anchor where uwb_anchor.labid=uwb_lab.id and uwb_lab.id in (%s) order by uwb_lab.id asc",bindLabList); 
      if(res){
        MYSQL_ROW row;
        TUWBLabConfig *lab=ackBody->labConfigs;
        while((row=mysql_fetch_row(res)) && labCount<MAX_BINDED_NUM){
          lab->labID=atoi(row[0]);
          lab->algorithm=atoi(row[1]);
          lab->ab_mm=round(atof(row[2])*1000);
          lab->ac_mm=round(atof(row[3])*1000);
          lab->bc_mm=round(atof(row[4])*1000);
          lab->da_mm=round(atof(row[5])*1000);
          lab->db_mm=round(atof(row[6])*1000);
          lab->dc_mm=round(atof(row[7])*1000);
          lab->anchorCount=atoi(row[8]);
          lab++;
          labCount++;
        }
        mysql_free_result(res);   
        error_code=0;
      }
    }
  }
  if(error_code==0){
    ackBody->labCount=labCount;
    msg->bodylen=sizeof(TMSG_SUA_GETBINDLIST)+labCount*sizeof(TUWBLabConfig);//重新计算实际消息体长度。
    msg_send(msg,packet,NULL);
  }
  else msg_ack_general(packet,error_code);
}

void Handle_MSG_USR_SWITCHLAB(TMcPacket *packet){
  TMSG_USR_SWITCHLAB *request=(TMSG_USR_SWITCHLAB *)packet->msg.body;
  char *bindLabList=((TTermUser *)packet->terminal)->bindedLabIDs;
  U8 errCode=-1;
  if(request->labID){
    char strLabID[8];
    str_itoa(request->labID,strLabID);
    if(str_itemSeek(bindLabList,strLabID,',') && UWBLab_switchUser(packet->terminal,FALSE,request->labID)) errCode=0;
  }
  else{
    if(UWBLab_switchUser(packet->terminal,FALSE,0))errCode=0;
  }
  msg_ack_general(packet,errCode);
}

