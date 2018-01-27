//---------------------------------------------------------------------------
#ifndef UWB_MODEL_TDOA_H
#define UWB_MODEL_TDOA_H
//---------------------------------------------------------------------------
#define UWB_FRAME_SIZE                    sizeof(UWBRawFrame)
#define FRAME_CACHE_BITLEN                6
#define FRAME_ASSEMBLE_TIMEOUT_MS         2500
#define TAGLIST_HASH_LEN                  256
#define KALMAN_WIDOWN_SIZE                50
//---------------------------------------------------------------------------
//为缓解服务器压力，不允许每计算一个点就向用户发送一次数据，
//而是设置一个时间与点数的条件范围(与关系)，超过这个范围则打包发送一次数据给用户。
#define MAXLEN_BUFFER_LOCATE_POINTS       32    //最大缓冲的定位点数
#define MAXLEN_BUFFER_LOCATE_TIMESPAN     1500  //最大缓冲的时间间隔
//---------------------------------------------------------------------------
enum {UWBERR_UNKNOWN=-1,UWBERR_NONE=0,UWBERR_INVALID_FRAME_FORMAT,UWBERR_CHECKNUM,UWBERR_SESSION,UWBERR_INVALID_TAG,UWBERR_INVALID_ANCHOR};
enum {ANCHOR_TYPE_SYNC=0,ANCHOR_TYPE_DATA=1};
//---------------------------------------------------------------------------
#pragma pack (push,1)
//---------------------------------------------------------------------------
typedef struct{
  U16 accX;     //x-asis 加速度信息.
  U16 accY;     //y-asis 加速度信息
  U16 accZ;     //z-asis 加速度信息
  U16 angX;     //x-asis 角速度
  U16 angY;     //y-asis 角速度
  U16 angZ;     //z-asis 角速度
}TAccData;

typedef struct{
  U32 x,y;
  TAccData accData;
}TAccAndPoint;

typedef struct{
  U32  tagID,labID;
  U16  fps;
  U16  pointCount;
  TAccAndPoint pointData[];
}TUWB_PackDataToUser;

typedef struct _UWB_RawFrame{
  U32 sessionID;   //回话ID
  U32 anchorID;    //基站/锚点ID
  U32 tagID;       //标签ID
  U8  syncID;      //标签的同步流水号(由标签设备生成，0-255循环自加,每一次定位时至少三个基站应收到同一个标签的相同SycID的定位数据）.
  U8  timeStamp[5];//标签的定位时间戳(Byte order: little endian, unit: ms)
  //时间戳分度为156.25ps，总共5个字节最大表示17.17s，程序中有换算距离公式
  TAccData accData;
  U8 checknum;  //所有字节的异或
}UWBRawFrame;
//---------------------------------------------------------------------------
#pragma pack (pop)
//---------------------------------------------------------------------------
typedef struct{
    int anchorID,tagID,syncID;
    U64 timeStamp;
   // U32 accX,accY,accZ;
    U32 recvTime;
}TDataProcessed;
//---------------------------------------------------------------------------
typedef struct {double x,y;}TPOINT;
typedef struct _TUWBLocalAreaBlock TUWBLocalAreaBlock;
//基站信息一旦初始化后，就基本固定不动了，运行期主要是读操作，不需要加锁。
typedef struct{
  TTerminal terminal;
  int index,mode;
  TPOINT position;
  TUWBLocalAreaBlock *lab;//belong to which UWB area block
  TDataProcessed frameCache[1<<FRAME_CACHE_BITLEN][1<<FRAME_CACHE_BITLEN];//tagNode data cache pool (hash map list)
}TUWBAnchor;
//---------------------------------------------------------------------------
typedef struct{
   int staFlag;
   int staCNT;//????
   int preloadCounter; //计数器
   double gafDataBuffer[3][36];
   double historyCoor[2][100];
   double precoor2[2];
   double preInputXY[2];
   double inputXY[2];
   double outputKX[2];
   double kC[3][2];
   double kP[2][2];
}KalmanTagParams;
//---------------------------------------------------------------------------
//房间（配置）信息没有超时删除机制
//由管理后台发送指令给算法后台来实现（UWB local area block的增删改更新）
//算法服务后台启动时不会立即从数据库载入所有激活状态的分组信息，只有在运行期用到（收到定位数据）时才动态创建节点并从数据库载入配置信息。
typedef struct{
  U32 labID;
  U32 ab_mm,bc_mm,ac_mm,da_mm,db_mm,dc_mm;
  U16 algorithm;//0:TDOA
  U16 anchorCount;//default=4
}TUWBLabConfig;

struct _TUWBLocalAreaBlock{
  int id;
  int algorithm;//0:TDOA
  int anchorCount;//default=4
  int compensate21,compensate31,compensate41;
  U32 ab_mm,bc_mm,ac_mm,da_mm,db_mm,dc_mm;
  TBinodeLink listeningUsers;
  TUWBAnchor *anchors[0]; //anchor list of A,B,C,etc.
  //根据配置信息分配实际的基站节点数内存
};
//---------------------------------------------------------------------------
//Tag节点，不固定于任何定位区域（UWB local area block),可以无缝切换于多个（UWB lab)
//Taglist  哈希表
//将收到标签数据写入tagNode的bufames时加锁，并更新deadline（另建线程扫描是否会超时），超时则删除节点。
typedef struct _TUWBTag{
  struct _TUWBTag  *up,*down,*prev,*next;
  int id; //tagID
  TPOINT coor1,coor2;
  TPOINT postion;//last caculated position ()
  KalmanTagParams kalmanTagParams;
  HAND semLock;     //互斥锁
  int  fps;//即时定位帧率(每次填满bufCoordArray时计算一次);
  int  bufPointIndex;
  U32  bufPointStartTime;//开始缓冲时间（用于计算定位帧率）,单位毫秒。
  TAccAndPoint bufPointArray[MAXLEN_BUFFER_LOCATE_POINTS];//缓冲定位数据包
  U32  deadline; //超时删除的时间。
}TUWBTag;
//---------------------------------------------------------------------------
void UWB_system_init(void);

void UWB_lab_init(TUWBLocalAreaBlock *lab);
void UWB_tag_init(TUWBTag *tag);
int  UWB_process_frame(UWBRawFrame *uwbFrame);
TPOINT *UWB_location_calculate(TUWBAnchor *curAchor,TUWBTag *tagNode,int curSyncID);
void UWB_location_post(TUWBLocalAreaBlock *lab,TUWBTag *tagNode,int pointCount);
BOOL UWB_frame_checkcrc(UWBRawFrame *frame);
void *UWBLab_load(U32 anchorID,U32 labID);
//---------------------------------------------------------------------------
BOOL load_lab_configs(int labID,TUWBLocalAreaBlock *configs,U32 *anchorIDs,int *anchorModes,int maxAnchorCount);
void UWBLab_checkDelete(TUWBLocalAreaBlock *lab);
BOOL UWBLab_switchUser(TTerminal *user,BOOL ownedUserLock,U32 newLabID);//暂时不考虑绑定关系
void UWB_update_labconfig(TUWBLabConfig *labcfg);
//---------------------------------------------------------------------------
void MemoPrint(const char *format, ...);
//---------------------------------------------------------------------------
#endif
