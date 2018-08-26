// yuliang.cpp : implementation file
//

//#include "stdafx.h"
#include "datastruct.h"
#include ".\yuliang.h"
#include "CoorTrans.h"
#include <Shlwapi.h>

//using namespace ATL;
Cyuliang::Cyuliang()

{
	StartTime=EndTime=0;
}

Cyuliang::~Cyuliang()
{
	if(m_TargetPoints.size()>0)
		DeleteList();
}


//altered by wh,�������ݿ��Ѿ�����Oracle������access��
// Cyuliang member functions
//void Cyuliang::OpenMDBCnn()
//{
//	CString CStr=m_RainFile;   //���������ݿ��·��
//	CStr=CStr.Left(CStr.ReverseFind('.'));
//	CString rainMDB,rainLDB,ConStr;
//	_bstr_t  CnnbStr;
//	pConnection.CreateInstance(__uuidof(ADODB::Connection));
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
//			hr=pConnection->Open(CnnbStr,"Admin","",-1);
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
//			hr=pConnection->Open(CnnbStr,"Admin","",-1);
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
//void Cyuliang::CloseMDBCnn()
//{
//	pConnection->Close();
//	//cout<<"CloseRainfallMDB"<<endl;
//}

//wh��x��yΪһ���Ӷε����꣬Ҫ��γ�����꣬�õ��ӶεĽ�������
void Cyuliang::Initialize(double x,double y,long StartHour, long Hours)
{
	m_PSeries.clear(); //��ԭ���Ľ������������

	int YearEnd,YearStart,MonthStart,MonthEnd,DayStart,DayEnd;
	long DayCount,temp;

	//ͨ��StartHour�����ʼ�����գ���ʼ��
	YearStart=YearSTD;      //��׼��
	MonthStart=MonthSTD;	//��׼��

	DayCount=long(StartHour/24); //ת��СʱΪd//���㿪ʼʱ������׼���d��
    temp=0;
	
	//wh������Ϊ�˽�Сʱת��Ϊ�ꡢ�¡���
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

	int i;//NumOfGage

	StartTime=StartHour;
	EndTime=StartHour+Hours;

	//cout<<"x="<<x<<";y="<<y<<"YearStart="<<YearStart<<"MonthStart="<<MonthStart<<"DayStart="<<DayStart<<endl;
	//cout<<"YearEnd="<<YearEnd<<"MonthEnd="<<MonthEnd<<"DayEnd="<<DayEnd<<endl;


	m_PSeries=GetDataByTS(x,y,YearStart,MonthStart,DayStart,YearEnd,MonthEnd,DayEnd);

	for(i=0;i<24;i++) PArray[i]=-1;  //��������洢����ֵ��Ϊ��1

}

//wh������Χ�ȽϽ�������վ��֯������m_TargetPoints����ĳ�Ա������
bool Cyuliang::GetDataByDW(float jx,float wy)    //����Ȩ�ط�
{
	//xyPerjw Dxy;
	jwCoord jw;
	jw.jCoord=jx;
	jw.wCoord =wy;
	//JwUnitToXYUnit(jw,&Dxy);   ��γ����ϵ��ÿһ�����Ȼ�γ�Ȼ���Ϊ�������ĳ���
	//ADODB::_ConnectionPtr pCon;
	
	ADODB::_RecordsetPtr  pRec;
	
	//CString CStr=m_RainFile;   //���������ݿ��·��
	//CString ConStr="Provider=Microsoft.Jet.OLEDB.4.0;Data Source="+CStr+";Persist Security Info=False";
	//_bstr_t  CnnbStr=ConStr.GetString();
	//pCon.CreateInstance(__uuidof(ADODB::Connection));
	pRec.CreateInstance(__uuidof(ADODB::Recordset));
    //HRESULT hr=pCon->Open(CnnbStr,"Admin","",-1);
	int n=1,t=0;n=0;
	float xMin,xMax,yMin,yMax;   //������ΧΪһ����γ�ȵ�������
	CString SelSQL;
	_bstr_t SQL;
	while(n<3)    //������3��վ�� 
	{
		
		//xMin=jx-0.01*t;
		//xMax=jx+0.01*t;
		//yMin=wy-0.01*t;
		//yMax=wy+0.01*t;       //�����ķ�Χ����Ϊ��γ����0.01

		//wh,Ϊ�˲�����
		xMin=jx-0.05*t;
		xMax=jx+0.05*t;
		yMin=wy-0.05*t;
		yMax=wy+0.05*t;       //�����ķ�Χ����Ϊ��γ����0.1

		//SelSQL.Format(_T("select * from NameTab where ESLO Between %.2f and %.2f and NRLA Between %.2f and %.2f"),xMin,xMax,yMin,yMax);

		//altered by wh,2008,�������ֶ�ͳһΪX��Y����Сʱ����һ��
		SelSQL.Format(_T("select * from NameTabDay where x Between %.2f and %.2f and y Between %.2f and %.2f"),xMin,xMax,yMin,yMax);

		SQL=SelSQL.GetString();
		pRec->Open(SQL,(ADODB::_Connection*)pConnection,ADODB::adOpenKeyset,ADODB::adLockOptimistic,ADODB::adCmdText);
		n=pRec->RecordCount;

		//���û�м�¼����������nΪ��ֵ��������0
		
		if(n<3)
		{
			pRec->Close();
			t+=1;   //��������
		}
		if(t>220)//ԭ����if(t>500)
		{
			if(!(n>=1))
			{
				xMin=-180;
				xMax=180;
				yMin=-90;
				yMax=90;
				SelSQL.Format(_T("select * from NameTabDay where x Between %.2f and %.2f and y Between %.2f and %.2f"),xMin,xMax,yMin,yMax);

				SQL=SelSQL.GetString();
				pRec->Open(SQL,(ADODB::_Connection*)pConnection,ADODB::adOpenKeyset,ADODB::adLockOptimistic,ADODB::adCmdText);
				n=pRec->RecordCount;
			}

			break;
		}
	}

	if(n>=1)
	//if( n>=3 )
	{
		DeleteList();

		//Ҫ������վԭʼ����Ϊ��γ�ȣ�
		while(!pRec->EndOfFile)
		{
			YuliangZhan * rainPoint=new YuliangZhan;
			_variant_t vary;
			//vary.lVal=pRec->Fields->Item["STCD"]->Value;//wh
			vary.lVal=pRec->Fields->Item["id"]->Value;
			rainPoint->lCode=vary.lVal;
			//vary.fltVal =pRec->Fields->Item["ESLO"]->Value;//wh
			vary.fltVal =pRec->Fields->Item["x"]->Value;
			rainPoint->jwLocation.jCoord=vary.fltVal;
			//vary.fltVal =pRec->Fields->Item["NRLA"]->Value;//wh
			vary.fltVal =pRec->Fields->Item["y"]->Value;
			rainPoint->jwLocation.wCoord =vary.fltVal;
			rainPoint->weight=0.0;
			m_TargetPoints.push_back(rainPoint);          //����������������վ�ı�ţ�λ��д��������
			pRec->MoveNext();
		}
		pRec->Close();
		bool ok=CalcuWt(jw);
		if (!ok) return false;
		//ofstream outfile("weight.txt",ios_base::out);
		//for(int i=0;i<m_TargetPoints.size();i++)
		//outfile<<m_TargetPoints[i]->lCode <<"   "<<m_TargetPoints[i]->weight<<endl;

	}
	else
	{
		cout<<xMin<<"  "<<xMax<<"  "<<yMin<<"  "<<yMax<<"  "<<endl;
		//pRec->Close();
		//pConnection->Close();
		//MessageBox(NULL,"����վ��̫ϡ�裬����̫��","����",MB_OK|MB_ICONERROR);
		cout<<"����:����վ��̫ϡ�裬����̫��."<<endl;
		return false;
	}

    //pConnection->Close();
	return true;
}


//������վ�������
bool Cyuliang::DeleteList(void)
{
	if(m_TargetPoints.size()>0)//vector< YuliangZhan * >  m_TargetPoints;
	{
		for(int n=0;n<m_TargetPoints.size();n++)
		{
		    delete (YuliangZhan*)(m_TargetPoints[n]);
			m_TargetPoints[n]=NULL;
		}
		
	}
	m_TargetPoints.clear();
	return true;
}	

//���վ���ĳ���㷨��Ϊ�˵õ�ÿ����������վ��Ȩ��
bool Cyuliang::CalcuWt(jwCoord jw)
{
	if(m_TargetPoints.size()==0)
		return false;

	PrjPoint_IUGG1975 PrjTrans;
	bool ok=PrjTrans.GetL0(jw);
	if (!ok) return false;

    YuliangZhan * rainPoint;
	xyCoord  xy;
    ok=PrjTrans.BL2xy(jw,&xy);
    if (!ok) return false;

    //�Ӷε�����
	m_CenterX=xy.xCoord;
	m_CenterY=xy.yCoord;

	//�õ���ÿ����ѡ��������վ����Ӷ����ĵľ��루dis��
	vector<PointPro> vecPoint(m_TargetPoints.size());
	for(int i=0;i<m_TargetPoints.size();i++)
	{
       rainPoint=m_TargetPoints[i];
	   jw.jCoord=rainPoint->jwLocation.jCoord;
	   jw.wCoord=rainPoint->jwLocation.wCoord;
	   ok=PrjTrans.BL2xy(jw,&xy);
	   if (!ok) return false;
	   rainPoint->xyLocation.xCoord=xy.xCoord;
	   rainPoint->xyLocation.yCoord=xy.yCoord;
	   vecPoint[i].dis=sqrt((xy.xCoord-m_CenterX)*(xy.xCoord-m_CenterX)+(xy.yCoord-m_CenterY)*(xy.yCoord-m_CenterY));//����վ����Ӷ����ĵľ���
	}

	float fltXishu=0,cosA;
	float fltNiXishu;
	float disCha;
	for(int j=0;j<m_TargetPoints.size();j++)
	{
		fltXishu=0;
		fltNiXishu=0;
		cosA=0;
		vecPoint[j].xiShu=0;
		for(int i=0;i<m_TargetPoints.size();i++)
		{
			if (i==j) continue;
            
			cosA=vecPoint[i].dis*vecPoint[i].dis+vecPoint[j].dis*vecPoint[j].dis-((m_TargetPoints[i]->xyLocation.xCoord-m_TargetPoints[j]->xyLocation.xCoord)
                 *(m_TargetPoints[i]->xyLocation.xCoord-m_TargetPoints[j]->xyLocation.xCoord)+(m_TargetPoints[i]->xyLocation.yCoord-m_TargetPoints[j]->xyLocation.yCoord)*(m_TargetPoints[i]->xyLocation.yCoord-m_TargetPoints[j]->xyLocation.yCoord));
		    cosA=cosA/(2*vecPoint[i].dis*vecPoint[j].dis);//��Ӷ�ΪC��A��BΪ��������վ�����˵õ��˽�ACB�����Ҷ���

			fltNiXishu=(cosA>=0)? cosA:0;
            disCha=vecPoint[i].dis-vecPoint[j].dis;
			if (disCha<0) fltXishu=fltNiXishu;
			else if (disCha<= 0.5 ) fltXishu=fltNiXishu/2;
            else if (disCha>0.5) fltXishu=0;
			
			if (fltXishu>vecPoint[j].xiShu) vecPoint[j].xiShu=fltXishu;
		}

	}

	float sum=0;
	for(int j=0;j<m_TargetPoints.size();j++)
	{
		sum+=((1-vecPoint[j].xiShu)/(vecPoint[j].dis*vecPoint[j].dis));
	}

	for(int i=0;i<m_TargetPoints.size();i++)    //��Ȩ��
	{
		m_TargetPoints[i]->weight=(1-vecPoint[i].xiShu)/(sum*vecPoint[i].dis*vecPoint[i].dis);
	}
    
	return true;
}

//wh���ú���ʵ���˽����ڼ�����ֹʱ��֮��ÿһd�Ľ�������֯������
PSeriesType Cyuliang::GetGageTS(long ID, short YearStart,short MonthStart, short DayStart,short YearEnd, short MonthEnd, short DayEnd)
{
	PSeriesType m_Result;
	if(YearStart>YearEnd) return m_Result;
	if((YearStart==YearEnd) && (MonthStart>MonthEnd) ) return m_Result;
	if((YearStart==YearEnd) && (MonthStart==MonthEnd) && (DayStart>DayEnd) ) return m_Result;
	
	//CString m_strRain=m_RainFile;

	//ADODB::_ConnectionPtr pCnn;
	//_bstr_t bSQL;
	//bSQL="Provider=Microsoft.Jet.OLEDB.4.0;Data Source="+m_strRain+";Persist Security Info=False";
	//try{
		//pCnn.CreateInstance(__uuidof(ADODB::Connection));
		//pCnn->CursorLocation=ADODB::adUseClient;
		//pCnn->Open(bSQL,"Admin","",-1);
	//}
	//catch(_com_error &e)
	//{
	//	return m_Result;   //�����ݿ�������ش�����롣
	//}
	
	if(MonthStart==MonthEnd && YearStart==YearEnd){
		ReadMiddle(m_Result,pConnection,ID,YearStart,MonthStart,DayStart,DayEnd);
		//pConnection->Close();
		return m_Result;
	}

	//wh������������YearStart,MonthStart�м�¼��DayStart���������һd������������֯������
	ReadLastHalf(m_Result,pConnection,ID,YearStart,MonthStart,DayStart);
	
	short NextYear, NextMonth,CurYear,CurMonth;
	CurYear=YearStart;
	CurMonth=MonthStart;
	MonthAdd(CurYear,CurMonth,NextYear,NextMonth);
	while(!((NextYear==YearEnd) && (NextMonth==MonthEnd))){
		CurYear=NextYear;
		CurMonth=NextMonth;
		ReadWholeMonth(m_Result,pConnection,ID,CurYear,CurMonth);
		MonthAdd(CurYear,CurMonth,NextYear,NextMonth);
	}
	ReadPrevHalf(m_Result,pConnection,ID,YearEnd,MonthEnd,DayEnd);
	//pConnection->Close();
	return m_Result;
}


//���ս�ĳ��ĳ�½���daystart��dayend��������֯������m_series
void Cyuliang::ReadMiddle(PSeriesType &m_series,ADODB::_ConnectionPtr pCnn,long ID,short Year, short Month,short DayStart,short DayEnd)
{
	CComVariant tempVar;
	CString tempStr;
	_bstr_t bSQL;
	ADODB::_RecordsetPtr pRst;
	short i;
	float tempflt;

	pRst.CreateInstance(__uuidof(ADODB::Recordset));
	tempStr.Format(_T("Select P1,P2,P3,P4,P5,P6,P7,P8,P9,P10,P11,P12,P13,P14,P15,P16,P17,P18,P19,P20,P21,P22,P23,P24,P25,P26,P27,P28,P29,P30,P31 \
				   from rainday where ID = %ld and Year = %d and Month = %d"),ID,Year,Month);//wh,��STCD��ΪID
	bSQL=tempStr.GetString();
	pRst->Open(bSQL,(ADODB::_Connection*)pCnn,ADODB::adOpenKeyset,ADODB::adLockOptimistic,ADODB::adCmdText);
	if(!pRst->EndOfFile){
		for(i=DayStart-1;i<DayEnd;i++)
		{
			tempVar=pRst->Fields->Item[i]->Value;
			tempflt=(tempVar.vt==VT_NULL ? 0:pRst->Fields->Item[i]->Value);
			if(tempflt<0) tempflt=0;		//�������С��0����ֵΪ0��
			m_series.push_back(tempflt);
		}
		pRst->Close();
		return ;
	}

	//�����������ݲ��������������������ݼ���
	pRst->Close();
	tempStr.Format(_T("Select P1,P2,P3,P4,P5,P6,P7,P8,P9,P10,P11,P12,P13,P14,P15,P16,P17,P18,P19,P20,P21,P22,P23,P24,P25,P26,P27,P28,P29,P30,P31 \
				from rainday where ID = %ld and Month = %d"),ID,Month);//wh,��STCD��ΪID
	bSQL=tempStr.GetString();
	pRst->Open(bSQL,(ADODB::_Connection*)pCnn,ADODB::adOpenKeyset,ADODB::adLockOptimistic,ADODB::adCmdText);
	if(!pRst->EndOfFile){
		for(i=DayStart-1;i<DayEnd;i++)
		{
			tempVar=pRst->Fields->Item[i]->Value;
			tempflt=(tempVar.vt==VT_NULL ? 0:pRst->Fields->Item[i]->Value);
			if(tempflt<0) tempflt=0;		//�������С��0����ֵΪ0��
			m_series.push_back(tempflt);
		}
	}
	pRst->Close();
}



void Cyuliang::ReadPrevHalf(PSeriesType &m_series, ADODB::_ConnectionPtr pCnn, long ID, short Year, short Month, short DayEnd)
{
	CComVariant tempVar;
	CString tempStr;
	_bstr_t bSQL;
	ADODB::_RecordsetPtr pRst;
	short i;
	float tempflt;

	pRst.CreateInstance(__uuidof(ADODB::Recordset));
	tempStr.Format(_T("Select P1,P2,P3,P4,P5,P6,P7,P8,P9,P10,P11,P12,P13,P14,P15,P16,P17,P18,P19,P20,P21,P22,P23,P24,P25,P26,P27,P28,P29,P30,P31 \
				   from rainday where ID = %ld and Year = %d and Month = %d"),ID,Year,Month);//wh,��STCD��ΪID
	bSQL=tempStr.GetString();
	pRst->Open(bSQL,(ADODB::_Connection*)pCnn,ADODB::adOpenKeyset,ADODB::adLockOptimistic,ADODB::adCmdText);
	if(!pRst->EndOfFile){
		for(i=0;i<DayEnd;i++)
		{
			tempVar=pRst->Fields->Item[i]->Value;
			tempflt=(tempVar.vt==VT_NULL ? 0:pRst->Fields->Item[i]->Value);
			if(tempflt<0) tempflt=0;		//�������С��0����ֵΪ0��
			m_series.push_back(tempflt);
		}
		pRst->Close();
		return ;
	}
	pRst->Close();
	
	//�����������ݲ��������������������ݼ���
	tempStr.Format(_T("Select P1,P2,P3,P4,P5,P6,P7,P8,P9,P10,P11,P12,P13,P14,P15,P16,P17,P18,P19,P20,P21,P22,P23,P24,P25,P26,P27,P28,P29,P30,P31 \
				   from rainday where ID = %ld and Month = %d"),ID,Month);//wh,��STCD��ΪID
	bSQL=tempStr.GetString();
	pRst->Open(bSQL,(ADODB::_Connection*)pCnn,ADODB::adOpenKeyset,ADODB::adLockOptimistic,ADODB::adCmdText);
	if(!pRst->EndOfFile){
		for(i=0;i<DayEnd;i++)
		{
			tempVar=pRst->Fields->Item[i]->Value;
			tempflt=(tempVar.vt==VT_NULL ? 0:pRst->Fields->Item[i]->Value);
			if(tempflt<0) tempflt=0;		//�������С��0����ֵΪ0��
			m_series.push_back(tempflt);
		}
	}
	pRst->Close();
}

//wh��������һ�м�¼��֯������
void Cyuliang::ReadWholeMonth(PSeriesType &m_series, ADODB::_ConnectionPtr pCnn, long ID, short Year, short Month)
{
	CComVariant tempVar;
	CString tempStr;
	_bstr_t bSQL;
	ADODB::_RecordsetPtr pRst;
	short MonthDays,i;
	float tempflt;
	MonthDays=GetMonthDays(Year,Month);
	pRst.CreateInstance(__uuidof(ADODB::Recordset));
	tempStr.Format(_T("Select P1,P2,P3,P4,P5,P6,P7,P8,P9,P10,P11,P12,P13,P14,P15,P16,P17,P18,P19,P20,P21,P22,P23,P24,P25,P26,P27,P28,P29,P30,P31 \
				   from rainday where ID = %ld and Year = %d and Month = %d"),ID,Year,Month);//wh,��STCD��ΪID
	bSQL=tempStr.GetString();
	pRst->Open(bSQL,(ADODB::_Connection*)pCnn,ADODB::adOpenKeyset,ADODB::adLockOptimistic,ADODB::adCmdText);
	if(!pRst->EndOfFile){
		for(i=0;i<MonthDays;i++)
		{
			tempVar=pRst->Fields->Item[i]->Value;
			tempflt=(tempVar.vt==VT_NULL ? 0:pRst->Fields->Item[i]->Value);
			if(tempflt<0) tempflt=0;		//�������С��0����ֵΪ0��
			m_series.push_back(tempflt);
		}
		pRst->Close();
		return;
	}
	pRst->Close();

	//�����������ݲ��������������������ݼ���
	tempStr.Format(_T("Select P1,P2,P3,P4,P5,P6,P7,P8,P9,P10,P11,P12,P13,P14,P15,P16,P17,P18,P19,P20,P21,P22,P23,P24,P25,P26,P27,P28,P29,P30,P31 \
				   from rainday where ID = %ld and Month = %d"),ID,Month);//wh,��STCD��ΪID
	bSQL=tempStr.GetString();
	pRst->Open(bSQL,(ADODB::_Connection*)pCnn,ADODB::adOpenKeyset,ADODB::adLockOptimistic,ADODB::adCmdText);
	if(!pRst->EndOfFile){
		for(i=0;i<MonthDays;i++)
		{
			tempVar=pRst->Fields->Item[i]->Value;
			tempflt=(tempVar.vt==VT_NULL ? 0:pRst->Fields->Item[i]->Value);
			if(tempflt<0) tempflt=0;		//�������С��0����ֵΪ0��
			m_series.push_back(tempflt);
		}
	}
	pRst->Close();
}



void Cyuliang::ReadLastHalf(PSeriesType &m_series, ADODB::_ConnectionPtr pCnn, long ID, short Year, short Month, short DayStart)
{
	CComVariant tempVar;
	CString tempStr;
	_bstr_t bSQL;
	ADODB::_RecordsetPtr pRst;
	short MonthDays,i;
	float tempflt;
	MonthDays=GetMonthDays(Year,Month);
	pRst.CreateInstance(__uuidof(ADODB::Recordset));
	tempStr.Format(_T("Select P1,P2,P3,P4,P5,P6,P7,P8,P9,P10,P11,P12,P13,P14,P15,P16,P17,P18,P19,P20,P21,P22,P23,P24,P25,P26,P27,P28,P29,P30,P31 \
				   from rainday where ID = %ld and Year = %d and Month = %d"),ID,Year,Month);//wh,��STCD��ΪID
	bSQL=tempStr.GetString();
	pRst->Open(bSQL,(ADODB::_Connection*)pCnn,ADODB::adOpenKeyset,ADODB::adLockOptimistic,ADODB::adCmdText);
	if(!pRst->EndOfFile){
		for(i=DayStart-1;i<MonthDays;i++)
		{
			tempVar=pRst->Fields->Item[i]->Value;
			tempflt=(tempVar.vt==VT_NULL ? 0:pRst->Fields->Item[i]->Value);
			if(tempflt<0) tempflt=0;		//�������С��0����ֵΪ0��
			m_series.push_back(tempflt);
		}
		pRst->Close();
		return ;
	}

	//�����������ݲ��������������������ݼ���
	pRst->Close();
	tempStr.Format(_T("Select P1,P2,P3,P4,P5,P6,P7,P8,P9,P10,P11,P12,P13,P14,P15,P16,P17,P18,P19,P20,P21,P22,P23,P24,P25,P26,P27,P28,P29,P30,P31 \
				from rainday where ID = %ld and Month = %d"),ID,Month);//wh,��STCD��ΪID
	bSQL=tempStr.GetString();
	pRst->Open(bSQL,(ADODB::_Connection*)pCnn,ADODB::adOpenKeyset,ADODB::adLockOptimistic,ADODB::adCmdText);
	if(!pRst->EndOfFile){
		for(i=DayStart-1;i<MonthDays;i++)
		{
			tempVar=pRst->Fields->Item[i]->Value;
			tempflt=(tempVar.vt==VT_NULL ? 0:pRst->Fields->Item[i]->Value);
			if(tempflt<0) tempflt=0;		//�������С��0����ֵΪ0��
			m_series.push_back(tempflt);
		}
	}
	pRst->Close();
}

//Ϊ�˵õ�һ�����ж���d��
short Cyuliang::GetMonthDays(short Year, short Month)
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

//a��b�£��ٹ�һ���������������
void Cyuliang::MonthAdd(short CurYear, short CurMonth, short &NextYear, short &NextMonth)
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

//wh���õ�������ֹʱ���ڣ��ӶεĽ������С��������е�ÿһ��Ϊһd������DayStart��DayEnd��
PSeriesType Cyuliang::GetDataByTS(float jx,float wy,short YearStart,short MonthStart, short DayStart, short YearEnd, short MonthEnd, short DayEnd)
{
	PSeriesType m_Result, m_tempSeries;
	int i,j,NumOfGage,NumOfTimeStep;//NumOfGage����Ӷν�������վ������վ����Ŀ
	try
	{
		if(!GetDataByDW(jx,wy))//��ʱ����Ӷν�������վ�Ѿ���֯������
		{
			cout<<"GetDataByDW(jx,wy)����"<<endl;
			return m_Result;
		}
	}
	catch (...) 
	{
		cout<<"Error While Reading RainFall."<<endl;
		exit(0);
	}
	float tempflt;
	NumOfGage=m_TargetPoints.size();
	
	//cout<<"NumofGage="<<NumOfGage<<endl;
   
	vector<PSeriesType> mySeriesArray;
	if(NumOfGage<1) return m_Result; 
	for(i=0;i<NumOfGage;i++){
		m_tempSeries=GetGageTS(m_TargetPoints[i]->lCode,YearStart,MonthStart,DayStart,YearEnd,MonthEnd,DayEnd);//Ϊ�˵õ�ÿ������վ�Ľ�������
		mySeriesArray.push_back(m_tempSeries);
	}

	NumOfTimeStep=((PSeriesType)mySeriesArray[0]).size();
	for(i=0;i<NumOfTimeStep;i++){
		tempflt=0;
		for(j=0;j<NumOfGage;j++){
			tempflt=tempflt+((PSeriesType)mySeriesArray[j])[i]*m_TargetPoints[j]->weight;//m_Result��ʱ��Ϊ�ᣬÿ���ְ����������ÿ������վ����Ϣ��
		}
		m_Result.push_back(tempflt);
	}
	
	//ofstream outfile("PSeries.txt",ios_base::out);
	//for(int i=0;i<m_Result.size();i++)
	//outfile<<m_Result[i] <<endl;
	return m_Result;
}

//Day:û�н��������,dt(s)
float Cyuliang::P(double HourOffset,float hour,double* Day,float dt)
{
	if((HourOffset<StartTime) || (HourOffset>EndTime)) return -1;
	
	int i;
	long l_HourOffset=long(HourOffset); //��HourOffset����������ȡ����
	float Result=0,tempflt;
	i=int((HourOffset-StartTime)/24);
	//cout<<"i="<<i<<endl;

	//�������ݵĿ�ʼʱ���ڼ��㿪ʼʱ��֮��
	if(i<0)
	{
		(*Day)=(StartTime-HourOffset)*3600;
		return 0;
	}

	//2009,added by wanghao,��Ϊ����վ��������Ϣ����ֹʱ��ܿ���С��ʵ�ʼ�����ֹʱ�䣬����������㵽11��1�գ�����������ݿ�ֻ��10��31��
	//��ʱm_PSeries[i]�ͻ�Խ���ˣ���˼���������ж�
	if(m_PSeries.size()<i+1)
	{
		i=m_PSeries.size()-1;
		tempflt=m_PSeries[i];
	}
	else
	{
		tempflt=m_PSeries[i];//wh���������1000���϶���ת����m����˼�ɡ�
	}

	
	//wh
	if(tempflt<=1e-5)
	{
		(*Day)=(24-hour)/24;//һ�쵱�л�ʣ��0.x��û�н��֮꣬ǰ��û����Ҳ������
	}

	//�ж��ǲ��ǵ������һ�������ж���else�����ǲ���ִ�еģ�����Խ��
	if(i==m_PSeries.size()-1)
	{
		tempflt=1;
	}
	else
	{
		tempflt=m_PSeries[i+1];
	}

	//wh
	while(tempflt<=1e-5)
	{
		(*Day)+=1;
		if(i==m_PSeries.size()-1)
		{
			break;
		}
		i++;
		tempflt=m_PSeries[i];
	}

	(*Day)=(*Day)*24*3600;
	if((*Day)>0) { return 0;}

	//cout<<"HourOffset-StartTime="<<(HourOffset-StartTime)<<"tempflt="<<tempflt<<endl;
	
	//wh�����Ϊע��
	Result=ScatterDayToHours((HourOffset-StartTime),tempflt/1000);//m
	
	return Result*dt/3600;
}



//wh�����ս���������Сʱ������
float Cyuliang::ScatterDayToHours(long HourOffset,float rainfall)
{
	int i,tempInt,tempInt1;
	float tempflt;
	bool NeedReset=false;
	vector <int> IndexVec;
	vector <float> ValueVec;
	if(PArray[0]<0 || (HourOffset%24)==1)  NeedReset=true;//PArray[0]��û�г�ʼ��С��0��������iʵ�ʴ�1��ʼ

	//wh�������µ�һd��һ���Ӿ�ȫ������ˣ�����NeedReset����Ϊfalse
	if(NeedReset){
		IndexVec.clear();
		ValueVec.clear();
		for(i=0;i<24;i++) PArray[i]=0;
		// Seed the random-number generator with current time so that the numbers will be different every time we run. 
		if(rainfall<5.0){
			//srand( (unsigned)time( NULL ) );
			tempInt=rand()%24;//The rand function returns a pseudorandom integer in the range 0 to RAND_MAX (32767). 
			IndexVec.push_back(tempInt);
			ValueVec.push_back(rainfall);
		}
		else if(rainfall>4.9999 && rainfall<20)
		{
			for(i=1;i<2;i++)
			{
				//srand( (unsigned)time( NULL ) );
				tempInt=rand()%24;
				IndexVec.push_back(tempInt);
				//srand( (unsigned)time( NULL ) );
				tempInt=rand()%20;
				tempflt=float(tempInt)/(10.0*(3-i))*rainfall;//�൱�ڵõ�һ��0��1֮��α��������ٳ���rainfall
				ValueVec.push_back(tempflt);
				rainfall=rainfall-tempflt;
			}
			//srand( (unsigned)time( NULL ) );
			tempInt=rand()%24;//���ܺ����涼��ͬһ��Сʱ�ģ����û���⡣
			IndexVec.push_back(tempInt);
			ValueVec.push_back(rainfall);
		}
		else if(rainfall>19.9999 && rainfall<40){
			for(i=1;i<3;i++){
				//srand( (unsigned)time( NULL ) );
				tempInt=rand()%24;
				IndexVec.push_back(tempInt);
				//srand( (unsigned)time( NULL ) );
				tempInt=rand()%20;
				tempflt=float(tempInt)/(10.0*(4-i))*rainfall;
				ValueVec.push_back(tempflt);
				rainfall=rainfall-tempflt;
			}
			//srand( (unsigned)time( NULL ) );
			tempInt=rand()%24;
			IndexVec.push_back(tempInt);
			ValueVec.push_back(rainfall);
		}
		else if(rainfall>39.9999 && rainfall<60){
			for(i=1;i<4;i++){
				//srand( (unsigned)time( NULL ) );
				tempInt=rand()%24;
				IndexVec.push_back(tempInt);
				//srand( (unsigned)time( NULL ) );
				tempInt=rand()%20;
				tempflt=float(tempInt)/(10.0*(5-i))*rainfall;
				ValueVec.push_back(tempflt);
				rainfall=rainfall-tempflt;
			}
			//srand( (unsigned)time( NULL ) );
			tempInt=rand()%24;
			IndexVec.push_back(tempInt);
			ValueVec.push_back(rainfall);
		}
		else if(rainfall>59.9999 && rainfall<80){
			for(i=1;i<5;i++){
				//srand( (unsigned)time( NULL ) );
				tempInt=rand()%24;
				IndexVec.push_back(tempInt);
				//srand( (unsigned)time( NULL ) );
				tempInt=rand()%20;
				tempflt=float(tempInt)/(10.0*(6-i))*rainfall;
				ValueVec.push_back(tempflt);
				rainfall=rainfall-tempflt;
			}
			//srand( (unsigned)time( NULL ) );
			tempInt=rand()%24;
			IndexVec.push_back(tempInt);
			ValueVec.push_back(rainfall);
		}
		else if(rainfall>79.9999 && rainfall<100){
			for(i=1;i<6;i++){
				//srand( (unsigned)time( NULL ) );
				tempInt=rand()%24;
				IndexVec.push_back(tempInt);
				//srand( (unsigned)time( NULL ) );
				tempInt=rand()%20;
				tempflt=float(tempInt)/(10.0*(7-i))*rainfall;
				ValueVec.push_back(tempflt);
				rainfall=rainfall-tempflt;
			}
			//srand( (unsigned)time( NULL ) );
			tempInt=rand()%24;
			IndexVec.push_back(tempInt);
			ValueVec.push_back(rainfall);
		}
		else if(rainfall>99.9999 && rainfall<120){
			for(i=1;i<7;i++){
				//srand( (unsigned)time( NULL ) );
				tempInt=rand()%24;
				IndexVec.push_back(tempInt);
				//srand( (unsigned)time( NULL ) );
				tempInt=rand()%20;
				tempflt=float(tempInt)/(10.0*(8-i))*rainfall;
				ValueVec.push_back(tempflt);
				rainfall=rainfall-tempflt;
			}
			//srand( (unsigned)time( NULL ) );
			tempInt=rand()%24;
			IndexVec.push_back(tempInt);
			ValueVec.push_back(rainfall);
		}
		else if(rainfall>119.9999 && rainfall<150){
			tempInt=rand()%15;
			for(i=1;i<8;i++){
				//srand( (unsigned)time( NULL ) );
				//tempInt=rand()%24;
				IndexVec.push_back(tempInt);
				//srand( (unsigned)time( NULL ) );
				tempInt1=rand()%20;
				tempflt=float(tempInt1)/(10.0*(9-i))*rainfall;
				ValueVec.push_back(tempflt);
				rainfall=rainfall-tempflt;
				tempInt++;
			}
			//srand( (unsigned)time( NULL ) );
			//tempInt=rand()%24;
			IndexVec.push_back(tempInt);
			ValueVec.push_back(rainfall);
		}
		else if(rainfall>149.9999 && rainfall<200){
			tempInt=rand()%14;
			for(i=1;i<9;i++){
				//srand( (unsigned)time( NULL ) );
				IndexVec.push_back(tempInt);
				//srand( (unsigned)time( NULL ) );
				tempInt1=rand()%20;
				tempflt=float(tempInt1)/(10.0*(10-i))*rainfall;
				ValueVec.push_back(tempflt);
				rainfall=rainfall-tempflt;
				tempInt++;
			}
			//srand( (unsigned)time( NULL ) );
			//tempInt=rand()%24;
			IndexVec.push_back(tempInt);
			ValueVec.push_back(rainfall);
		}
		else if(rainfall>199.9999 && rainfall<250){
			tempInt=rand()%13;
			for(i=1;i<10;i++){
				//srand( (unsigned)time( NULL ) );
				//tempInt=rand()%24;
				IndexVec.push_back(tempInt);
				//srand( (unsigned)time( NULL ) );
				tempInt1=rand()%20;
				tempflt=float(tempInt1)/(10.0*(11-i))*rainfall;
				ValueVec.push_back(tempflt);
				rainfall=rainfall-tempflt;
				tempInt++;
			}
			//srand( (unsigned)time( NULL ) );
			//tempInt=rand()%24;
			IndexVec.push_back(tempInt);
			ValueVec.push_back(rainfall);
		}
		else {
			for(i=1;i<24;i++){
				//srand( (unsigned)time( NULL ) );
				tempInt=rand()%24;
				IndexVec.push_back(tempInt);
				//srand( (unsigned)time( NULL ) );
				tempInt=rand()%20;
				tempflt=float(tempInt)/(10.0*(25-i))*rainfall;
				ValueVec.push_back(tempflt);
				rainfall=rainfall-tempflt;
			}
			//srand( (unsigned)time( NULL ) );
			tempInt=rand()%24;
			IndexVec.push_back(tempInt);
			ValueVec.push_back(rainfall);
		}
		for(i=0;i<IndexVec.size();i++){
			int ljh=IndexVec[i];
			PArray[ljh]+=ValueVec[i];
		}
	}
    return PArray[HourOffset%24];
}
