#include "mc_routine.h"
#include "uwb_model.h"

#define UWB_LAB_TIMEOUT_MS   120000
static TUWBTag *hashTagList=NULL;
static HAND uwb_global_locker=NULL;
HAND   dtmr_labLinks=NULL;
/*
˼·��
�յ�1֡���ݺ�
(1) ͨ�� tagID����tagNodeList�в���tagNode�ڵ�(HASH�б����)�������������½�tagNode��
(2) �����ݸ��µ�tagNode��frameQueue�У�Ҫ��������ʱ�䣩�����queue�����ݽṹҪ�������
(3) ��鵱ǰtagNode��Ӧ��anchor�Ƿ���ڣ�������Ҫ��������ʼ����ϢҪ�����ݿ�������ļ��ж�ȡ��
(4) ���uwb lab�Ƿ���ڣ�������򴴽�����ʼ����ϢҪ�����ݿ�������ļ��ж�ȡ��
(5) �ڵ�ǰtagNode�����ڵ�anchorGoup�ڣ��ж�������վ��Ա�Ƿ������ˣ���ǰtagNode������ͬ��֡��ע:2s��ʱ����
(6) �����2s��������ͬһtagNode��ͬһuwb lab�����л�վ��ͬ��֡����ʼ��λ���㣻
(7) kalman
(8) �ύ������

todo: ��ʱ���ƣ�
����һ����ʱ����̣߳�ÿ�������ӱ���һ�����еı�ǩ�Լ���վ�ڵ㣺
(1) ���ڳ�ʱ��ǩ�������ƶ���ɾ������������ֱ���ͷţ����ٴ����ã����������ݿ⣮
(2) ���������վ�����ɾ���ˣ���ɾ������ڵ㣮
*/

#define  UWB_GLOBAL_LOCK() os_obtainSemphore(uwb_global_locker)
#define  UWB_GLOBAL_UNLOCK() os_releaseSemphore(uwb_global_locker)

void UWB_global_lock(BOOL lock){
  if(lock)UWB_GLOBAL_LOCK();
  else UWB_GLOBAL_UNLOCK();
}

static void init_tag_list(){
  int i,tagListSize=TAGLIST_HASH_LEN*sizeof(TUWBTag);
  hashTagList=(TUWBTag *)malloc(tagListSize);
  memset(hashTagList,0,tagListSize);
  for(i=0;i<TAGLIST_HASH_LEN;i++)
  { TUWBTag *node=hashTagList+i;
    BINODE_ISOLATE(node,up,down);
  }
}

static TUWBTag *UWB_obtain_tag(int tagID){
  //�ӱ�ǩ�б���������ǩ�ڵ�,�������󷵻ء�
  //��ǩ�ڵ㲻�������Զ�����
  //��hash���в���
  label_start:
  UWB_GLOBAL_LOCK(); //����ȫ����Դǰ�ȼ���
  U32 curTime=os_msRunTime();
  TUWBTag *node,*dummy=hashTagList+(tagID % TAGLIST_HASH_LEN);
  for(node=dummy->down;node!=dummy;node=node->down){
    if(node->id==tagID)break;
    else if(node->deadline>curTime){//��ʱ�ͷ�
      HAND semLock=node->semLock;
      if(os_tryObtainSemphore(semLock)){
        TUWBTag *delNode=node;     
        node=node->up;
        BINODE_REMOVE(delNode,down,up);
        free(delNode);
        os_destroySemphore(semLock);
      }
    }
  }
  if(node==dummy){//û���ҵ���ǩ�򴴽�
    node=(TUWBTag *)malloc(sizeof(TUWBTag));
    memset(node,0,sizeof(TUWBTag));
    UWB_tag_init(node);
    node->id=tagID;
    node->deadline=curTime+HEARTBEAT_OVERTIME_MS; 
    node->bufPointStartTime=curTime;
    os_createSemphore(&node->semLock, 0);/*�ź�����ֵΪ0����ʾ����*/
    BINODE_INSERT(node,dummy,down,up);//����hash����ͷ��
    UWB_GLOBAL_UNLOCK(); //�ͷ�ȫ����
    return node;
  }
  else if(os_tryObtainSemphore(node->semLock)){
    UWB_GLOBAL_UNLOCK(); //�ͷ�ȫ����
    return node;
  }
  else{
    UWB_GLOBAL_UNLOCK(); //�ͷ�ȫ����
    if(os_obtainSemphore(node->semLock)){  //�Ա�ǩ����
      if(node->id==tagID){//ȷ��һ�����ж��ڼ��Ƿ��޸�
        return node;
      }
      else{
        os_releaseSemphore(node->semLock);
        goto label_start;
      }
    }
  }
  return NULL;
}

//��ǩ�ڵ����
static void UWB_release_tag(TUWBTag *tag){
  os_releaseSemphore(tag->semLock);  //�Ա�ǩ����
}

//������֡���µ���Ӧ�����frameCache�У������ظ�������frameCahce�е�ָ��
static void UWB_updateCache(TDataProcessed *cacheData,UWBRawFrame *frame,U32 updateTime){
  cacheData->syncID=frame->syncID;
  cacheData->tagID=frame->tagID;
  cacheData->anchorID=frame->anchorID;
 // cacheData->accX = frame->accX;
//  cacheData->accY = frame->accY;
 // cacheData->accZ = frame->accZ;
  cacheData->timeStamp=(U32)frame->timeStamp[0]+((U32)frame->timeStamp[1]<<8)+((U32)frame->timeStamp[2]<<16)+((U32)frame->timeStamp[3]<<24) |((U64)frame->timeStamp[4]<<32);
  cacheData->recvTime=updateTime;
}




//�����Ѵ��ڵĻ�վ�ڵ�,�������󷵻ء�
//�����վ�ڵ㲻�������Զ�����
//ע��ÿ��lab�µĻ�վ������ʱ���ǳ�ʱֹͣ�����ǳ�ʱ�ͷš�
//��һ��lab�µĻ�վȫ����ʱ�󣬿��Զ����������Ŀǰû��ʵ�֡�
TUWBAnchor *_UWB_anchor_load(U32 anchorID,U32 labID){
  //�ӵ�ǰ�Ѿ����ڵ����л�վ�����в��һ�վ�ڵ�
   TUWBLocalAreaBlock *node,*uwbLab;
   TUWBAnchor *desAnchor=NULL;
   if(!labID)return NULL;//����anchorIDΪ0,����labID����Ϊ0;
   uwbLab=dtmr_findById(dtmr_labLinks,labID,TRUE);
   if(uwbLab){
     if(anchorID){
       int i;
       TUWBAnchor **pAnchor=uwbLab->anchors;
       for(i=0;i<uwbLab->anchorCount;i++,pAnchor++){
         if((*pAnchor)->terminal.id==anchorID){
           U32 dtmrOptions=DTMR_LOCK|DTMR_TIMEOUT_STOP|DTMR_ENABLE;
           desAnchor=*pAnchor;
           if(!dtmr_update(desAnchor,HEARTBEAT_OVERTIME_MS,dtmrOptions)){
             //Should never reach here
             exit_with_exception("uwb lab system error!");
           }
           break;
         }
       }
       if(!desAnchor){
         //Should never reach here
         exit_with_exception("uwb lab system error!");
       }
     }
     else{//��ָ��anchorIDʱ�����صڣ���anchor,�Ҳ�������
       desAnchor=uwbLab->anchors[0];
     }
     dtmr_unlock(uwbLab,UWB_LAB_TIMEOUT_MS);  
   }
   else{//��վ�ڵ㼰����鶼�����ڣ����½���վ���鼰�����л�վ�ڵ�
     const int  maxAnchorCount=6;
     U32 dtmrOptions=DTMR_LOCK|DTMR_ENABLE|DTMR_CYCLE|DTMR_TIMEOUT_STOP|DTMR_NOVERRIDE;
     int mem_size=sizeof(TUWBLocalAreaBlock)+maxAnchorCount*sizeof(TUWBAnchor *);
     uwbLab=dtmr_add(dtmr_labLinks,labID,0,NULL,NULL,mem_size,UWB_LAB_TIMEOUT_MS,&dtmrOptions);   
     if(uwbLab){
       U32 anchorIDs[maxAnchorCount];
       int anchorModes[maxAnchorCount];
       memset(uwbLab,0,mem_size);
       BINODE_ISOLATE(&uwbLab->listeningUsers,prev,next);
       if(load_lab_configs(labID,uwbLab,anchorIDs,anchorModes,maxAnchorCount) && uwbLab->anchorCount>=3){
         int anchorCount=uwbLab->anchorCount;
         int i,anchorIndex=-1;
         if(anchorID){
           for(i=0;i<anchorCount;i++){
             if(anchorIDs[i]==anchorID){
               anchorIndex=i;
               break;
             }
           }
           if(i==anchorCount)anchorCount=0;//û���ҵ�lab��ƥ���labID
         }
         if(anchorCount>0){
           int index;
           uwbLab->id=labID;
           uwbLab->anchorCount=anchorCount;
           for(index=0;index<anchorCount;index++){
             U32 options=DTMR_LOCK|DTMR_NOVERRIDE|DTMR_TIMEOUT_STOP|((anchorIndex==index)?DTMR_ENABLE:DTMR_DISABLE);
             U32 new_sessionid=session_new();
             TUWBAnchor *anchorNode=(TUWBAnchor *)dtmr_add(dtmr_termLinks,new_sessionid,0,NULL,NULL,sizeof(TUWBAnchor),HEARTBEAT_OVERTIME_MS,&options);
             if(anchorNode && !(options&DTMR_EXIST)){
               anchorNode->index=index;
               anchorNode->lab=uwbLab;
               anchorNode->mode=anchorModes[index];
               anchorNode->terminal.term_state=DEV_STATE_OFFLINE;
               anchorNode->terminal.id=anchorIDs[index];
               anchorNode->terminal.sessionid=new_sessionid;
               uwbLab->anchors[index]=anchorNode;
               if(index==anchorIndex) desAnchor=anchorNode;
               else dtmr_unlock(anchorNode,0);
             }
             else{
              //Should never reach here
               exit_with_exception("fail to create anchor node!");
             }
           }
           UWB_lab_init(uwbLab);
           if(!anchorID) desAnchor=uwbLab->anchors[0];//��ָ��anchorIDʱ�����صڣ���anchor�����Ҳ�����
         }
       }
       else{//load config fail!
         dtmr_unlock(uwbLab,0);
         dtmr_delete(uwbLab);
         return NULL;
       }
       dtmr_unlock(uwbLab,0);
     }
     else{
       //Should never reach here
       exit_with_exception("uwb lab memory malloc fail!");
     }
   }
   return desAnchor;
}

TUWBAnchor *UWB_anchor_load(U32 anchorID,U32 labID){
   TUWBAnchor *ret;
   UWB_GLOBAL_LOCK();
   ret=_UWB_anchor_load(anchorID,labID);
   UWB_GLOBAL_UNLOCK();
   return ret;
}


int UWB_process_frame(UWBRawFrame *uwbFrame){
  int ret_error;
  U32 curTagID=uwbFrame->tagID;
  U32 curAnchorID=uwbFrame->anchorID;
  U32 frame_session=uwbFrame->sessionID;
  if(frame_session==0 || curTagID==0 || curAnchorID==0)ret_error=UWBERR_INVALID_FRAME_FORMAT;
  else if(!UWB_frame_checknum(uwbFrame))ret_error=UWBERR_CHECKNUM;
  else {
    //�ӱ�ǩ�б��л�ȡ��ǩ�ڵ㲢����(�����������̲߳����޸Ĵ˱�ǩ)��
    //�����ǩ���������Զ���������ʼ��
    TUWBTag *curTag=UWB_obtain_tag(curTagID);
    if(!curTag)return ret_error=UWBERR_INVALID_TAG;
    else {
      //����anchorID�Լ���ǰ��ص�tagID���ҵ�anchorNode�������¶�Ӧ��ϵ���������򴴽���
      int curTime=os_msRunTime(); //ϵͳ��ǰʱ��
      TUWBAnchor *curAnchor=(TUWBAnchor *)dtmr_findById(dtmr_termLinks,frame_session,FALSE);//����Ҫ����(��վ�ڵ㶼��ֻ����Ϣ))
      if(!curAnchor || curAnchor->terminal.id!=curAnchorID)ret_error=UWBERR_INVALID_ANCHOR;
      else {
        int curSyncID=uwbFrame->syncID;
        int tagID_cacheIndex=curTagID&((1<<FRAME_CACHE_BITLEN)-1);
        int syncID_cacheIndex=curSyncID&((1<<FRAME_CACHE_BITLEN)-1);
        TUWBLocalAreaBlock *curLab=curAnchor->lab;
        TUWBAnchor **anchors=curLab->anchors;
        int lab_anchorCount=curLab->anchorCount-1;//ȥ��һ��ͬ����վ
        int i,cacheHitCount=0;
        for(i=0;i<lab_anchorCount;i++){
          TDataProcessed *cacheData=&anchors[i]->frameCache[tagID_cacheIndex][syncID_cacheIndex];//tagNode data cache pool (hash map list)
          if(anchors[i]->terminal.id==curAnchorID){
            cacheHitCount++;
            //������֡���µ���Ӧ�����frameCache�С�
            UWB_updateCache(cacheData,uwbFrame,curTime);
          }
          else{
            if(cacheData->tagID==curTagID && cacheData->syncID==curSyncID && curTime-cacheData->recvTime<FRAME_ASSEMBLE_TIMEOUT_MS ){
              cacheHitCount++;
            }
          }
        }
        if(cacheHitCount==lab_anchorCount){//���뵱ǰ��ǩ��ͬһ������ê���Ͻ��յ�����
          TPOINT *coord=UWB_location_calculate(curAnchor,curTag,curSyncID);//ִ�ж�λ����
          if(coord){
            int used_time_ms=curTime-curTag->bufPointStartTime;
            int bufPointIndex=curTag->bufPointIndex++;
            TAccAndPoint *point=&curTag->bufPointArray[bufPointIndex];
            point->accData=uwbFrame->accData;
            point->x=(U32)(coord->x*1000);//unit transform from float type of m into integer type of mm
            point->y=(U32)(coord->y*1000);//unit transform from float type of m into integer type of mm
            if(used_time_ms>MAXLEN_BUFFER_LOCATE_TIMESPAN){
              if(bufPointIndex>0){
                label_data_ready:
                curTag->fps=bufPointIndex*1000/used_time_ms; //���㶨λƵ��(frame per second)
                //�û����Ͷ�λ���ݰ���ͬʱ��¼�����ݿ�
                UWB_location_post(curLab,curTag,bufPointIndex);//���û����Ͷ�λ���ݰ���ͬʱ��¼�����ݿ�
                curTag->bufPointIndex=0;
              }
              curTag->bufPointStartTime=curTime;
            }
            else if(bufPointIndex==MAXLEN_BUFFER_LOCATE_POINTS){
              if(used_time_ms>0)goto label_data_ready;
              else curTag->bufPointIndex=0;
            }
          }
        }
        ret_error=UWBERR_NONE;
      }
      curTag->deadline=curTime+HEARTBEAT_OVERTIME_MS;
      UWB_release_tag(curTag);//����
    }
  }
  return ret_error;
}

void UWB_location_post(TUWBLocalAreaBlock *lab,TUWBTag *tagNode,int pointCount){
  //����λ���ݰ��Լ�֡�ʷ��͸��û���������λ��Ϣ���浽���ݿ�(todo later)
  //�û��б���䵱ǰuwb lab�Ϲҿ��ĵ�½�û������в��ң��ж����½�û�����������͡�
  TBinodeLink *userLinks=&lab->listeningUsers;
  TBinodeLink *userNode=userLinks->next;
  if(userNode && userNode!=userLinks){ 
    int pointDataSize=pointCount*sizeof(TAccAndPoint);
    TMcMsg *msg=msg_alloc(MSG_SUR_POSTDATA,sizeof(TUWB_PackDataToUser)+pointDataSize);
    TUWB_PackDataToUser *postData=(TUWB_PackDataToUser *)msg->body;
    postData->labID=lab->id;
    postData->tagID=tagNode->id;
    postData->fps=tagNode->fps;
    postData->pointCount=pointCount;
    memcpy(postData->pointData,tagNode->bufPointArray,pointDataSize);
    while(userNode!=userLinks){
      TTermUser *termUser=T_PARENT_NODE(TTermUser,listenLinker,userNode);
      TNetAddr *anchorAddr=&termUser->terminal.loginAddr;
      if(anchorAddr->socket){
         msg_sendto(msg,anchorAddr);
      } 
      userNode=userNode->next;
    }
  }
}

//lab cannot be deleted until all users and anchors are offline
static void _UWBLab_checkDelete(TUWBLocalAreaBlock *lab){
  TBinodeLink *listeningUsers=&lab->listeningUsers;
  if(listeningUsers->next==listeningUsers){//no any user is listening this lab
    int i,anchorCount=lab->anchorCount;
    for(i=0;i<anchorCount;i++){
      TUWBAnchor *anchor=lab->anchors[i];
      if(anchor->terminal.term_state>DEV_STATE_OFFLINE)break; 
    } 
    if(i==anchorCount){//all lab anchors are offline
      dtmr_delete(lab);
      printf("##########Lab#%d deleted\n",lab->id);
    }
  }
}
void UWBLab_logoutAnchor(TTerminal *anchor){
  puts("####logoutAnchor");
  TUWBLocalAreaBlock *lab=((TUWBAnchor *)anchor)->lab;
  UWB_GLOBAL_LOCK();
  if(lab) _UWBLab_checkDelete(lab);
  UWB_GLOBAL_UNLOCK();
}

void UWBLab_logoutUser(TTerminal *user){//���û�����������lab���Ƴ�
  puts("####logoutUser");
  TBinodeLink *node=&((TTermUser *)user)->listenLinker;
  if(node->next && node->next!=node){
    UWB_GLOBAL_LOCK();
    dtmr_lock(user);
    BINODE_REMOVE(node,prev,next);
    BINODE_ISOLATE(node,prev,next);

    TUWBLocalAreaBlock *lab=((TTermUser *)user)->currentLab;
    if(lab){
      ((TTermUser *)user)->currentLab=NULL;
       _UWBLab_checkDelete(lab);
    }
    dtmr_unlock(user,0);
    UWB_GLOBAL_UNLOCK();
  }
}

BOOL UWBLab_switchUser(TTerminal *user,U32 newLabID){//��ʱ�����ǰ󶨹�ϵ
  if(newLabID){
    BOOL ret=FALSE;
    TUWBLocalAreaBlock *uwbLab=(TUWBLocalAreaBlock *)((TTermUser *)user)->currentLab;
    if(uwbLab && uwbLab->id==newLabID){
      ret=TRUE;
    }
    else{
      TBinodeLink *listenNode;
      dtmr_lock(user);
      listenNode=&((TTermUser *)user)->listenLinker;
      if(listenNode->next && listenNode->next!=listenNode)BINODE_REMOVE(listenNode,prev,next);
      if(uwbLab){
        _UWBLab_checkDelete(uwbLab);
      }

      uwbLab=(TUWBLocalAreaBlock *)dtmr_findById(dtmr_labLinks,newLabID,TRUE);
      if(uwbLab){
        BINODE_INSERT(listenNode,&uwbLab->listeningUsers,prev,next);
        ((TTermUser *)user)->currentLab=uwbLab;
        ret=TRUE;
        dtmr_unlock(uwbLab,UWB_LAB_TIMEOUT_MS);  
      }
      else{
        TUWBAnchor *anchor=_UWB_anchor_load(0,newLabID);
        if(anchor){
          uwbLab=anchor->lab;  
          BINODE_INSERT(listenNode,&uwbLab->listeningUsers,prev,next);
          ((TTermUser *)user)->currentLab=uwbLab;
          ret=TRUE;
        }
      }
      dtmr_unlock(user,0);
    }
    return ret;
  }
  else{
    UWBLab_logoutUser(user); 
    return TRUE;
  }
}

void UWB_update_labconfig(TUWBLabConfig *labcfg){
  U32 labID=labcfg->labID;
  if(labID>0){
    TUWBLocalAreaBlock *uwbLab=(TUWBLocalAreaBlock *)dtmr_findById(dtmr_labLinks,labID,TRUE);
    if(uwbLab){
      uwbLab->ab_mm=labcfg->ab_mm;
      uwbLab->bc_mm=labcfg->bc_mm;
      uwbLab->ac_mm=labcfg->ac_mm;
      uwbLab->da_mm=labcfg->da_mm;
      uwbLab->db_mm=labcfg->db_mm;
      uwbLab->dc_mm=labcfg->dc_mm;
      UWB_lab_init(uwbLab);
      dtmr_unlock(uwbLab,UWB_LAB_TIMEOUT_MS);  
    }
  }
}

BOOL UWB_frame_checknum(UWBRawFrame *frame){
 return msg_ValidChecksum(frame,sizeof(UWBRawFrame)); 
}


void UWB_system_init(void){
  os_createSemphore(&uwb_global_locker, 1);/*�ź�����ֵΪ1����������*/
  dtmr_labLinks=dtmr_create(32,UWB_LAB_TIMEOUT_MS,NULL,"dtmr_labLinks");
  init_tag_list();
  //��ʼ�����б��dummy header
}
//---------------------------------------------------------------------------
void udp_process_packet(void *packetData,int packetLen,TNetAddr *peerAddr){
  if(packetLen==UWB_FRAME_SIZE){
    UWB_process_frame((UWBRawFrame *)packetData);
  }                    
  //printf("get frame size of %d:\r\n",packetLen);
}
