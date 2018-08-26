// 新安江模型接口.h : C新安江模型接口 的声明

#pragma once
#include "resource.h"       // 主符号

#include "My.h"
#include "CWaterYield.h"


#if defined(_WIN32_WCE) && !defined(_CE_DCOM) && !defined(_CE_ALLOW_SINGLE_THREADED_OBJECTS_IN_MTA)
#error "Windows CE 平台(如不提供完全 DCOM 支持的 Windows Mobile 平台)上无法正确支持单线程 COM 对象。定义 _CE_ALLOW_SINGLE_THREADED_OBJECTS_IN_MTA 可强制 ATL 支持创建单线程 COM 对象实现并允许使用其单线程 COM 对象实现。rgs 文件中的线程模型已被设置为“Free”，原因是该模型是非 DCOM Windows CE 平台支持的唯一线程模型。"
#endif



// C新安江模型接口

class ATL_NO_VTABLE C新安江模型接口 :
	public CComObjectRootEx<CComSingleThreadModel>,
	public CComCoClass<C新安江模型接口, &CLSID_My>,
	public IMy
{
public:
	C新安江模型接口()
	{
	}

DECLARE_REGISTRY_RESOURCEID(IDR_My)


BEGIN_COM_MAP(C新安江模型接口)
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

OBJECT_ENTRY_AUTO(__uuidof(My), C新安江模型接口)
