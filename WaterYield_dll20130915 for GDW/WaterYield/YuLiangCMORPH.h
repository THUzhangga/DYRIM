#pragma once

using namespace std;

typedef struct PrecipitationSS{
	double TimeStart;
	double TimeEnd;
	float Precipitation;
	PrecipitationSS *next;
}PrecipitationSS,*pPrecipitationSS;

class YuLiangCMORPH
{
public:
	YuLiangCMORPH(void);
	~YuLiangCMORPH(void);

	short GetMonthDays(short Year, short Month);
	float P(double HourOffset);
	short m_YearStart,m_YearEnd,m_MonthStart,m_MonthEnd,m_DayStart,m_DayEnd;
	CString m_RainFile;
	void Initialize(double x,double y,long StartHour, long Hours, float XYpara);////增加CMORPH分辨率参数,shy,20130905


	bool DeleteList(void);

public:
	ADODB::_ConnectionPtr pCon;//wh,convert private to public,2008.1.14

private:
	pPrecipitationSS ptrP;	
	long StartTime;
	long EndTime;
	inline void FreePSeries(pPrecipitationSS &pPSeries);
	inline void MonthAdd(short CurYear, short CurMonth, short &NextYear, short &NextMonth);
	double GetNumOfHour(short YearS,short MonthS,short DayS,float HourS,short YearE,short MonthE,short DayE,float HourE);

public:
	int MSTEP;//WH,2008.3.24
};