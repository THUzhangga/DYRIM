
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
//	CString CStr=m_RainFile;   //���������ݿ��·��
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
//				//MessageBox(NULL,"�����ݿ�"+rainMDB+"����","����",MB_OK|MB_ICONERROR);
//				cout<<"����:�����ݿ�"<<rainMDB<<"����"<<endl;
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
//				//MessageBox(NULL,"�����ݿ�"+rainMDB+"����","����",MB_OK|MB_ICONERROR);
//				cout<<"����:�����ݿ�"<<rainMDB<<"����"<<endl;
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
	//��ʼ����صı���Ҫע�⣬��Ϊ�������ǲ�����ĳ�Ա��������˹��캯��ȫ��ִֻ��һ�Σ�����Ӷ���أ���
	//Ӧ�÷���Initialize������
	pRev=ptrP=pHead=NULL;
	StartTime=EndTime=0;

	//wh
	//flag_Head=false;
	this->NumofHours=Hours;//wh
	this->HourStart=StartHour;//wh

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
	float HourInDay=StartHour-24*long(StartHour/24);//wh
	m_HourStart=HourInDay;


	YearEnd=YearStart;
	MonthEnd=MonthStart;
	DayCount=short((Hours+HourInDay-1)/24)+1;  //ת��СʱΪd

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

	ADODB::_RecordsetPtr  pRec;
	CComVariant ResultID;//20070209,xiaofc,Ϊ��Ӧ���ܵ��ַ����Ͳ�վ��
	float tempX,tempY,MinDistance,tempDistance;
	CString tempStr;
	
	//tempStr=m_RainFile;   //���������ݿ��·��
	//CString ConStr="Provider=Microsoft.Jet.OLEDB.4.0;Data Source="+tempStr+";Persist Security Info=False";
	//_bstr_t  bSQL=ConStr.GetString();
	//pCon.CreateInstance(__uuidof(ADODB::Connection));
	pRec.CreateInstance(__uuidof(ADODB::Recordset));
    //pCon->Open(bSQL,"Admin","",0); //�����ݿ�����
	//Ѱ�Ҿ������������վ��ID(��ʼ)
	_bstr_t  bSQL;
	bSQL="Select ID,X,Y from NameTabHour";//��nametab��Ϊnametabhour
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
		//Ѱ�Ҿ������������վ��ID(����)
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

	//20070209,xiaofc,ͨ��CComVariant�������ж���վ��ŵ��ֶ����ͣ��Ӷ��Զ�������ȷ��SQL��䡣
	//20070505,xioafc,�������VT_BSTR,��ChangType()������������ͳһһЩ
	CString tempStr2;
	try
	{
		if(ResultID.vt!=VT_BSTR)
		{
			ResultID.ChangeType(VT_BSTR);
			tempStr2=ResultID.bstrVal;
			tempStr.Format(_T("Select Year,Month,Day,HourS,HourE,P from rainhour where ID=%s and Year>=%d and Year<=%d order by Year,Month,Day,HourS"),tempStr2,YearStart,(YearEnd+1));//wh,��rain��Ϊrainhour
		}
		else
		{
			tempStr2=ResultID.bstrVal;
			tempStr.Format(_T("Select Year,Month,Day,HourS,HourE,P from rainhour where ID='%s' and Year>=%d and Year<=%d order by Year,Month,Day,HourS"),tempStr2,YearStart,(YearEnd+1));//wh,��rain��Ϊrainhour
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

		//��HourS��HourE������������
		//if((HourStart-int(HourStart))>0.3) HourS=int(HourStart)+1;
		//else HourS=int(HourStart);
		//if((HourEnd-int(HourEnd))>0.3) HourE=int(HourEnd)+1;
		//else HourE=int(HourEnd);

		//��һ���ֶ�HourS��HourE������������
		//��һ���ֲ�������������
		HourStart=int(HourStart)+(HourStart-int(HourStart))/0.6;
		HourEnd=int(HourEnd)+(HourEnd-int(HourEnd))/0.6;

		if(!ptrP){ pHead=ptrP=pTemp=(pPrecipitationS)malloc(sizeof(PrecipitationS)); pTemp->next=NULL;}
		else {pTemp->next=(pPrecipitationS)malloc(sizeof(PrecipitationS));pTemp=pTemp->next; pTemp->next=NULL;}
		tempflt=HourEnd-HourStart;//ʱ�β��Ȼ�Ǻ��ǰ,20060315�����Ĵ���

		if(tempflt<=0) 
			tempflt=tempflt+24;

		pTemp->Precipitation=Percipitation/tempflt;
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

bool YuLiangChBG::DeleteList(void)
{
	//FreePSeries(ptrP);
	FreePSeries(pHead);//wh
	StartTime=EndTime=0;
	return true;
}

//wh��Hours���������(s),ʱ�䲽��dt(s)
//ptrP:ͣ�����������������
float YuLiangChBG::P(double HourOffset,double* Hours,float dt)
{
	if((HourOffset<StartTime) || (HourOffset>EndTime)) return -1;

	(*Hours)=0.0;//wh
	
	float Result=0;
	pPrecipitationS pTemp;

	//wh,��¼����ǰһ��Ԫ�ص�ַ(�Ѿ��ŵ��������)
	//pPrecipitationS pRev=NULL;

	pTemp=ptrP;

	//���pTempΪNULL��˵���������ݵ���ֹʱ���ڼ����ֹʱ��֮ǰ������Ҫ���⴦��������*Hours����0ֵ��FVMunit::TimeStart�޷����ӣ�������������ѭ����
	if(pTemp==NULL)
	{
		(*Hours)=(HourStart+NumofHours-HourOffset)*3600+1;//���1�룬�³��ָ�������������
		return -1;
	}

	//�����ǰʱ�����꣬��while�жϣ�������꣬��������²��ң�ֱ���ҵ����������ֹ.
	while(pTemp)
	{
		//�����Ǹ�1e-5�ǳ���Ҫ�����û�оͲ��ϵĽ��������Ǹ�if���������Ƚϵ����⣬(*Hours)��һ������С����һ��ע��
		if(pTemp->TimeStart>HourOffset+1e-5)
		{
			//�����������С����ʱ����ڼ��㿪ʼʱ�䣬��ȻҲҪ�ͼ������ʱ��Ƚϣ�ȡСֵ��pRev=NULL
			//��ʱpTemp��λ��������һ��Ԫ��
			
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

		//if(HourOffset>=(pTemp->TimeStart) && HourOffset<=(pTemp->TimeEnd)) 20070713,xiaofc,�����������������������
		if(pTemp->TimeStart-HourOffset<1e-5 && HourOffset-pTemp->TimeEnd<1e-5)
		{
			Result=pTemp->Precipitation;//mm/Сʱ
			//20061218,xiaofc:����0ʱ�����Ĵ���ʱ��
			//cout<<pTemp->TimeStart<<"\t"<<pTemp->TimeEnd<<":"<<Result<<endl;
			if(Result>1e-5)
			{
				//�н���ʱ����Ҫ֪��*Hour��ֵ��ֻ�践���������ɣ���ȻpRevҲ����Ҫ�ں�
				ptrP=pTemp;
				return Result*dt/3600.0*0.001;//����ֵΪm
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

	//Ϊ�˵õ�HourOffsetʱ���Ժ��ж೤ʱ��û�н���,��ʱpTemp->TimeStartһ������HourOffset
	while(pTemp)
	{
		//������꣬ʱ���ۼ�
		if(pTemp->Precipitation<1e-5 && pTemp->TimeStart<(HourStart+NumofHours)-1e-5) 
		{ 
			if(pRev==NULL)
			{
				(*Hours) += (pTemp->TimeEnd - pTemp->TimeStart);
			}
			else
			{
				//����pRev��Ϊ�˽��HourS��HourEʱ����ܲ����������⣬��ǰһ���HourEnd�͵ڶ����HourS����ȵ���������ʱ��Ҳ������ġ�
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

	//���Թ��pTempΪNULL��ptrP�ò���ֵ�����
	ptrP=pTemp;

	//HourOffsetʱ��û�н���ʱ����0,ֻ��ע*Hours�Ĵ�С
	(*Hours)*=3600;//Сʱ��Ϊ��

	return -1;

	//return Result*dt/3600.0*0.001;//����ֵΪm
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