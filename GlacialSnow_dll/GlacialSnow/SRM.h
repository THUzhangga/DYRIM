// SRM.h : CSRM 的声明

#pragma once
#include "resource.h"       // 主符号

#include "GlacialSnow.h"
#include "input.h"


#if defined(_WIN32_WCE) && !defined(_CE_DCOM) && !defined(_CE_ALLOW_SINGLE_THREADED_OBJECTS_IN_MTA)
#error "Windows CE 平台(如不提供完全 DCOM 支持的 Windows Mobile 平台)上无法正确支持单线程 COM 对象。定义 _CE_ALLOW_SINGLE_THREADED_OBJECTS_IN_MTA 可强制 ATL 支持创建单线程 COM 对象实现并允许使用其单线程 COM 对象实现。rgs 文件中的线程模型已被设置为“Free”，原因是该模型是非 DCOM Windows CE 平台支持的唯一线程模型。"
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
