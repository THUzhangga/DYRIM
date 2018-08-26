#pragma once
#include "stdafx.h"
#include "datastruct.h"
#include <shlwapi.h>
#include <fstream>
#include "savestatus.h"
#include ".\YuLiang.h"
#include "CWaterYield.h"
#include <cassert>

//只能放到这里了，要不然不知道如何进行智能指针传递（两个cpp都识别）
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

	//存储状态用
	sStatus mStatus;
	mStatus.StatusBSCode=mBSCode;
	mStatus.E=0.0f;//StatusTime内的蒸发量mm
	mStatus.P=0.0f;//StatusTime内的降雨量mm
	
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

	float TotalArea=0.0f;//元流域面积
	if(myPara.AreaS>0.0f) { TotalArea+=myPara.AreaS; }	
	if(myPara.AreaL>0.0f) { TotalArea+=myPara.AreaL; }
	if(myPara.AreaR>0.0f) { TotalArea+=myPara.AreaR; }

	float FR,WWMM,EM,A,PE,KSSD,KGD;
	int WM,SM;
	float R,RS,RSS,RG;
	float W=0,WU=0,WL=0,WD=0,WU1=0,WU2=0,WL2=0,WD2=0;
	float EU=0,EL=0,ED=0;//各层土壤的实际蒸发量
	float AU,S,SSM,FRR;
	int D = 24*60/this->MSTEP;//一天内的时段数
	
	KSSD = ( 1- pow(1-(this->XAJ.KG+this->XAJ.KSS), 1/D) ) / ( 1 + XAJ.KG/XAJ.KSS );//壤中流时段出流系数
    KGD = KSSD*XAJ.KG/XAJ.KSS;//地下径流时段出流系数
	WM = this->XAJ.WUM + XAJ.WLM + XAJ.WDM;
	WWMM = WM * (1+XAJ.B);
	SM = XAJ.SM;
	SSM = (1+XAJ.EX) * SM;

	S = this->XAJ.S0;
	WU = this->XAJ.WU0;
	WL = this->XAJ.WL0;
	WD = this->XAJ.WD0;
	W = WU + WL + WD;//当前土壤蓄水量
	this->QRG[0] = this->XAJ.QRG0 * myPara.DrainingArea / this->BasinArea;
	this->QRSS[0] = this->XAJ.QRSS0 * myPara.DrainingArea / this->BasinArea;
	this->Q[0] = this->XAJ.QRS0 * myPara.DrainingArea / this->BasinArea + QRG[0] + QRSS[0];
	this->ReadLE(mBSCode);//将tvarparameter表中年蒸发能力转化成小时蒸发能力

	float StepRain=0;//MSTEP分钟内的降雨量mm

	//2008,wh,for srm
	/******************************************/
	if(this->SnowModelType.MakeLower()=="srm")
	{
		float Havg = (myPara.UElevation+myPara.DElevation)/2;
		SRM->SnowInitialize(myPara.X,myPara.Y,mBSCode.RegionIndex,mBSCode.Length,mBSCode.Value,A,Havg);
	}
	/******************************************/

	////////////////////////////////////////////////////////////////////////////////////////////////
	//计算过程的时间循环
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
		EM = this->StepEPI( float(HourStart + float(i) / StepsInHour), StepRain );//得到时段蒸发能力
		PE = StepRain - EM;

		//张力水蓄满产流计算
		if( PE <= 0 )
			R = 0;
		else
		{
			A = WWMM*( 1 - pow(1-W/WM, 1/(1+XAJ.B)) );
			if( PE + A >= WWMM )
			{
				R = PE - ( WM - W );//水面蒸发按照蒸发能力来蒸发
			}
			else
			{
				R = PE-((WM-W)-WM*pow(1-(PE+A)/WWMM, 1+XAJ.B));
			}
		}

		//土壤蒸散发计算
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

		//土壤蓄水量计算
		WU2 = WU1 - EU - R;

		if( WU2 >= XAJ.WUM )/*表层土壤*/
		{
			WU = XAJ.WUM;
			WL2 = WL - EL + ( WU2 - XAJ.WUM);
		}
		else
		{
			WU = WU2;
			WL2 = WL - EL;
		}
		if( WL2 >= XAJ.WLM)/*中层土壤*/
		{
			WL = XAJ.WLM;
			WD2 = WD - ED + ( WL2 - XAJ.WLM );
		}
		else
		{
			WL = WL2;
			WD2 = WD - ED;
		}
		if( WD2 >= XAJ.WDM)/*深层土壤*/
		{
			WD = XAJ.WDM;//有一部分水流失到更下层，忽略不计
		}
		else
		{
			WD = WD2;
		}
		W = WU + WL + WD;

		//流域产流计算
		AU = SSM * ( 1-pow(1-S/SM, 1/(1+XAJ.EX)) );//与自由水蓄水量S对应的蓄水容量曲线纵坐标
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
			FRR = R / PE;//时段流域平均产流面积
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

		//流域汇流计算
		QRSS[i] = QRSS[i-1] * pow(XAJ.KKSS, 1/D) + RSS * ( 1-pow(XAJ.KKSS, 1/D) ) * TotalArea / 1000 / MSTEP / 60;
		QRG[i] = QRG[i-1] * pow(XAJ.KKG, 1/D) + RG * ( 1-pow(XAJ.KKG, 1/D) ) * TotalArea / 1000 / MSTEP / 60;
		Q[i] = QRSS[i] + QRG[i] + RS * TotalArea * (1-XAJ.IMP)/1000/MSTEP/60 + ( PE>0 ? PE:0 ) * TotalArea * XAJ.IMP /1000/MSTEP/60;

		//if(RS>5) { cout<<RS<<"mm"<<endl;} 

		//这个W要给的是含水量:水重/沙重*100,%之前的部分,即是40,而不是40%=0.4
		pWLM[i]=W*myPara.AreaL/(myPara.AreaL*myPara.SlopeL*myPara.LengthL/2.0f*(1-myPara.MSita1-myPara.MSita2)*2650)*100;
		pWRM[i]=W*myPara.AreaR/(myPara.AreaR*myPara.SlopeR*myPara.LengthR/2.0f*(1-myPara.MSita1-myPara.MSita2)*2650)*100;

		if(	i/StepsInHour>=StatusTime && i%StepsInHour==0 && (i/StepsInHour)%StatusTime==0 )//wh,20060406,将status表的存储周期外置，1天1次有时数据量很大。
		{
			mStatus.HourOffset=i/StepsInHour+HourStart;

			mStatus.W = W / 1000 * TotalArea;//土壤当前蓄水量m^3
			mStatus.QRG = QRG[i];//m^3/s
			mStatus.QRSS = QRSS[i];//m^3/s

			mSaveStatus.DoSave(&mStatus);

			mStatus.E=0.0f;//statustime内的蒸发量mm
			mStatus.P=0.0f;//statustime内的降雨量mm
		}

		if( !_finite(Q[i]) )
		{
			cout<<"INFINITE 新安江模型 Yield Result:"<<endl;
			cout<<"RegionIndex="<<mBSCode.RegionIndex<<"\tBSValue="<<mBSCode.Value<<"\tBSLength="<<mBSCode.Length<<endl;
			cout<<"t="<<i<<endl;
			cout<<"QRSS[t]="<<QRSS[i]<<"\tQRG[t]="<<QRG[i]<<"\tQ[t]="<<Q[i]<<endl;
			exit(0);
		}
	
	}//时间循环结束

	mSaveStatus.finalize();

	if( RainType.MakeLower() == "dayaverage" )
	{
		PMin.DeleteList();
	}
	else if( RainType.MakeLower() == "timepoint" )
	{
		PMinCBG.DeleteList();
	}

	//for srm，一条河段执行一次
	if(this->SnowModelType.MakeLower()=="srm")
	{
		SRM->ReleaseHeap();
	}

	this->finalize();
}


//得到每小时的蒸发能力mm
void WaterYield::ReadLE(BSCode mBSCode)
{
	//读LAI EPI
	ADODB::_RecordsetPtr pStatRst;//连接tvarparameter表
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
	
	//如果没有数据取的默认值
	float WaterEVA;
	if( pStatRst->EndOfFile )
	{
		WaterEVA = 2000.0f;//年蒸发能力，mm/年。
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

	AvgEPI = WaterEVA/24.0f/365.0f;//每小时的蒸发能力，mm
	pStatRst->Close();
}


//得到步长蒸发能力
float WaterYield::StepEPI(float HourOffset, float P)
{
	//将平均潜在蒸发转化为相应时间步长上的正弦分布后的值
	float HourInDay;
	float EPI;//小时蒸发能力（按照正弦曲线分配后）
	HourInDay = long(floor(HourOffset)) % 24 + HourOffset - floor(HourOffset);

	if( HourInDay<6 || HourInDay>=18 || P>0 )
	{
		EPI=0.0f;
	}
	else
	{
		//正弦曲线平均分配的原则，积分后在12个小时内的蒸发总量为24*AvgEPI，平均每小时为2*AvgEPI，维持蒸发总量不能变化
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
