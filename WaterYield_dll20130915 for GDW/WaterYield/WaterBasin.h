// WaterBasin.h : CWaterBasin ������
#pragma once
#include "resource.h"       // ������

#include "WaterYield.h"
#include "CWaterYield.h"


#if defined(_WIN32_WCE) && !defined(_CE_DCOM) && !defined(_CE_ALLOW_SINGLE_THREADED_OBJECTS_IN_MTA)
#error "Windows CE ƽ̨(�粻�ṩ��ȫ DCOM ֧�ֵ� Windows Mobile ƽ̨)���޷���ȷ֧�ֵ��߳� COM ���󡣶��� _CE_ALLOW_SINGLE_THREADED_OBJECTS_IN_MTA ��ǿ�� ATL ֧�ִ������߳� COM ����ʵ�ֲ�����ʹ���䵥�߳� COM ����ʵ�֡�rgs �ļ��е��߳�ģ���ѱ�����Ϊ��Free����ԭ���Ǹ�ģ���Ƿ� DCOM Windows CE ƽ̨֧�ֵ�Ψһ�߳�ģ�͡�"
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
