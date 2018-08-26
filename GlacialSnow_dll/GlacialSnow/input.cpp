#include "stdafx.h"
#include "input.h"
#include <cmath>

//�ú���Ϊȫ�ֺ������������м����ڼ�ֻ����һ�Ρ�
void input::initialzie()
{
	//modified by wanghao���ŵ��˺�������Ա�֤�ڲ��м����ʼĩ��ֻ����һ�Ρ�
	MonthEnd_1 = MonthEnd + 1;
	ReadElevationAndA();//��ʱallscaʼ�ն������ݿ���snow_sca������ֵ������Ҫÿ�ζ���һ�顣
	this->GetNumofDay();//wh��ȫ�ֲ���ģ��ŵ����
	numofhour = numofday*24;
	dayhappen = new dayvalue[numofday];//wh
	houroffsetstart = date2houroffset(YearStart,MonthStart,1,0);
	hourhappen = new hourvalue[numofhour];
}

//�ú���Ϊȫ�ֺ������������м����ڼ�ֻ����һ�Ρ�
void input::finalize()
{
	pConn->Close();//ȫ�ִ򿪣���Ȼȫ�ֹر�
	delete[] dayhappen;
	delete[] hourhappen;
	delete[] needsca;
}


//wh,Ŀ���ǰ�houroffsetת��Ϊ�ꡢ�¡��գ����Ǻ��������⣬�Ѿ�ע����
void input::houroffset2date(long houroffset)
{
	int YearS,MonthS,DayS;
	long DayCount,temp;

	int m_YearStart;
	int m_MonthStart;
	int m_DayStart;

	//ͨ��StartHour�����ʼ�����գ���ʼ��
	YearS=YearSTD;      //��׼��
	MonthS=MonthSTD;	//��׼��
	//ֱ���ã�DayStart = DaySTD;//��׼��

	DayCount=long(houroffset/24); //ת��СʱΪ��
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
	//wh,����������û�õģ�m_YearStart��m_MonthStart��m_DayStart�Ǿֲ��������ڴ˺����������ģ�����ִ����ϣ���Щ
	//����ͳͳ��ʧ�ˣ��Ǵ˺�������ʲô�õģ�
	m_YearStart=YearS;
	m_MonthStart=MonthS;
	m_DayStart=DayS;
}

//��������ת��Ϊhouroffset���������Ӧ����û������ģ����������á�
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

//���һ���µ�������
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

//���ܣ���������º��ټ�һ���£�����·ֱ��Ϊ���١�
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

//�õ�����վ������ÿ��ֻ��������վ�������Ϣ��
bool input::GetDataByDW(void)    //����Ȩ�ط�
{
	//xyPerjw Dxy;
	jwCoord jw;
	jw.jCoord=jx;
	jw.wCoord =wy;
	
	ADODB::_RecordsetPtr  pRec;
	pRec.CreateInstance(__uuidof(ADODB::Recordset));

	int n=1,t=0;
	float xMin,xMax,yMin,yMax;   //������ΧΪһ����γ�ȵ�������
	CString SelSQL;
	_bstr_t SQL;
	while(n<3)    //������3��վ��
	{		
		xMin=jx-0.05*t;
		xMax=jx+0.05*t;
		yMin=wy-0.05*t;
		yMax=wy+0.05*t;       //�����ķ�Χ����Ϊ��γ����0.01
		SelSQL.Format(_T("select * from gauge where longitude Between %.2f and %.2f and latitude Between %.2f and %.2f"),xMin,xMax,yMin,yMax);
		SQL=SelSQL.GetString();
	    //������������Ҫ�ӣ�
		pRec->Open(SQL,(ADODB::_Connection*)pConn,ADODB::adOpenKeyset,ADODB::adLockOptimistic,ADODB::adCmdText);//!!!!����
		n=pRec->RecordCount;
		if(n<3)
		{
			pRec->Close();
			t+=1;   //��������
		}
		if(t>220)
		{
			//���һ����û�ҳ�������ȫ����ֵ
			if(!(n>=1))
			{
				xMin=-180;
				xMax=180;
				yMin=-90;
				yMax=90;

				//ע�⣬������䲻��ֱ��д���֣�Ҫ��float����
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
		//DeleteList();//���㿪ʼ�������ǿ�����deleteall���Ѿ�ɾ���ˡ�

		while(!pRec->EndOfFile)
		{
			QixiangZhan * rainPoint=new QixiangZhan;			
			rainPoint->lCode = pRec->Fields->Item["id"]->Value;			
			rainPoint->jwLocation.jCoord = pRec->Fields->Item["Longitude"]->Value;			
			rainPoint->jwLocation.wCoord = pRec->Fields->Item["Latitude"]->Value;
			rainPoint->weight=0.0;
			m_TargetPoints.push_back(rainPoint);          //����������������վ�ı�ţ�λ��д��������
			pRec->MoveNext();
		}
		pRec->Close();
		bool ok=CalcuWt(jw);//��������վȨ��
		if (!ok) return false;
	}
	else
	{
		cout<<"����:����վ��̫ϡ�裬����̫��"<<endl;
		return false;
	}
	return true;
}


//���ÿ����һ���Ӷ�Ӧ�ñ����ã���������ռ�Խ��Խ�󣬲��ϵı�new��
//ԭ���ú���û�б����ã����ǲ��Եģ���Ϊmonhappen������������delete������������ӣ��Ͳ��Ǵӵ�һ����ʼ�ˡ�
void input::Deleteall(void)
{
	//ɾ������վ��λ��������Ϊÿ��һ���Ӷζ�����ͬ��
	if(m_TargetPoints.size()>0)
	{
		for(int n=0;n<m_TargetPoints.size();n++)
		{			
			delete (QixiangZhan*)(m_TargetPoints[n]);
			m_TargetPoints[n]=NULL;
		}		
	}
	m_TargetPoints.clear();

	//ÿ��һ���Ӷζ���ͬ��Ҳ����վ����Ϣ����Ƴ�����)
	if(monhappen.size()>0)
	{
		for (int j=0; j<monhappen.size();j++)
		{
			delete (monvalue*)(monhappen[j]);
			monhappen[j] = NULL;
		}
	}
	monhappen.clear();

	
	//ÿ��һ���Ӷζ���ͬ
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


bool input::CalcuWt(jwCoord jw)//��һ���Ӷι̶���(ͨ��jw)
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

	for(int i=0;i<m_TargetPoints.size();i++)    //��Ȩ��
	{
		//m_TargetPoints[i]->weight=(1-vecPoint[i].xiShu)/(sum*vecPoint[i].dis*vecPoint[i].dis);
		m_TargetPoints[i]->weight=1/(sum*vecPoint[i].dis*vecPoint[i].dis);//right???
	}	
	return true;//i��վ���Ȩ��m_TargetPoints[i]->weight
}

//������վ����Ϣ��֯��һ�������ú����������Ԫ�ص�ID����ͬ��
//Ҳ���Ǵ�Ķ���ͬһ������վ�ġ�m_Result�����е�ÿһ��Ԫ�ض��ǡ���ȡ�ĸ�����վ��Ϣ�ĵ�ַ����
//�ú�����GetDataByTS���á�
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
	timeend.Format("%d%s%d%s%d",YearEnd,"-",MonthEnd_1,"-",1);//æɷ��Ҳ20080415*/
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
		
		m_Result.push_back(mongaugeT);//sha???why!!why!!!why!!!why!!!ok!typedef�Ǵ��������֣����Ǵ�������!!!!	
		pRst->MoveNext();
	}
	pRst->Close();
	return m_Result;
}

//�õ��úӶΣ���ͬʱ��Ĳ�ͬ�¶ȡ�ÿһ��Ԫ�ش���һ��������ʱ�䣬��������Ԫ�ض���ָ�úӶεġ�
void input::GetDataByTS(void)
{
	//TSeriesType m_Result;
	TGSeriesType m_tempSeries;
	int i,j,NumOfGage;
	try
	{
		if(!GetDataByDW())
		{
			cout<<"GetDataByDW(jx,wy)����"<<endl;
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
	
	/*vector<TGSeriesType> mySeriesArray;*///�������溯��ִ�к��޷�ɾ��
	//mySeriesArray.clear();

	if(NumOfGage<1) 
	{
		cout<<"NumOfGage<1"<<endl;
	}
	for(i=0;i<NumOfGage;i++)
	{
		m_tempSeries=GetGageTS(m_TargetPoints[i]->lCode);//��T(rain)������
		
		//wh��mySeriesArray�����е�վ�ģ��ںӶθ�����վ��������Ԫ�ص�վ�Ų�ͬ��ÿ��Ԫ�ض���һ������ͬһ��վ��ͬʱ����¶ȣ���
		mySeriesArray.push_back(m_tempSeries);	
	}
	numofmonhappen=((TGSeriesType)mySeriesArray[0]).size();
	for(i=0;i<numofmonhappen;i++)//��iʱ��(houroffset)��jվ��(j=3)
	{
		monvalue * monchazhi = new monvalue;
		
		monchazhi->Year = mySeriesArray[0][i]->monzhi.Year;//��TGSeries�е����ݸ���TSeries��
		monchazhi->Mon = mySeriesArray[0][i]->monzhi.Mon;

		float Tmax=0;
		float Tmin=0;
		for(j=0;j<NumOfGage;j++)//NumOfGage�Ӷθ�����վ��ĸ���
		{
			Tmax=Tmax+((TGSeriesType)mySeriesArray[j])[i]->monzhi.Tmax*m_TargetPoints[j]->weight;
			Tmin=Tmin+((TGSeriesType)mySeriesArray[j])[i]->monzhi.Tmin*m_TargetPoints[j]->weight;
			
		}		
		monchazhi->Tmax = Tmax;
		monchazhi->Tmin = Tmin;
		monhappen.push_back(monchazhi);
	}	
	//return monhappen;//��һ��NumOfTimeStep��Ԫ�صĽṹ�����飬�Ǹ�JW�µ�year,mon,Tmax,Tmin(������)
}


///**********�Ѿ��������ݸ�ֵ�ˣ�������Ҫ�������ݸ�ֵ������ʱ���ݸ�ֵ!!!**********///


//wh:���ο���ÿ�δ�sonw_sca���ж�ȡ����Ϣ����һ���ģ�Ϊʲôÿ���Ӷζ�Ҫ���أ�ֻ��һ�Σ�allsca���ݲ�ɾ�����Ϳ�����ô��
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
	//	Rcount = pRst->RecordCount;//SCA��¼��Ŀ	
	//}
	//catch(...)
	//{
	//	cout<<"error while reading CSA:<"<<endl;
	//	exit(0);
	//}
	//allsca = new SCA[Rcount];//wh,modified��allsca�������������÷��ء�

	//needsca = new SCAcertain[Rcount];//wh���ŵ��ˡ�ֻ��Ҫnewһ�Σ�Ȼ��ÿ��ֵ�ᱻ���ϸ��ǣ�ԭ�����ϵ�new��������ġ�

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
	//return allsca;//����Ҫreturn��allsca����������������Ѿ�����ı��ˣ�������return�ˡ�
//}


void input::ReadElevationAndA(void)//����Oracle
{
	//��һС��ֻ��Ϊ��֪��needsca��new����
	ADODB::_RecordsetPtr pRst;
	pRst.CreateInstance(__uuidof(ADODB::Recordset));
    CString cSQL;
	_bstr_t SQL;
	
	try
	{
		cSQL.Format(_T("select * from snow_sca order by month,day"));
		SQL = cSQL.GetString();
		pRst->Open(SQL,(ADODB::_Connection*)pConn,ADODB::adOpenStatic,ADODB::adLockOptimistic,ADODB::adCmdText);
		
		Rcount = pRst->RecordCount;//SCA��¼��Ŀ
		needsca = new SCAcertain[Rcount];//wh���ŵ��ˡ�ֻ��Ҫnewһ�Σ�Ȼ��ÿ��ֵ�ᱻ���ϸ��ǣ�ԭ�����ϵ�new��������ġ�

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

	//wh,A��ƽ���߳�Havg�Ҹ���Ӳ���ģ�ʹ��룬�Ͳ�������ÿ�ζ�Ҫ���ˣ��ܷ�ʱ�ġ�
	/******************************************************************************/
	//SCAcertain* needsca;//��������ظ������Ǿ��Բ�����ġ�
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
	
	//modifed by wanghao,�Ѿ��ŵ�SRM.cpp�е�OpenOracle������
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


//��������ʱ���ڵ�����
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

//1����mdb��2����oracle
void input::ReadDayValue(void)
{
	//��Ҫ�õ� needcsa,monhappen
	//int numofsca = sizeof(needsca);
	//int numofmonhappen = sizeof(monhappen);
	int CurYear,CurMonth,NextYear,NextMonth;
	NextYear=0;
	NextMonth=0;
	//int monthdays;
	CurYear = YearStart;
	CurMonth = MonthStart;
	//numofday = GetNumofDay();//wh
	
	//������ֹʱ�����ڲ��м���ȫ�־Ͷ��˵ģ�����һ��new�����ˣ�������ô��new�����ɾ���ã��ڴ������ġ�
	//��Ҳ����Ϊʲô����ȷ��ʱ���Ƿ�仯��ԭ��
	//dayhappen = new dayvalue[numofday];//wh

    long j = 0;//����numofday��dayhappen���飬��ֵ���꣬�£���!!!
	
	//wh:
	//���whileѭ����©������Ҳ����Ϊʲô�������е�ԭ������������whileѭ����û������ģ�
	//��Ϊ�����ѭ����Ȼ������������һ������Ϊj�Ѿ�����numofday�ˣ��Ѿ���������ˡ�������
	//�������д���������j>numofday�����������������ֹʱ����2008��10��5�ţ���ôwhile(k<= monthdays)��һֱѭ����
	//��ʱjҲ����++��10�·ݵ����һ�죬��������Խ���ˡ�
	//while(!((NextYear==YearEnd)&&(NextMonth==MonthEnd_1)))
	//{		
	
	while(j<numofday)//numofdayҲ��dayhappen�����Ԫ�ظ���
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
			if(j==numofday){ break;}//wh,��ӡ�	
		}

		if(j==numofday){break;}//wh,��ӡ���֤j==numofdayʱ�������в�ִ�С�
		MonthAdd(CurYear,CurMonth,NextYear,NextMonth);
		CurYear = NextYear;
		CurMonth = NextMonth;
	}

	//}


	//�����ݸ�ֵ�����������գ��õ���SCA,Tmax��Tmin

	//ofstream out_day("D:\\c++\\dayhappen.txt",ios::app);
	for(long i=0;i<numofday;i++)
	{
		for(int numsca=0;numsca<Rcount;numsca++)
		{
			//ʵ���ϲ��õ��ǡ����ȷ��ַ�����ֻҪʱ��һ�¾��У�����վ���������Զ��
			//wh�����ʱ��û����ȫ��Ӧ�ϵ���ô���أ�����᲻���sca��㸳һ��ֵ�أ�
			if((dayhappen[i].Mon == needsca[numsca].Mon)&&(dayhappen[i].Day == needsca[numsca].Day))
			{
				dayhappen[i].SCA = needsca[numsca].SCA;

				break;//wh,��ʵ��ѩ�����Ժ��ٶȽ��ͺܶ࣬��Ϊ����ѭ��̫�࣬����û��ʱbreak����
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
	//��Ҫ�õ�dayhappen
    //long numofhour = numofday*24;//wh
	//long houroffsetstart;//wh
	//houroffsetstart = date2houroffset(YearStart,MonthStart,1,0);//wh

	float T;
	float HourInDay;

	int CurYear,CurMonth,NextYear,NextMonth;
	NextYear = 0;//��ʼ��
	NextMonth = 0;
	//int monthdays;
	CurYear = YearStart;
	CurMonth = MonthStart;
	//hourhappen = new hourvalue[numofhour];//wh
	//��䱾����ǲ��Եģ�hourhappen�Ѿ��������ж����ˣ����ﲻ����ͬ�������֣����������ָ����ң���֪�������õ��ĸ���

    long j = 0;//����numofday��dayhappen���飬��ֵ���꣬�£���!!!
	//���ѭ������һ����Ϊj==numofhour�ˣ�����������while��û�õģ����������MonthEnd_1��1����鷳��
	//while(!((NextYear==YearEnd)&&(NextMonth==MonthEnd_1)))
	//{		

	//����ԭ����д����ͬ����������Խ����������ʵ������ô�鷳��һ��̶���24Сʱ��hourhappen�϶�
	//���������dayhappen��24����ֱ�Ӵ�0��23չ�������ˣ���ͬMonthAdd�Ⱥ�����
	houroffsetstart--;//wh
	while(j<numofhour)
	{
		int monthdays = GetMonthDays(CurYear,CurMonth);
		int k = 1;
		while(k <= monthdays)
		{
			//�������к�������ظ���ֵ�ˡ�
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
				j++;//wh,��������������һ������ʹj����numofhour�ˣ������ڴ����ѭ��Ҳ����ȥ���Ұ�λ��Ҳ���㻻�ˣ�������¡�
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

	//}//�꣬�£�1�գ�1ʱ����ֵ���!!!!

	for(long i=0;i<numofhour;i++)
	{
		//hourhappen[i].houroffset = date2houroffset(hourhappen[i].Year,hourhappen[i].Mon,hourhappen[i].Day,hourhappen[i].Hour);//wh���ŵ�����ȥ��

		//HourInDay = (int(hourhappen[i].houroffset))%24 + hourhappen[i].houroffset - int(hourhappen[i].houroffset);//wh,��Ϊ���houroffset�����ڷ��ӵ�����������㿴�£��������������ȵģ�������Сʱ��
		HourInDay = (hourhappen[i].houroffset) % 24;//�Ͷ��ϸ�ʽ�ӵȼۡ�

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

//added by wanghao��Ϊ��ʵ�������������������ѩ�Ͳ������ѿ���
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
			hourhappen[j].P = (*HourRain);//wanghao,������ǽ����ַ��������ѩ�Խ���ĸı䣬����ͻ��֪�������仯
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

			if(T<0)//����ѩ!!!!
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
		cout<<"mdb open error!!"<<endl;  //�����ݿ�������ش�����롣
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
//		pConn.CreateInstance(__uuidof(ADODB::Connection));//��������
//		pConn->Open(bSQL2,m_user.GetString(),m_password.GetString(),0);
//	}
//	catch(...)
//	{
//		cout<<"oracle open error!!"<<endl;
//		exit(0);
//	}
//	return pConn;
//}
