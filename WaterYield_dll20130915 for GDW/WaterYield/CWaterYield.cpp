#pragma once
#include "stdafx.h"
#include "datastruct.h"
#include "MParameters.h"
#include "Leaf.h"
#include "MidSoil.h"
#include "DeepSoil.h"
#include "SurfaceWaterA.h"
#include ".\vapotranspiration.h"
//#include "Trans_simple.h"
#include <shlwapi.h>
#include <fstream>
#include <vector>
#include "savestatus.h"
#include ".\YuLiang.h"
#include "CWaterYield.h"
#include <cassert>

#include "E:\models\GlacialSnow_dll\GlacialSnow\GlacialSnow.h"
#include "E:\models\GlacialSnow_dll\GlacialSnow\GlacialSnow_i.c"
CComQIPtr<ISRM,&IID_ISRM> SRM;

using namespace std;

//wh,for srm
void WaterYield::InitializeSRM(BSTR user,BSTR password,BSTR sid)
{
	HRESULT hr = SRM.CoCreateInstance(CLSID_SRM);
	assert(SUCCEEDED(hr));
	SRM->OpenOracle(user,password,sid);
}

//wh,for srm
void WaterYield::FinalizeSRM()
{
	SRM->SRMFinalize();
	SRM.Release();
}

void WaterYield::Calc(BSCode mBSCode,Para myPara)
{
	//20090921,xiaofc, add the outputs of time consuming
	double CalTime;
	double RainfallTime;
	double SSTime;
	double PreTime;
	double NowTime;
	
	CalTime=0;
	RainfallTime=0;
	SSTime=0;
	
	//TCL start
	PreTime=GetTickCount();

	this->initialize();//wh

	float waterdown; //能够逐层向下传递的水量
	//float srate;

	//CTrans_Simple InnerTrans;

	Evapotranspiration cEvaptransp(emethod, thetab, thetaw, N, E0_a,MSTEP);

	//altered by weihong
	MParameters paraS(UpInitWaterContent, MidInitWaterContent, DownInitWaterContent,sccd);
	MParameters paraL(UpInitWaterContent, MidInitWaterContent, DownInitWaterContent,sccd);
	MParameters paraR(UpInitWaterContent, MidInitWaterContent, DownInitWaterContent,sccd);

	Leaf cLeafS;
	Leaf cLeafL;
	Leaf cLeafR;

	SurfaceWaterA surfaceS(MSTEP,this->SEEquation);
	SurfaceWaterA surfaceL(MSTEP,this->SEEquation);
	SurfaceWaterA surfaceR(MSTEP,this->SEEquation);

	SoilWater soilS(MSTEP);
	SoilWater soilL(MSTEP);
	SoilWater soilR(MSTEP);

	MidSoil midS(MSTEP);
	MidSoil midL(MSTEP);
	MidSoil midR(MSTEP);

	DeepSoil deepS;
	DeepSoil deepL;
	DeepSoil deepR;

	//记录计算的天数和结束的日期
	long DayCount=0;
	short YearEnd,MonthEnd,DayEnd;

	//每小时的雨量
	//引入MSTEP之后，HourRain实际上已经是代表MSTEP分钟内的降雨量mm
	float HourRain;

	float XYpara;//增加CMORPH分辨率参数,shy,20130905

	//TCL End
	//TDB:rainfall Start
	NowTime=GetTickCount();
	CalTime+=NowTime-PreTime;
	PreTime=NowTime;

	//读入降雨量序列
	//20050109 李铁键 增加对不同降雨量数据类型的判断
	//wh，此时的代码要求经纬度坐标，因为里面有坐标转化函数，稍微改下riversegs也可以用大地坐标。
	//20090918,xiaofc,降雨量初始化高度依赖于数据库效率
	if(RainType=="DayAverage")
	{
		this->PMin.pConnection = this->pCnn;
		PMin.Initialize(myPara.X,myPara.Y,HourStart,NumofHours); //直接用YuLiangMin来取得降雨量,最后一个参数是计算历时Hours，由用户给

		//记录开始时间20051109byiwish
		//这个用到随月更新的参数的读取中，可能有问题，但现在还没有用。20060109，李铁键
		//20070122,更新此部分功能为有效
		YearEnd=PMin.m_YearStart;
		MonthEnd=PMin.m_MonthStart;
		DayEnd=PMin.m_DayStart;

		paraS.YearEnd=PMin.m_YearEnd;
		paraS.MonthEnd=PMin.m_MonthEnd;

		paraS.YearStart=PMin.m_YearStart;
		paraS.MonthStart=PMin.m_MonthStart;
	}
	else if(RainType=="TimePoint")
	{
		PMinCBG.pCon = this->pCnn;
		PMinCBG.Initialize(myPara.X,myPara.Y,HourStart,NumofHours);

		//这个用到随月更新的参数的读取中，可能有问题，但现在还没有用。20060109，李铁键
		//20070122,更新此部分功能为有效
		YearEnd=PMinCBG.m_YearStart;
		MonthEnd=PMinCBG.m_MonthStart;
		DayEnd=PMinCBG.m_DayStart;

		paraS.YearEnd=PMinCBG.m_YearEnd;
		paraS.MonthEnd=PMinCBG.m_MonthEnd;

		paraS.YearStart=PMinCBG.m_YearStart;
		paraS.MonthStart=PMinCBG.m_MonthStart;
		paraR.YearEnd=PMinCBG.m_YearEnd;
		paraR.MonthEnd=PMinCBG.m_MonthEnd;

		paraR.YearStart=PMinCBG.m_YearStart;
		paraR.MonthStart=PMinCBG.m_MonthStart;
		paraL.YearEnd=PMinCBG.m_YearEnd;
		paraL.MonthEnd=PMinCBG.m_MonthEnd;

		paraL.YearStart=PMinCBG.m_YearStart;
		paraL.MonthStart=PMinCBG.m_MonthStart;
	}
	else if(RainType=="CMORPH8")//20130904, shy,add CMORPH8
	{
		PMinCMORPH.pCon = this->pCnn;
		XYpara=0.07277;
		PMinCMORPH.Initialize(myPara.X,myPara.Y,HourStart,NumofHours,XYpara);

		YearEnd=PMinCMORPH.m_YearStart;
		MonthEnd=PMinCMORPH.m_MonthStart;
		DayEnd=PMinCMORPH.m_DayStart;

		paraS.YearEnd=PMinCMORPH.m_YearEnd;
		paraS.MonthEnd=PMinCMORPH.m_MonthEnd;

		paraS.YearStart=PMinCMORPH.m_YearStart;
		paraS.MonthStart=PMinCMORPH.m_MonthStart;
	}
	else if(RainType=="CMORPH25")//20130905, shy,add CMORPH25
	{
		PMinCMORPH.pCon = this->pCnn;
		XYpara=0.25;
		PMinCMORPH.Initialize(myPara.X,myPara.Y,HourStart,NumofHours,XYpara);

		YearEnd=PMinCMORPH.m_YearStart;
		MonthEnd=PMinCMORPH.m_MonthStart;
		DayEnd=PMinCMORPH.m_DayStart;

		paraS.YearEnd=PMinCMORPH.m_YearEnd;
		paraS.MonthEnd=PMinCMORPH.m_MonthEnd;

		paraS.YearStart=PMinCMORPH.m_YearStart;
		paraS.MonthStart=PMinCMORPH.m_MonthStart;
	}

	//TDB: rainfall End
	//TDB: status start
	NowTime=GetTickCount();
	RainfallTime+=NowTime-PreTime;
	PreTime=NowTime;

	int j=1;

	paraS.lHourStart=HourStart;//计算开始时间
	paraS.pTabFileCnn=pCnn;
	paraS.GetParameters(&myPara,'S',mBSCode);//myPara由NewRouting.exe传入
	paraL.lHourStart=HourStart;
	paraL.pTabFileCnn=pCnn;
	paraL.GetParameters(&myPara,'L',mBSCode);
	paraR.lHourStart=HourStart;
	paraR.pTabFileCnn=pCnn;
	paraR.GetParameters(&myPara,'R',mBSCode);
	
	//20070122,xiaofc,第一次读取LAI & E0
	paraS.ReadLE(mBSCode);
	paraL.ReadLE(mBSCode);
	paraR.ReadLE(mBSCode);


	if(mBSCode.RegionIndex==1122012001 & mBSCode.Value==0 & mBSCode.Length==940)
		int xxx=0;

	ofstream myFileslope;
	if (mBSCode.RegionIndex==0 && mBSCode.Length==4 && mBSCode.Value==0)
	{		
		 myFileslope.open("example3.txt",ios::trunc);
        if (myFileslope.is_open()) {
            myFileslope << "Cao Ping Sediment and Flow discharge on Hillslope recording.\n";
			myFileslope.precision(18);
        }

	}
	//TDB: status End
	//TCL start
	NowTime=GetTickCount();
	SSTime+=NowTime-PreTime;
	PreTime=NowTime;

	//20070116,xiaofc
	float TotalArea;//三个坡面的总面积
	TotalArea=0.0f;
	if(paraS.Area>0.0f)
		TotalArea+=paraS.Area;
	if(paraL.Area>0.0f)
		TotalArea+=paraL.Area;
	if(paraR.Area>0.0f)
		TotalArea+=paraR.Area;

	//初始化各区对象
	cLeafS.initialize(&myPara,&paraS,'S');
	cLeafL.initialize(&myPara,&paraL,'L');
	cLeafR.initialize(&myPara,&paraR,'R');
	cLeafL.isDebug=isDebug;

	surfaceS.initialize(paraS);
	surfaceL.initialize(paraL);
	surfaceR.initialize(paraR);
	surfaceL.isDebug=isDebug;
	
	soilS.initialize(paraS);
	soilL.initialize(paraL);
	soilR.initialize(paraR);
	
	midS.initialize(paraS);
	midL.initialize(paraL);
	midR.initialize(paraR);

	deepS.initialize(paraS);
	deepL.initialize(paraL);
	deepR.initialize(paraR);

	//存储状态用
	long DayIndex;
	sStatus *mStatus;//一个结构体,对应status表的项
	DayCount=NumofHours/StatusTime+1;
	mStatus=new sStatus[DayCount];
	mStatus[1].StatusBSCode=mBSCode;//20051109byiwish
	mStatus[1].E=0.0f;//日蒸发
	mStatus[1].P=0.0f;//日降雨量
	mStatus[1].SlopeErosion = 0.0f;//日坡面侵蚀量
	
		
	//altered by wh
	cEvaptransp.initialize(&paraS);

	ssT[0]=0.0f;
	qsS[0]=0.0f;
	qsL[0]=0.0f;
	qsR[0]=0.0f;
	qsT[0]=0.0f;

	
	//2008,wh,for srm
	if(this->SnowModelType.MakeLower()=="srm")
	{
		float Havg = (myPara.UElevation+myPara.DElevation)/2;
		SRM->SnowInitialize(myPara.X,myPara.Y,mBSCode.RegionIndex,mBSCode.Length,mBSCode.Value,myPara.A,Havg);
	}

	////////////////////////////////////////////////////////////////////////////////////////////////
	//计算过程的时间循环
	///////////////////////////////////////////////////////////////////////////////////////////////
	for(long long i=1; i<=NumofHours*StepsInHour; i++)
	{
		if (i==8280)
			i=i;
		DayIndex=(i-1)/StepsInHour/StatusTime+1;//when i=240, DayIndex is still 1 to save for the 1st day
		try
		{
			//降雨量
			//20050109 李铁键 增加对不同降雨量数据类型的判断
			if(RainType=="DayAverage")
				HourRain=PMin.P(double(HourStart+double(i)/StepsInHour));  //直接用YuLiangMin来取得降雨量
			else if(RainType=="TimePoint")
				HourRain=PMinCBG.P(double(HourStart+double(i)/StepsInHour));//得到步长雨量
			else if(RainType=="CMORPH8" || RainType=="CMORPH25")
				HourRain=PMinCMORPH.P(double(HourStart+double(i)/StepsInHour));//得到步长雨量
			
			//2008,wanghao,for srm
			if(this->SnowModelType.MakeLower()=="srm")
			{
				SRM->SnowCalc(long( HourStart + i/StepsInHour ),&HourRain);
			}

			
			mStatus[DayIndex].P += HourRain*1000;//20070202,xiaofc累加每个时间步长的P,mm//HourRain本身是m，status中存储的单位是mm

			//if(HourRain>0)
			//	cout<<"Time="<<double(HourStart+double(i)/StepsInHour)<<"; HourRain="<<HourRain<<endl;

			//david,加入变量NPS、NPL、NPR分别用于输出源坡面、左坡面和右坡面的净雨量（net precipitation）
			//david,加入变量WPS、WPL、WPR分别用于输出源坡面、左坡面和右坡面的渗入表层土的水深（WaterPenetrated）
			float NPS,NPL,NPR,WPS,WPL,WPR,P1,P2,P3;

			if(abs(paraS.Area+1)>1e-5)//源
			{

				//20070128,xiaofc: cEvaptransp对象在左右源坡面上重复使用，在para中关系到各个初始量，
				//因而，并不是像原来那样所认为的，S,L,R任意一个都行
				//cEvaptransp.initialize(&paraS,double(HourStart+double(i)/StepsInHour),HourRain);
				//NextHour实有下一个时间步长，下同
				
				//wh解读：先处理蒸发，把叶面、土壤的含水量都调整一番
				cEvaptransp.NextHour(&cLeafS,&soilS,&midS,&deepS,double(HourStart+double(i)/StepsInHour),HourRain);
				mStatus[DayIndex].E+=cEvaptransp.E*paraS.Area/TotalArea;//20070116,xiaofc//单位mm

				//出流量赋初值0
				qsS[i]=0;
				P1=HourRain;

				//植被截流//此时waterdown为透过叶面的净雨
				waterdown=cLeafS.NextHour(HourRain);//水深,m

				//david,上一步的waterdown即为净雨深
				NPS=waterdown;

				//srate=soilS.GetSRate();
				waterdown=surfaceS.NextHour(waterdown,paraS,soilS.Sita,soilS.W,soilS.Wmax2);//这里就变为m^3,水量

				//david,输出surface中的变量,Area竟然是个私有成员
				WPS=surfaceS.WaterPenetrated;

				//waterdown=surfaceS.WaterPenetrated;
				qsS[i]+=surfaceS.WaterY;
				ssS[i]=surfaceS.SedimentYield;
				//20080303,xiaofc,将坡面侵蚀量保存到status表, kg
			    mStatus[DayIndex].SlopeErosion+=surfaceS.SedimentYield*MSTEP*60;

				//srate=midS.GetSRate();
				waterdown=soilS.NextHour(waterdown,paraS,midS.Sita,midS.W,midS.Wmax2);//waterdown从地表径流进入表层土的水量
				qsS[i]+=soilS.Wout;

				waterdown=midS.NextHour(waterdown,paraS,deepS.Sita,deepS.DSW,deepS.Wmax2);
				qsS[i]+=midS.Wout;

				deepS.NextHour(waterdown,midS.imd);

				//将出流 m^3/h 转为 m^3/s
				qsS[i]/=3600.0f/StepsInHour;

			}
			else
			{
				qsS[0]=-1;
				NPS=-1;
			}

			//cout<<"WYL2"<<endl;
			//左和右
			if(abs(paraL.Area+1)>1e-5)
			{
				//cEvaptransp.initialize(&paraL,double(HourStart+double(i)/StepsInHour),HourRain);
				cEvaptransp.NextHour(&cLeafL,&soilL,&midL,&deepL,double(HourStart+double(i)/StepsInHour),HourRain);//gao+CString EMethod
				mStatus[DayIndex].E+=cEvaptransp.E*paraL.Area/TotalArea;//20070116,xiaofc


				//出流量赋初值0
				qsL[i]=0;
				P2=HourRain;

				//植被截流
				waterdown=cLeafL.NextHour(HourRain);//水深,m

			    //david,上一步的waterdown即为净雨深
				NPL=waterdown;

				if(isDebug)
					cout<<"HourRain="<<HourRain<<"\twaterdown="<<waterdown<<endl;

				//srate=soilL.GetSRate();
				waterdown=surfaceL.NextHour(waterdown,paraL,soilL.Sita,soilL.W,soilL.Wmax2);

			    //david,输出surface中的变量,Area竟然是个私有成员
				WPL=surfaceL.WaterPenetrated;

				//waterdown=surfaceL.WaterPenetrated;
				qsL[i]+=surfaceL.WaterY;
				if(isDebug)
					cout<<"SurfaceOutL="<<surfaceL.WaterY<<endl;

				ssL[i]=surfaceL.SedimentYield;
				//20080303,xiaofc,将坡面侵蚀量保存到status表, kg
			    mStatus[DayIndex].SlopeErosion+=surfaceL.SedimentYield*MSTEP*60;

				//srate=midL.GetSRate();
				waterdown=soilL.NextHour(waterdown,paraL,midL.Sita,midL.W,midL.Wmax2);
				qsL[i]+=soilL.Wout;

				if(isDebug)
					cout<<"SoilOutL="<<soilL.Wout<<endl;

				waterdown=midL.NextHour(waterdown,paraL,deepL.Sita,deepL.DSW,deepL.Wmax2);
				qsL[i]+=midL.Wout;

				if(isDebug)
					cout<<"imd=\t"<<midL.imd<<"\t"<<"midw=\t"<<midL.W<<"\t"<<"midout=\t"<<midL.Wout<<"\t";

				deepL.NextHour(waterdown,midL.imd);
				//qsL[i]+=deepL.Wout;

				//将出流 m^3/h 转为 m^3/s
				qsL[i]/=3600.0f/StepsInHour;

				if(isDebug)
					cout<<"qsL[i]="<<qsL[i]<<endl;

			}
			else
			{
				qsL[0]=-1;
				NPL=-1;
			}

			if(abs(paraR.Area+1)>1e-5)//右
			{
				//cEvaptransp.initialize(&paraR,double(HourStart+double(i)/StepsInHour),HourRain);
				cEvaptransp.NextHour(&cLeafR,&soilR,&midR,&deepR,double(HourStart+double(i)/StepsInHour),HourRain);//gao+CString EMethod
				mStatus[DayIndex].E+=cEvaptransp.E*paraR.Area/TotalArea;//20070116,xiaofc

				//出流量赋初值0
				qsR[i]=0;
				P3=HourRain;

				//植被截流
				waterdown=cLeafR.NextHour(HourRain);//水深,m

				//david,上一步的waterdown即为净雨深
				NPR=waterdown;

				//srate=soilR.GetSRate();
				waterdown=surfaceR.NextHour(waterdown,paraR,soilR.Sita,soilR.W,soilR.Wmax2);

				//david,输出surface中的变量,Area竟然是个私有成员
				WPR=surfaceR.WaterPenetrated;

				//waterdown=surfaceR.WaterPenetrated;
				qsR[i]+=surfaceR.WaterY;
				ssR[i]=surfaceR.SedimentYield;
				//20080303,xiaofc,将坡面侵蚀量保存到status表, kg
			    mStatus[DayIndex].SlopeErosion+=surfaceR.SedimentYield*MSTEP*60;

				//srate=midR.GetSRate();
				waterdown=soilR.NextHour(waterdown,paraR,midR.Sita,midR.W,midR.Wmax2);
				qsR[i]+=soilR.Wout;


				waterdown=midR.NextHour(waterdown,paraR,deepR.Sita,deepR.DSW,deepR.Wmax2);
				qsR[i]+=midR.Wout;

				deepR.NextHour(waterdown,midR.imd);
				//qsR[i]+=deepR.Wout;

				//将出流 m^3/时段 转为 m^3/s
				qsR[i]/=3600.0f/StepsInHour;

			}
			else
			{
				qsR[0]=-1;
				NPR=-1;
			}

			//记录WLM WRM值序列,
			//20060328,这个W要给的是含水量:水重/沙重*100,%之前的部分,即是40,而不是40%=0.4
			pWLM[i]=midL.W*1000/(myPara.AreaL*myPara.SlopeL*myPara.LengthL/2.0f*(1-myPara.MSita1-myPara.MSita2)*2650)*100;
			pWRM[i]=midR.W*1000/(myPara.AreaR*myPara.SlopeR*myPara.LengthR/2.0f*(1-myPara.MSita1-myPara.MSita2)*2650)*100;
			//cout<<pWLM[i]<<endl;
			
			//隔24小时存一次状态值
			//李铁键，20070116,状态值中增加日蒸发量数据
			//if( i%StepsInHour==0 && (i/StepsInHour+HourStart) % 24 ==0 )
			if(	i/StepsInHour>=StatusTime && i%StepsInHour==0 && (i/StepsInHour)% StatusTime==0 )//wh,20060406,将status表的存储周期外置，1天1次有时数据量很大。
			{
				//DayCount++;//记天数
				//DayCount += StatusTime/24;//记天数,wh
				//cout<<"DayCount:"<<DayCount<<endl;
				mStatus[DayIndex].StatusBSCode=mBSCode;
				mStatus[DayIndex].HourOffset=i/StepsInHour+HourStart;
				mStatus[DayIndex].WLL=cLeafL.W;
				mStatus[DayIndex].WLU=soilL.W;
				mStatus[DayIndex].WLM=midL.W;
				mStatus[DayIndex].WLD=deepL.DSW;
				mStatus[DayIndex].WRL=cLeafR.W;
				mStatus[DayIndex].WRU=soilR.W;
				mStatus[DayIndex].WRM=midR.W;
				mStatus[DayIndex].WRD=deepR.DSW;
				mStatus[DayIndex].WSL=cLeafS.W;
				mStatus[DayIndex].WSU=soilS.W;
				mStatus[DayIndex].WSM=midS.W;
				mStatus[DayIndex].WSD=deepS.DSW;

				//20070116,xiaofc,存完一天的蒸发量和降水量后，将其清0
				if(DayIndex<DayCount-1)
				{
					mStatus[DayIndex+1].E=0.0f;
					mStatus[DayIndex+1].P=0.0f;
					mStatus[DayIndex+1].SlopeErosion=0.0f; //20080303,xiaofc,增加坡面土壤侵蚀量
				}
			}

			//20060406,wh,将下面的if部分从上面的if中拿出来，因为StatusTime可能大于30天，就不能再放到里面了。
			//隔一个月读一次LAI EPI
			if(i%StepsInHour==0 && (i/StepsInHour+HourStart)%24==0)//每隔一天执行一次
			{
				//cout<<"Prepared to Read LE"<<endl;
				DayEnd++;
				if(DayEnd>PMin.GetMonthDays(YearEnd,MonthEnd))
				{
					//cout<<"Refresh LAI & E0......"<<endl;
					DayEnd=DayEnd-PMin.GetMonthDays(YearEnd,MonthEnd);
					MonthEnd++;
					if(MonthEnd==13)
					{
						MonthEnd=1;
						YearEnd++;
					}
					paraS.ReadLE(mBSCode,YearEnd,MonthEnd);
					paraL.ReadLE(mBSCode,YearEnd,MonthEnd);
					paraR.ReadLE(mBSCode,YearEnd,MonthEnd);

					cEvaptransp.initialize(&paraS);
				}
			}

			//累加产流量和产沙量
			if(abs(qsS[0]+qsL[0]+qsR[0]+3)<1e-5)
			{
				ssT[i]=0.0f;
				qsT[i]=0.0f;
			}
			else if(abs(qsS[0]+1)<1e-5)
			{	
				qsT[i]=qsL[i]+qsR[i];
				if(qsT[i]<1e-5)
					ssT[i]=0.0f;
				else
					ssT[i]=(ssL[i]+ssR[i])/qsT[i];
					//ssT[i]=0.0f;
			}
			else
			{
				qsT[i]=qsS[i]+qsL[i]+qsR[i];
				if(qsT[i]<1e-5)
					ssT[i]=0.0f;
				else
					ssT[i]=(ssL[i]+ssR[i]+ssS[i])/qsT[i];
				    //ssT[i]=0.0f;
			}

			if(ssT[i]<0)
				ssT[i]=0;

			if (myFileslope.is_open())
			{
				if((i>=29668 && i<=29960) || (i>=20080 && i<=20380) || (i>=30950 && i<=31280))
				{
					myFileslope<<i<<" "<<qsS[i]<<" "<<ssS[i]<<" "<<P1<<" "<<NPS<<" "<<WPS<<" "<<surfaceS.WaterY/360.0f<<" "<<surfaceS.SedimentYield<<" "<<soilS.Wout/360.0f<<" "<<midS.Wout/360.0f<<" "<<qsL[i]<<" "<<ssL[i]<<" "<<P2<<" "<<NPL<<" "<<WPL<<" "<<surfaceL.WaterY/360.0f<<" "<<surfaceL.SedimentYield<<" "<<soilL.Wout/360.0f<<" "<<midL.Wout/360.0f<<" "<<qsR[i]<<" "<<ssR[i]<<" "<<P3<<" "<<NPR<<" "<<WPR<<" "<<surfaceR.WaterY/360.0f<<" "<<surfaceR.SedimentYield<<" "<<soilR.Wout/360.0f<<" "<<midR.Wout/360.0f<<" "<<qsT[i]<<" "<<ssT[i]<<"\n";
				}
				
			}

			if(!_finite (ssT[i]))
			{
				cout<<"INFINITE Hillslope Sediment Yield Result:"<<endl;
				cout<<"RegionIndex="<<mBSCode.RegionIndex<<"\tBSValue="<<mBSCode.Value<<"\tBSLength="<<mBSCode.Length<<endl;
				cout<<"t="<<i<<endl;
				cout<<"qsS[t]="<<qsS[i]<<"\tqsL[t]="<<qsL[i]<<"\tqsR[t]="<<qsR[i]<<endl;
				cout<<"ssS[t]="<<ssS[i]<<"\tssL[t]="<<ssL[i]<<"\tssR[t]="<<ssR[i]<<endl;
				exit(0);
			}

		}
		catch(...)
		{
			cout<<"error!"<<endl;//gao20071218补充
		}
	}//时间循环结束
	if (myFileslope.is_open())
	{
		myFileslope.close();
	}
	//20070724,xiaofc,因为大量的负反应
	//如果要修正负反应，又会引起严重的水量平衡问题，因此废除刘家宏的这个类
	//InnerTrans.GetBasinOutletQ(&myPara,qsS,qsL,qsR,qsT,NumofHours);

	//TCL end
	//TDB Status start
	NowTime=GetTickCount();
	CalTime+=NowTime-PreTime;
	PreTime=NowTime;

	//20090922,xiaofc,Batchly save status
	SaveStatus mSaveStatus;
	mSaveStatus.sccd = this->sccd;//wh,added 2008.3.24
	mSaveStatus.initialize(mBSCode,HourStart,NumofHours,StatusTime,pCnn);
	mSaveStatus.DoSave(mStatus,StatusTime);
	mSaveStatus.finalize();
	delete []mStatus;

	//20050109 李铁键 增加对不同降雨量数据类型的判断
	if(RainType=="DayAverage")
		PMin.DeleteList();
	else if(RainType=="TimePoint")
		PMinCBG.DeleteList();
	else if(RainType=="CMORPH8" || RainType=="CMORPH25")
		PMinCMORPH.DeleteList();

	this->finalize();

	//for srm，一条河段执行一次
	if(this->SnowModelType.MakeLower()=="srm")
	{
		SRM->ReleaseHeap();
	}

	//TDB Status end
	NowTime=GetTickCount();
	SSTime+=NowTime-PreTime;

	wchar_t *pch =new wchar_t[processor_name.GetLength()+1]; 
	pch=processor_name.GetBuffer(0);//0小于str实际长度，GetBuffer此时会按str实际长度作为缓冲区长度 
	processor_name.ReleaseBuffer(); //调用GetBuffer后一定要调用ReleaseBuffer 


	wcout<<"#"<<pch<<","<<rank<<",TCL,"<<CalTime/1000<<":Runoff yield"<<endl;
	wcout<<"#"<<pch<<","<<rank<<",TDB,"<<RainfallTime/1000<<":Rainfall"<<endl;
	wcout<<"#"<<pch<<","<<rank<<",TDB,"<<SSTime/1000<<":Status"<<endl;

}

int WaterYield::initialize(void)

{
	qsS=new float[Steps+1];
	qsL=new float[Steps+1];
	qsR=new float[Steps+1];

	ssS=new float[Steps+1];
	ssL=new float[Steps+1];
	ssR=new float[Steps+1];
	
	isDebug=false;
	return 1;
}

void WaterYield::finalize(void)

{
	if(qsS!=NULL)
		delete []qsS;
	if(qsL!=NULL)
		delete []qsL;
	if(qsR!=NULL)
		delete []qsR;

	qsT=NULL;

	if(ssS!=NULL)
		delete []ssS;
	if(ssL!=NULL)
		delete []ssL;
	if(ssR!=NULL)
		delete []ssR;

	ssT=NULL;

}
