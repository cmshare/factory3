#include "mc_routine.h"
#include "uwb_model.h"

#define CALIBRATION_TIMEOUT_MS        8000
//---------------------------------------------------------------------------
#pragma pack (push,1)
//---------------------------------------------------------------------------
typedef struct{
  U32  labID;
}TMSG_USR_CALIBRATION_START;

typedef struct{
  U8 rangeMode;//1:主测距模式;2:从测距模式
  U8 anchorCount;
}TMSG_SDR_CALIBRATION_RANGING;

typedef struct{
 U32 desAnchorID;
 U32 distance;//unit mm
}TMSG_DSR_CALIBRATION_RESULT;   


typedef struct{
  U32 anchorID;
  U32 distance;
}TAnchorDistance;

typedef struct{
 U32 count;
 TAnchorDistance dist[0];//unit mm
}TMSG_DSA_CALIBRATE;   

//---------------------------------------------------------------------------
#pragma pack (pop) 
//---------------------------------------------------------------------------
typedef struct{
  int stage;
  TUWBLocalAreaBlock *lab;
  TTerminal *user;
  U32 rangingAckMask;
  U32 distances[0];//unit:mm
}TCalibrationControl;

//---------------------------------------------------------------------------
void GeneralResponse_MSG_SUR_CALIBRATION_PROGRESS(TMcPacket *response,void *extraData){
//  puts("GeneralResponse_MSG_SUR_CALIBRATION_PROGRESS");
}
//---------------------------------------------------------------------------
#if 1 //version 2018/2/2

static void sdr_ranging_request(TUWBAnchor *anchor){
  TMcMsg *msg=msg_alloc(MSG_SDR_CALIBRATE,0);
  msg_request(msg,(TTerminal *)anchor,NULL,0);
}

void Response_MSG_DSA_CALIBRATE(TMcPacket *response,void *extraData){
  U8 errnum=-1;
  TMSG_DSA_CALIBRATE *ackbody=(TMSG_DSA_CALIBRATE *)response->msg.body;
  int distCount=ackbody->count;
  if(distCount>=3){
    TUWBAnchor *anchor=(TUWBAnchor *)response->terminal;
    TUWBLocalAreaBlock *lab=anchor->lab;
    TCalibrationControl *ctrl=(TCalibrationControl *)dtmr_findByID(dtmr_commLinks,MSG_USR_CALIBRATION_START,lab->id,TRUE);             
    if(ctrl){
      int stage=ctrl->stage;
      int anchorCount=lab->anchorCount;
      TUWBAnchor **anchors=lab->anchors;
      if(anchor==anchors[stage] && anchorCount==distCount+1){
        int i,j,rangeCount=0;
        for(j=0;j<anchorCount;j++){
          U32 desAnchorID=anchors[j]->terminal.id;
          for(i=0;i<distCount;i++){
            if(ackbody->dist[i].anchorID==desAnchorID){
              U32 *pDistance=ctrl->distances+stage*distCount+rangeCount++;
              *pDistance=ackbody->dist[i].distance;
              break;
            }
          }
        }
        if(rangeCount==distCount){//收集齐一轮数据
          if(stage<distCount){//还有下一轮要继续
            ctrl->stage=++stage;
            calibrate_progress_notify(ctrl->user,0x80000000|ctrl->stage); 

            if(stage<distCount) sdr_ranging_request(ctrl->lab->anchors[stage]);
            else{//已经完成所有轮测距
              //进行均值计算，并向客户端返回计算结果
              #define get_distance(Stage,Index) ctrl->distances[Stage*distCount+Index]
              TMcMsg *msg=msg_alloc(MSG_SUR_CALIBRATION_RESULT,sizeof(TUWBLabConfig));
              TUWBLabConfig *retBody=(TUWBLabConfig *)msg->body;
              retBody->labID=lab->id;
              retBody->algorithm=0;;
              retBody->ab_mm=get_distance(0,0);
              retBody->ac_mm=get_distance(0,1);
              retBody->da_mm=get_distance(0,2);
              retBody->bc_mm=get_distance(1,1);
              retBody->db_mm=get_distance(1,2);
              retBody->dc_mm=get_distance(2,2);
              retBody->anchorCount=anchorCount;
              msg_request(msg,ctrl->user,NULL,0);
            }
          }  
        }
        errnum=0;
      }else errnum=3;
      dtmr_unlock(ctrl,CALIBRATION_TIMEOUT_MS);        
    }else errnum=2;
  }
  else{
    errnum=1;
    puts("##########1");
  }

}

void Handle_MSG_USR_CALIBRATION_START(TMcPacket *packet){
  TMSG_USR_CALIBRATION_START *request=(TMSG_USR_CALIBRATION_START *)packet->msg.body;
  U8 errnum=-1;
  U32 labID=request->labID;
  if(labID){
    TUWBLocalAreaBlock *lab=dtmr_findById(dtmr_labLinks,labID,FALSE);
    if(lab){
      int i,anchorCount=lab->anchorCount;
      int dataAnchorCount=anchorCount-1;//去掉校准基站
      TUWBAnchor **anchors=lab->anchors;
      for(i=0;i<dataAnchorCount;i++){
        if(anchors[i]->terminal.term_state==DEV_STATE_OFFLINE){
          printf("anchor#%d is offline\n",anchors[i]->terminal.id);
          errnum=1;
          break;
        }
      }
      if(i==dataAnchorCount){
        U32 dtmrOptions=DTMR_LOCK|DTMR_ENABLE|DTMR_TIMEOUT_DELETE|DTMR_NOVERRIDE;
        int memsize=sizeof(TCalibrationControl)+dataAnchorCount*dataAnchorCount*sizeof(U32);
        TCalibrationControl *ctrl=(TCalibrationControl *)dtmr_add(dtmr_commLinks,MSG_USR_CALIBRATION_START,labID,NULL,NULL,memsize,CALIBRATION_TIMEOUT_MS,&dtmrOptions);             
        memset(ctrl,0,memsize);
        ctrl->lab=lab;
        ctrl->user=packet->terminal;
        sdr_ranging_request(ctrl->lab->anchors[0]);
        dtmr_unlock(ctrl,0);
        errnum=0;
      }
    }
    else{
       errnum=2;
      // printf("failt to find labid:%d\n",labID);
    }
  }
  else errnum=3;
  msg_ack_general(packet,errnum);
}
#else //version 2018/1/20

static void sdr_ranging_request(TCalibrationControl *ctrl){
  TUWBLocalAreaBlock *lab=ctrl->lab;
  int anchorCount=lab->anchorCount;
  int i,stage=ctrl->stage;
  TMcMsg *msg=msg_alloc(MSG_SDR_CALIBRATION_RANGING,sizeof(MSG_SDR_CALIBRATION_RANGING));
  TMSG_SDR_CALIBRATION_RANGING *msgBody=(TMSG_SDR_CALIBRATION_RANGING *)msg->body;
  msgBody->anchorCount=anchorCount;
  for(i=0;i<anchorCount;i++){
    TUWBAnchor *anchor=lab->anchors[i];
    if(i==stage)msgBody->rangeMode=1;//主测距模式
    else msgBody->rangeMode=2;//从测距模式
    if(i>0)msg_updateSyncID(msg);
    msg_request(msg,&anchor->terminal,NULL,0);
  }
}

void Handle_MSG_USR_CALIBRATION_START(TMcPacket *packet){
  TMSG_USR_CALIBRATION_START *request=(TMSG_USR_CALIBRATION_START *)packet->msg.body;
  U8 errnum=-1;
  U32 labID=request->labID;
  if(labID){
    TUWBLocalAreaBlock *lab=dtmr_findById(dtmr_labLinks,labID,FALSE);
    if(lab){
      int i,anchorCount=lab->anchorCount;
      TUWBAnchor **anchors=lab->anchors;
      for(i=0;i<anchorCount;i++){
        if(anchors[i]->terminal.term_state==DEV_STATE_OFFLINE){
          errnum=1;
          break;
        }
      }
      if(i==anchorCount){
        U32 dtmrOptions=DTMR_LOCK|DTMR_ENABLE|DTMR_TIMEOUT_DELETE|DTMR_NOVERRIDE;
        int memsize=sizeof(TCalibrationControl)+anchorCount*anchorCount*sizeof(U32);
        TCalibrationControl *ctrl=(TCalibrationControl *)dtmr_add(dtmr_commLinks,MSG_USR_CALIBRATION_START,labID,NULL,NULL,memsize,CALIBRATION_TIMEOUT_MS,&dtmrOptions);             
        memset(ctrl,0,memsize);
        ctrl->lab=lab;
        ctrl->user=packet->terminal;
        sdr_ranging_request(ctrl);
        dtmr_unlock(ctrl,0);
        errnum=0;
      }
    }
    else{
       errnum=2;
      // printf("failt to find labid:%d\n",labID);
    }
  }
  else errnum=3;
  msg_ack_general(packet,errnum);
}
#endif

void calibrate_progress_notify(TTerminal *user,U32 progress){ 
  TMcMsg *msg=msg_alloc(MSG_SUR_CALIBRATION_PROGRESS,sizeof(U32));
  *((U32 *)msg->body)=progress;
  msg_request(msg,user,NULL,0);
}

void GeneralResponse_MSG_SUR_CALIBRATION_RESULT(TMcPacket *response,void *extraData){
}

void GeneralResponse_MSG_SDR_CALIBRATION_RANGING(TMcPacket *response,void *extraData){
  TUWBAnchor *anchor=(TUWBAnchor *)response->terminal;
  TUWBLocalAreaBlock *lab=anchor->lab;
  TCalibrationControl *ctrl=(TCalibrationControl *)dtmr_findByID(dtmr_commLinks,MSG_USR_CALIBRATION_START,lab->id,TRUE);             
  if(ctrl){
    int i,anchorCount=lab->anchorCount;
    TUWBAnchor **anchors=lab->anchors;
    for(i=0;i<anchorCount;i++){
      if(anchors[i]==anchor){
        ctrl->rangingAckMask|=(1<<i);
        break;
      }
    }
    dtmr_unlock(ctrl,CALIBRATION_TIMEOUT_MS);        

    calibrate_progress_notify(ctrl->user,ctrl->rangingAckMask); 
    if(ctrl->rangingAckMask==(1<<anchorCount)-1){
       //收到所有应答,等待基站返回测量的距离
    }
  }
  
}


void Handle_MSG_DSR_CALIBRATION_RESULT(TMcPacket *packet){
  U8 errnum=-1;
  TMSG_DSR_CALIBRATION_RESULT *rangebody=(TMSG_DSR_CALIBRATION_RESULT *)packet->msg.body;
  if(rangebody->distance){
    TUWBAnchor *anchor=(TUWBAnchor *)packet->terminal;
    TUWBLocalAreaBlock *lab=anchor->lab;
    TCalibrationControl *ctrl=(TCalibrationControl *)dtmr_findByID(dtmr_commLinks,MSG_USR_CALIBRATION_START,lab->id,TRUE);             
    if(ctrl){
      int i,rangeCount=0,stage=ctrl->stage;
      int anchorCount=lab->anchorCount;
      TUWBAnchor **anchors=lab->anchors;
      if(anchor==anchors[stage]){
        U32 desAnchorID=rangebody->desAnchorID;
        for(i=0;i<anchorCount;i++){
          U32 *pDistance=ctrl->distances+stage*anchorCount+i;
          if(anchors[i]->terminal.id==desAnchorID){
            *pDistance=rangebody->distance;
            rangeCount++; 
          }
          else if(*pDistance!=0){
            rangeCount++; 
          }
        }
        if(rangeCount==anchorCount-1){//收集齐一轮数据
          if(stage<anchorCount){//还有下一轮要继续
            ctrl->stage=++stage;
            sdr_ranging_request(ctrl);
            calibrate_progress_notify(ctrl->user,0x80000000|ctrl->stage); 
            if(stage==anchorCount){//已经完成所有轮测距
              //进行均值计算，并向客户端返回计算结果
              #define get_distance(Stage,Index) ctrl->distances[Stage*anchorCount+Index]
              TMcMsg *msg=msg_alloc(MSG_SUR_CALIBRATION_RESULT,sizeof(TUWBLabConfig));
              TUWBLabConfig *retBody=(TUWBLabConfig *)msg->body;
              retBody->labID=lab->id;
              retBody->algorithm=0;;
              retBody->ab_mm=(get_distance(0,1)+get_distance(1,0))>>1;
              retBody->ac_mm=(get_distance(0,2)+get_distance(2,0))>>1;
              retBody->bc_mm=(get_distance(1,2)+get_distance(2,1))>>1;
              retBody->da_mm=(get_distance(3,0)+get_distance(0,3))>>1;
              retBody->db_mm=(get_distance(3,1)+get_distance(1,3))>>1;
              retBody->dc_mm=(get_distance(3,2)+get_distance(2,3))>>1;
              retBody->anchorCount=anchorCount;
              msg_request(msg,ctrl->user,NULL,0);
           }
          }  
        }
        errnum=0;
      }else errnum=3;
      dtmr_unlock(ctrl,CALIBRATION_TIMEOUT_MS);        
    }else errnum=2;
  }
  else{
    errnum=1;
    puts("##########1");
  }
  msg_ack_general(packet,errnum);
}
