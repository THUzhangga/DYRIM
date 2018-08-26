// WaterBasin.cpp : CWaterBasin ��ʵ��
#pragma once
#include "stdafx.h"
#include "WaterBasin.h"
#include "assert.h"

//û�а취������.h��i.cֻ�ܱ�wateryield.dll����һ�Σ��������ᱨ��������CWaterYield.cpp�������ˣ���Ϊ�����޷�����Ҳ�Ͳ�����
//����spSRM����ָ�롣��������ָ��ǰ����ʹ��extern��
//����SRMģ��
//#include "C:\Hydro\Hydro\Program\WH_Reaction\GlacialSnow\GlacialSnow\GlacialSnow.h"
//#include "C:\Hydro\Hydro\Program\WH_Reaction\GlacialSnow\GlacialSnow\GlacialSnow_i.c"
//CComQIPtr <ISRM,&IID_ISRM> spSRM;

// CWaterBasin

//������ϵnewrouting
//wh,����������Ķ��Ǵ�NewRouting�д����һ���Բ������������ż�������ٸı�ġ�
//��һ��SRM����ֻ��ʾSRMģ���Ƿ���ڣ���������������κ�ģ�Ͷ���Ҫ���ӱ���������Ӧ����safearray����bstr���飬����
//�Ƚ��鷳���Ժ���Ը��¡�
STDMETHODIMP CWaterBasin::Initialize(BSTR name,long rank,BSTR SRM, long StatusTime,/*_Connection** pCnn*/BSTR user,BSTR password,BSTR sid,long steps,long HourStart,long NumofHours,VARIANT_BOOL isDebug,BSTR sccd,BSTR sRainType,float UpInitWaterContent, float MidInitWaterContent, float DownInitWaterContent,BSTR emethod, float thetab, float thetaw, int N, float E0_a,BSTR SoilErosionEquation)
{
	CString RainType(sRainType);
	CString User(user),Password(password),Sid(sid),memethod(emethod),Sccd(sccd);

	CString Name(name);

	//wh,������ȶ���һ��CString��BSTRת��ΪCStringֱ�Ӹ�ֵ�����ˣ�����������������assert���жϡ�
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

	mWaterYield.StepsInHour = steps/NumofHours;//һ��Сʱ�ж��ٸ�����

	int timestep = 60*NumofHours/steps; //timestep ÿ���������ٷ���
	if(timestep>60 || timestep<1 || 60%timestep!=0)
		cout<<"TIMESTEP is OUT OF range!"<<endl;//LTJ
	mWaterYield.PMin.MSTEP = timestep;
	mWaterYield.PMinCBG.MSTEP = timestep;
	mWaterYield.PMinCMORPH.MSTEP = timestep;//add CMORPH,shy, 20130905
	mWaterYield.MSTEP = timestep;
	
	//_Connection* x;//���ڴ�x����ת��ΪADODB::_ConnectionPtr�����Ƿ�֮���ܡ�

	CString CSSQL;
	CSSQL.Format(_T("Provider=OraOLEDB.Oracle.1;Persist Security Info=False;User ID=%s;Data Source=%s;Extended Properties=''"),User,Sid);
	_bstr_t bSQL=CSSQL.GetString();
	try
	{
		mWaterYield.pCnn.CreateInstance(__uuidof(ADODB::Connection));//��������
		mWaterYield.pCnn->Open(bSQL,User.GetString(),Password.GetString(),0);
	}
	catch(_com_error e)
	{
		//�������ݿ�����ʧ�ܵľ���.
		cout<<"Failture Of Opening Oracle In WaterYield.dll."<<endl;
		e.ErrorMessage();
		e.Description();
		return S_FALSE;
	}

	//����һ��֮���Էŵ���������Ϊ���ڸ������˵ֻ���ڴˣ����ܱ�֤�ڲ��м����ʼ��ִֻ��һ�Ρ�
	
	
	CString srm(SRM);
	if(srm.MakeLower()=="srm")//��if��֤û��GlacialSnow.dllʱ��WaterYield.dll���Ҳ���������С�
	{
		mWaterYield.InitializeSRM(user,password,sid);
	}

	return S_OK;
	cout<<"End init"<<endl;
}



//wh��������ǻ����ż�����̸ı�Ĳ�������Ϊ������ǵ�ַ������dll������еĸı䣬NewRoutingҲ���֪����
STDMETHODIMP CWaterBasin::calc(BSTR SnowModelType,struct BSCode mBSCode, struct Para mPara,FLOAT* pQin, FLOAT* pSin, FLOAT* pWLM, FLOAT* pWRM)
{
	mWaterYield.SnowModelType = SnowModelType;
	mWaterYield.qsT = pQin;//qsT�ĸı伴�൱��pQin�ĸı�
	mWaterYield.ssT = pSin;
	mWaterYield.pWLM = pWLM;//����avpm�е�������ʴ����
	mWaterYield.pWRM = pWRM;//����avpm�е�������ʴ����

	//cout<<"hourstart:"<<this->mWaterYield.HourStart<<"mstep:"<<this->mWaterYield.MSTEP<<"numofhours:"<<this->mWaterYield.NumofHours<<endl;//right

	mWaterYield.Calc(mBSCode,mPara);

	return S_OK;
}

//ȫ��ִ��һ�Ρ�
STDMETHODIMP CWaterBasin::Finalize(BSTR SRM)
{
	// TODO: �ڴ����ʵ�ִ���
	CString srm(SRM);
	if(srm.MakeLower()=="srm")
	{
		mWaterYield.FinalizeSRM();
	}
	return S_OK;
}
