// �°���ģ��.cpp : DLL ������ʵ�֡�


#include "stdafx.h"
#include "resource.h"
#include "My.h"


class CMyModule : public CAtlDllModuleT< CMyModule >
{
public :
	DECLARE_LIBID(LIBID_�°���ģ��Lib)
	DECLARE_REGISTRY_APPID_RESOURCEID(IDR_MY, "{80251BA1-89CA-4552-8027-447703DE4E27}")
};

CMyModule _AtlModule;

class CMyApp : public CWinApp
{
public:

// ��д
    virtual BOOL InitInstance();
    virtual int ExitInstance();

    DECLARE_MESSAGE_MAP()
};

BEGIN_MESSAGE_MAP(CMyApp, CWinApp)
END_MESSAGE_MAP()

CMyApp theApp;

BOOL CMyApp::InitInstance()
{
    return CWinApp::InitInstance();
}

int CMyApp::ExitInstance()
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

