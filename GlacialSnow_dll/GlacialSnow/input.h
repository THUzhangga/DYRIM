//input.h和input.cpp为高洁的融雪类。
#pragma once
#include "CoorTrans.h"
#include <Shlwapi.h>
#include <fstream>
#include <iostream>
#include <vector>

using namespace std;

//该河段，某年某月某日某时的信息。
struct hourvalue
{
	long houroffset;
	int Year;
	int Mon;
	int Day;
	int Hour;
	float SCA;
	float Tmin;
	float Tmax;
	float Thour;
	float P;
	float A;
};

//该河段，某年某月某天的最高、最低气温和sca
struct dayvalue
{
	int Year;
	int Mon;
	int Day;
	float Tmax;
	float Tmin;
	
	float SCA;
};

//该河段，某年某月的最高气温和最低气温
struct monvalue
{
	int Year;
	int Mon;
	float Tmax;
	float Tmin;
};

//某年某月某气象站的最高气温和最低气温
struct mon_gaugevalue
{
	monvalue monzhi;
	long id;
};


//该河段(元流域），某年某月的特征sca（由平均高程决定）
struct SCAcertain
{
	int Mon;
	int Day;
	float SCA;
};


//一个结构体大致对应gauge表中的一条记录
struct QixiangZhan
{
	jwCoord jwLocation;  //雨量站的位置
	xyCoord xyLocation;
	float weight;   //权重
	long lCode;     //雨量站的代码
};


struct PointInfo
{
	float dis;
	//float xiShu;
};


typedef vector<monvalue *> TSeriesType;//河段的信息

typedef vector<mon_gaugevalue *>TGSeriesType;//气象站的信息。

class input
{

//被全局接口调用函数（并行计算的始终只运行一次)：
public:
	void ReadElevationAndA(void);//被initialzie()调用
	void initialzie();  //被全局接口OpenOracle()调用
	void finalize();    //被全局接口SRMFinalize()调用



//被局部接口调用函数：
public:

	//以下三个函数被局部接口SnowInitialize()调用，每算一条河段执行一次。
	void GetDataByTS(void);
	void ReadDayValue(void);//月的展平到日
	void ReadHourValue(void);//日的展平到小时
	
	//以下一个函数被局部接口SnowCalc()调用，每个时间步长执行一次
	void ToWaterYield(long houroffset,float* HourRainIn);//added by wanghao
	
	//以下一个函数被局部接口ReleaseHeap()调用，每算一条河段执行一次。
	void Deleteall(void);


//气象站插值相关的中间函数
public:
	bool GetDataByDW(void);
	bool CalcuWt(jwCoord jw);
	TGSeriesType GetGageTS(long ID);
	

//时间转换函数
private:
	void houroffset2date(long houroffset);
	long date2houroffset(int Year,int Month,int Day,int Hour);
	int GetMonthDays(int Year, int Month);
	void MonthAdd(int CurYear, int CurMonth, int &NextYear, int &NextMonth);
	void GetNumofDay(void);
	
	

//数据库变量
public:
	ADODB::_ConnectionPtr pConn;


//链表变量
private:
	vector< QixiangZhan* >  m_TargetPoints;//气象站位置信息
	vector< TGSeriesType >  mySeriesArray;	
	
	hourvalue* hourhappen;
	dayvalue* dayhappen;
	TSeriesType monhappen;//整个链表代表该河段（元流域），元素元素之间的区别在于时间（月份）不同，自然元素间的温度也就不同。


//其他变量
private:
	SCAcertain* needsca;
	long Rcount;//needsca的元素个数，全局值，不变化。
	int MonthEnd_1;
	int numofmonhappen;//int numofmonhappen = sizeof(monhappen);monhappen的个数
	

	
//河段变量，由产流模型传入
public:
	float jx;
	float wy;
	unsigned long long RegionIndex;
	long Length;
	unsigned long long Value;
	float A;
	float Havg;
	double m_CenterX;
	double m_CenterY;


//时间变量，计算起止时间，不变化。
public:
	int YearStart;
	int MonthStart;
	int DayStart;
	int YearEnd;
	int MonthEnd;
	int DayEnd;
	long numofday;//总天数。
	long numofhour;
	long houroffsetstart;


};

//该河段（元流域），某年某月的各种sca（对于不同高程）
//struct SCA
//{
//	int Mon;
//	int Day;
//	float SCA1;
//	float SCA2;
//	float SCA3;
//	float SCA4;
//};

//typedef vector<float> PSeriesType;//你并没有使用

//bool DeleteList(void);
//int numofsca = sizeof(needsca);
//SCA* allsca;
//SCA* ReadSCA(void);
//void ReadSCA(void);//modifed by wanghao
//ADODB::_Connection* OpenMDB(CString m_File);
//ADODB::_ConnectionPtr pCnn;
//ADODB::_Connection* OpenOracle(CString m_user, CString m_password, CString m_sid);