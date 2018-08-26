// WaterBasin.cpp : CWaterBasin 的实现
#pragma once
#include "stdafx.h"
#include "WaterBasin.h"
#include "assert.h"

//没有办法，发现.h和i.c只能被wateryield.dll引用一次，否则编译会报错，所以在CWaterYield.cpp中引用了，因为这里无法引用也就不能再
//声明spSRM智能指针。而且智能指针前不能使用extern。
//调用SRM模型
//#include "C:\Hydro\Hydro\Program\WH_Reaction\GlacialSnow\GlacialSnow\GlacialSnow.h"
//#include "C:\Hydro\Hydro\Program\WH_Reaction\GlacialSnow\GlacialSnow\GlacialSnow_i.c"
//CComQIPtr <ISRM,&IID_ISRM> spSRM;

// CWaterBasin

//以下联系newrouting
//wh,这个函数做的都是从NewRouting中传入的一次性参数，不会随着计算过程再改变的。
//第一个SRM参数只表示SRM模型是否存在，如果还有其他的任何模型都需要增加变量，本来应该用safearray传递bstr数组，但是
//比较麻烦，以后可以改下。
STDMETHODIMP CWaterBasin::Initialize(BSTR name,long rank,BSTR SRM, long StatusTime,/*_Connection** pCnn*/BSTR user,BSTR password,BSTR sid,long steps,long HourStart,long NumofHours,VARIANT_BOOL isDebug,BSTR sccd,BSTR sRainType,float UpInitWaterContent, float MidInitWaterContent, float DownInitWaterContent,BSTR emethod, float thetab, float thetaw, int N, float E0_a,BSTR SoilErosionEquation)
{
	CString RainType(sRainType);
	CString User(user),Password(password),Sid(sid),memethod(emethod),Sccd(sccd);

	CString Name(name);

	//wh,这里得先定义一下CString，BSTR转化为CString直接赋值就行了，如果不这样做下面的assert会中断。
	assert( RainType == _T("DayAverage") || RainType == _T("TimePoint") || RainType == _T("CMORPH8") || RainType == _T("CMORPH25"));

	if(RainType == _T("TimePoint"))
	{
		mWaterYield.RainType = RainType;
	
	}
	if(RainType == _T("DayAverage"))
	{
		mWaterYield.RainType = RainType;
	}
	if(RainType == _T("CMORPH8"))
	{
		mWaterYield.RainType = RainType;
	}
	if(RainType == _T("CMORPH25"))
	{
		mWaterYield.RainType = RainType;
	}

	//2008.3.23
	mWaterYield.processor_name=Name;
	mWaterYield.rank=rank;

	mWaterYield.StatusTime = StatusTime;
	mWaterYield.UpInitWaterContent = UpInitWaterContent;
	mWaterYield.MidInitWaterContent = MidInitWaterContent;
	mWaterYield.DownInitWaterContent = DownInitWaterContent;
	mWaterYield.N = N;
	mWaterYield.E0_a = E0_a;
	mWaterYield.thetab = thetab;
	mWaterYield.thetaw = thetaw;

	mWaterYield.SEEquation = SoilErosionEquation;//wh,20080803

	mWaterYield.isDebug = isDebug;
	mWaterYield.NumofHours = NumofHours;
	mWaterYield.HourStart = HourStart;
	mWaterYield.Steps = steps;

	mWaterYield.emethod = memethod;
	mWaterYield.sccd = Sccd;

	mWaterYield.StepsInHour = steps/NumofHours;//一个小时有多少个步长

	int timestep = 60*NumofHours/steps; //timestep 每个步长多少分钟
	if(timestep>60 || timestep<1 || 60%timestep!=0)
		cout<<"TIMESTEP is OUT OF range!"<<endl;//LTJ
	mWaterYield.PMin.MSTEP = timestep;
	mWaterYield.PMinCBG.MSTEP = timestep;
	mWaterYield.PMinCMORPH.MSTEP = timestep;//add CMORPH,shy, 20130905
	mWaterYield.MSTEP = timestep;
	
	//_Connection* x;//对于此x可以转化为ADODB::_ConnectionPtr，但是反之不能。

	CString CSSQL;
	CSSQL.Format(_T("Provider=OraOLEDB.Oracle.1;Persist Security Info=False;User ID=%s;Data Source=%s;Extended Properties=''"),User,Sid);
	_bstr_t bSQL=CSSQL.GetString();
	try
	{
		mWaterYield.pCnn.CreateInstance(__uuidof(ADODB::Connection));//创建对象
		mWaterYield.pCnn->Open(bSQL,User.GetString(),Password.GetString(),0);
	}
	catch(_com_error e)
	{
		//给出数据库连接失败的警告.
		cout<<"Failture Of Opening Oracle In WaterYield.dll."<<endl;
		e.ErrorMessage();
		e.Description();
		return S_FALSE;
	}

	//以下一句之所以放到这里是因为对于该组件来说只有在此，才能保证在并行计算的始终只执行一次。
	
	
	CString srm(SRM);
	if(srm.MakeLower()=="srm")//该if保证没有GlacialSnow.dll时，WaterYield.dll组件也能正常运行。
	{
		mWaterYield.InitializeSRM(user,password,sid);
	}

	return S_OK;
	cout<<"End init"<<endl;
}



//wh，传入的是会随着计算过程改变的参数，因为传入的是地址，所以dll对其进行的改变，NewRouting也会感知到。
STDMETHODIMP CWaterBasin::calc(BSTR SnowModelType,struct BSCode mBSCode, struct Para mPara,FLOAT* pQin, FLOAT* pSin, FLOAT* pWLM, FLOAT* pWRM)
{
	mWaterYield.SnowModelType = SnowModelType;
	mWaterYield.qsT = pQin;//qsT的改变即相当于pQin的改变
	mWaterYield.ssT = pSin;
	mWaterYield.pWLM = pWLM;//用于avpm中的重力侵蚀计算
	mWaterYield.pWRM = pWRM;//用于avpm中的重力侵蚀计算

	//cout<<"hourstart:"<<this->mWaterYield.HourStart<<"mstep:"<<this->mWaterYield.MSTEP<<"numofhours:"<<this->mWaterYield.NumofHours<<endl;//right

	mWaterYield.Calc(mBSCode,mPara);

	return S_OK;
}

//全局执行一次。
STDMETHODIMP CWaterBasin::Finalize(BSTR SRM)
{
	// TODO: 在此添加实现代码
	CString srm(SRM);
	if(srm.MakeLower()=="srm")
	{
		mWaterYield.FinalizeSRM();
	}
	return S_OK;
}
