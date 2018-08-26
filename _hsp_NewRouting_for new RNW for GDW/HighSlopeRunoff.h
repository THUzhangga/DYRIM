#ifndef HIGHSLOPERUNOFF_H
#define HIGHSLOPERUNOFF_H
#pragma once
#include "DataStruct.h"
#include "YuLiang.h"
#include "YuLiangChBG.h"
#include "YuLiangCMORPH.h"//add CMORPH,shy
#include "FVMunit.h"
#include "WaterInRockSnow.h"
#include "WaterBelowChannel.h"
#include "WaterInChannel.h"
#include ".\savestatus.h"
#include <iostream>
#include <fstream>
using namespace std;

//基于物理机理的高边坡产流模型
class HighSlopeRunoff
{
public:
	HighSlopeRunoff(void);
public:
	~HighSlopeRunoff(void);

/*********模型全局变量，计算中不变化*********/
public:
	CString RainType;

	//计算结果采样周期(s)，采样步长总数
	int MSTEP,Steps,StatusTime;

	//全流域集水面积
	float BasinArea; 

	static bool debug;
	static long HourStart;
	static long NumofHours;
	
	HSParameter Min;
	
public:
	static float E;//m3
	static float P;//m3
	
	//TOPAZ提取的元流域基本参数
	static Para* myPara;

	//河段二叉树编码
	static BSCode mBSCode;

private:
	ADODB::_ConnectionPtr pCnn;

	//记录报错信息
	ofstream myFile;

	//日均雨量，经纬坐标。
	Cyuliang PMin;
   
	//时段雨量，大地坐标。
	YuLiangChBG PMinCBG;

	//CMORPH雨量，经纬坐标，shy
	YuLiangCMORPH PMinCMORPH;
	
	//饱和及非饱和土壤水
	std::vector<FVMunit> fvL,fvR,fvS;

	//临界高程组
	std::vector<pair<int,int>> lv;
	
	//沟道地下水
	WaterBelowChannel Bchannel;
	
	//沟道明渠水
	WaterInChannel Ichannel;

	//SaveStatus mSaveStatus;

public:
	//岩石裂隙水
	static WaterInRockSnow Rock;


private:
	//gh平滩河面宽;gm平滩水深;ef平滩河底宽;theta沟道两边坡的坡角(弧度)
	float gh,gm,ef,theta1,theta2;
	
	//图中的K,L
	float KK[2],LL[2],KS[2];

	//K,L两点之间的距离，可能包含直线段和圆弧段
	float KL;


private:
	//坡面漫流中转变量,在这里实际上以空间间换空间，对于一条特定河段，只new一次，因为没必要每一个时间步长都new一次，再delete一次。
	//new和delete都是有时间开销的，但是放到类里的问题是只有到运行结束内存才释放，不够及时。
	//dL:坡面空间步长序列,axL:下降牛顿迭代求得的增量
	double *aL,*aR,*aS,*bL,*bR,*bS,*cL,*cR,*cS,*fL,*fR,*fS,*xL,*xR,*xS;
	double *dL,*dR,*dS,*afL,*afR,*afS,*axL,*axR,*axS;

	//上游来流序列(m3/s),出口流量序列(m3/s),各个坡面的单宽流量(m2/s)
	//Qu:上游两支流的明渠水流量过程；Qd:上游两支流的沟道地下水流量过程
	float *Qu1,*Qu2,*Qd1,*Qd2,*Quout,*Qdout,*FlowB,*FlowH,*FlowV;



public:
	void Calc(BSCode mBSCode0,Para* myPara0,float* Qu10,float* Qu20,float* Qd10,float* Qd20,float* Quout0,float* Qdout0,float* FlowB0,float* FlowH0,float* FlowV0);
	void Initiallize(ADODB::_ConnectionPtr pCnn0,CString RainType0,long NumofHours0,long HourStart0,long StatusTime0,float BasinArea0,int MSTEP0,int Steps0);
		                        
private:
	//获取元流域沟道参数
	void RiverChannel(void);

	//生成元流域计算网格
	void MeshRegion(void);

	//坡面漫流计算
	void SlopeRunoff(string sType,float p,float dt);

	//饱和土壤水运动计算
	void SaturatedSoil(string sType,float dt);

	//沟道地下水计算
	void BellowChannel(float dt);

	//每条河段执行一次
	void InitializeOneRiver(BSCode mBSCode0,Para* myPara0,float* Qu10,float* Qu20,float* Qd10,float* Qd20,float* Quout0,float* Qdout0,float* FlowB0,float* FlowH0,float* FlowV0);

	//每条河段执行一次
	void FinalizeOneRiver(void);

};



#endif
