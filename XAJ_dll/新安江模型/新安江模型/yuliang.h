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
	jwCoord jwLocation;  //����վ��λ��
	xyCoord xyLocation;
	float weight;   //Ȩ��
	long lCode;     //����վ�Ĵ���
};

struct PointPro
{
	float dis;//����վ����Ӷεľ���
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
	PSeriesType m_PSeries;//�ӶεĽ��������У�������ֹʱ��֮���ж����죬������ж����
	
	PSeriesType GetGageTS(long ID,short YearStart,short MonthStart, short DayStart,short YearEnd, short MonthEnd, short DayEnd);

	PSeriesType GetDataByTS(float jx,float wy,short YearStart,short MonthStart, short DayStart,short YearEnd, short MonthEnd, short DayEnd);

	void ReadPrevHalf(PSeriesType &m_series, ADODB::_ConnectionPtr pCnn, long ID, short Year, short Month, short DayEnd);
	void ReadLastHalf(PSeriesType &m_series, ADODB::_ConnectionPtr pCnn, long ID, short Year, short Month, short DayStart);
	void ReadWholeMonth(PSeriesType &m_series, ADODB::_ConnectionPtr pCnn,long ID, short Year, short Month);
	void ReadMiddle(PSeriesType &m_series, ADODB::_ConnectionPtr pCnn,long ID, short Year, short Month, short DayStart, short DayEnd);
	void MonthAdd(short CurYear,short CurMonth,short &NextYear, short &NextMonth); 

	long StartTime;//���㿪ʼʱ������׼ʱ���Сʱ��
	long EndTime;
	float PArray[24];//��������̯��ÿ��Сʱ
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

