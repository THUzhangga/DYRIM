#pragma once
#include "stdafx.h"
#include "datastruct.h"
#include <shlwapi.h>
#include <fstream>
#include "savestatus.h"
#include ".\YuLiang.h"
#include "CWaterYield.h"
#include <cassert>

//ֻ�ܷŵ������ˣ�Ҫ��Ȼ��֪����ν�������ָ�봫�ݣ�����cpp��ʶ��
#include "C:\Hydro_Cool\Graduation\SoftWare\src 1.0\GlacialSnow_dll\GlacialSnow\GlacialSnow.h"
#include "C:\Hydro_Cool\Graduation\SoftWare\src 1.0\GlacialSnow_dll\GlacialSnow\GlacialSnow_i.c"
CComQIPtr<ISRM,&IID_ISRM> SRM;

using namespace std;

void WaterYield::InitializeSRM(BSTR user,BSTR password,BSTR sid)
{
	HRESULT hr = SRM.CoCreateInstance(CLSID_SRM);
	assert(SUCCEEDED(hr));
	SRM->OpenOracle(user,password,sid);
}

void WaterYield::FinalizeSRM()
{
	SRM->SRMFinalize();
	SRM.Release();
}

void WaterYield::Calc(BSCode mBSCode,Para myPara)
{
	this->initialize();

	//�洢״̬��
	sStatus mStatus;
	mStatus.StatusBSCode=mBSCode;
	mStatus.E=0.0f;//StatusTime�ڵ�������mm
	mStatus.P=0.0f;//StatusTime�ڵĽ�����mm
	
	SaveStatus mSaveStatus;	
	mSaveStatus.sccd = this->sccd;
	mSaveStatus.pCnn = this->pCnn;
	mSaveStatus.initialize(mBSCode,HourStart,NumofHours,StatusTime);

	if(RainType.MakeLower()=="dayaverage")
	{
		this->PMin.pConnection = this->pCnn;
		PMin.Initialize(myPara.X,myPara.Y,HourStart,NumofHours); 
		this->YearStart = PMin.m_YearStart;
		this->MonthStart = PMin.m_MonthStart;
		this->YearEnd = PMin.m_YearEnd;
		this->MonthEnd = PMin.m_MonthEnd;
	}
	else if(RainType.MakeLower()=="timepoint")
	{
		PMinCBG.pCon = this->pCnn;
		PMinCBG.Initialize(myPara.X,myPara.Y,HourStart,NumofHours);
		this->YearStart = PMin.m_YearStart;
		this->MonthStart = PMin.m_MonthStart;
		this->YearEnd = PMin.m_YearEnd;
		this->MonthEnd = PMin.m_MonthEnd;
	}

	float TotalArea=0.0f;//Ԫ�������
	if(myPara.AreaS>0.0f) { TotalArea+=myPara.AreaS; }	
	if(myPara.AreaL>0.0f) { TotalArea+=myPara.AreaL; }
	if(myPara.AreaR>0.0f) { TotalArea+=myPara.AreaR; }

	float FR,WWMM,EM,A,PE,KSSD,KGD;
	int WM,SM;
	float R,RS,RSS,RG;
	float W=0,WU=0,WL=0,WD=0,WU1=0,WU2=0,WL2=0,WD2=0;
	float EU=0,EL=0,ED=0;//����������ʵ��������
	float AU,S,SSM,FRR;
	int D = 24*60/this->MSTEP;//һ���ڵ�ʱ����
	
	KSSD = ( 1- pow(1-(this->XAJ.KG+this->XAJ.KSS), 1/D) ) / ( 1 + XAJ.KG/XAJ.KSS );//������ʱ�γ���ϵ��
    KGD = KSSD*XAJ.KG/XAJ.KSS;//���¾���ʱ�γ���ϵ��
	WM = this->XAJ.WUM + XAJ.WLM + XAJ.WDM;
	WWMM = WM * (1+XAJ.B);
	SM = XAJ.SM;
	SSM = (1+XAJ.EX) * SM;

	S = this->XAJ.S0;
	WU = this->XAJ.WU0;
	WL = this->XAJ.WL0;
	WD = this->XAJ.WD0;
	W = WU + WL + WD;//��ǰ������ˮ��
	this->QRG[0] = this->XAJ.QRG0 * myPara.DrainingArea / this->BasinArea;
	this->QRSS[0] = this->XAJ.QRSS0 * myPara.DrainingArea / this->BasinArea;
	this->Q[0] = this->XAJ.QRS0 * myPara.DrainingArea / this->BasinArea + QRG[0] + QRSS[0];
	this->ReadLE(mBSCode);//��tvarparameter��������������ת����Сʱ��������

	float StepRain=0;//MSTEP�����ڵĽ�����mm

	//2008,wh,for srm
	/******************************************/
	if(this->SnowModelType.MakeLower()=="srm")
	{
		float Havg = (myPara.UElevation+myPara.DElevation)/2;
		SRM->SnowInitialize(myPara.X,myPara.Y,mBSCode.RegionIndex,mBSCode.Length,mBSCode.Value,A,Havg);
	}
	/******************************************/

	////////////////////////////////////////////////////////////////////////////////////////////////
	//������̵�ʱ��ѭ��
	///////////////////////////////////////////////////////////////////////////////////////////////
	for(long long i=1; i<=this->Steps; i++)
	{
		if( RainType.MakeLower() == "dayaverage" )
		{
			StepRain = PMin.P( double( HourStart + double(i) / StepsInHour ) );  
		}
		else if( RainType.MakeLower() == "timepoint" )
		{
			StepRain = PMinCBG.P( double( HourStart + double(i) / StepsInHour ) );
		}

		//2008,wanghao,for srm
		/******************************************/
		if(this->SnowModelType.MakeLower()=="srm")
		{
			SRM->SnowCalc(long( HourStart + i/StepsInHour ),&StepRain);
		}
		/******************************************/

		mStatus.P += StepRain*1000;//mm
		EM = this->StepEPI( float(HourStart + float(i) / StepsInHour), StepRain );//�õ�ʱ����������
		PE = StepRain - EM;

		//����ˮ������������
		if( PE <= 0 )
			R = 0;
		else
		{
			A = WWMM*( 1 - pow(1-W/WM, 1/(1+XAJ.B)) );
			if( PE + A >= WWMM )
			{
				R = PE - ( WM - W );//ˮ������������������������
			}
			else
			{
				R = PE-((WM-W)-WM*pow(1-(PE+A)/WWMM, 1+XAJ.B));
			}
		}

		//������ɢ������
		WU1 = WU + StepRain;
		if(WU1 >= EM)
		{
			EU = EM;
			EL = ED = 0;
		}
		else
		{
			EU = WU1;
			if( WL / XAJ.WLM >= XAJ.C )
			{
				EL = (EM - EU) * WL / XAJ.WLM;
				ED = 0;
			}
			else
			{
				if(WL >= XAJ.C*(EM - EU)) { EL = XAJ.C * (EM - EU);  ED = 0; }

				else{ EL = WL; ED = XAJ.C*(EM - EU) - EL; } 
			}
		}

		mStatus.E += ( EU + EL + ED);

		//������ˮ������
		WU2 = WU1 - EU - R;

		if( WU2 >= XAJ.WUM )/*�������*/
		{
			WU = XAJ.WUM;
			WL2 = WL - EL + ( WU2 - XAJ.WUM);
		}
		else
		{
			WU = WU2;
			WL2 = WL - EL;
		}
		if( WL2 >= XAJ.WLM)/*�в�����*/
		{
			WL = XAJ.WLM;
			WD2 = WD - ED + ( WL2 - XAJ.WLM );
		}
		else
		{
			WL = WL2;
			WD2 = WD - ED;
		}
		if( WD2 >= XAJ.WDM)/*�������*/
		{
			WD = XAJ.WDM;//��һ����ˮ��ʧ�����²㣬���Բ���
		}
		else
		{
			WD = WD2;
		}
		W = WU + WL + WD;

		//�����������
		AU = SSM * ( 1-pow(1-S/SM, 1/(1+XAJ.EX)) );//������ˮ��ˮ��S��Ӧ����ˮ��������������
		FR = 1-pow(1-W/WM, XAJ.B/(1+XAJ.B));
		if( PE<=0 )
		{
			RS = 0;
			RSS = S*KSSD*FR;
			RG = S*KGD*FR;
			S = (1-KSSD-KGD) * S;
		}
		else
		{
			FRR = R / PE;//ʱ������ƽ���������
			if( PE+AU<SSM )
			{
				RS = ( PE - SM + S + SM * pow(1-(PE+AU)/SSM, 1 + XAJ.EX) ) * FRR;
				RSS = ( SM - SM * pow(1-(PE+AU)/SSM, 1 + XAJ.EX) ) * FRR * KSSD;
				RG = ( SM - SM * pow(1-(PE+AU)/SSM, 1 + XAJ.EX) ) * FRR * KGD;
				S = ( 1-KSSD-KGD ) * ( SM - SM * pow(1-(PE+AU)/SSM, 1+XAJ.EX) );
			}
			else
			{
				RS = ( PE - SM + S ) * FRR;
				RSS = SM * KSSD * FRR;
				RG = SM * KGD * FRR;
				S = ( 1-KSSD-KGD ) * SM;
			}
		}

		//�����������
		QRSS[i] = QRSS[i-1] * pow(XAJ.KKSS, 1/D) + RSS * ( 1-pow(XAJ.KKSS, 1/D) ) * TotalArea / 1000 / MSTEP / 60;
		QRG[i] = QRG[i-1] * pow(XAJ.KKG, 1/D) + RG * ( 1-pow(XAJ.KKG, 1/D) ) * TotalArea / 1000 / MSTEP / 60;
		Q[i] = QRSS[i] + QRG[i] + RS * TotalArea * (1-XAJ.IMP)/1000/MSTEP/60 + ( PE>0 ? PE:0 ) * TotalArea * XAJ.IMP /1000/MSTEP/60;

		//if(RS>5) { cout<<RS<<"mm"<<endl;} 

		//���WҪ�����Ǻ�ˮ��:ˮ��/ɳ��*100,%֮ǰ�Ĳ���,����40,������40%=0.4
		pWLM[i]=W*myPara.AreaL/(myPara.AreaL*myPara.SlopeL*myPara.LengthL/2.0f*(1-myPara.MSita1-myPara.MSita2)*2650)*100;
		pWRM[i]=W*myPara.AreaR/(myPara.AreaR*myPara.SlopeR*myPara.LengthR/2.0f*(1-myPara.MSita1-myPara.MSita2)*2650)*100;

		if(	i/StepsInHour>=StatusTime && i%StepsInHour==0 && (i/StepsInHour)%StatusTime==0 )//wh,20060406,��status��Ĵ洢�������ã�1��1����ʱ�������ܴ�
		{
			mStatus.HourOffset=i/StepsInHour+HourStart;

			mStatus.W = W / 1000 * TotalArea;//������ǰ��ˮ��m^3
			mStatus.QRG = QRG[i];//m^3/s
			mStatus.QRSS = QRSS[i];//m^3/s

			mSaveStatus.DoSave(&mStatus);

			mStatus.E=0.0f;//statustime�ڵ�������mm
			mStatus.P=0.0f;//statustime�ڵĽ�����mm
		}

		if( !_finite(Q[i]) )
		{
			cout<<"INFINITE �°���ģ�� Yield Result:"<<endl;
			cout<<"RegionIndex="<<mBSCode.RegionIndex<<"\tBSValue="<<mBSCode.Value<<"\tBSLength="<<mBSCode.Length<<endl;
			cout<<"t="<<i<<endl;
			cout<<"QRSS[t]="<<QRSS[i]<<"\tQRG[t]="<<QRG[i]<<"\tQ[t]="<<Q[i]<<endl;
			exit(0);
		}
	
	}//ʱ��ѭ������

	mSaveStatus.finalize();

	if( RainType.MakeLower() == "dayaverage" )
	{
		PMin.DeleteList();
	}
	else if( RainType.MakeLower() == "timepoint" )
	{
		PMinCBG.DeleteList();
	}

	//for srm��һ���Ӷ�ִ��һ��
	if(this->SnowModelType.MakeLower()=="srm")
	{
		SRM->ReleaseHeap();
	}

	this->finalize();
}


//�õ�ÿСʱ����������mm
void WaterYield::ReadLE(BSCode mBSCode)
{
	//��LAI EPI
	ADODB::_RecordsetPtr pStatRst;//����tvarparameter��
	CString SQL;
	SQL.Format(_T("Select * from ( select * from tvarparameter where\
		RegionIndex=%I64u and BSValue=%I64u and BSLength=%d and year between %d and %d \
		minus \
		select * from tvarparameter where RegionIndex=%I64u and BSValue=%I64u and BSLength=%d and ( (year=%d and mon<%d) or (year=%d and mon>%d) ) ) \
		order by year,mon"),\
		mBSCode.RegionIndex,mBSCode.Value,mBSCode.Length,YearStart,YearEnd,\
		mBSCode.RegionIndex,mBSCode.Value,mBSCode.Length,YearStart,MonthStart,YearEnd,MonthEnd);

	_bstr_t bSQL=SQL.GetString();

	try
	{
		pStatRst.CreateInstance(__uuidof(ADODB::Recordset));
		pStatRst->Open(bSQL,(ADODB::_Connection*)pCnn,ADODB:: adOpenKeyset,ADODB::adLockOptimistic,ADODB::adCmdText);
	}
	catch(_com_error e)
	{
		cout<<"Open TVarParameter Table Error: "<<SQL<<endl;
		e.Description();
		e.ErrorMessage();
	}
	
	//���û������ȡ��Ĭ��ֵ
	float WaterEVA;
	if( pStatRst->EndOfFile )
	{
		WaterEVA = 2000.0f;//������������mm/�ꡣ
	}
	else
	{
		try
		{
			WaterEVA=pStatRst->Fields->Item["WaterEVA"]->Value;
		}
		catch(...)
		{
			cout<<"Error while reading table TVarParameter, maybe NULL value encountered."<<endl;
		}
	}

	AvgEPI = WaterEVA/24.0f/365.0f;//ÿСʱ������������mm
	pStatRst->Close();
}


//�õ�������������
float WaterYield::StepEPI(float HourOffset, float P)
{
	//��ƽ��Ǳ������ת��Ϊ��Ӧʱ�䲽���ϵ����ҷֲ����ֵ
	float HourInDay;
	float EPI;//Сʱ���������������������߷����
	HourInDay = long(floor(HourOffset)) % 24 + HourOffset - floor(HourOffset);

	if( HourInDay<6 || HourInDay>=18 || P>0 )
	{
		EPI=0.0f;
	}
	else
	{
		//��������ƽ�������ԭ�򣬻��ֺ���12��Сʱ�ڵ���������Ϊ24*AvgEPI��ƽ��ÿСʱΪ2*AvgEPI��ά�������������ܱ仯
		EPI=AvgEPI*sin((HourInDay-6)*PI/12)*PI;
	}
	
	return EPI/60*MSTEP;
}


void WaterYield::initialize(void)
{
	QRSS = new float[Steps+1];
	QRG = new float[Steps+1];
}


void WaterYield::finalize(void)
{
	delete[] QRSS;
	delete[] QRG;
}
