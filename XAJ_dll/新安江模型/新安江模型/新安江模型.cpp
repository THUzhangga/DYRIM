// 新安江模型.cpp : DLL 导出的实现。


#include "stdafx.h"
#include "resource.h"
#include "My.h"


class CMyModule : public CAtlDllModuleT< CMyModule >
{
public :
	DECLARE_LIBID(LIBID_新安江模型Lib)
	DECLARE_REGISTRY_APPID_RESOURCEID(IDR_MY, "{80251BA1-89CA-4552-8027-447703DE4E27}")
};

CMyModule _AtlModule;

class CMyApp : public CWinApp
{
public:

// 重写
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


// 用于确定 DLL 是否可由 OLE 卸载
STDAPI DllCanUnloadNow(void)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());
    return (AfxDllCanUnloadNow()==S_OK && _AtlModule.GetLockCount()==0) ? S_OK : S_FALSE;
}


// 返回一个类工厂以创建所请求类型的对象
STDAPI DllGetClassObject(REFCLSID rclsid, REFIID riid, LPVOID* ppv)
{
    return _AtlModule.DllGetClassObject(rclsid, riid, ppv);
}


// DllRegisterServer - 将项添加到系统注册表
STDAPI DllRegisterServer(void)
{
    // 注册对象、类型库和类型库中的所有接口
    HRESULT hr = _AtlModule.DllRegisterServer();
	return hr;
}


// DllUnregisterServer - 将项从系统注册表中移除
STDAPI DllUnregisterServer(void)
{
	HRESULT hr = _AtlModule.DllUnregisterServer();
	return hr;
}

