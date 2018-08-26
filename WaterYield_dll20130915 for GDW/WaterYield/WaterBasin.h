// WaterBasin.h : CWaterBasin 的声明
#pragma once
#include "resource.h"       // 主符号

#include "WaterYield.h"
#include "CWaterYield.h"


#if defined(_WIN32_WCE) && !defined(_CE_DCOM) && !defined(_CE_ALLOW_SINGLE_THREADED_OBJECTS_IN_MTA)
#error "Windows CE 平台(如不提供完全 DCOM 支持的 Windows Mobile 平台)上无法正确支持单线程 COM 对象。定义 _CE_ALLOW_SINGLE_THREADED_OBJECTS_IN_MTA 可强制 ATL 支持创建单线程 COM 对象实现并允许使用其单线程 COM 对象实现。rgs 文件中的线程模型已被设置为“Free”，原因是该模型是非 DCOM Windows CE 平台支持的唯一线程模型。"
#endif



// CWaterBasin

class ATL_NO_VTABLE CWaterBasin :
	public CComObjectRootEx<CComSingleThreadModel>,
	public CComCoClass<CWaterBasin, &CLSID_WaterBasin>,
	public IWaterBasin
{
public:
	CWaterBasin()
	{
	}

DECLARE_REGISTRY_RESOURCEID(IDR_WATERBASIN)


BEGIN_COM_MAP(CWaterBasin)
	COM_INTERFACE_ENTRY(IWaterBasin)
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
	STDMETHOD(Initialize)(BSTR name,long rank,BSTR SRM, long StatusTime,/*_Connection** pCnn*/BSTR user,BSTR password,BSTR sid,long steps,long HourStart,long NumofHours,VARIANT_BOOL isDebug,BSTR sccd,BSTR sRainType,float UpInitWaterContent, float MidInitWaterContent, float DownInitWaterContent,BSTR emethod, float thetab, float thetaw, int N, float E0_a,BSTR SoilErosionEquation);
	STDMETHOD(calc)(BSTR SnowModelType,struct BSCode mBSCode,struct Para mPara,FLOAT* pQin, FLOAT* pSin, FLOAT* pWLM, FLOAT* pWRM);
	STDMETHOD(Finalize)(BSTR SRM);

private:
	WaterYield mWaterYield;
	
};

OBJECT_ENTRY_AUTO(__uuidof(WaterBasin), CWaterBasin)
