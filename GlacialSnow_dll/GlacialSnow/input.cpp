#include "stdafx.h"
#include "input.h"
#include <cmath>

//该函数为全局函数，整个并行计算期间只运行一次。
void input::initialzie()
{
	//modified by wanghao，放到此函数里可以保证在并行计算的始末，只运行一次。
	MonthEnd_1 = MonthEnd + 1;
	ReadElevationAndA();//此时allsca始终都是数据库中snow_sca的所有值，不需要每次都读一遍。
	this->GetNumofDay();//wh，全局不变的，放到这里。
	numofhour = numofday*24;
	dayhappen = new dayvalue[numofday];//wh
	houroffsetstart = date2houroffset(YearStart,MonthStart,1,0);
	hourhappen = new hourvalue[numofhour];
}

//该函数为全局函数，整个并行计算期间只运行一次。
void input::finalize()
{
	pConn->Close();//全局打开，自然全局关闭
	delete[] dayhappen;
	delete[] hourhappen;
	delete[] needsca;
}


//wh,目的是把houroffset转化为年、月、日，但是后面有问题，已经注明。
void input::houroffset2date(long houroffset)
{
	int YearS,MonthS,DayS;
	long DayCount,temp;

	int m_YearStart;
	int m_MonthStart;
	int m_DayStart;

	//通过StartHour获得起始年月日（开始）
	YearS=YearSTD;      //基准年
	MonthS=MonthSTD;	//基准月
	//直接用：DayStart = DaySTD;//基准日

	DayCount=long(houroffset/24); //转换小时为天
    temp=0;
	while(temp+GetMonthDays(YearS,MonthS)<DayCount)
	{
		temp+=GetMonthDays(YearS,MonthS);
		MonthS++;
		if(MonthS==13)
		{
			MonthS=1;
			YearS++;
		}
	}
	DayS=int(DaySTD+DayCount-temp);
	temp=DayS-GetMonthDays(YearS,MonthS);
	if(temp>0)
	{
		MonthS++;
		if(MonthS==13)
		{
			MonthS=1;
			YearS++;
		}
		DayS=int(temp);
	}
	//wh,以下三行是没用的，m_YearStart，m_MonthStart，m_DayStart是局部变量，在此函数中声明的，函数执行完毕，这些
	//变量统统消失了，那此函数还有什么用的？
	m_YearStart=YearS;
	m_MonthStart=MonthS;
	m_DayStart=DayS;
}

//将年月日转化为houroffset，这个函数应该是没有问题的，被两处调用。
long input::date2houroffset(int Year,int Month,int Day, int Hour)
{
	long numofhour;
	int YearS = YearSTD;
	int MonthS = MonthSTD;
	int DayS = DaySTD;
	int HourS = HourSTD;

	if(MonthS==Month && YearS==Year)
	{
		numofhour=(Day-DayS)*24+(Hour-HourS);
		return numofhour;
	}
	int CurYear,CurMonth,NextYear,NextMonth;
	CurYear=YearS;
	CurMonth=MonthS;
	long sum = 0;
	sum=sum+GetMonthDays(CurYear,CurMonth);
	MonthAdd(CurYear,CurMonth,NextYear,NextMonth);
	while(!((NextYear==Year) && (NextMonth==Month)))
	{
		sum=sum+GetMonthDays(NextYear,NextMonth);
		CurYear=NextYear;
		CurMonth=NextMonth;
		MonthAdd(CurYear,CurMonth,NextYear,NextMonth);
	}
	numofhour=(sum+(Day-DayS))*24+(Hour-HourS);	
	return numofhour;
}

//获得一个月的天数。
int input::GetMonthDays(int Year, int Month)
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

//功能：给定年和月后，再加一个月，年和月分别变为多少。
void input::MonthAdd(int CurYear, int CurMonth, int &NextYear, int &NextMonth)
{
	if(CurMonth==12)
	{
		NextYear=CurYear+1;
		NextMonth=1;
		return;
	}
	NextYear=CurYear;
	NextMonth=CurMonth+1;
	return;
}

//得到气象站的链表，每项只代表气象站本身的信息。
bool input::GetDataByDW(void)    //距离权重法
{
	//xyPerjw Dxy;
	jwCoord jw;
	jw.jCoord=jx;
	jw.wCoord =wy;
	
	ADODB::_RecordsetPtr  pRec;
	pRec.CreateInstance(__uuidof(ADODB::Recordset));

	int n=1,t=0;
	float xMin,xMax,yMin,yMax;   //搜索范围为一个经纬度的正方形
	CString SelSQL;
	_bstr_t SQL;
	while(n<3)    //搜索出3个站点
	{		
		xMin=jx-0.05*t;
		xMax=jx+0.05*t;
		yMin=wy-0.05*t;
		yMax=wy+0.05*t;       //搜索的范围步长为经纬坐标0.01
		SelSQL.Format(_T("select * from gauge where longitude Between %.2f and %.2f and latitude Between %.2f and %.2f"),xMin,xMax,yMin,yMax);
		SQL=SelSQL.GetString();
	    //副主程序中需要加：
		pRec->Open(SQL,(ADODB::_Connection*)pConn,ADODB::adOpenKeyset,ADODB::adLockOptimistic,ADODB::adCmdText);//!!!!阿门
		n=pRec->RecordCount;
		if(n<3)
		{
			pRec->Close();
			t+=1;   //搜索次数
		}
		if(t>220)
		{
			//如果一个都没找出来，就全部插值
			if(!(n>=1))
			{
				xMin=-180;
				xMax=180;
				yMin=-90;
				yMax=90;

				//注意，下面语句不能直接写数字，要用float变量
				SelSQL.Format(_T("select * from gauge where longitude Between %.2f and %.2f and latitude Between %.2f and %.2f"),xMin,xMax,yMin,yMax);

				SQL=SelSQL.GetString();
				pRec->Open(SQL,(ADODB::_Connection*)pConn,ADODB::adOpenKeyset,ADODB::adLockOptimistic,ADODB::adCmdText);
				n=pRec->RecordCount;
			}

			break;
		}
	}
		
	if( n>=1)
	{
		//DeleteList();//计算开始本来就是空链表，deleteall中已经删除了。

		while(!pRec->EndOfFile)
		{
			QixiangZhan * rainPoint=new QixiangZhan;			
			rainPoint->lCode = pRec->Fields->Item["id"]->Value;			
			rainPoint->jwLocation.jCoord = pRec->Fields->Item["Longitude"]->Value;			
			rainPoint->jwLocation.wCoord = pRec->Fields->Item["Latitude"]->Value;
			rainPoint->weight=0.0;
			m_TargetPoints.push_back(rainPoint);          //将满足条件的雨量站的编号，位置写入链表中
			pRec->MoveNext();
		}
		pRec->Close();
		bool ok=CalcuWt(jw);//计算气象站权重
		if (!ok) return false;
	}
	else
	{
		cout<<"警告:雨量站点太稀疏，精度太低"<<endl;
		return false;
	}
	return true;
}


//这个每算完一条河段应该被调用，否则链表空间越来越大，不断的被new。
//原来该函数没有被调用，这是不对的，因为monhappen类变量，如果不delete，他不断往后加，就不是从第一个开始了。
void input::Deleteall(void)
{
	//删除雨量站点位置链表，因为每算一条河段都不相同。
	if(m_TargetPoints.size()>0)
	{
		for(int n=0;n<m_TargetPoints.size();n++)
		{			
			delete (QixiangZhan*)(m_TargetPoints[n]);
			m_TargetPoints[n]=NULL;
		}		
	}
	m_TargetPoints.clear();

	//每算一条河段都不同（也是由站点信息间接推出来的)
	if(monhappen.size()>0)
	{
		for (int j=0; j<monhappen.size();j++)
		{
			delete (monvalue*)(monhappen[j]);
			monhappen[j] = NULL;
		}
	}
	monhappen.clear();

	
	//每算一条河段都不同
	for(int i=0; i<mySeriesArray.size(); i++)
	{
		if(mySeriesArray[i].size()>0)
		{
			for(int j=0; j<mySeriesArray[i].size(); j++)
			{
				delete mySeriesArray[i][j];
				mySeriesArray[i][j] = NULL;
			}
			mySeriesArray[i].clear();
		}

	}
	mySeriesArray.clear();

	//pCnn->Close();
	//delete [] allsca;
	//delete [] needsca;
	//delete [] dayhappen;
    //delete [] hourhappen;//ERROR,WHY?????
}


bool input::CalcuWt(jwCoord jw)//当一个河段固定后(通过jw)
{
	jw.jCoord = jx;
	jw.jCoord = wy;
	if(m_TargetPoints.size()==0)
	return false;
	PrjPoint_IUGG1975 PrjTrans;//sha
	bool ok=PrjTrans.GetL0(jw);
	if (!ok) return false;
    QixiangZhan * rainPoint;
	xyCoord  xy;
    ok=PrjTrans.BL2xy(jw,&xy);
    if (!ok) return false;
    m_CenterX=xy.xCoord;
	m_CenterY=xy.yCoord;
	vector<PointInfo> vecPoint(m_TargetPoints.size());
	for(int i=0;i<m_TargetPoints.size();i++)
	{
       rainPoint=m_TargetPoints[i];
	   jw.jCoord=rainPoint->jwLocation.jCoord;
	   jw.wCoord=rainPoint->jwLocation.wCoord;
	   ok=PrjTrans.BL2xy(jw,&xy);//sha??
	   if (!ok) return false;
	   rainPoint->xyLocation.xCoord=xy.xCoord;
	   rainPoint->xyLocation.yCoord=xy.yCoord;
	   vecPoint[i].dis=sqrt((xy.xCoord-m_CenterX)*(xy.xCoord-m_CenterX)+(xy.yCoord-m_CenterY)*(xy.yCoord-m_CenterY));
	}

	float sum=0;
	for(int j=0;j<m_TargetPoints.size();j++)
	{
		//sum+=((1-vecPoint[j].xiShu)/(vecPoint[j].dis*vecPoint[j].dis));
		sum+=(1/(vecPoint[j].dis*vecPoint[j].dis));
	}

	for(int i=0;i<m_TargetPoints.size();i++)    //求权重
	{
		//m_TargetPoints[i]->weight=(1-vecPoint[i].xiShu)/(sum*vecPoint[i].dis*vecPoint[i].dis);
		m_TargetPoints[i]->weight=1/(sum*vecPoint[i].dis*vecPoint[i].dis);//right???
	}	
	return true;//i个站点的权重m_TargetPoints[i]->weight
}

//把气象站的信息组织成一个链表，该函数里的所有元素的ID都相同，
//也就是存的都是同一个气象站的。m_Result链表中的每一个元素都是“读取的该气象站信息的地址”。
//该函数被GetDataByTS调用。
TGSeriesType input::GetGageTS(long ID)
{
	TGSeriesType m_Result;
	if(YearStart>YearEnd) return m_Result;
	if((YearStart==YearEnd) && (MonthStart>MonthEnd_1) ) return m_Result;
	if((YearStart==YearEnd) && (MonthStart==MonthEnd_1) && (DayStart>DayEnd) ) return m_Result;
	
	CComVariant tempVar;
	CString tempStr;
	_bstr_t bSQL;
	ADODB::_RecordsetPtr pRst;
	//short i;
	//float tempflt;
	CString timestart;
	CString timeend;
	
	/*timestart.Format("%d%s%d%s%d",YearStart,"-",MonthStart,"-",1);
	timeend.Format("%d%s%d%s%d",YearEnd,"-",MonthEnd_1,"-",1);//忙煞我也20080415*/
	timestart.Format(_T("%d%s%d%s%d"),1,"-",MonthStart,"-",YearStart);
	timeend.Format(_T("%d%s%d%s%d"),1,"-",MonthEnd_1,"-",YearEnd);//gao20090510

	
	pRst.CreateInstance(__uuidof(ADODB::Recordset));
	/*tempStr.Format("Select id,year,mon,Tmax,Tmin from T where id = %ld and time between #%s# and #%s#\
				   order by id,year,mon",ID,timestart,timeend);*/
	tempStr.Format(_T("Select id,year,month,Tmax,Tmin from T where id = %ld and time>=to_date('%s','dd-mm-yyyy')and time<to_date('%s','dd-mm-yyyy')\
				   order by id,year,month"),ID,timestart,timeend);//gao20080510

	bSQL=tempStr.GetString();
	pRst->Open(bSQL,(ADODB::_Connection*)pConn,ADODB::adOpenKeyset,ADODB::adLockOptimistic,ADODB::adCmdText);
	while(!pRst->EndOfFile)
	{
		mon_gaugevalue * mongaugeT = new mon_gaugevalue;
		mongaugeT->id = pRst->Fields->Item["id"]->Value;
		mongaugeT->monzhi.Year = pRst->Fields->Item["year"]->Value;
		mongaugeT->monzhi.Mon = pRst->Fields->Item["month"]->Value;
		mongaugeT->monzhi.Tmax = pRst->Fields->Item["Tmax"]->Value;
		mongaugeT->monzhi.Tmin = pRst->Fields->Item["Tmin"]->Value;
		
		m_Result.push_back(mongaugeT);//sha???why!!why!!!why!!!why!!!ok!typedef是创建新名字，而非创建对象!!!!	
		pRst->MoveNext();
	}
	pRst->Close();
	return m_Result;
}

//得到该河段，不同时间的不同温度。每一个元素代表一个单独的时间，但是所有元素都是指该河段的。
void input::GetDataByTS(void)
{
	//TSeriesType m_Result;
	TGSeriesType m_tempSeries;
	int i,j,NumOfGage;
	try
	{
		if(!GetDataByDW())
		{
			cout<<"GetDataByDW(jx,wy)错误！"<<endl;
			exit(0);
		}
	}
	catch (...) 
	{
		cout<<"Error While Reading T."<<endl;
		exit(0);
	}
	//float tempfltTmax,tempfltTmin;
	NumOfGage=m_TargetPoints.size();
	
	/*vector<TGSeriesType> mySeriesArray;*///放在里面函数执行后无法删除
	//mySeriesArray.clear();

	if(NumOfGage<1) 
	{
		cout<<"NumOfGage<1"<<endl;
	}
	for(i=0;i<NumOfGage;i++)
	{
		m_tempSeries=GetGageTS(m_TargetPoints[i]->lCode);//读T(rain)表数据
		
		//wh，mySeriesArray是所有的站的（在河段附近的站），各个元素的站号不同，每个元素都是一条链表（同一个站不同时间的温度）。
		mySeriesArray.push_back(m_tempSeries);	
	}
	numofmonhappen=((TGSeriesType)mySeriesArray[0]).size();
	for(i=0;i<numofmonhappen;i++)//第i时刻(houroffset)第j站点(j=3)
	{
		monvalue * monchazhi = new monvalue;
		
		monchazhi->Year = mySeriesArray[0][i]->monzhi.Year;//将TGSeries中的数据赋到TSeries中
		monchazhi->Mon = mySeriesArray[0][i]->monzhi.Mon;

		float Tmax=0;
		float Tmin=0;
		for(j=0;j<NumOfGage;j++)//NumOfGage河段附近的站点的个数
		{
			Tmax=Tmax+((TGSeriesType)mySeriesArray[j])[i]->monzhi.Tmax*m_TargetPoints[j]->weight;
			Tmin=Tmin+((TGSeriesType)mySeriesArray[j])[i]->monzhi.Tmin*m_TargetPoints[j]->weight;
			
		}		
		monchazhi->Tmax = Tmax;
		monchazhi->Tmin = Tmin;
		monhappen.push_back(monchazhi);
	}	
	//return monhappen;//是一个NumOfTimeStep个元素的结构体数组，是该JW下的year,mon,Tmax,Tmin(月数据)
}


///**********已经给月数据赋值了，接下来要给日数据赋值，最后给时数据赋值!!!**********///


//wh:供参考：每次从sonw_sca表中读取的信息都是一样的，为什么每条河段都要读呢？只读一次，allsca内容不删除不就可以了么？
//void input::ReadSCA(void)
//{
	//ADODB::_RecordsetPtr pRst;
	//pRst.CreateInstance(__uuidof(ADODB::Recordset));
    //CString cSQL;
	//_bstr_t SQL;
	//
	//try
	//{
	//	cSQL.Format(_T("select * from snow_sca order by month,day"));
	//	SQL = cSQL.GetString();
	//	pRst->Open(SQL,(ADODB::_Connection*)pConn,ADODB::adOpenStatic,ADODB::adLockOptimistic,ADODB::adCmdText);
	//	Rcount = pRst->RecordCount;//SCA记录数目	
	//}
	//catch(...)
	//{
	//	cout<<"error while reading CSA:<"<<endl;
	//	exit(0);
	//}
	//allsca = new SCA[Rcount];//wh,modified，allsca是类变量，最后不用返回。

	//needsca = new SCAcertain[Rcount];//wh，放到此。只需要new一次，然后每个值会被不断覆盖，原来不断的new，会溢出的。

	//pRst->MoveFirst();
    //long i=0;
	//while(!pRst->EndOfFile)
	//{
	//	//SCA* allsc = new SCA;
	//	allsca[i].Mon = pRst->Fields->Item["MONTH"]->Value;
	//	allsca[i].Day = pRst->Fields->Item["DAY"]->Value;
	//	allsca[i].SCA1 = pRst->Fields->Item["SCA1"]->Value;
	//	allsca[i].SCA2 = pRst->Fields->Item["SCA2"]->Value;
	//	allsca[i].SCA3 = pRst->Fields->Item["SCA3"]->Value;
	//	allsca[i].SCA4 = pRst->Fields->Item["SCA4"]->Value;
	//	pRst->MoveNext();
	//	i++;		
	//}
	//pRst->Close();
	//return allsca;//不需要return，allsca是类变量，函数里已经将其改变了，不用再return了。
//}


void input::ReadElevationAndA(void)//连接Oracle
{
	//这一小段只是为了知道needsca能new多少
	ADODB::_RecordsetPtr pRst;
	pRst.CreateInstance(__uuidof(ADODB::Recordset));
    CString cSQL;
	_bstr_t SQL;
	
	try
	{
		cSQL.Format(_T("select * from snow_sca order by month,day"));
		SQL = cSQL.GetString();
		pRst->Open(SQL,(ADODB::_Connection*)pConn,ADODB::adOpenStatic,ADODB::adLockOptimistic,ADODB::adCmdText);
		
		Rcount = pRst->RecordCount;//SCA记录数目
		needsca = new SCAcertain[Rcount];//wh，放到此。只需要new一次，然后每个值会被不断覆盖，原来不断的new，会溢出的。

		long i=0;
		while(!pRst->EndOfFile)
		{
			needsca[i].Mon = pRst->Fields->Item["MONTH"]->Value;
			needsca[i].Day = pRst->Fields->Item["DAY"]->Value;
			if(Havg>=1500 && Havg<2000) { needsca[i].SCA = pRst->Fields->Item["SCA1"]->Value;}
			if(Havg>=2000 && Havg<3000) { needsca[i].SCA = pRst->Fields->Item["SCA2"]->Value;}
			if(Havg>=3000 && Havg<4000) { needsca[i].SCA = pRst->Fields->Item["SCA3"]->Value;}
			if(Havg>=4000)              { needsca[i].SCA = pRst->Fields->Item["SCA4"]->Value;}
			pRst->MoveNext();
			i++;		
		}

		pRst->Close();
	}
	catch(...)
	{
		cout<<"error while reading from table snow_sca:<"<<endl;
		exit(0);
	}

	//wh,A和平均高程Havg我给你从产流模型传入，就不用在这每次都要读了，很费时的。
	/******************************************************************************/
	//SCAcertain* needsca;//和类变量重复，这是绝对不允许的。
	//float Hmax,Hmin,Havg;
	//ADODB::_RecordsetPtr pRst;
	//pRst.CreateInstance(__uuidof(ADODB::Recordset));
	//CString cSQL;
	//_bstr_t SQL;
	//cSQL.Format(_T("select UELEVATION,DELEVATION,A from riversegs where regionindex=%I64u and bslength=%ld and bsvalue=%I64u"),RegionIndex,Length,Value);
	//SQL = cSQL.GetString();
	//pRst->Open(SQL,(ADODB::_Connection*)pConn,ADODB::adOpenStatic,ADODB::adLockOptimistic,ADODB::adCmdText);
	//Hmax = pRst->Fields->Item["UELEVATION"]->Value;
	//Hmin = pRst->Fields->Item["DELEVATION"]->Value;
	//Havg = (Hmax+Hmin)/2;
	//float a;
	//a = pRst->Fields->Item["A"]->Value;
	//A = a;
	
	//modifed by wanghao,已经放到SRM.cpp中的OpenOracle函数中
	//allsca = ReadSCA();//key!!!!
	//needsca = new SCAcertain[Rcount];//modifed by wanghao
    //Rcount = sizeof(allsca);
	/******************************************************************************/


	/*if(Havg>=1500 && Havg<2000)
	{
		for(int i=0;i<Rcount;i++)
		{
			needsca[i].Mon = allsca[i].Mon;
			needsca[i].Day = allsca[i].Day;
			needsca[i].SCA = allsca[i].SCA1;			
		}
	}
	if(Havg>=2000 && Havg<3000)
	{
		for(int i=0;i<Rcount;i++)
		{
			needsca[i].Mon = allsca[i].Mon;
			needsca[i].Day = allsca[i].Day;
			needsca[i].SCA = allsca[i].SCA2;
		}
	}
	if(Havg>=3000 && Havg<4000)
	{
		for(int i=0;i<Rcount;i++)
		{
			needsca[i].Mon = allsca[i].Mon;
			needsca[i].Day = allsca[i].Day;
			needsca[i].SCA = allsca[i].SCA3;
		}
	}
	if(Havg>=4000)
	{
		for(int i=0;i<Rcount;i++)
		{
			needsca[i].Mon = allsca[i].Mon;
			needsca[i].Day = allsca[i].Day;
			needsca[i].SCA = allsca[i].SCA4;
		}
	}
	pRst->Close();*/

	//return needsca;
}


//计算起至时间内的天数
void input::GetNumofDay(void)
{
	//long numofday;
	if(YearStart>YearEnd) numofday= -1;
	if((YearStart==YearEnd) && (MonthStart>MonthEnd_1) ) numofday= -1;
		
	int CurYear,CurMonth,NextYear,NextMonth;
	long sum = 0;
	CurYear=YearStart;
	CurMonth=MonthStart;
	sum=sum+GetMonthDays(CurYear,CurMonth);
	MonthAdd(CurYear,CurMonth,NextYear,NextMonth);
	while(!((NextYear==YearEnd) && (NextMonth==MonthEnd_1)))
	{
		sum=sum+GetMonthDays(NextYear,NextMonth);
		CurYear=NextYear;
		CurMonth=NextMonth;
		MonthAdd(CurYear,CurMonth,NextYear,NextMonth);
	}
	numofday=sum;
	//return numofday;
}

//1连接mdb，2连接oracle
void input::ReadDayValue(void)
{
	//需要用到 needcsa,monhappen
	//int numofsca = sizeof(needsca);
	//int numofmonhappen = sizeof(monhappen);
	int CurYear,CurMonth,NextYear,NextMonth;
	NextYear=0;
	NextMonth=0;
	//int monthdays;
	CurYear = YearStart;
	CurMonth = MonthStart;
	//numofday = GetNumofDay();//wh
	
	//计算起止时间在在并行计算全局就定了的，所以一次new就行了，否则这么多new，最后删不好，内存会溢出的。
	//这也是我为什么跟你确定时间是否变化的原因。
	//dayhappen = new dayvalue[numofday];//wh

    long j = 0;//计数numofday，dayhappen数组，赋值：年，月，日!!!
	
	//wh:
	//这个while循环有漏洞，这也是你为什么不能运行的原因，首先最外侧的while循环是没有意义的，
	//因为里面的循环既然能跳出来，那一定是因为j已经等于numofday了，已经符合情况了。但现在
	//按照这个写法，会出现j>numofday的情况，比如计算的终止时间是2008年10月5号，那么while(k<= monthdays)会一直循环，
	//此时j也被迫++到10月份的最后一天，所以数组越界了。
	//while(!((NextYear==YearEnd)&&(NextMonth==MonthEnd_1)))
	//{		
	
	while(j<numofday)//numofday也是dayhappen数组的元素个数
	{
		int monthdays = GetMonthDays(CurYear,CurMonth);
		int k = 1;
		while(k <= monthdays)
		{
			dayhappen[j].Year = CurYear;
			dayhappen[j].Mon = CurMonth;
			dayhappen[j].Day = k;
			k++;
			j++;
			if(j==numofday){ break;}//wh,添加。	
		}

		if(j==numofday){break;}//wh,添加。保证j==numofday时下面三行不执行。
		MonthAdd(CurYear,CurMonth,NextYear,NextMonth);
		CurYear = NextYear;
		CurMonth = NextMonth;
	}

	//}


	//日数据赋值：根据年月日，得到：SCA,Tmax，Tmin

	//ofstream out_day("D:\\c++\\dayhappen.txt",ios::app);
	for(long i=0;i<numofday;i++)
	{
		for(int numsca=0;numsca<Rcount;numsca++)
		{
			//实际上采用的是“最先发现法”，只要时间一致就行，不管站点离流域多远。
			//wh：如果时间没有完全对应上的怎么办呢？程序会不会把sca随便赋一个值呢？
			if((dayhappen[i].Mon == needsca[numsca].Mon)&&(dayhappen[i].Day == needsca[numsca].Day))
			{
				dayhappen[i].SCA = needsca[numsca].SCA;

				break;//wh,其实融雪加入以后，速度降低很多，因为里面循环太多，而且没及时break掉。
			}
		}
		for(int numT=0;numT<numofmonhappen;numT++)
		{
			if((dayhappen[i].Year==monhappen[numT]->Year)&&(dayhappen[i].Mon==monhappen[numT]->Mon))
			{
				dayhappen[i].Tmax = monhappen[numT]->Tmax;
				dayhappen[i].Tmin = monhappen[numT]->Tmin;
				break;//wh
			}
		}
		//cout<<setw(10)<<dayhappen[i].Year<<setw(10)<<dayhappen[i].Mon<<setw(10)<<dayhappen[i].Day<<setw(10)<<setw(10)<<dayhappen[i].SCA<<setw(10)<<dayhappen[i].Tmax<<endl;
	
		/*out_day<<setw(10)<<RegionIndex<<setw(10)<<Length<<setw(10)<<Value<<setw(10)<<dayhappen[i].Year<<setw(10)<<dayhappen[i].Mon<<setw(10)<<dayhappen[i].Day\
			<<setw(10)<<dayhappen[i].SCA<<setw(20)<<dayhappen[i].Tmax<<setw(20)<<dayhappen[i].Tmin<<endl;*/
	}
	//out_day.close();//ok!
	//return dayhappen;
}

void input::ReadHourValue(void)
{
	//需要用到dayhappen
    //long numofhour = numofday*24;//wh
	//long houroffsetstart;//wh
	//houroffsetstart = date2houroffset(YearStart,MonthStart,1,0);//wh

	float T;
	float HourInDay;

	int CurYear,CurMonth,NextYear,NextMonth;
	NextYear = 0;//初始化
	NextMonth = 0;
	//int monthdays;
	CurYear = YearStart;
	CurMonth = MonthStart;
	//hourhappen = new hourvalue[numofhour];//wh
	//这句本身就是不对的，hourhappen已经在类中有定义了，这里不能用同样的名字，否则下面会指针混乱，不知道你引用的哪个。

    long j = 0;//计数numofday，dayhappen数组，赋值：年，月，日!!!
	//里层循环跳出一定因为j==numofhour了，所以最外层的while是没用的，还可能造成MonthEnd_1加1后的麻烦。
	//while(!((NextYear==YearEnd)&&(NextMonth==MonthEnd_1)))
	//{		

	//按照原来的写法，同样存在数组越界的情况。其实不用这么麻烦，一天固定的24小时，hourhappen肯定
	//数组个数是dayhappen的24倍，直接从0到23展开就行了，不同MonthAdd等函数。
	houroffsetstart--;//wh
	while(j<numofhour)
	{
		int monthdays = GetMonthDays(CurYear,CurMonth);
		int k = 1;
		while(k <= monthdays)
		{
			//以下三行和下面的重复赋值了。
			//hourhappen[j].Year = CurYear;
			//hourhappen[j].Mon = CurMonth;
			//hourhappen[j].Day = k;
			int kk = 0;
			while(kk<24)
			{
				hourhappen[j].Year = CurYear;
				hourhappen[j].Mon = CurMonth;
				hourhappen[j].Day = k;
				hourhappen[j].Hour = kk;
				hourhappen[j].houroffset ++;//wh

				kk++;
				j++;//wh,这里的问题跟上面一样，即使j大于numofhour了，他现在从里侧循环也出不去。我把位置也跟你换了，你对照下。
				if(j==numofhour){ break;}//wh
			}
			if(j==numofhour){break;}//wh
			k++;
		}
		if(j==numofhour){break;}//wh
		MonthAdd(CurYear,CurMonth,NextYear,NextMonth);
		CurYear = NextYear;
		CurMonth = NextMonth;
		//cout<<setw(10)<<hourhappen[j].Year<<setw(10)<<hourhappen[j].Mon<<setw(10)<<hourhappen[j].Day<<setw(10)<<hourhappen[j].Hour<<endl;
	}	

	//}//年，月，1日，1时，赋值完毕!!!!

	for(long i=0;i<numofhour;i++)
	{
		//hourhappen[i].houroffset = date2houroffset(hourhappen[i].Year,hourhappen[i].Mon,hourhappen[i].Day,hourhappen[i].Hour);//wh，放到上面去了

		//HourInDay = (int(hourhappen[i].houroffset))%24 + hourhappen[i].houroffset - int(hourhappen[i].houroffset);//wh,因为你的houroffset不存在分钟的情况，所以你看下，后两项本来就是相等的，都是整小时。
		HourInDay = (hourhappen[i].houroffset) % 24;//和俄上个式子等价。

		hourhappen[i].P = 0.0f;
		for(long numday=0;numday<numofday;numday++)
		{
			if((hourhappen[i].Year==dayhappen[numday].Year)&&(hourhappen[i].Mon==dayhappen[numday].Mon)&&(hourhappen[i].Day==dayhappen[numday].Day))
			{
				hourhappen[i].SCA = dayhappen[numday].SCA;
				hourhappen[i].Tmax = dayhappen[numday].Tmax;
				hourhappen[i].Tmin = dayhappen[numday].Tmin;
				break;//wh
			}

		}
		T = -((hourhappen[i].Tmax-hourhappen[i].Tmin)/2)*cos(HourInDay*PI/12)+(hourhappen[i].Tmax+hourhappen[i].Tmin)/2;
		hourhappen[i].Thour = T;		

 	}
	/*ofstream out_hour("d:\\c++\\hourhappen.txt",ios::app);
	for (long ii=0;ii<numofhour;ii++)
	{
		out_hour<<setw(10)<<RegionIndex<<setw(10)<<Length<<setw(10)<<Value<<setw(10)<<hourhappen[ii].houroffset<<setw(10)<<hourhappen[ii].Year<<setw(10)<<hourhappen[ii].Mon<<setw(10)<<hourhappen[ii].Day<<setw(10)<<hourhappen[ii].Hour\
			<<setw(10)<<hourhappen[ii].SCA<<setw(10)<<hourhappen[ii].P<<setw(20)<<hourhappen[ii].Tmax<<setw(20)<<hourhappen[ii].Tmin<<setw(20)<<hourhappen[ii].Thour<<endl;
	}
	out_hour.close();//ok!*/
	//return hourhappen;
}

//added by wanghao，为了实现真正的组件化，让融雪和产流割裂开。
void input::ToWaterYield(long houroffset,float* HourRain)
{
	float a,T,S,P,Pliquid,Psml,Psnowmelt,W0=0.0f;

	for(long j=0;j<numofday*24;j++)
	{
		if(hourhappen[j].houroffset==houroffset)
		{
			a = A;
			T = hourhappen[j].Thour;
			S = hourhappen[j].SCA;
			hourhappen[j].P = (*HourRain);//wanghao,传入的是降雨地址，这样融雪对降雨的改变，降雨就会感知并放生变化
			P = hourhappen[j].P;

			if(T<0)
			{
				Pliquid = 0.0f;
				W0 = W0 + P;
			}
			if(T>=0)
			{
				Pliquid = P*(1-S);
				W0 = W0 + P*S;
			}

			if(T<0)//不融雪!!!!
			{
				Psnowmelt = 0.0f;
			}
			else
			{
				if(W0>=a*T*(1-S))
				{
					Psnowmelt = a*T*(1-S);										
				}
				else if((W0>=0)&&(W0<a*T*(1-S)))
				{
					Psnowmelt = W0;						
				}
				else
				{
					Psnowmelt = 0.0f;						
				}

				W0 = W0 - Psnowmelt;
			}
			Psml =  Pliquid + Psnowmelt;
			(*HourRain) = Psml;//wanghao
			break;//added by wanghao

			/*out_Prunoff<<setw(10)<<mBSCode.RegionIndex<<setw(10)<<mBSCode.Length<<setw(10)<<mBSCode.Value<<setw(10)<<hour<<setw(15)<<HourRain_1<<setw(10)<<hourhappen[j].Thour\
			<<setw(10)<<hourhappen[j].SCA<<setw(10)<<A<<setw(15)<<W0<<setw(15)<<HourRain<<endl;*/

		}//end if
	}//end for

}



/*ADODB::_Connection* input::OpenMDB(CString m_File)
{
	
	_bstr_t bSQL;
	bSQL="Provider=Microsoft.Jet.OLEDB.4.0;Data Source="+m_File+";Persist Security Info=False";
	try
	{
		pCnn.CreateInstance(__uuidof(ADODB::Connection));
		pCnn->CursorLocation=ADODB::adUseClient;
		pCnn->Open(bSQL,"Admin","",-1);
	}
	catch(_com_error &e)
	{
		cout<<"mdb open error!!"<<endl;  //打开数据库出错，返回错误代码。
		exit(0);
	}
	return pCnn;
}*/


//ADODB::_Connection* input::OpenOracle(ATL::CString m_user, ATL::CString m_password, ATL::CString m_sid)
//{
//	_bstr_t bSQL2;
//	CString SQL;
//	SQL.Format(_T("Provider=OraOLEDB.Oracle.1;Persist Security Info=False;User ID=%s;Data Source=%s;Extended Properties=''"),m_user,m_sid);
//	bSQL2=SQL.GetString();
//	try
//	{
//		pConn.CreateInstance(__uuidof(ADODB::Connection));//创建对象
//		pConn->Open(bSQL2,m_user.GetString(),m_password.GetString(),0);
//	}
//	catch(...)
//	{
//		cout<<"oracle open error!!"<<endl;
//		exit(0);
//	}
//	return pConn;
//}
