#pragma once
#include "DataStruct.h"
#include "CpuUsageCount.h"//20060327,李铁键,加入CPU利用率,CpuUsageCount类由夏清提供
#include <iostream>
#undef SEEK_SET
#undef SEEK_END
#undef SEEK_CUR
#include "mpi.h"
using namespace std;


//wh，这里存的是数据库中HydroUsePara表所有参数，各种模型所需要的参数都存储在该表中，这些参数与FilePath.ini中的参数对应
typedef struct SystemParameter
{
	long HourStart;
	long NumofHours;
	long PrepareHours;//准备计算时间，计算而不保存的时段，用于消除没有河道内初始流量的影响
	long StatusTime;//20080406，wh，增加status表存储数据的周期，原来是1d1存，数据量比较大。

	//CString m_RainFile;
	CString CSDatasource;
	CString CSUser;
	CString CSPassword;

	//CString sRoutingMethod;//汇流方法
    CString sRainType;//降雨量数据的类型	
	CString CompRegions;

	int TaskUnitSize;   //分子树任务单元的期望大小
	int MinTaskUnitSize;//能承受的最小任务子树的大小
	int iCalGravityErosion;//20050316,李铁键,增加是否计算重力侵蚀的参数.0.不计算;1.全计算;2.除Regionindex==0&&BSValue==0之外的计算
	int iCalcSediTrans;//20070608,xiaofc,是否计算泥沙输送，暂简化为是否保存

	bool bCalRsvUp;//是否计算水库以上
	bool bSaveGravityEvents;//20060327,李铁键,是否保存重力侵蚀事件
	bool bSaveFlowPattern;//20070622,xiaofc,Save the flow pattern, namely B, H, v
	bool bSaveAllDischarge;//wh,20080217是否保存所有河段的流量信息，为0时只保存definednodes中定义的河段
	
	float GravityErosioinP2;//20060324,李铁键,发生重力侵蚀的纵向范围的概率
	float fAlphaErosion;//冲刷时的恢复饱和系数
	float fAlphaDeposition;//淤积时的恢复饱和系数
	CString SediTransCapF;//"Fei""Zhang"

	// coefficients needed in "parameters"
	float UpInitWaterContent;
	float MidInitWaterContent;
	float DownInitWaterContent;

	// coefficients needed in "evapotranspiration"
	CString emethod;
	float thetab;//毛管断裂含水率
	float thetaw;//调萎含水率
	int N;//指数
	float E0_a; //E0/a

	CString sccd;//wh，方案编号
	int MSTEP;//wh，时间步长

    CString SoilErosionEquation;//wh,20080803
	
}SystemParameter;



//wh,该类实现MasterProcess、SlaveProcess、TransferProcess的一些公共操作
class CommenProcess
{
public:
	CommenProcess(void);
	~CommenProcess(void);

public:
	void ProcessInitialize(int worldsize,int Rank,CString user,CString password,CString sid);
	void ReadHydroUsePara(void);//从数据库中读取系统参数
	void ReadRegionConnection(void);//从RegionConnection表中读取信息

protected://下面的这些变量各进程都是要用到的，注意了。
	
	//---MPI共享参数---//
	int WorldSize;//世界就这么大:)
	int rank;
	int namelen;
	int TransferProcessRank;//中转进程的标号
	char processor_name[MPI_MAX_PROCESSOR_NAME];
	MPI_Status Status;

	//---CPU利用率共享参数--//
	CCpuUsageCount m_CpuUsage;//20060327,李铁键,加入CPU利用率
	unsigned long lCpuUsage;
	int iCpuUsage;

	//---数据库指针共享参数--//
	ADODB::_ConnectionPtr pCnn;//连接Oracle数据库
	ADODB::_RecordsetPtr pRst;//连接RiverSegs表

	//---内存变量共享参数---//
	SystemParameter SParameter;//HydroUsePara表变量
	XAJParameter XAJ;//对应XAJUSEPARA表
	BSCode* RegionConnection;
	CComVariant tempCom;//20070204,xiaofc,字段值在win2000下不会自动转为unsigned long long,定义临时ComVariant
	int RCCount;
	int GradeTwoLoop;

	long Steps;
	long lTimeInterval;

};
