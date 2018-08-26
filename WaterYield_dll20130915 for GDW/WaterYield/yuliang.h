#pragma once
#ifndef YULIANG_H
#define YULIANG_H
#include <vector>
#include <math.h>
#include "CoorTrans.h"
#include "datastruct.h"
using namespace std;
// Cyuliang command target
struct xyPerjw
{
	float xUnit;
	float yUnit;
};


struct YuliangZhan
{
	jwCoord jwLocation;  //雨量站的位置
	xyCoord xyLocation;
	float weight;   //权重
	long lCode;     //雨量站的代码
};

struct PointPro
{
	float dis;//雨量站距离河段的距离
	float xiShu;
};

typedef vector<float> PSeriesType;

class Cyuliang
{

public:
	Cyuliang();
	virtual ~Cyuliang();
	short GetMonthDays(short Year, short Month);
	float P(double HourOffset);
	short m_YearStart,m_YearEnd,m_MonthStart,m_MonthEnd,m_DayStart,m_DayEnd;
	CString m_FilePath,m_RainFile;
	void Initialize(double x,double y,long StartHour, long Hours);
	//void OpenMDBCnn();
	//void CloseMDBCnn();
	bool DeleteList(void);

public:
	ADODB::_ConnectionPtr pConnection;//wh,convert private to public,2008.1.14
	int MSTEP;//wh,2008.3.24

private:

	bool GetDataByDW(float jx,float wy);

	inline void JwUnitToXYUnit(jwCoord jw,xyPerjw *Dxy);
	
	vector< YuliangZhan * >  m_TargetPoints;
	double m_CenterX;
	double m_CenterY;
	PSeriesType m_PSeries;//河段的降雨量序列，计算起止时间之间有多少天，链表就有多少项。
	
	PSeriesType GetGageTS(long ID,short YearStart,short MonthStart, short DayStart,short YearEnd, short MonthEnd, short DayEnd);

	PSeriesType GetDataByTS(float jx,float wy,short YearStart,short MonthStart, short DayStart,short YearEnd, short MonthEnd, short DayEnd);

	void ReadPrevHalf(PSeriesType &m_series, ADODB::_ConnectionPtr pCnn, long ID, short Year, short Month, short DayEnd);
	void ReadLastHalf(PSeriesType &m_series, ADODB::_ConnectionPtr pCnn, long ID, short Year, short Month, short DayStart);
	void ReadWholeMonth(PSeriesType &m_series, ADODB::_ConnectionPtr pCnn,long ID, short Year, short Month);
	void ReadMiddle(PSeriesType &m_series, ADODB::_ConnectionPtr pCnn,long ID, short Year, short Month, short DayStart, short DayEnd);
	void MonthAdd(short CurYear,short CurMonth,short &NextYear, short &NextMonth); 

	long StartTime;//计算开始时间距离基准时间的小时数
	long EndTime;
	float PArray[24];//日雨量分摊到每个小时
	float ScatterDayToHours(long HourOffset,float rainfall);
	
protected:
	bool CalcuWt(jwCoord jw);
	
};

void Cyuliang::JwUnitToXYUnit(jwCoord jw,xyPerjw* Dxy)
{
    float MiddleY,Rx,Ry;
	MiddleY=jw.wCoord/180.0f*3.1415926f;
	Rx=6378.245f*cos(MiddleY)/sqrt(1.0f-0.006693422f*sin(MiddleY)*sin(MiddleY));
	Ry=6378.245f;
	Dxy->xUnit=Rx;
	Dxy->yUnit =Ry;

}
#endif

