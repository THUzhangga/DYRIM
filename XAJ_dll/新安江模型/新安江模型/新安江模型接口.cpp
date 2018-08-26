// �°���ģ�ͽӿ�.cpp : C�°���ģ�ͽӿ� ��ʵ��

#include "stdafx.h"
#include "�°���ģ�ͽӿ�.h"

//CComQIPtr <ISRM,&IID_ISRM> spSRM;//����ѩģ�͵Ľӿ�


// C�°���ģ�ͽӿ�


STDMETHODIMP C�°���ģ�ͽӿ�::Initialize(BSTR SRM, FLOAT BasinArea, struct XAJParameter XAJ, BSTR sccd, BSTR user, BSTR password, BSTR sid, BSTR RainType, LONG HourStart, LONG NumofHours, LONG TimeStep, LONG Steps, LONG StatusTime)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	// TODO: �ڴ����ʵ�ִ���
	CString sRainType(RainType),User(user),Password(password),Sid(sid),Sccd(sccd);
	//wh,������ȶ���һ��CString��BSTRת��ΪCStringֱ�Ӹ�ֵ�����ˣ�����������������assert���жϡ�
	
	mWaterYield.BasinArea = BasinArea;
	mWaterYield.RainType = sRainType;
	mWaterYield.XAJ = XAJ;
	mWaterYield.StatusTime = StatusTime;
	mWaterYield.NumofHours = NumofHours;
	mWaterYield.HourStart = HourStart;
	mWaterYield.Steps = Steps;
	mWaterYield.sccd = Sccd;

	mWaterYield.StepsInHour = Steps/NumofHours;//һ��Сʱ�ж��ٸ�����

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
		mWaterYield.pCnn.CreateInstance(__uuidof(ADODB::Connection));//��������
		mWaterYield.pCnn->Open(bSQL,User.GetString(),Password.GetString(),0);
		//mWaterYield.pCnn = pCnn;
	}
	catch(_com_error e)
	{
		//�������ݿ�����ʧ�ܵľ���.
		cout<<"Failture Of Opening Oracle In �°���ģ��.dll."<<endl;
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
}


STDMETHODIMP C�°���ģ�ͽӿ�::Calc(BSTR SnowModelType,struct BSCode mBSCode, struct Para mPara, FLOAT* pQin, FLOAT* pWLM, FLOAT* pWRM)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	// TODO: �ڴ����ʵ�ִ���
	mWaterYield.SnowModelType = SnowModelType;
	mWaterYield.Q = pQin;//Q�ĸı伴�൱��pQin�ĸı�
	mWaterYield.pWLM = pWLM;//����avpm�е�������ʴ����
	mWaterYield.pWRM = pWRM;//����avpm�е�������ʴ����

	mWaterYield.Calc(mBSCode,mPara);

	return S_OK;
}


STDMETHODIMP C�°���ģ�ͽӿ�::Finalize(BSTR SRM)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	// TODO: �ڴ����ʵ�ִ���
	CString srm(SRM);
	if(srm.MakeLower()=="srm")
	{
		mWaterYield.FinalizeSRM();
	}
	return S_OK;
}
