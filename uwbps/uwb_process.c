#include "mc_routine.h"
#include "uwb_model.h"

#define UWB_LAB_TIMEOUT_MS   120000
static TUWBTag *hashTagList=NULL;
static HAND uwb_global_locker=NULL;
HAND   dtmr_labLinks=NULL;
/*
思路：
收到1帧数据后：
(1) 通过 tagID，在tagNodeList中查找tagNode节点(HASH列表查找)，若不存在则新建tagNode。
(2) 将数据更新到tagNode的frameQueue中（要包括接收时间），这个queue的数据结构要重新设计
(3) 检查当前tagNode对应的anchor是否存在，不存则要创建（初始化信息要从数据库或配置文件中读取）
(4) 检查uwb lab是否存在，不存的则创建（初始化信息要从数据库或配置文件中读取）
(5) 在当前tagNode的所在的anchorGoup内，判断其他基站组员是否收齐了，当前tagNode的最新同步帧（注:2s超时）；
(6) 如果在2s内收齐了同一tagNode在同一uwb lab内所有基站的同步帧，则开始定位计算；
(7) kalman
(8) 提交计算结果

todo: 超时机制：
建立一个超时检查线程，每隔２分钟遍历一次所有的标签以及基站节点：
(1) 对于超时标签，将其移动到删除链表，而不是直接释放，可再次利用，并更新数据库．
(2) 如果三个基站都标记删除了，则删除分组节点．
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
  //从标签列表中搜索标签节点,并加锁后返回。
  //标签节点不存在则自动创建
  //从hash表中查找
  label_start:
  UWB_GLOBAL_LOCK(); //操作全局资源前先加锁
  U32 curTime=os_msRunTime();
  TUWBTag *node,*dummy=hashTagList+(tagID % TAGLIST_HASH_LEN);
  for(node=dummy->down;node!=dummy;node=node->down){
    if(node->id==tagID)break;
    else if(node->deadline>curTime){//超时释放
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
  if(node==dummy){//没有找到标签则创建
    node=(TUWBTag *)malloc(sizeof(TUWBTag));
    memset(node,0,sizeof(TUWBTag));
    UWB_tag_init(node);
    node->id=tagID;
    node->deadline=curTime+HEARTBEAT_OVERTIME_MS; 
    node->bufPointStartTime=curTime;
    os_createSemphore(&node->semLock, 0);/*信号量初值为0，表示上锁*/
    BINODE_INSERT(node,dummy,down,up);//插入hash链表头部
    UWB_GLOBAL_UNLOCK(); //释放全局锁
    return node;
  }
  else if(os_tryObtainSemphore(node->semLock)){
    UWB_GLOBAL_UNLOCK(); //释放全局锁
    return node;
  }
  else{
    UWB_GLOBAL_UNLOCK(); //释放全局锁
    if(os_obtainSemphore(node->semLock)){  //对标签加锁
      if(node->id==tagID){//确认一下在中断期间是否被修改
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

//标签节点解锁
static void UWB_release_tag(TUWBTag *tag){
  os_releaseSemphore(tag->semLock);  //对标签解锁
}

//将数据帧更新到相应标符的frameCache中，并返回该数据在frameCahce中的指针
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




//搜索已存在的基站节点,并加锁后返回。
//如果基站节点不存在则自动创建
//注：每个lab下的基站心跳超时后，是超时停止而不是超时释放。
//当一个lab下的基站全部超时后，可以定期清理掉，目前没有实现。
void *UWBLab_load(U32 anchorID,U32 labID){
  //从当前已经存在的所有基站分组中查找基站节点
   if(!labID)return NULL;//允许anchorID为0,但是labID不能为0;
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
           if(dtmr_update(desAnchor,HEARTBEAT_OVERTIME_MS,dtmrOptions)){//失败
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
     else{//不指定anchorID时，返回Lab地址(加锁)。
       //desAnchor=uwbLab->anchors[0];
       return uwbLab;
     }
   }
   else{//基站节点及其分组都不存在，则新建基站分组及其所有基站节点
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
           if(i==anchorCount)anchorCount=0;//没有找到lab下匹配的labID
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
               anchorNode->mode=anchorModes[index];//普通定位基站还是同步基站(同步基站必须在最后一个)
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
           if(!anchorID)return uwbLab;// 不指定anchorID时，返回第加锁的UwbLab
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
  //将定位数据包以及帧率发送给用户，并将定位信息保存到数据库(todo later)
  //用户列表从其当前uwb lab上挂靠的登陆用户链表中查找，有多个登陆用户，则遍历发送。
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
        //由于listenNode挂载在lab的listeningUsers链表中，所以要事先要对lab加锁
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
  os_createSemphore(&uwb_global_locker, 1);/*信号量初值为1，作互斥量*/
  dtmr_labLinks=dtmr_create(32,UWB_LAB_TIMEOUT_MS,NULL,"dtmr_labLinks");
  init_tag_list();
  //初始房间列表的dummy header
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
    //从标签列表中获取标签节点并加锁(不允许其它线程并发修改此标签)。
    //如果标签不存在则自动创建并初始化
    TUWBTag *curTag=UWB_obtain_tag(curTagID);
    if(!curTag) ret_error=UWBERR_INVALID_TAG;
    else {
      //根据anchorID以及当前相关的tagID，找到anchorNode，并更新对应关系（不存在则创建）
      int curTime=os_msRunTime(); //系统当前时间
      TUWBAnchor *curAnchor=(TUWBAnchor *)dtmr_findById(dtmr_termLinks,frame_session,FALSE);//不需要加锁(基站节点都是只读信息))
      if(!curAnchor || curAnchor->terminal.id!=curAnchorID)ret_error=UWBERR_INVALID_ANCHOR;
      else {
        int curSyncID=FRAMES(syncID);
        int tagID_cacheIndex=curTagID&((1<<FRAME_CACHE_BITLEN)-1);
        int syncID_cacheIndex=curSyncID&((1<<FRAME_CACHE_BITLEN)-1);
        TUWBLocalAreaBlock *curLab=curAnchor->lab;
        TUWBAnchor **anchors=curLab->anchors;
        int lab_anchorCount=curLab->anchorCount-1;//去掉一个同步基站
        int i,cacheHitCount=0;
        for(i=0;i<lab_anchorCount;i++){
          TDataProcessed *cacheData=&anchors[i]->frameCache[tagID_cacheIndex][syncID_cacheIndex];//tagNode data cache pool (hash map list)
          if(anchors[i]->terminal.id==curAnchorID){
            cacheHitCount++;
            //将数据帧更新到相应标符的frameCache中。
            UWB_updateCache(cacheData,(UWBRawFrame *)packetData,curTime);
          }
          else{
            if(cacheData->tagID==curTagID && cacheData->syncID==curSyncID && curTime-cacheData->recvTime<FRAME_ASSEMBLE_TIMEOUT_MS ){
              cacheHitCount++;
            }
          }
        }
        if(cacheHitCount==lab_anchorCount){//收齐当前标签在同一组所有锚点上接收的数据
          TPOINT *coord=UWB_location_calculate(curAnchor,curTag,curSyncID);//执行定位计算
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
                curTag->fps=bufPointIndex*1000/used_time_ms; //计算定位频率(frame per second)
                //用户发送定位数据包，同时记录到数据库
                UWB_location_post(curLab,curTag,bufPointIndex);//向用户发送定位数据包，同时记录到数据库
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
      UWB_release_tag(curTag);//解锁
    }
  }
  if(ret_error)printf("###uwb_data: error=%d\n",ret_error);
  //return ret_error;
}


