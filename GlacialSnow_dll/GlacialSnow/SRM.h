// SRM.h : CSRM ������

#pragma once
#include "resource.h"       // ������

#include "GlacialSnow.h"
#include "input.h"


#if defined(_WIN32_WCE) && !defined(_CE_DCOM) && !defined(_CE_ALLOW_SINGLE_THREADED_OBJECTS_IN_MTA)
#error "Windows CE ƽ̨(�粻�ṩ��ȫ DCOM ֧�ֵ� Windows Mobile ƽ̨)���޷���ȷ֧�ֵ��߳� COM ���󡣶��� _CE_ALLOW_SINGLE_THREADED_OBJECTS_IN_MTA ��ǿ�� ATL ֧�ִ������߳� COM ����ʵ�ֲ�����ʹ���䵥�߳� COM ����ʵ�֡�rgs �ļ��е��߳�ģ���ѱ�����Ϊ��Free����ԭ���Ǹ�ģ���Ƿ� DCOM Windows CE ƽ̨֧�ֵ�Ψһ�߳�ģ�͡�"
#endif



// CSRM

class ATL_NO_VTABLE CSRM :
	public CComObjectRootEx<CComSingleThreadModel>,
	public CComCoClass<CSRM, &CLSID_SRM>,
	public IDispatchImpl<ISRM, &IID_ISRM, &LIBID_GlacialSnowLib, /*wMajor =*/ 1, /*wMinor =*/ 0>
{
public:
	CSRM()
	{
	}

DECLARE_REGISTRY_RESOURCEID(IDR_SRM)


BEGIN_COM_MAP(CSRM)
	COM_INTERFACE_ENTRY(ISRM)
	COM_INTERFACE_ENTRY(IDispatch)
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
	input minput;

public:
	STDMETHOD(OpenOracle)(/*_Connection** pCnn*/BSTR user, BSTR password, BSTR sid);
public:
	STDMETHOD(SnowInitialize)(FLOAT myParaX, FLOAT myParaY, ULONGLONG RegionIndex, LONG Length, ULONGLONG Value, FLOAT A, FLOAT Havg);
public:
	STDMETHOD(SnowCalc)(LONG Houroffset, FLOAT* HourRainIn);
public:
	STDMETHOD(SRMFinalize)(void);
public:
	STDMETHOD(ReleaseHeap)(void);
};

OBJECT_ENTRY_AUTO(__uuidof(SRM), CSRM)
