// �°���ģ�ͽӿ�.h : C�°���ģ�ͽӿ� ������

#pragma once
#include "resource.h"       // ������

#include "My.h"
#include "CWaterYield.h"


#if defined(_WIN32_WCE) && !defined(_CE_DCOM) && !defined(_CE_ALLOW_SINGLE_THREADED_OBJECTS_IN_MTA)
#error "Windows CE ƽ̨(�粻�ṩ��ȫ DCOM ֧�ֵ� Windows Mobile ƽ̨)���޷���ȷ֧�ֵ��߳� COM ���󡣶��� _CE_ALLOW_SINGLE_THREADED_OBJECTS_IN_MTA ��ǿ�� ATL ֧�ִ������߳� COM ����ʵ�ֲ�����ʹ���䵥�߳� COM ����ʵ�֡�rgs �ļ��е��߳�ģ���ѱ�����Ϊ��Free����ԭ���Ǹ�ģ���Ƿ� DCOM Windows CE ƽ̨֧�ֵ�Ψһ�߳�ģ�͡�"
#endif



// C�°���ģ�ͽӿ�

class ATL_NO_VTABLE C�°���ģ�ͽӿ� :
	public CComObjectRootEx<CComSingleThreadModel>,
	public CComCoClass<C�°���ģ�ͽӿ�, &CLSID_My>,
	public IMy
{
public:
	C�°���ģ�ͽӿ�()
	{
	}

DECLARE_REGISTRY_RESOURCEID(IDR_My)


BEGIN_COM_MAP(C�°���ģ�ͽӿ�)
	COM_INTERFACE_ENTRY(IMy)
END_COM_MAP()



	DECLARE_PROTECT_FINAL_CONSTRUCT()

	HRESULT FinalConstruct()
	{
		return S_OK;
	}

	void FinalRelease()
	{
	}


public:
	STDMETHOD(Initialize)(BSTR SRM, FLOAT BasinArea, struct XAJParameter XAJ, BSTR sccd, BSTR user, BSTR password, BSTR sid, BSTR RainType, LONG HourStart, LONG NumofHours, LONG TimeStep, LONG Steps, LONG StatusTime);
public:
	STDMETHOD(Calc)(BSTR SnowModelType,struct BSCode mBSCode, struct Para mPara, FLOAT* pQin, FLOAT* pWLM, FLOAT* pWRM);

private:
	WaterYield mWaterYield;
	STDMETHOD(Finalize)(BSTR SRM);
};

OBJECT_ENTRY_AUTO(__uuidof(My), C�°���ģ�ͽӿ�)
