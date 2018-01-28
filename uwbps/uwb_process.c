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
  cacheData->timeStamp=(U32)frame->timeStamp[0]+((U32)frame->timeStamp[1]<<8)+((U32)frame->timeStamp[2]<<16)+((U32)frame->timeStamp[3]<<24) + ((U64)frame->timeStamp[4]<<32);
  cacheData->recvTime=updateTime;
}




//�����Ѵ��ڵĻ�վ�ڵ�,�������󷵻ء�
//�����վ�ڵ㲻�������Զ�����
//ע��ÿ��lab�µĻ�վ������ʱ���ǳ�ʱֹͣ�����ǳ�ʱ�ͷš�
//��һ��lab�µĻ�վȫ����ʱ�󣬿��Զ����������Ŀǰû��ʵ�֡�
void *UWBLab_load(U32 anchorID,U32 labID){
  //�ӵ�ǰ�Ѿ����ڵ����л�վ�����в��һ�վ�ڵ�
   if(!labID)return NULL;//����anchorIDΪ0,����labID����Ϊ0;
   TUWBAnchor *desAnchor=NULL;
   TUWBLocalAreaBlock *uwbLab=dtmr_findById(dtmr_labLinks,labID,TRUE);
   if(uwbLab){
     LABEL_EXIST_LAB:
     if(anchorID){
       int i;
       TUWBAnchor **pAnchors=uwbLab->anchors;
       for(i=0;i<uwbLab->anchorCount;i++){
         if(pAnchors[i]->terminal.id==anchorID){
           U32 dtmrOptions=DTMR_LOCK|DTMR_TIMEOUT_STOP|DTMR_ENABLE;
           desAnchor=pAnchors[i];
           if(dtmr_update(desAnchor,HEARTBEAT_OVERTIME_MS,dtmrOptions)){//ʧ��
              dtmr_unlock(uwbLab,UWB_LAB_TIMEOUT_MS);  
              return desAnchor;
           }
           else{
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
     else{//��ָ��anchorIDʱ������Lab��ַ(����)��
       //desAnchor=uwbLab->anchors[0];
       return uwbLab;
     }
   }
   else{//��վ�ڵ㼰����鶼�����ڣ����½���վ���鼰�����л�վ�ڵ�
     const int  maxAnchorCount=6;
     U32 dtmrOptions=DTMR_LOCK|DTMR_ENABLE|DTMR_CYCLE|DTMR_TIMEOUT_STOP|DTMR_NOVERRIDE;
     int mem_size=sizeof(TUWBLocalAreaBlock)+maxAnchorCount*sizeof(TUWBAnchor *);
     uwbLab=dtmr_add(dtmr_labLinks,labID,0,NULL,NULL,mem_size,UWB_LAB_TIMEOUT_MS,&dtmrOptions);   
     if(uwbLab){
       if(dtmrOptions&DTMR_EXIST)goto LABEL_EXIST_LAB;
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
             if(anchorNode){
               anchorNode->index=index;
               anchorNode->lab=uwbLab;
               anchorNode->mode=anchorModes[index];//��ͨ��λ��վ����ͬ����վ(ͬ����վ���������һ��)
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
           if(!anchorID)return uwbLab;// ��ָ��anchorIDʱ�����صڼ�����UwbLab
         }
       }
       else{//load config fail!
         dtmr_unlock(uwbLab,DTMR_UNLOCK_DELETE);
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

static void _UWBLab_checkDelete(TUWBLocalAreaBlock *lab){
  TBinodeLink *listeningUsers=&lab->listeningUsers;
  if(listeningUsers->next==listeningUsers){//no any user is listening this lab
    int i,anchorCount=lab->anchorCount;
    for(i=0;i<anchorCount;i++){
      TUWBAnchor *anchor=lab->anchors[i];
      if(anchor->terminal.term_state>DEV_STATE_OFFLINE)break; 
    } 
    if(i==anchorCount){//all lab anchors are offline
      for(i=0;i<anchorCount;i++){
        TUWBAnchor *anchor=lab->anchors[i];
        dtmr_delete(anchor);
      }
      dtmr_unlock(lab,DTMR_UNLOCK_DELETE);
      printf("##########Lab#%d and anchors deleted\n",lab->id);
      return;
    }
  }
}
//lab cannot be deleted until all users and anchors are offline
void UWBLab_checkDelete(TUWBLocalAreaBlock *lab){
  if(lab){
    if(dtmr_lock(lab)){
      _UWBLab_checkDelete(lab);
      dtmr_unlock(lab,0);
    }
    else exit_with_exception("uwb lab lock fail!");
  }
}

BOOL UWBLab_switchUser(TTerminal *user,BOOL ownedUserLock,U32 newLabID){
  U32 currentLabID=((TTermUser *)user)->currentLabID;
  if(currentLabID==newLabID)return TRUE;
  else{
    BOOL ret=FALSE;
    if(!ownedUserLock)dtmr_lock(user);
    TBinodeLink *listenNode=&((TTermUser *)user)->listenLinker;
#if 1 //errorCheck
    if(currentLabID){
      if(!listenNode->next || listenNode->next==listenNode){
        exit_with_exception("currentLabID<>0 but listenNode is empty!");
      }
    }
    else if(listenNode->next && listenNode->next!=listenNode){
       exit_with_exception("currentLabID==0 but listenNode is not empty!");
    } 
#endif
    if(currentLabID){
      TUWBLocalAreaBlock *uwbLab=(TUWBLocalAreaBlock *)dtmr_findById(dtmr_labLinks,currentLabID,TRUE);
      if(uwbLab){ 
        //����listenNode������lab��listeningUsers�����У�����Ҫ����Ҫ��lab����
        BINODE_REMOVE(listenNode,prev,next);
        BINODE_ISOLATE(listenNode,prev,next);
        _UWBLab_checkDelete(uwbLab);
        ((TTermUser *)user)->currentLabID=0;
        dtmr_unlock(uwbLab,0);  
        if(newLabID==0)ret=TRUE;
      }
    }
    if(newLabID){
      TUWBLocalAreaBlock *uwbLab=(TUWBLocalAreaBlock *)dtmr_findById(dtmr_labLinks,newLabID,TRUE);
      if(!uwbLab){
        puts("###load lab 1");
        uwbLab=(TUWBLocalAreaBlock *)UWBLab_load(0,newLabID);
        puts("###load lab 2");
      }
      if(uwbLab){
        BINODE_INSERT(listenNode,&uwbLab->listeningUsers,prev,next);
        ((TTermUser *)user)->currentLabID=newLabID;
        dtmr_unlock(uwbLab,UWB_LAB_TIMEOUT_MS);  
        ret=TRUE;
      }
    }
    if(!ownedUserLock)dtmr_unlock(user,0);
    return ret;
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

void UWB_system_init(void){
  os_createSemphore(&uwb_global_locker, 1);/*�ź�����ֵΪ1����������*/
  dtmr_labLinks=dtmr_create(32,UWB_LAB_TIMEOUT_MS,NULL,"dtmr_labLinks");
  init_tag_list();
  //��ʼ�����б��dummy header
}
//---------------------------------------------------------------------------
void udp_process_packet(void *packetData,int packetLen,TNetAddr *peerAddr){
  #define FRAMES(name)  ((UWBRawFrame *)packetData)->name
  if(packetLen!=UWB_FRAME_SIZE){
    printf("packet len=%d\n",packetLen);
    return;
  }
  int ret_error;
  U32 curTagID=FRAMES(tagID);
  U32 curAnchorID=FRAMES(anchorID);
  U32 frame_session=FRAMES(sessionID);
  if(frame_session==0 || curTagID==0 || curAnchorID==0)ret_error=UWBERR_INVALID_FRAME_FORMAT;
  else if(!msg_ValidChecksum(packetData,sizeof(UWBRawFrame)))ret_error=UWBERR_CHECKNUM;
  else {
    //�ӱ�ǩ�б��л�ȡ��ǩ�ڵ㲢����(�����������̲߳����޸Ĵ˱�ǩ)��
    //�����ǩ���������Զ���������ʼ��
    TUWBTag *curTag=UWB_obtain_tag(curTagID);
    if(!curTag) ret_error=UWBERR_INVALID_TAG;
    else {
      //����anchorID�Լ���ǰ��ص�tagID���ҵ�anchorNode�������¶�Ӧ��ϵ���������򴴽���
      int curTime=os_msRunTime(); //ϵͳ��ǰʱ��
      TUWBAnchor *curAnchor=(TUWBAnchor *)dtmr_findById(dtmr_termLinks,frame_session,FALSE);//����Ҫ����(��վ�ڵ㶼��ֻ����Ϣ))
      if(!curAnchor || curAnchor->terminal.id!=curAnchorID)ret_error=UWBERR_INVALID_ANCHOR;
      else {
        int curSyncID=FRAMES(syncID);
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
            UWB_updateCache(cacheData,(UWBRawFrame *)packetData,curTime);
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
            point->accData=FRAMES(accData);
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
  if(ret_error)printf("###uwb_data: error=%d\n",ret_error);
  //return ret_error;
}


