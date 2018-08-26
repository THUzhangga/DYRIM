// GlacialSnow.cpp : DLL ������ʵ�֡�


#include "stdafx.h"
#include "resource.h"
#include "GlacialSnow.h"


class CGlacialSnowModule : public CAtlDllModuleT< CGlacialSnowModule >
{
public :
	DECLARE_LIBID(LIBID_GlacialSnowLib)
	DECLARE_REGISTRY_APPID_RESOURCEID(IDR_GLACIALSNOW, "{3A0FDCD2-FD66-4178-A70D-145C3FDE57C1}")
};

CGlacialSnowModule _AtlModule;

class CGlacialSnowApp : public CWinApp
{
public:

// ��д
    virtual BOOL InitInstance();
    virtual int ExitInstance();

    DECLARE_MESSAGE_MAP()
};

BEGIN_MESSAGE_MAP(CGlacialSnowApp, CWinApp)
END_MESSAGE_MAP()

CGlacialSnowApp theApp;

BOOL CGlacialSnowApp::InitInstance()
{
    return CWinApp::InitInstance();
}

int CGlacialSnowApp::ExitInstance()
{
    return CWinApp::ExitInstance();
}


// ����ȷ�� DLL �Ƿ���� OLE ж��
STDAPI DllCanUnloadNow(void)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());
    return (AfxDllCanUnloadNow()==S_OK && _AtlModule.GetLockCount()==0) ? S_OK : S_FALSE;
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

