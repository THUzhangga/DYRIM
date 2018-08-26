
#include "stdafx.h"
#include "datastruct.h"
#include ".\yuliangcmorph.h"


YuLiangCMORPH::YuLiangCMORPH(void)
{
	ptrP=NULL;
	StartTime=EndTime=0;
}

YuLiangCMORPH::~YuLiangCMORPH(void)
{
}

void YuLiangCMORPH::Initialize(double x,double y,long StartHour, long Hours, float XYpara)//����CMORPH�ֱ��ʲ���,shy,20130905
{

	int YearEnd,YearStart,MonthStart,MonthEnd,DayStart,DayEnd;
	long DayCount,temp;

	//ͨ��StartHour�����ʼ�����գ���ʼ��
	YearStart=YearSTD;      //��׼��
	MonthStart=MonthSTD;	//��׼��

	DayCount=long(StartHour/24); //ת��СʱΪd
    temp=0;
	while(temp+GetMonthDays(YearStart,MonthStart)<DayCount){
		temp+=GetMonthDays(YearStart,MonthStart);
		MonthStart++;
		if(MonthStart==13)
		{
			MonthStart=1;
			YearStart++;
		}
	}
	DayStart=short(DaySTD+DayCount-temp);
	temp=DayStart-GetMonthDays(YearStart,MonthStart);
	if(temp>0){
		MonthStart++;
		if(MonthStart==13)
		{
			MonthStart=1;
			YearStart++;
		}
		DayStart=short(temp);
	}
	m_YearStart=YearStart;
	m_MonthStart=MonthStart;
	m_DayStart=DayStart;
	//ͨ��StartHour�����ʼ�����գ�������

	//ͨ��Hours�����ֹ�����գ���ʼ��
	YearEnd=YearStart;
	MonthEnd=MonthStart;
	DayCount=short((Hours-1)/24)+1;  //ת��СʱΪ��
	
	temp=0;
	while(temp+GetMonthDays(YearEnd,MonthEnd)<DayCount){
		temp+=GetMonthDays(YearEnd,MonthEnd);
		MonthEnd++;
		if(MonthEnd==13)
		{
			MonthEnd=1;
			YearEnd++;
		}
	}
	DayEnd=short(DayStart+DayCount-temp);
	temp=DayEnd-GetMonthDays(YearEnd,MonthEnd);
	if(temp>0){
		MonthEnd++;
		if(MonthEnd==13)
		{
			MonthEnd=1;
			YearEnd++;
		}
		DayEnd=short(temp);
	}
	m_YearEnd=YearEnd;
	m_MonthEnd=MonthEnd;
	m_DayEnd=DayEnd;
	//ͨ��Hours�����ֹ�����գ�������

	StartTime=StartHour;
	EndTime=StartHour+Hours;

	::CoInitialize(NULL);

	//Ѱ�Ҿ������������վ��ID,20130904, shy
	ADODB::_RecordsetPtr  pRec;
	CComVariant ResultID;       //20070209,xiaofc,Ϊ��Ӧ���ܵ��ַ����Ͳ�վ��
	float RX,RY;
	int R,C;
	CString tempStr;
	pRec.CreateInstance(__uuidof(ADODB::Recordset));
	_bstr_t  bSQL;
	//���·����ȶ�λ�Ӷ�����դ���ٶ�λ����ֵ
	//RX=int(x*4+0.5)/4.0;
	//RY=int(y*4+0.5)/4.0;
	//R=0.5-(RY-58.875)/0.25;
	//C=(RX-0.125)/0.25+0.5;
	//ResultID=R*10000+C;
	
	//Ҳ�������¹���ֱ�Ӷ�λ����
	if(abs(XYpara-0.07277)<1e-5)
	{
		R=int(-(y-59.963614)/0.072756669)+1;
		C=int((x-0.036378335)/0.072771377)+1;
		ResultID=R*10000+C;
	}
	else if(abs(XYpara-0.25)<1e-5)
	{
		R=int(-(y-58.875)/0.25)+1;
		C=int((x-0.125)/0.25)+1;
		ResultID=R*10000+C;
	}
	//Ѱ��ID����
	int curYear,curMonth,curDay;//,HourS,HourE;
	long tempLong;
	float HourStart,HourEnd,Percipitation;
	pPrecipitationSS pTemp;
	StartTime=GetNumOfHour(YearSTD,MonthSTD,DaySTD,HourSTD,YearStart,MonthStart,DayStart,0);
	EndTime=GetNumOfHour(YearSTD,MonthSTD,DaySTD,HourSTD,YearEnd,MonthEnd,DayEnd,24);

	//20070209,xiaofc,ͨ��CComVariant�������ж���վ��ŵ��ֶ����ͣ��Ӷ��Զ�������ȷ��SQL��䡣
	//20070505,xioafc,�������VT_BSTR,��ChangType()�����������ͳһһЩ
	CString tempStr2;
	try
	{
		if(ResultID.vt!=VT_BSTR)
		{
			ResultID.ChangeType(VT_BSTR);
			tempStr2=ResultID.bstrVal;
			if(abs(XYpara-0.07277)<1e-5)
				tempStr.Format(_T("Select Year,Month,Day,HourS,HourE,P from rainCMorph30minh8km where ID=%s and Year>=%d and Year<=%d order by Year,Month,Day,HourS"),tempStr2,YearStart,(YearEnd+1));
			else
				tempStr.Format(_T("Select Year,Month,Day,HourS,HourE,P from rainCMorph3h25km where ID=%s and Year>=%d and Year<=%d order by Year,Month,Day,HourS"),tempStr2,YearStart,(YearEnd+1));
		}
		else
		{
			tempStr2=ResultID.bstrVal;
			if(abs(XYpara-0.07277)<1e-5)
				tempStr.Format(_T("Select Year,Month,Day,HourS,HourE,P from rainCMorph30minh8km where ID=%s and Year>=%d and Year<=%d order by Year,Month,Day,HourS"),tempStr2,YearStart,(YearEnd+1));
			else
				tempStr.Format(_T("Select Year,Month,Day,HourS,HourE,P from rainCMorph3h25km where ID='%s' and Year>=%d and Year<=%d order by Year,Month,Day,HourS"),tempStr2,YearStart,(YearEnd+1));
		}	
	}
	catch (...)
	{
		cout<<"Data type of field 'ID' in the rain table is incorrect!"<<endl;
		return;
	}
	
	bSQL=tempStr.GetString();
    pRec->Open(bSQL,(ADODB::_Connection*)pCon,ADODB::adOpenKeyset,ADODB::adLockOptimistic,ADODB::adCmdText);
	while(!pRec->EndOfFile){
		curYear=pRec->Fields->Item["Year"]->Value;
		curMonth=pRec->Fields->Item["Month"]->Value;
		curDay=pRec->Fields->Item["Day"]->Value;
		tempLong=GetNumOfHour(curYear,curMonth,curDay,0,YearStart,MonthStart,DayStart,0);
		if(tempLong<=0)
			break;  //���ߵ���ʱ�����ʱ��ֹͣ����
		pRec->MoveNext();
	}//�˲�ѭ����Ϊ���ҵ�ʱ������

	while(!pRec->EndOfFile)
	{
		curYear=pRec->Fields->Item["Year"]->Value;
		curMonth=pRec->Fields->Item["Month"]->Value;
		curDay=pRec->Fields->Item["Day"]->Value;
		HourStart=pRec->Fields->Item["HourS"]->Value;
		HourEnd=pRec->Fields->Item["HourE"]->Value;
		Percipitation=pRec->Fields->Item["P"]->Value;
		tempLong=(long)GetNumOfHour(curYear,curMonth,curDay,0,YearEnd,MonthEnd,DayEnd,0);
		if(tempLong<0)
			break;  //���߹���ʱ���յ�λ�þ�ֹͣ����

		//CMORPH����ֱ���ǰٷ��Ƶ�, ���Ϊ��, ������������, shy, 20130904
		//HourStart=int(HourStart)+(HourStart-int(HourStart))/0.6;
		//HourEnd=int(HourEnd)+(HourEnd-int(HourEnd))/0.6;

		if(!ptrP){ ptrP=pTemp=(pPrecipitationSS)malloc(sizeof(PrecipitationSS)); pTemp->next=NULL;}
		else {pTemp->next=(pPrecipitationSS)malloc(sizeof(PrecipitationSS));pTemp=pTemp->next; pTemp->next=NULL;}

		pTemp->Precipitation=Percipitation;//shy, CMORPH�������mm/h
		pTemp->TimeStart=GetNumOfHour(YearSTD,MonthSTD,DaySTD,HourSTD,curYear,curMonth,curDay,HourStart);
		pTemp->TimeEnd=GetNumOfHour(YearSTD,MonthSTD,DaySTD,HourSTD,curYear,curMonth,curDay,HourEnd);
		
		//20061218,xiaofc:ԭ����0ʱ������û��������������ڣ�
		//һ�������¼��Day�ֶ�ֻ�ǿ�ʼ����d�����HourEnd-HourStart<0������
		//��ô����������һd��Ȼ��Day�ֶ�֮�����d������������ж�
		if(HourEnd<HourStart)
			pTemp->TimeEnd+=24;
		//end of 20061218,xiaofc
		//cout<<pTemp->TimeStart<<"\t"<<pTemp->TimeEnd<<": "<<pTemp->Precipitation<<endl;
		
		pRec->MoveNext();
	}//�˲�ѭ����Ϊ��ִ����㵽�յ�λ�ü�Ľ���������

	pRec->Close();

	::CoUninitialize();
}

bool YuLiangCMORPH::DeleteList(void)
{
	FreePSeries(ptrP);
	StartTime=EndTime=0;
	return true;
}

float YuLiangCMORPH::P(double HourOffset)
{
	if((HourOffset<StartTime) || (HourOffset>EndTime)) return -1;
	
	float Result=0;
	pPrecipitationSS pTemp;
	pTemp=ptrP;
	while(pTemp)
	{
		//if(HourOffset>=(pTemp->TimeStart) && HourOffset<=(pTemp->TimeEnd)) 20070713,xiaofc,���������������������
		//20100422,xiaofc, by ���Ǵ��� 10:37:03
		//"ԭ��YuLiangChBG.cpp�е�P������һ��bug��if(pTemp->TimeStart-HourOffset<1e-5 && HourOffset-pTemp->TimeEnd<1e-5)���ѵ�һ��1e-5��Ϊ-1e-5
		//"��������������ʵ�ʴ�
		//if(pTemp->TimeStart-HourOffset<1e-5 && HourOffset-pTemp->TimeEnd<1e-5)
		if(pTemp->TimeStart-HourOffset<-1e-5 && HourOffset-pTemp->TimeEnd<1e-5)
		{
			Result=pTemp->Precipitation;
			//20061218,xiaofc:����0ʱ�����Ĵ���ʱ��
			//cout<<pTemp->TimeStart<<"\t"<<pTemp->TimeEnd<<":"<<Result<<endl;

			ptrP=pTemp;//2009,added by wanghao,����ÿ�δ�ͷ��ʼ���ң���Ϊʱ�䵥�����������Լ����ϸ�ʱ�䲽�����Ҽ��ɡ�

			break;
		}
		pTemp=pTemp->next;
	}
	return Result*MSTEP/60.0*0.001;//����ֵΪm
}

inline void YuLiangCMORPH::FreePSeries(pPrecipitationSS &pPSeries)
{
	pPrecipitationSS pCur,pTemp;
	if(!pPSeries) return;
	pCur=pPSeries;
	while(pCur){
		pTemp=pCur->next;
		free(pCur);
		pCur=pTemp;
	}
	pPSeries=NULL;
	return;
}

double YuLiangCMORPH::GetNumOfHour(short YearS,short MonthS,short DayS,float HourS,short YearE,short MonthE,short DayE,float HourE)
{
	double m_iResult=0,sum=0;
	if(YearS>YearE) return -1;
	if((YearS==YearE) && (MonthS>MonthE) ) return -1;
	if((YearS==YearE) && (MonthS==MonthE) && (DayS>DayE) ) return -1;
	if((YearS==YearE) && (MonthS==MonthE) && (DayS==DayE)  && (HourS>HourE)) return -1;

	if(MonthS==MonthE && YearS==YearE){
		m_iResult=(DayE-DayS)*24+(HourE-HourS);
		return m_iResult;
	}
	short CurYear,CurMonth,NextYear,NextMonth;
	CurYear=YearS;
	CurMonth=MonthS;
	sum=sum+GetMonthDays(CurYear,CurMonth);
	MonthAdd(CurYear,CurMonth,NextYear,NextMonth);
	while(!((NextYear==YearE) && (NextMonth==MonthE))){
		sum=sum+GetMonthDays(NextYear,NextMonth);
		CurYear=NextYear;
		CurMonth=NextMonth;
		MonthAdd(CurYear,CurMonth,NextYear,NextMonth);
	}
	m_iResult=(sum+(DayE-DayS))*24+(HourE-HourS);
	
	return m_iResult;
}

short YuLiangCMORPH::GetMonthDays(short Year, short Month)
{
	bool IsLeapYear=0;
	if(((Year%4)==0 && (Year%100)!=0) || (Year%400)==0 )
		IsLeapYear=true;

	if(Month==2 && IsLeapYear) return 29;
	if(Month==2 && !IsLeapYear) return 28;
	if(Month==8)  return 31;
	if(Month<8 && Month%2==0) return 30;
	if(Month<8 && Month%2!=0) return 31;
	if(Month>8 && Month%2==0) return 31;
	return 30;
}

inline void YuLiangCMORPH::MonthAdd(short CurYear, short CurMonth, short &NextYear, short &NextMonth)
{
	if(CurMonth==12){
		NextYear=CurYear+1;
		NextMonth=1;
		return ;
	}
	NextYear=CurYear;
	NextMonth=CurMonth+1;
	return;
}