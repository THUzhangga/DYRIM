// WaterYield.cpp : DLL ������ʵ�֡�


#include "stdafx.h"
#include "resource.h"
#include "WaterYield.h"


class CWaterYieldModule : public CAtlDllModuleT< CWaterYieldModule >
{
public :
	DECLARE_LIBID(LIBID_WaterYieldLib)
	DECLARE_REGISTRY_APPID_RESOURCEID(IDR_WATERYIELD, "{1089ACF6-56B4-4800-AC79-FEAAEE68E990}")
};

CWaterYieldModule _AtlModule;


#ifdef _MANAGED
#pragma managed(push, off)
#endif

// DLL ��ڵ�
extern "C" BOOL WINAPI DllMain(HINSTANCE hInstance, DWORD dwReason, LPVOID lpReserved)
{
	hInstance;
    return _AtlModule.DllMain(dwReason, lpReserved); 
}

#ifdef _MANAGED
#pragma managed(pop)
#endif




// ����ȷ�� DLL �Ƿ���� OLE ж��
STDAPI DllCanUnloadNow(void)
{
    return _AtlModule.DllCanUnloadNow();
}


// ����һ���๤���Դ������������͵Ķ���
STDAPI DllGetClassObject(REFCLSID rclsid, REFIID riid, LPVOID* ppv)
{
    return _AtlModule.DllGetClassObject(rclsid, riid, ppv);
}


// DllRegisterServer - ������ӵ�ϵͳע���
STDAPI DllRegisterServer(void)
{
    // ע��������Ϳ�����Ϳ��е����нӿ�
    HRESULT hr = _AtlModule.DllRegisterServer();
	return hr;
}


// DllUnregisterServer - �����ϵͳע������Ƴ�
STDAPI DllUnregisterServer(void)
{
	HRESULT hr = _AtlModule.DllUnregisterServer();
	return hr;
}

