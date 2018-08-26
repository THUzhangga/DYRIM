#pragma once

using namespace std;

typedef struct PrecipitationS{
	double TimeStart;
	double TimeEnd;
	float Precipitation;
	PrecipitationS *next;
}PrecipitationS,*pPrecipitationS;

class YuLiangChBG
{
public:
	YuLiangChBG(void);
	~YuLiangChBG(void);

	short GetMonthDays(short Year, short Month);
	float P(double HourOffset,double* Hours,float dt);
	short m_YearStart,m_YearEnd,m_MonthStart,m_MonthEnd,m_DayStart,m_DayEnd;

	float m_HourStart;//wh
	
	float HourStart,NumofHours;//wh,计算开始时间和总计算小时数

	CString m_RainFile;
	void Initialize(double x,double y,long StartHour, long Hours);
	//void OpenMDBCnn();
	//void CloseMDBCnn();
	bool DeleteList(void);

public:
	ADODB::_ConnectionPtr pCon;//wh,convert private to public,2008.1.14

private:
	pPrecipitationS ptrP;
	
	//wh
	pPrecipitationS pRev;//ptrP的前一个节点指针

	//wh
	pPrecipitationS pHead;//仍然是头节点
	
	long StartTime;
	long EndTime;
	inline void FreePSeries(pPrecipitationS &pPSeries);
	inline void MonthAdd(short CurYear, short CurMonth, short &NextYear, short &NextMonth);
	double GetNumOfHour(short YearS,short MonthS,short DayS,float HourS,short YearE,short MonthE,short DayE,float HourE);

public:
	int MSTEP;//WH,2008.3.24
};
