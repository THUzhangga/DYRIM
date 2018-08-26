// 新安江模型接口.cpp : C新安江模型接口 的实现

#include "stdafx.h"
#include "新安江模型接口.h"

//CComQIPtr <ISRM,&IID_ISRM> spSRM;//给融雪模型的接口


// C新安江模型接口


STDMETHODIMP C新安江模型接口::Initialize(BSTR SRM, FLOAT BasinArea, struct XAJParameter XAJ, BSTR sccd, BSTR user, BSTR password, BSTR sid, BSTR RainType, LONG HourStart, LONG NumofHours, LONG TimeStep, LONG Steps, LONG StatusTime)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	// TODO: 在此添加实现代码
	CString sRainType(RainType),User(user),Password(password),Sid(sid),Sccd(sccd);
	//wh,这里得先定义一下CString，BSTR转化为CString直接赋值就行了，如果不这样做下面的assert会中断。
	
	mWaterYield.BasinArea = BasinArea;
	mWaterYield.RainType = sRainType;
	mWaterYield.XAJ = XAJ;
	mWaterYield.StatusTime = StatusTime;
	mWaterYield.NumofHours = NumofHours;
	mWaterYield.HourStart = HourStart;
	mWaterYield.Steps = Steps;
	mWaterYield.sccd = Sccd;

	mWaterYield.StepsInHour = Steps/NumofHours;//一个小时有多少个步长

	if(TimeStep>60 || TimeStep<1 || 60%TimeStep!=0)
		cout<<"TIMESTEP is OUT OF range!"<<endl;//LTJ
	mWaterYield.PMin.MSTEP = TimeStep;
	mWaterYield.PMinCBG.MSTEP = TimeStep;
	mWaterYield.MSTEP = TimeStep;
	
	//ADODB::_ConnectionPtr pCnn;
	CString CSSQL;
	CSSQL.Format(_T("Provider=OraOLEDB.Oracle.1;Persist Security Info=False;User ID=%s;Data Source=%s;Extended Properties=''"),User,Sid);
	_bstr_t bSQL=CSSQL.GetString();
	try
	{
		mWaterYield.pCnn.CreateInstance(__uuidof(ADODB::Connection));//创建对象
		mWaterYield.pCnn->Open(bSQL,User.GetString(),Password.GetString(),0);
		//mWaterYield.pCnn = pCnn;
	}
	catch(_com_error e)
	{
		//给出数据库连接失败的警告.
		cout<<"Failture Of Opening Oracle In 新安江模型.dll."<<endl;
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
}


STDMETHODIMP C新安江模型接口::Calc(BSTR SnowModelType,struct BSCode mBSCode, struct Para mPara, FLOAT* pQin, FLOAT* pWLM, FLOAT* pWRM)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	// TODO: 在此添加实现代码
	mWaterYield.SnowModelType = SnowModelType;
	mWaterYield.Q = pQin;//Q的改变即相当于pQin的改变
	mWaterYield.pWLM = pWLM;//用于avpm中的重力侵蚀计算
	mWaterYield.pWRM = pWRM;//用于avpm中的重力侵蚀计算

	mWaterYield.Calc(mBSCode,mPara);

	return S_OK;
}


STDMETHODIMP C新安江模型接口::Finalize(BSTR SRM)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	// TODO: 在此添加实现代码
	CString srm(SRM);
	if(srm.MakeLower()=="srm")
	{
		mWaterYield.FinalizeSRM();
	}
	return S_OK;
}
