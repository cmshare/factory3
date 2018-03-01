#include "mc_routine.h"
#include "uwb_model.h"
#include <math.h>
//---------------------------------------------------------------------------
#define METERDIV 0.004690356868
//常量数据
const double gafFilterCoef[36]={-0.001538, -0.0029955, -0.004767, -0.0065768, -0.0080205, -0.0085981, -0.0077706,
    -0.0050336, 2.4424e-018, 0.0075222, 0.017469, 0.029489, 0.042947, 0.056967, 0.070515, 0.082506, 0.091921,
    0.097933, 0.1, 0.097933, 0.091921, 0.082506, 0.070515, 0.056967, 0.042947, 0.029489, 0.017469, 0.0075222,
    2.4424e-018, -0.0050336, -0.0077706, -0.0085981, -0.0080205, -0.0065768, -0.004767, -0.0029955};

void UWB_tag_init(TUWBTag *tag){
  KalmanTagParams *kalParams=&tag->kalmanTagParams;
  kalParams->outputKX[0] = 1;
  kalParams->outputKX[1] = 1;
}

void UWB_lab_init(TUWBLocalAreaBlock *lab){
  double ab = lab->ab_mm/1000.0;
  double bc = lab->bc_mm/1000.0;
  double ac = lab->ac_mm/1000.0;
  double angleB = acos((bc * bc + ab * ab - ac * ac) / (2 * bc * ab));
  TUWBAnchor **anchors=lab->anchors;
  anchors[0]->position.x=ab;
  anchors[0]->position.y=0;
  anchors[1]->position.x=0;
  anchors[1]->position.y=0;
  anchors[2]->position.x=bc * cos(angleB);
  anchors[2]->position.y=bc * sin(angleB);

  lab->compensate21 = ((int)(lab->db_mm-lab->da_mm))/1000.0/METERDIV;
  lab->compensate31 = ((int)(lab->dc_mm-lab->da_mm))/1000.0/METERDIV;
  lab->compensate41 = 0;

   /*
  KalmanGroupParams *kalParams=&lab->kalmanGroupParams;
  kalParams->TTT[0][0] = anchors[0].position.x;
  kalParams->TTT[0][1] = anchors[0].position.y;
  kalParams->TTT[1][0] = anchors[1].position.x;
  kalParams->TTT[1][1] = anchors[1].position.y;
  kalParams->TTT[2][0] = anchors[2].position.x;
  kalParams->TTT[2][1] = anchors[2].position.y;
   */
}


static int brinv(double *a,int n)
{
    int *is,*js,i,j,k,w,u,v;
    double d,p;

    is=(int *)malloc(n*sizeof(int));
    js=(int *)malloc(n*sizeof(int));
    for (k=0; k<n; k++)
    {
        d=0.0;
        for (i=k; i<n; i++)
        for (j=k; j<n; j++)
        { w=i*n+j;
          p=a[w];
          if(p<0)p=-p;
          if (p>d)
           { d=p; is[k]=i; js[k]=j;}
        }
        if (d+1.0==1.0)
        {
            free(is); free(js);

            return 0;
        }
        if (is[k]!=k)
        for (j=0; j<=n-1; j++)
        { u=k*n+j; v=is[k]*n+j;
           p=a[u]; a[u]=a[v]; a[v]=p;
        }
        if (js[k]!=k)
        for (i=0; i<=n-1; i++)
        { u=i*n+k; v=i*n+js[k];
            p=a[u]; a[u]=a[v]; a[v]=p;
        }
        w=k*n+k;
        a[w]=1.0/a[w];
        for (j=0; j<=n-1; j++)
          if (j!=k)
          { u=k*n+j; a[u]=a[u]*a[w];}
        for (i=0; i<=n-1; i++)
          if (i!=k)
            for (j=0; j<=n-1; j++)
              if (j!=k)
              { u=i*n+j;
                 a[u]=a[u]-a[i*n+k]*a[k*n+j];
              }
        for (i=0; i<=n-1; i++)
          if (i!=k)
          { u=i*n+k; a[u]=-a[u]*a[w];}
      }
      for (k=n-1; k>=0; k--)
      {
          if (js[k]!=k)
          for (j=0; j<=n-1; j++)
          { u=k*n+j; v=js[k]*n+j;
             p=a[u]; a[u]=a[v]; a[v]=p;
          }
          if (is[k]!=k)
          for (i=0; i<=n-1; i++)
          { u=i*n+k; v=i*n+is[k];
             p=a[u]; a[u]=a[v]; a[v]=p;
          }
      }
    free(is); free(js);
    return(1);
  }

static void brmul(double *a,double *b,int m,int n,int k,double *c)
{ int i,j,l,u;
  for (i=0; i<m; i++)//2
    for (j=0; j<k; j++){//1
      u=i*k+j;
      c[u]=0.0;
      for (l=0; l<n; l++)//2
        c[u]=c[u]+a[i*n+l]*b[l*k+j];
    }

}

static void CalcCoordinates(TUWBAnchor *anchors[3],double r21,double r31,TPOINT *coor1,TPOINT*coor2)
{   int i;
    double B[2],C[2],N[2],M[2],K[3],A[2][2];
    A[0][0] = anchors[1]->position.x-anchors[0]->position.x;
    A[0][1] = anchors[1]->position.y-anchors[0]->position.y;
    A[1][0] = anchors[2]->position.x-anchors[0]->position.x;
    A[1][1] = anchors[2]->position.y-anchors[0]->position.y;
    brinv(&A[0][0],2);
    B[0] = -r21;
    B[1] = -r31;
    brmul(&A[0][0],B,2,2,1,N);
    for(i=0;i<3;i++)
    {   double x=anchors[i]->position.x;
        double y=anchors[i]->position.y;
        K[i] = x*x+y*y;
    }
    C[0] = -0.5*(B[0]*B[0]+K[0]-K[1]);
    C[1] = -0.5*(B[1]*B[1]+K[0]-K[2]);
    brmul(&A[0][0],C,2,2,1,M);

    double a = N[0]*N[0]+N[1]*N[1]-1;
    if(a!=0){
      double b = 2*N[0]*(M[0]-anchors[0]->position.x)+2*N[1]*(M[1]-anchors[0]->position.y);
      double c = (M[0]-anchors[0]->position.x)*(M[0]-anchors[0]->position.x)+(M[1]-anchors[0]->position.y)*(M[1]-anchors[0]->position.y);
      double x=b*b-4*a*c;
      if(x>=0){
        x=sqrt(x);
        a*=2;
        double root1 = (-b+x)/a;
        double root2 = (-b-x)/a;
        coor1->x = root1*N[0]+M[0];
        coor1->y = root1*N[1]+M[1];
        coor2->x = root2*N[0]+M[0];
        coor2->y = root2*N[1]+M[1];
      }
    }
}


static void calcMeanCoor(int staCnt,KalmanTagParams *kalmanTagParams,double meanCoor[]){
   int i;
   meanCoor[0] = 0;
   meanCoor[1] = 0;
   for(i=0;i<staCnt;i++){
        meanCoor[0] += kalmanTagParams->historyCoor[0][99-i];
        meanCoor[1] += kalmanTagParams->historyCoor[1][99-i];
   }
   meanCoor[0] = meanCoor[0]/(double)staCnt;
   meanCoor[1] = meanCoor[1]/(double)staCnt;
}

static void MatrixTranspose(double* fMatrixA,unsigned int m,unsigned n,double* fMatrixB){
    unsigned int index_i,index_j,index_u,index_v;
    for (index_i=0;index_i<m;index_i++)
        for (index_j=0;index_j<n;index_j++){
            index_u = index_j*m+index_i;
            index_v = index_i*n+index_j;
            fMatrixB[index_u] = fMatrixA[index_v];
        }
}

static void MatrixAdd( double* fMatrixA,double* fMatrixB,double* Result,unsigned int m,unsigned int n ){
    unsigned int index_i,index_j,itemp;
    for (index_i=0;index_i<m;index_i++)
        for (index_j=0;index_j<n;index_j++){
            itemp = index_i*n+index_j;
            *(Result+itemp) = *(fMatrixA+itemp) + *(fMatrixB+itemp);
        }
}

static void MatrixSub( double* fMatrixA,double* fMatrixB,double* Result,unsigned int m,unsigned int n ){
    unsigned int index_i,index_j,itemp;
    for (index_i=0;index_i<m;index_i++)
        for (index_j=0;index_j<n;index_j++)
        {
            itemp = index_i*n+index_j;
            *(Result+itemp) = *(fMatrixA+itemp) - *(fMatrixB+itemp);
        }
}

static void Kalman_Filter(TUWBLocalAreaBlock *lab,KalmanTagParams *kalmanTagParams){
//KalmanGroupParams *kalmanGroupParams
    int i;
    double *outputKX=kalmanTagParams->outputKX;
    double *kYm=kalmanTagParams->inputXY;
    static double kA[2][2] = {{1,0},{0,1}};
    static double kQ[2][2] = {{0.1,0},{0,0.1}};
    static double kR[3][3] = {{5,0,0},{0,5,0},{0,0,5}};
    static double kI[2][2] = {{1,0},{0,1}};
    double kM[2][2],kAP[2][2],kAT[2][2],kAPAT[2][2],kK[2][3],kCT[2][3],kMCT[2][3];
    double kCMCT[3][3],kCMCTaR[3][3],Fsk[3],kYmFsk[3],kY[3],kKkYmFsk[2],kKkC[2][2],kImkKkC[2][2];
//    TAnchor *anchors=lab->anchors;
    for(i=0;i<3;i++){
        double ax=lab->anchors[i]->position.x;
        double ay=lab->anchors[i]->position.y;

        double *kC=kalmanTagParams->kC[i];
        kY[i] = (kYm[0] - ax)*(kYm[0] - ax) + (kYm[1] - ay)*(kYm[1] - ay);
        kY[i] = (kY[i]>0)?sqrt(kY[i]):0;
        Fsk[i] = (outputKX[0] - ax)*(outputKX[0] - ax) + (outputKX[1] - ay)*(outputKX[1] - ay);
        Fsk[i] = (Fsk[i]>0)?sqrt(Fsk[i]):0;
        kC[0] = (outputKX[0] - ax)/Fsk[i];
        kC[1] = (outputKX[1] - ay)/Fsk[i];
    }
    brmul(kA[0],&kalmanTagParams->kP[0][0],2,2,2,kAP[0]);
    MatrixTranspose(kA[0],2,2,kAT[0]);
    brmul(kAP[0],kAT[0],2,2,2,kAPAT[0]);
    MatrixAdd(kAPAT[0],kQ[0],kM[0],2,2);
    MatrixTranspose(kalmanTagParams->kC[0],3,2,kCT[0]);
    brmul(kM[0],kCT[0],2,2,3,kMCT[0]);
    brmul(kalmanTagParams->kC[0],kMCT[0],3,2,3,kCMCT[0]);
    MatrixAdd(kCMCT[0],kR[0],kCMCTaR[0],3,3);
    brinv(kCMCTaR[0],3);
    brmul(kMCT[0],kCMCTaR[0],2,3,3,kK[0]);
    MatrixSub(kY,Fsk,kYmFsk,3,1);
    brmul(kK[0],kYmFsk,2,3,1,kKkYmFsk);
    MatrixAdd(outputKX,kKkYmFsk,outputKX,2,1);
    brmul(kK[0],kalmanTagParams->kC[0],2,3,2,kKkC[0]);
    MatrixSub(kI[0],kKkC[0],kImkKkC[0],2,2);
    brmul(kImkKkC[0],kM[0],2,2,2,kalmanTagParams->kP[0]);
}

 //定位计算环节
TPOINT *UWB_location_calculate(TUWBAnchor *curAchor,TUWBTag *tagNode,int curSyncID){
  TUWBLocalAreaBlock *lab=curAchor->lab;
  KalmanTagParams *kalmanTagParams=&tagNode->kalmanTagParams;
  TUWBAnchor **block_anchors=lab->anchors;
  int i;
  int curTagID=tagNode->id;
  int tagID_cacheIndex=curTagID&((1<<FRAME_CACHE_BITLEN)-1);
  int syncID_cacheIndex=curSyncID&((1<<FRAME_CACHE_BITLEN)-1);
  TDataProcessed *cacheData0=&block_anchors[0]->frameCache[tagID_cacheIndex][syncID_cacheIndex];
  TDataProcessed *cacheData1=&block_anchors[1]->frameCache[tagID_cacheIndex][syncID_cacheIndex];
  TDataProcessed *cacheData2=&block_anchors[2]->frameCache[tagID_cacheIndex][syncID_cacheIndex];

  //MemoPrint("%I64d,%I64d,%I64d\n",cacheData0->timeStamp,cacheData1->timeStamp,cacheData2->timeStamp);

  int T21 = cacheData1->timeStamp-cacheData0->timeStamp;
  int T31 = cacheData2->timeStamp-cacheData0->timeStamp;
  double R21 = (T21+lab->compensate21)*METERDIV;
  double R31 = (T31+lab->compensate31)*METERDIV;
  /*
  TDataProcessed *cacheData3=&block_anchors[3].frameCache[tagID_cacheIndex][syncID_cacheIndex];
  int T41 = cacheData3->recvTime-cacheData0->recvTime;
  double R41 = (T41+kalmanGroupParams->compensate41)*METERDIV;
 */
  CalcCoordinates(lab->anchors,R21,R31,&tagNode->coor1,&tagNode->coor2);

  double selfCoor_0 = tagNode->coor2.x;
  double selfCoor_1 = tagNode->coor2.y;
  if(selfCoor_0!=selfCoor_0 || selfCoor_1!=selfCoor_1 || selfCoor_0>10||selfCoor_0<-10 || selfCoor_1>10||selfCoor_1<-10 ){
    selfCoor_0 = kalmanTagParams->preInputXY[0];
    selfCoor_1 =kalmanTagParams-> preInputXY[1];
  }

  double coorerr_0 = selfCoor_0 - kalmanTagParams->precoor2[0];
  double coorerr_1 = selfCoor_1 - kalmanTagParams->precoor2[1];
  double Tcoorerr = sqrt(coorerr_0*coorerr_0+coorerr_1*coorerr_1);
  double meanCoor[2] = {0};
  double *inputXY=kalmanTagParams->inputXY;

  //MemoLog("###Tcoorerr=%f,inputXY[0]=%f,inputXY[1]=%f",Tcoorerr,inputXY[0],inputXY[1]);

  BOOL kalman_buffering_completed;
  if(kalmanTagParams->preloadCounter<KALMAN_WIDOWN_SIZE){
    kalman_buffering_completed=FALSE;
    kalmanTagParams->preloadCounter++;
     //MemoLog("***inputXY======%f, %f",inputXY[0],inputXY[1]);
    inputXY[0] = selfCoor_0;
    inputXY[1] = selfCoor_1;
    kalmanTagParams->historyCoor[0][99] =  inputXY[0];
    kalmanTagParams->historyCoor[1][99] =  inputXY[1];
    for(i=0;i<99;i++){
      kalmanTagParams->historyCoor[0][i] = kalmanTagParams->historyCoor[0][i+1];
      kalmanTagParams->historyCoor[1][i] = kalmanTagParams->historyCoor[1][i+1];
    }
    for(i=0;i<100;i++){
      meanCoor[0] += kalmanTagParams->historyCoor[0][i];
      meanCoor[1] += kalmanTagParams->historyCoor[1][i];
    }
    meanCoor[0] = meanCoor[0]/100;
    meanCoor[1] = meanCoor[1]/100;
    //MemoLog("###inputXY======%f, %f",inputXY[0],inputXY[1]);
  }
  else{/* end of==>if(kalmanTagParams->waitTemp<TRACK_POINT_BUFFER) */
    kalman_buffering_completed=TRUE;
    if(Tcoorerr>0.1){//0.1
      if(Tcoorerr>0.2){//0.2
        if(Tcoorerr>2){//0.5
          if(Tcoorerr>5){//3
            if(Tcoorerr>7){//5
              inputXY[0] = 0.95*inputXY[0]+0.05*selfCoor_0;
              inputXY[1] = 0.95*inputXY[1]+0.05*selfCoor_1;
            }
            else{
              inputXY[0] = 0.9*inputXY[0]+0.1*selfCoor_0;
              inputXY[1] = 0.9*inputXY[1]+0.1*selfCoor_1;
            }
          }
          else{
            inputXY[0] = 0.85*inputXY[0]+0.15*selfCoor_0;
            inputXY[1] = 0.85*inputXY[1]+0.15*selfCoor_1;
          }
        }
        else{
          inputXY[0] = 0.65*inputXY[0]+0.35*selfCoor_0;
          inputXY[1] = 0.65*inputXY[1]+0.35*selfCoor_1;
        }
      }
      else{
        inputXY[0] = 0.4*inputXY[0]+0.6*selfCoor_0;
        inputXY[1] = 0.4*inputXY[1]+0.6*selfCoor_1;
      }
    }
    else{
      inputXY[0] = selfCoor_0;
      inputXY[1] = selfCoor_1;
    }
    //LowPassFir(&inputXY[0][0],&inputXY[0][1],buf.tagID);
    for(i=0;i<99;i++){
      kalmanTagParams->historyCoor[0][i] = kalmanTagParams->historyCoor[0][i+1];
      kalmanTagParams->historyCoor[1][i] = kalmanTagParams->historyCoor[1][i+1];
    }
    kalmanTagParams->historyCoor[0][99] =  inputXY[0];
    kalmanTagParams->historyCoor[1][99] =  inputXY[1];

    #if 0 //这段代码什么意思？(原代码中是启用的，这里暂不用）
    if(kalmanTagParams->staFlag == 1){
      kalmanTagParams->staCNT++;
      if(kalmanTagParams->staCNT>100)kalmanTagParams->staCNT = 100;
      for(i=0;i<kalmanTagParams->staCNT;i++){
        meanCoor[0] += kalmanTagParams->historyCoor[0][99-i];
        meanCoor[1] += kalmanTagParams->historyCoor[1][99-i];
      }
      meanCoor[0] = meanCoor[0]/kalmanTagParams->staCNT;
      meanCoor[1] = meanCoor[1]/kalmanTagParams->staCNT;
      inputXY[0] = meanCoor[0];
      inputXY[1] = meanCoor[1];
    }
    else{
      kalmanTagParams->staCNT = 0;
    }
    kalmanTagParams->staFlag = curAchor->frameCache[tagID_cacheIndex][syncID_cacheIndex].accX;
    #endif

    #if 0
    if(kalmanGroupParams->staCNT > 10){
      calcMeanCoor(kalmanGroupParams,kalmanTagParams,meanCoor);
      inputXY[0] = meanCoor[0];
      inputXY[1] = meanCoor[1];
    }
    kalmanTagParams->staFlag = 0;
    for(i=0;i<8;i++){
      if(kalmanTagParams->historyAcc[i]<0.5)kalmanTagParams->staFlag++;
    }
    if(kalmanTagParams->staFlag == 8){
      inputXY[0] = meanCoor[0];
      inputXY[1] = meanCoor[1];
    }
    #endif
  }

  #if 0
  kalmanTagParams->historyCoor[0][19] =  inputXY[0];
  kalmanTagParams->historyCoor[1][19] =  inputXY[1];
  for(i=0;i<19;i++) {
    kalmanTagParams->historyCoor[0][i] = kalmanTagParams->historyCoor[0][i+1];
    kalmanTagParams->historyCoor[1][i] = kalmanTagParams->historyCoor[1][i+1];
  }
  else{
    inputXY[0] = selfCoor_0;
    inputXY[1] = selfCoor_1;
  }
  #endif

  kalmanTagParams->preInputXY[0] = inputXY[0];
  kalmanTagParams->preInputXY[1] = inputXY[1];
  kalmanTagParams->precoor2[0] = kalmanTagParams->preInputXY[0];
  kalmanTagParams->precoor2[1] = kalmanTagParams->preInputXY[1];

// MemoLog("***Input==[%f, %f] Output==[%f, %f]",kalmanTagParams->inputXY[0],kalmanTagParams->inputXY[1],kalmanTagParams->outputKX[0],kalmanTagParams->outputKX[1]);
  Kalman_Filter(lab,kalmanTagParams);
 // MemoLog("###Input==[%f, %f] Output==[%f, %f]",kalmanTagParams->inputXY[0],kalmanTagParams->inputXY[1],kalmanTagParams->outputKX[0],kalmanTagParams->outputKX[1]);
  //printf("[Tag%02d:%02d][%f, %f]  [%f, %f]  [%f, %f]",tagNode->id,curSyncID,kalmanTagParams->outputKX[0],kalmanTagParams->outputKX[1],tagNode->coor2.x,tagNode->coor2.y,inputXY[0],inputXY[1]);

#if 1
  return (kalman_buffering_completed)?(TPOINT *)kalmanTagParams->outputKX:NULL;
#else
  if(kalman_buffering_completed)
  { double x = kalmanTagParams->outputKX[0];
    double y = kalmanTagParams->outputKX[1];
    double x1 = tagNode->coor2.x;
    double y1 = tagNode->coor2.y;
    double x2 = inputXY[0];
    double y2 = inputXY[1];

    int bufCoordIndex=tagNode->bufCoordIndex;
    U32 *bufCoord=tagNode->bufCoordArray[bufCoordIndex];
    bufCoord[0]=(U32)(x*1000);
    bufCoord[1]=(U32)(y*1000);

    bufCoordIndex++;
    U32 now_time=os_msRunTime();
    int used_time_ms=now_time-tagNode->bufCoordStartTime;
    if(used_time_ms>MAXLEN_BUFFER_LOCATE_TIMESPAN || bufCoordIndex==MAXLEN_BUFFER_LOCATE_POINTS){
       //计算定位频率
       if(used_time_ms>0)tagNode->fps=bufCoordIndex*1000/used_time_ms;
       //用户发送定位数据包，同时记录到数据库
       UWB_location_post(lab,tagNode,bufCoordIndex);//向用户发送定位数据包，同时记录到数据库
       bufCoordIndex=0;
       tagNode->bufCoordStartTime=now_time;
    }
    tagNode->bufCoordIndex=bufCoordIndex;
  }
  return NULL;
#endif
}



