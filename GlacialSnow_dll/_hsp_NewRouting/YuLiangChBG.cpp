
//#include "stdafx.h"
#include "datastruct.h"
#include ".\yuliangchbg.h"


YuLiangChBG::YuLiangChBG(void)
{
	/*pRev=ptrP=pHead=NULL;
	StartTime=EndTime=0;*/
}

YuLiangChBG::~YuLiangChBG(void)
{
}

//altered by wh,the rain data had been write into oracle
//void YuLiangChBG::OpenMDBCnn()
//{
//	CString CStr=m_RainFile;   //降雨量数据库的路径
//	CStr=CStr.Left(CStr.ReverseFind('.'));
//	CString rainMDB,rainLDB,ConStr;
//	_bstr_t  CnnbStr;
//	pCon.CreateInstance(__uuidof(ADODB::Connection));
//	HRESULT hr;
//
//	short i=0;
//	rainMDB.Format(_T("%s%d.mdb"),CStr,i);
//	rainLDB.Format(_T("%s%d.ldb"),CStr,i);
//	
//	try
//	{
//	
//		while(PathFileExists(rainMDB.GetString()) && PathFileExists(rainLDB.GetString()))
//		{	i++;
//			rainMDB.Format(_T("%s%d.mdb"),CStr,i);
//			rainLDB.Format(_T("%s%d.ldb"),CStr,i);
//		}
//		if(PathFileExists(rainMDB.GetString()) )
//		{
//			ConStr=_T("Provider=Microsoft.Jet.OLEDB.4.0;Data Source=") + rainMDB + _T(";Persist Security Info=False");
//			CnnbStr=ConStr.GetString();
//			hr=pCon->Open(CnnbStr,"Admin","",-1);
//			if(!SUCCEEDED(hr))
//			{
//				//MessageBox(NULL,"打开数据库"+rainMDB+"出错","警告",MB_OK|MB_ICONERROR);
//				cout<<"警告:打开数据库"<<rainMDB<<"出错"<<endl;
//			}
//		}
//		else
//		{
//			CopyFile(m_RainFile,rainMDB,false);
//			ConStr=_T("Provider=Microsoft.Jet.OLEDB.4.0;Data Source=") + rainMDB + _T(";Persist Security Info=False");
//			CnnbStr=ConStr.GetString();
//			hr=pCon->Open(CnnbStr,"Admin","",-1);
//			if(!SUCCEEDED(hr))
//			{
//				//MessageBox(NULL,"打开数据库"+rainMDB+"出错","警告",MB_OK|MB_ICONERROR);
//				cout<<"警告:打开数据库"<<rainMDB<<"出错"<<endl;
//			}
//		}
//	}//end of try
//	catch (...) 
//	{
//		cout<<"Error While Open MDB Cnn."<<endl;
//		exit(0);
//	}
//}
//
//void YuLiangChBG::CloseMDBCnn()
//{
//	pCon->Close();
//	//cout<<"CloseRainfallMDB"<<endl;
//}


void YuLiangChBG::Initialize(double x,double y,long StartHour, long Hours)
{
	//初始化相关的变量要注意，因为雨量类是产流类的成员变量，因此构造函数全局只执行一次，如果河段相关，则
	//应该放在Initialize函数中
	pRev=ptrP=pHead=NULL;
	StartTime=EndTime=0;

	//wh
	//flag_Head=false;
	this->NumofHours=Hours;//wh
	this->HourStart=StartHour;//wh

	int YearEnd,YearStart,MonthStart,MonthEnd,DayStart,DayEnd;
	long DayCount,temp;

	//通过StartHour获得起始年月日（开始）
	YearStart=YearSTD;      //基准年
	MonthStart=MonthSTD;	//基准月

	DayCount=long(StartHour/24); //转换小时为d
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

	//通过StartHour获得起始年月日（结束）

	//通过Hours获得终止年月日（开始）
	float HourInDay=StartHour-24*long(StartHour/24);//wh
	m_HourStart=HourInDay;


	YearEnd=YearStart;
	MonthEnd=MonthStart;
	DayCount=short((Hours+HourInDay-1)/24)+1;  //转换小时为d

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
	//通过Hours获得终止年月日（结束）

	StartTime=StartHour;
	EndTime=StartHour+Hours;


	::CoInitialize(NULL);

	ADODB::_RecordsetPtr  pRec;
	CComVariant ResultID;//20070209,xiaofc,为适应可能的字符串型测站号
	float tempX,tempY,MinDistance,tempDistance;
	CString tempStr;
	
	//tempStr=m_RainFile;   //降雨量数据库的路径
	//CString ConStr="Provider=Microsoft.Jet.OLEDB.4.0;Data Source="+tempStr+";Persist Security Info=False";
	//_bstr_t  bSQL=ConStr.GetString();
	//pCon.CreateInstance(__uuidof(ADODB::Connection));
	pRec.CreateInstance(__uuidof(ADODB::Recordset));
    //pCon->Open(bSQL,"Admin","",0); //打开数据库连接
	//寻找距离最近的雨量站的ID(开始)
	_bstr_t  bSQL;
	bSQL="Select ID,X,Y from NameTabHour";//将nametab改为nametabhour
	try
	{
		pCon->Cancel();
		pRec->Open(bSQL,(ADODB::_Connection*)pCon,ADODB::adOpenKeyset,ADODB::adLockOptimistic,ADODB::adCmdText);
		if(!pRec->EndOfFile){
			ResultID=pRec->Fields->Item["ID"]->Value;
			tempX=pRec->Fields->Item["X"]->Value;
			tempY=pRec->Fields->Item["Y"]->Value;
			MinDistance=(x-tempX)*(x-tempX)+(y-tempY)*(y-tempY);
			pRec->MoveNext();
		}
		while(!pRec->EndOfFile){
			tempX=pRec->Fields->Item["X"]->Value;
			tempY=pRec->Fields->Item["Y"]->Value;
			tempDistance=(x-tempX)*(x-tempX)+(y-tempY)*(y-tempY);
			if(tempDistance<MinDistance){
				ResultID=pRec->Fields->Item["ID"]->Value;
				MinDistance=tempDistance;
			}
			pRec->MoveNext();
		}
		pRec->Close();
		//寻找距离最近的雨量站的ID(结束)
	}
	catch(_com_error e)
	{
		cout<<e.ErrorMessage();
		cout<<e.Source();
		cout<<e.Description();
	}

	int curYear,curMonth,curDay;//,HourS,HourE;
	long tempLong;
	float HourStart,HourEnd,Percipitation,tempflt;
	pPrecipitationS pTemp;
	StartTime=GetNumOfHour(YearSTD,MonthSTD,DaySTD,HourSTD,YearStart,MonthStart,DayStart,0);
	EndTime=GetNumOfHour(YearSTD,MonthSTD,DaySTD,HourSTD,YearEnd,MonthEnd,DayEnd,24);

	//20070209,xiaofc,通过CComVariant的类型判定测站编号的字段类型，从而自动生成正确的SQL语句。
	//20070505,xioafc,如果不是VT_BSTR,均ChangType()，这样处理更统一一些
	CString tempStr2;
	try
	{
		if(ResultID.vt!=VT_BSTR)
		{
			ResultID.ChangeType(VT_BSTR);
			tempStr2=ResultID.bstrVal;
			tempStr.Format(_T("Select Year,Month,Day,HourS,HourE,P from rainhour where ID=%s and Year>=%d and Year<=%d order by Year,Month,Day,HourS"),tempStr2,YearStart,(YearEnd+1));//wh,将rain改为rainhour
		}
		else
		{
			tempStr2=ResultID.bstrVal;
			tempStr.Format(_T("Select Year,Month,Day,HourS,HourE,P from rainhour where ID='%s' and Year>=%d and Year<=%d order by Year,Month,Day,HourS"),tempStr2,YearStart,(YearEnd+1));//wh,将rain改为rainhour
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
			break;  //当走到了时间起点时就停止下来
		pRec->MoveNext();
	}//此步循环是为了找到时间的起点

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
			break;  //当走过了时间终点位置就停止下来

		//对HourS和HourE进行四舍五入
		//if((HourStart-int(HourStart))>0.3) HourS=int(HourStart)+1;
		//else HourS=int(HourStart);
		//if((HourEnd-int(HourEnd))>0.3) HourE=int(HourEnd)+1;
		//else HourE=int(HourEnd);

		//上一部分对HourS和HourE进行四舍五入
		//下一部分不进行四舍五入
		HourStart=int(HourStart)+(HourStart-int(HourStart))/0.6;
		HourEnd=int(HourEnd)+(HourEnd-int(HourEnd))/0.6;

		if(!ptrP){ pHead=ptrP=pTemp=(pPrecipitationS)malloc(sizeof(PrecipitationS)); pTemp->next=NULL;}
		else {pTemp->next=(pPrecipitationS)malloc(sizeof(PrecipitationS));pTemp=pTemp->next; pTemp->next=NULL;}
		tempflt=HourEnd-HourStart;//时段差必然是后减前,20060315纠正的错误

		if(tempflt<=0) 
			tempflt=tempflt+24;

		pTemp->Precipitation=Percipitation/tempflt;
		pTemp->TimeStart=GetNumOfHour(YearSTD,MonthSTD,DaySTD,HourSTD,curYear,curMonth,curDay,HourStart);
		pTemp->TimeEnd=GetNumOfHour(YearSTD,MonthSTD,DaySTD,HourSTD,curYear,curMonth,curDay,HourEnd);
		
		//20061218,xiaofc:原来跨0时的问题没有真正解决，在于：
		//一个降雨记录的Day字段只是开始的那d，如果HourEnd-HourStart<0发生了
		//那么，结束的那一d必然是Day字段之后的那d，因此增加了判断
		if(HourEnd<HourStart)
			pTemp->TimeEnd+=24;
		//end of 20061218,xiaofc
		//cout<<pTemp->TimeStart<<"\t"<<pTemp->TimeEnd<<": "<<pTemp->Precipitation<<endl;
		
		pRec->MoveNext();
	}//此步循环是为了执行起点到终点位置间的降雨量过程

	pRec->Close();

	::CoUninitialize();
}

bool YuLiangChBG::DeleteList(void)
{
	//FreePSeries(ptrP);
	FreePSeries(pHead);//wh
	StartTime=EndTime=0;
	return true;
}

//wh，Hours无雨的秒数(s),时间步长dt(s)
//ptrP:停留在有雨的链表项里
float YuLiangChBG::P(double HourOffset,double* Hours,float dt)
{
	if((HourOffset<StartTime) || (HourOffset>EndTime)) return -1;

	(*Hours)=0.0;//wh
	
	float Result=0;
	pPrecipitationS pTemp;

	//wh,记录链表前一个元素地址(已经放到类变量里)
	//pPrecipitationS pRev=NULL;

	pTemp=ptrP;

	//如果pTemp为NULL，说明雨量数据的终止时间在计算截止时间之前，必须要特殊处理，否则*Hours返回0值，FVMunit::TimeStart无法增加，主类中陷入死循环。
	if(pTemp==NULL)
	{
		(*Hours)=(HourStart+NumofHours-HourOffset)*3600+1;//多加1秒，怕出现浮点数运算问题
		return -1;
	}

	//如果当前时刻有雨，则while中断，如果无雨，则继续往下查找，直到找到有雨项才终止.
	while(pTemp)
	{
		//下面那个1e-5非常重要，如果没有就不断的进入下面那个if，浮点数比较的问题，(*Hours)是一个无穷小量，一定注意
		if(pTemp->TimeStart>HourOffset+1e-5)
		{
			//此种情况是最小雨量时间大于计算开始时间，当然也要和计算结束时间比较，取小值，pRev=NULL
			//此时pTemp定位到链表第一个元素
			
			float EndHours;
			if(pTemp->TimeStart>(HourStart+NumofHours))
			{
				EndHours=NumofHours;
			}
			else
			{
				EndHours=pTemp->TimeStart-HourOffset;
			}

			(*Hours)+=EndHours; 
			break;
		}

		//if(HourOffset>=(pTemp->TimeStart) && HourOffset<=(pTemp->TimeEnd)) 20070713,xiaofc,这样处理浮点数相等有问题
		if(pTemp->TimeStart-HourOffset<1e-5 && HourOffset-pTemp->TimeEnd<1e-5)
		{
			Result=pTemp->Precipitation;//mm/小时
			//20061218,xiaofc:调过0时雨量的错误时用
			//cout<<pTemp->TimeStart<<"\t"<<pTemp->TimeEnd<<":"<<Result<<endl;
			if(Result>1e-5)
			{
				//有降雨时不需要知道*Hour的值，只需返回雨量即可，自然pRev也不需要在乎
				ptrP=pTemp;
				return Result*dt/3600.0*0.001;//返回值为m
			}
			else
			{
				(*Hours) += ( pTemp->TimeEnd - max(pTemp->TimeStart,HourOffset) );
				
				pRev=pTemp;
				pTemp=pTemp->next;

				break;
			}
		}
		
		pRev=pTemp;
		pTemp=pTemp->next;
	}

	//为了得到HourOffset时刻以后有多长时间没有降雨,此时pTemp->TimeStart一定大于HourOffset
	while(pTemp)
	{
		//如果无雨，时间累加
		if(pTemp->Precipitation<1e-5 && pTemp->TimeStart<(HourStart+NumofHours)-1e-5) 
		{ 
			if(pRev==NULL)
			{
				(*Hours) += (pTemp->TimeEnd - pTemp->TimeStart);
			}
			else
			{
				//引入pRev是为了解决HourS，HourE时间可能不连续的问题，即前一天的HourEnd和第二天的HourS不相等的情况，这段时间也是无雨的。
				(*Hours) += (pTemp->TimeEnd - pRev->TimeEnd);
			}

			pRev=pTemp;
			pTemp=pTemp->next;
		}
		else 
		{ 
			//ptrP=pTemp; 
			break;
		}
	}

	//可以规避pTemp为NULL，ptrP得不到值的情况
	ptrP=pTemp;

	//HourOffset时刻没有降雨时返回0,只关注*Hours的大小
	(*Hours)*=3600;//小时变为秒

	return -1;

	//return Result*dt/3600.0*0.001;//返回值为m
}



inline void YuLiangChBG::FreePSeries(pPrecipitationS &pPSeries)
{
	pPrecipitationS pCur,pTemp;
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

double YuLiangChBG::GetNumOfHour(short YearS,short MonthS,short DayS,float HourS,short YearE,short MonthE,short DayE,float HourE)
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

short YuLiangChBG::GetMonthDays(short Year, short Month)
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

inline void YuLiangChBG::MonthAdd(short CurYear, short CurMonth, short &NextYear, short &NextMonth)
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
