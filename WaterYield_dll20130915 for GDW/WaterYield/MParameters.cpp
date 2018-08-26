
#include "stdafx.h"
#include ".\MParameters.h"
#include <fstream>
#include <string>
#include "datastruct.h"

using namespace std;

MParameters::MParameters(void)
{
}

MParameters::~MParameters(void)
{
}

//��Para�ṹ���еĲ����Լ��úӶ�ǰһ���������ˮ����Ϣ������parameter����
bool MParameters::GetParameters(struct Para * pPara,char Position,BSCode mBSCode)
{
	bool isOK;

	if(GetWaterInSoil(mBSCode,*pPara))
	{
		if(Position=='S')
		{
			RouteLength=pPara->LengthS;
			SlopeJ=pPara->SlopeS;
			Area=pPara->AreaS;
			SoilW=WSU;//Ǳˮ���ʼˮ��
			MidSoilW=WSM;
			DSW=WSD; //������ĳ�ʼˮ��
			LeafW=WSL; //Ҷ���ˮ�m

			isOK= 1;
		}
		else if(Position=='L')
		{
			RouteLength=pPara->LengthL;
			SlopeJ=pPara->SlopeL;
			Area=pPara->AreaL;
			SoilW=WLU;//Ǳˮ���ʼˮ��
			MidSoilW=WLM;
			DSW=WLD; //������ĳ�ʼˮ��
			LeafW=WLL; //Ҷ���ˮ�m
			
			isOK= 1;
		}
		else if(Position=='R')
		{
			RouteLength=pPara->LengthR;
			SlopeJ=pPara->SlopeR;
			Area=pPara->AreaR;
			SoilW=WRU;//Ǳˮ���ʼˮ��
			MidSoilW=WRM;
			DSW=WRD; //������ĳ�ʼˮ��
			LeafW=WRL; //Ҷ���ˮ�m
			
			isOK= 1;
		}
		else
		{
			isOK= 0;
		}

		SediD=pPara->D50;

		//ÿСʱ��������ˮ�� m/h
		PKV0=pPara->PKV0;
		PKV1=pPara->PKV1;
		PKV2=pPara->PKV2;//addbyiwish20060220
		
		//Ǳˮ������ˮƽ��͸ϵ��, m/second
		//����ˮƽ��͸�ʵ�λ��������ͳһΪm/hr
		PKH1=pPara->PKH1/3600;
		PKH2=pPara->PKH2/3600;

		USita1=pPara->Sita1;
		USita2=pPara->Sita2;
		MSita1=pPara->MSita1;
		MSita2=pPara->MSita2;
		DSita1=pPara->DSita1;
		DSita2=pPara->DSita2;

		UDepth=pPara->UDepth;
		DSDepth=pPara->DDepth;

		Roughnessn=pPara->Manning;//0.12f;//��riversegs����
		
		//20051219 Ѧ�� ���Ӳ�ɳk
		ErosionK=pPara->ErosionK;
		ErosionBeta=pPara->ErosionBeta;//parameter����

		//David �����ɳģ��
	    ErosionK1=pPara->ErosionK1;
	    ErosionK2=pPara->ErosionK2;
	    ErosionBeta1=pPara->ErosionBeta1;
	    ErosionBeta2=pPara->ErosionBeta2;

		//addbyiwish 20060222
		//au=0.5;
		//bu=3.5;
		//am=1;
		//bm=4.21;
		//ad=1.5;
		//bd=4.21;

		//20070118,xiaofc,����ǫ�Ĺ�ʽ���������kPa,Ϊת��ˮͷm,��Ҫ/Rou/g,���Ը����е�a����һ��10
		//au=6.7377/10;
		//bu=2.5283;
		//am=0.1973;
		//bm=2.325;
		//ad=0.1;
		//bd=4.21;

		//20141229
		au=0.674;
		bu=2.528;
		am=0.674;
		bm=2.325;
		ad=0.674;
		bd=2.325;

		//20111208,qjh,shy
		//au=6.7377/10;
		//bu=2.5283;
		//am=0.1973;
		//bm=2.325;
		//ad=0.1;
		//bd=4.21;

		return isOK;
		}
	else 
		return false;

}

// ��״̬�����ļ�
bool MParameters::SaveStatusFile(void)
{
	
	string pfilename("status.txt");
	const char* cfilename=pfilename.c_str();
	ofstream fps(cfilename,ios_base::out);
	
	if( !fps )
	{
		//MessageBox(NULL,"Open file status.txt error.","ERROR",MB_OK);
		cout<<"Open file status.txt error."<<endl;
		return false;	
	}
	
	fps<<"SoilW\tDeepSoilW"<<endl;

	fps<<SoilW<<"\t"<<DSW;
	
	fps.close();

	return true;

}

//��ÿ���Ӷεĳ�ʼ���������ˮ��д��status���У���ͬʱ�����ڴ��е���Ӧ������
bool MParameters::GetWaterInSoil(BSCode mBSCode,Para myPara)
{
	//�������ʼ��ˮ��
	CComVariant tempCom(VT_EMPTY);
	ADODB::_RecordsetPtr pStatRst;
	CString SQL;

	SQL.Format(_T("Select * from Status where sccd='%s' and RegionIndex=%I64u and BSValue=%I64u and BSLength=%d and Houroffset=%d"),sccd,mBSCode.RegionIndex,mBSCode.Value,mBSCode.Length,long(lHourStart/24)*24);
	_bstr_t bSQL=SQL.GetString();
	try
	{
		pStatRst.CreateInstance(__uuidof(ADODB::Recordset));
		pStatRst->Open(bSQL,(ADODB::_Connection*)pTabFileCnn,ADODB:: adOpenKeyset,ADODB::adLockOptimistic,ADODB::adCmdText);
	}
	catch(_com_error e)
	{
		cout<<"Open Status table error: "<<SQL<<endl;
		e.ErrorMessage();
		e.Description();
		//�������Ӵ��󾯸�
	}

	if(pStatRst->EndOfFile)//û���Ͱ�Ĭ��ֵд��ȥ
	{
		//altered by wh,������ˮ����ֵ��Ϊ�����ݿ��ȡ
		//20070607,xiaofc,������ˮ����ֵ��Ϊ���ļ���ȡ
		//��ˮ����ֵΪ Vwater/Vtotal
		//TCHAR path[MAX_PATH];
		//::GetModuleFileName(NULL,path,MAX_PATH);
		//CString Str(path);
		//int i=Str.ReverseFind('\\');
		//CString m_FilePath=Str.Left(i+1);

		//CString m_strFilePath=m_FilePath+"FilePath.ini";
		//TCHAR temp[100];

		//float UpInitWaterContent;
		//float MidInitWaterContent;
		//float DownInitWaterContent;

		//::GetPrivateProfileString(_T("SOIL INIT"),_T("UpWaterContent"),_T("0"),temp,sizeof(temp),m_strFilePath);
		//UpInitWaterContent=_wtof(temp);
		if(UpInitWaterContent<1e-5) UpInitWaterContent=1e-5;
		//ZeroMemory(temp,sizeof(temp));

		//::GetPrivateProfileString(_T("SOIL INIT"),_T("MidWaterContent"),_T("0"),temp,sizeof(temp),m_strFilePath);
		//MidInitWaterContent=_wtof(temp);
		if(MidInitWaterContent<1e-5) MidInitWaterContent=1e-5;
		//ZeroMemory(temp,sizeof(temp));

		//::GetPrivateProfileString(_T("SOIL INIT"),_T("DownWaterContent"),_T("0"),temp,sizeof(temp),m_strFilePath);
		//DownInitWaterContent=_wtof(temp);
		if(DownInitWaterContent<1e-5) DownInitWaterContent=1e-5;
		//ZeroMemory(temp,sizeof(temp));
		
		pStatRst->AddNew();  
		//ChannelIndex, HourOffset

		//wh,2008.3.25
		vtmp = this->sccd;
		pStatRst->Fields->Item["sccd"]->Value = _variant_t(vtmp);

		tempCom.ChangeType(VT_DECIMAL);
		tempCom.ullVal=mBSCode.RegionIndex;
		pStatRst->Fields->Item["RegionIndex"]->Value=tempCom;
		tempCom.ullVal=mBSCode.Value;
		pStatRst->Fields->Item["BSValue"]->Value=tempCom;
		pStatRst->Fields->Item["BSLength"]->Value=mBSCode.Length;
		pStatRst->Fields->Item["HourOffset"]->Value=long(lHourStart/24)*24;//�����쿪ʼ

		//����ֵΪ2004��ĩ��С����ʱ��������Ĭ�ϳ�ֵ������������˵ƫ��
		//20070117��xiaofc,ͨ������������ͳ�ƶԱȣ����ڻ�����ԭ����ʼ������һ��������ˮ�Ƕ���ģ��������в���������ϴ�����ˮ���Ѻľ����Ӷ��Խ���в���Ԥ�Ƶ�Ӱ�졣
		//20070118,xiaofc,���ú�Ƥ����1967��4��1�յ�ʵ�⺬ˮ����ת��Ϊ��ռ����ȣ���Ϊ��ʼ��ˮ��
		pStatRst->Fields->Item["WLL"]->Value=0.0f;/*Ҷ���ˮ��*/
		pStatRst->Fields->Item["WLU"]->Value=myPara.AreaL*myPara.UDepth*UpInitWaterContent;//0.159;//myPara.Sita1;//+myPara.AreaL*myPara.UDepth*myPara.Sita2/15.0f;
		
		if(myPara.SlopeL<1.1e-5)//20110927,xiaofc,�����¶�̫С���в��������С������ʧ��
			pStatRst->Fields->Item["WLM"]->Value=myPara.AreaL*myPara.UDepth*MidInitWaterContent;
		else
			pStatRst->Fields->Item["WLM"]->Value=myPara.AreaL*myPara.LengthL*myPara.SlopeL*MidInitWaterContent; //0.179;//myPara.MSita1;//+myPara.AreaL*myPara.LengthL*myPara.SlopeL*myPara.MSita2/15.0f;
		
		
		pStatRst->Fields->Item["WLD"]->Value=myPara.AreaL*myPara.DDepth*DownInitWaterContent; //(myPara.DSita1+myPara.DSita2);
		pStatRst->Fields->Item["WRL"]->Value=0.0f;
		pStatRst->Fields->Item["WRU"]->Value=myPara.AreaR*myPara.UDepth*UpInitWaterContent;//0.159;//myPara.Sita1;//+myPara.AreaR*myPara.UDepth*myPara.Sita2/15.0f;
		
		if(myPara.SlopeR<1.1e-5)//20110927,xiaofc,�����¶�̫С���в��������С������ʧ��
			pStatRst->Fields->Item["WRM"]->Value=myPara.AreaR*myPara.UDepth*MidInitWaterContent;
		else
			pStatRst->Fields->Item["WRM"]->Value=myPara.AreaR*myPara.LengthR*myPara.SlopeR*MidInitWaterContent;//0.179;//myPara.MSita1;//+myPara.AreaR*myPara.LengthR*myPara.SlopeR*myPara.MSita2/15.0f;
		
		
		pStatRst->Fields->Item["WRD"]->Value=myPara.AreaR*myPara.DDepth*DownInitWaterContent;//(myPara.DSita1+myPara.DSita2);
		pStatRst->Fields->Item["WSL"]->Value=0.0f;
		pStatRst->Fields->Item["WSU"]->Value=myPara.AreaS*myPara.UDepth*UpInitWaterContent;//0.159;//myPara.Sita1;//+myPara.AreaS*myPara.UDepth*myPara.Sita2/15.0f;
		
		
		if(myPara.SlopeS<1.1e-5)//20110927,xiaofc,�����¶�̫С���в��������С������ʧ��
			pStatRst->Fields->Item["WSM"]->Value=myPara.AreaS*myPara.UDepth*MidInitWaterContent;
		else
			pStatRst->Fields->Item["WSM"]->Value=myPara.AreaS*myPara.LengthS*myPara.SlopeS*MidInitWaterContent; //0.179;//myPara.MSita1;//+myPara.AreaS*myPara.LengthS*myPara.SlopeS*myPara.MSita2/15.0f;
		
		
		
		pStatRst->Fields->Item["WSD"]->Value=myPara.AreaS*myPara.DDepth*DownInitWaterContent; //(myPara.DSita1+myPara.DSita2);

		if(myPara.AreaL<-1e-6)
		{
			pStatRst->Fields->Item["WLL"]->Value=-1.0f;
			pStatRst->Fields->Item["WLU"]->Value=-1.0f;
			pStatRst->Fields->Item["WLM"]->Value=-1.0f;
			pStatRst->Fields->Item["WLD"]->Value=-1.0f;
		}

		if(myPara.AreaR<-1e-6)
		{
			pStatRst->Fields->Item["WRL"]->Value=-1.0f;
			pStatRst->Fields->Item["WRU"]->Value=-1.0f;
			pStatRst->Fields->Item["WRM"]->Value=-1.0f;
			pStatRst->Fields->Item["WRD"]->Value=-1.0f;
		}

		if(myPara.AreaS<-1e-6)
		{
			pStatRst->Fields->Item["WSL"]->Value=-1.0f;
			pStatRst->Fields->Item["WSU"]->Value=-1.0f;
			pStatRst->Fields->Item["WSM"]->Value=-1.0f;
			pStatRst->Fields->Item["WSD"]->Value=-1.0f;
		}

		pStatRst->Update();
	}

	//����
	WLL=pStatRst->Fields->Item["WLL"]->Value;
	WLU=pStatRst->Fields->Item["WLU"]->Value;
	WLM=pStatRst->Fields->Item["WLM"]->Value;
	WLD=pStatRst->Fields->Item["WLD"]->Value;
	WRL=pStatRst->Fields->Item["WRL"]->Value;
	WRU=pStatRst->Fields->Item["WRU"]->Value;
	WRM=pStatRst->Fields->Item["WRM"]->Value;
	WRD=pStatRst->Fields->Item["WRD"]->Value;
	WSL=pStatRst->Fields->Item["WSL"]->Value;
	WSU=pStatRst->Fields->Item["WSU"]->Value;
	WSM=pStatRst->Fields->Item["WSM"]->Value;
	WSD=pStatRst->Fields->Item["WSD"]->Value;

	pStatRst->Close();
	return true;
}


void MParameters::ReadLE(BSCode mBSCode)
{
	//��LAI EPI

	CString SQL;
	//SQL.Format(_T("Select * from ( select * from tvarparameter where RegionIndex=%I64u and BSValue=%I64u and BSLength=%d and year between %d and %d minus select * from tvarparameter where RegionIndex=%I64u and BSValue=%I64u and BSLength=%d and ( (year=%d and mon<%d) or (year=%d and mon>%d) ) ) order by year,mon"),mBSCode.RegionIndex,mBSCode.Value,mBSCode.Length,YearStart,YearEnd,mBSCode.RegionIndex,mBSCode.Value,mBSCode.Length,YearStart,MonthStart,YearEnd,MonthEnd);

    //altered by weihong
	//wh,��month��Ϊ��mon��ԭ������mon
	//SQL.Format(_T("Select * from ( select * from tvarparameter where\
	//	RegionIndex=%I64u and BSValue=%I64u and BSLength=%d and year between %d and %d \
	//	minus \
	//	select * from tvarparameter where RegionIndex=%I64u and BSValue=%I64u and BSLength=%d and ( (year=%d and mon<%d) or (year=%d and mon>%d) ) ) \
	//	order by year,mon"),\
	//	mBSCode.RegionIndex,mBSCode.Value,mBSCode.Length,YearStart,YearEnd,\
	//	mBSCode.RegionIndex,mBSCode.Value,mBSCode.Length,YearStart,MonthStart,YearEnd,MonthEnd);

	SQL.Format(_T("Select * from tvarparameter where RegionIndex=%I64u and BSValue=%I64u and BSLength=%d order by mon"),mBSCode.RegionIndex,mBSCode.Value,mBSCode.Length);

	//cout<<SQL<<endl;
//	ofstream a2333;
//	a2333.open("23333.txt",ios::app);
//		wchar_t * chTest = new wchar_t[SQL.GetLength()+1]; //��̬����ռ�
//       chTest = SQL.GetBuffer(0); //��ȡCString���ݵĵ�ַ
// 
//       for(int i = 0; i<SQL.GetLength();i++){
//    a2333<<(char)*(chTest+i);
//          }
//SQL.ReleaseBuffer();    //������GetBuffer��һ��Ҫ����ReleaseBuffer�ͷţ����������ڴ�й¶
//a2333<<endl<<flush;
//a2333.close();
	_bstr_t bSQL=SQL.GetString();

	try
	{
		pStatRst.CreateInstance(__uuidof(ADODB::Recordset));
		pStatRst->Open(bSQL,(ADODB::_Connection*)pTabFileCnn,ADODB:: adOpenKeyset,ADODB::adLockOptimistic,ADODB::adCmdText);
	}
	catch(_com_error e)//�򲻿��ͽ����µ��ٴ�
	{
		//�������Ӵ��󾯸�
		cout<<"Open TVarParameter Table Error: "<<SQL<<endl;
		e.Description();
		e.ErrorMessage();
	}
	
	//���û������ȡ��Ĭ��ֵ
	if(pStatRst->EndOfFile)
	{
		LAI=1.0f;
		WaterEVA=2000.0f;//������������mm/�ꡣ
		cout<<"111111111111111111111111111111111111111"<<endl;
	}
	else//wh��ֻȡ��һ����¼��,yes��û����ȷ�����ˣ�����yearstart�Ǳ仯�ġ�
	{
		try
		{
			LAI=pStatRst->Fields->Item["LAI"]->Value;
			WaterEVA=pStatRst->Fields->Item["WaterEVA"]->Value;//wh:���waterevaΪ�գ���þ�ᱨ��ġ�
		}
		catch(...)
		{
			cout<<"Error while reading table STATUS, maybe NULL value encountered."<<endl;
		}
	}
	
	//wh���ٿ���getrsdata�е�watereva�ǲ�����������������ÿ���µ����ݶ���һ���ġ�
	EPI=WaterEVA/24.0f/365.0f/1000.0f;//ÿСʱ������������mm��Ϊm

	//wh��ע�����ϵĴ���ʽ����Ȼ��tvarparameter����ѡȡ�˺ܶ�����¼��ÿ������һ���£�����ʵֻ����һ����¼����Ϣ��Ĭ��ÿ���´������������������������Ц�ġ�
	//wh���Ժ���ԸĽ����������ķ�̯��ʽ�����Ǽ򵥵�ƽ����̯�õ�EPI���������й�ϵ��

	//20070122,xiaofc,��ΪҪ��ζ�����tvarparameter�ļ�¼����������˲��ܽ���ص�
	//pStatRst->Close();

}
//20070122,xiaofc
void MParameters::ReadLE(BSCode mBSCode,short Year,short Month)
{
	if(pStatRst->EndOfFile)
		return;

	short RstCurYear;
	short RstCurMonth;
	RstCurYear=pStatRst->Fields->Item["Year"]->Value;
	RstCurMonth=pStatRst->Fields->Item["Mon"]->Value;
	
	//cout<<Year<<"\t"<<Month<<endl;
	//cout<<RstCurYear<<"\t"<<RstCurMonth<<endl;

	//if(Year>RstCurYear || (Year==RstCurYear && Month>RstCurMonth))
	//{
	//	pStatRst->MoveNext();
	//	if(pStatRst->EndOfFile)
	//		return;
	//	LAI=pStatRst->Fields->Item["LAI"]->Value;
	//	WaterEVA=pStatRst->Fields->Item["WaterEVA"]->Value;
	//	//cout<<"WaterEVA("<<Year<<","<<Month<<")="<<WaterEVA<<endl;
	//	EPI=WaterEVA/24.0f/365.0f/1000.0f;//ÿСʱ������������mm��Ϊm
	//}
	if (Month!=RstCurMonth)
	{
		pStatRst->MoveNext();
		if(pStatRst->EndOfFile)
			pStatRst->MoveFirst();
		LAI=pStatRst->Fields->Item["LAI"]->Value;
		WaterEVA=pStatRst->Fields->Item["WaterEVA"]->Value;
		EPI=WaterEVA/24.0f/365.0f/1000.0f;//ÿСʱ������������mm��Ϊm
	}

	//20070122,xiaofc,��ΪҪ��ζ�����tvarparameter�ļ�¼����������������һ�β���ʱ�ٹص���¼��
	if(Year==YearEnd && Month==MonthEnd)
		pStatRst->Close();
	
}