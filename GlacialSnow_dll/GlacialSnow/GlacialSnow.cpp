// GlacialSnow.cpp : DLL 导出的实现。


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

// 重写
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

