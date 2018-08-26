

/* this ALWAYS GENERATED file contains the definitions for the interfaces */


 /* File created by MIDL compiler version 8.00.0595 */
/* at Tue May 09 18:44:03 2017
 */
/* Compiler settings for WaterYield.idl:
    Oicf, W1, Zp8, env=Win64 (32b run), target_arch=AMD64 8.00.0595 
    protocol : dce , ms_ext, c_ext, robust
    error checks: allocation ref bounds_check enum stub_data 
    VC __declspec() decoration level: 
         __declspec(uuid()), __declspec(selectany), __declspec(novtable)
         DECLSPEC_UUID(), MIDL_INTERFACE()
*/
/* @@MIDL_FILE_HEADING(  ) */

#pragma warning( disable: 4049 )  /* more than 64k source lines */


/* verify that the <rpcndr.h> version is high enough to compile this file*/
#ifndef __REQUIRED_RPCNDR_H_VERSION__
#define __REQUIRED_RPCNDR_H_VERSION__ 475
#endif

#include "rpc.h"
#include "rpcndr.h"

#ifndef __RPCNDR_H_VERSION__
#error this stub requires an updated version of <rpcndr.h>
#endif // __RPCNDR_H_VERSION__

#ifndef COM_NO_WINDOWS_H
#include "windows.h"
#include "ole2.h"
#endif /*COM_NO_WINDOWS_H*/

#ifndef __WaterYield_h__
#define __WaterYield_h__

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

/* Forward Declarations */ 

#ifndef __IWaterBasin_FWD_DEFINED__
#define __IWaterBasin_FWD_DEFINED__
typedef interface IWaterBasin IWaterBasin;

#endif 	/* __IWaterBasin_FWD_DEFINED__ */


#ifndef __WaterBasin_FWD_DEFINED__
#define __WaterBasin_FWD_DEFINED__

#ifdef __cplusplus
typedef class WaterBasin WaterBasin;
#else
typedef struct WaterBasin WaterBasin;
#endif /* __cplusplus */

#endif 	/* __WaterBasin_FWD_DEFINED__ */


/* header files for imported files */
#include "oaidl.h"
#include "ocidl.h"
#include "InterfaceStruct.h"
#include "msado15.h"

#ifdef __cplusplus
extern "C"{
#endif 


/* interface __MIDL_itf_WaterYield_0000_0000 */
/* [local] */ 

#pragma once


extern RPC_IF_HANDLE __MIDL_itf_WaterYield_0000_0000_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_WaterYield_0000_0000_v0_0_s_ifspec;

#ifndef __IWaterBasin_INTERFACE_DEFINED__
#define __IWaterBasin_INTERFACE_DEFINED__

/* interface IWaterBasin */
/* [unique][helpstring][nonextensible][oleautomation][uuid][object] */ 


EXTERN_C const IID IID_IWaterBasin;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("F7013BFF-2387-43C7-85EB-A6F2DB04B309")
    IWaterBasin : public IUnknown
    {
    public:
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE Initialize( 
            /* [in] */ BSTR name,
            /* [in] */ long rank,
            /* [in] */ BSTR SRM,
            /* [in] */ long StatusTime,
            /* [in][in] */ BSTR user,
            /* [in] */ BSTR password,
            /* [in] */ BSTR sid,
            /* [in] */ long steps,
            /* [in] */ long HourStart,
            /* [in] */ long NumofHours,
            /* [in] */ VARIANT_BOOL isDebug,
            /* [in] */ BSTR sccd,
            /* [in] */ BSTR sRainType,
            /* [in] */ float UpInitWaterContent,
            /* [in] */ float MidInitWaterContent,
            /* [in] */ float DownInitWaterContent,
            /* [in] */ BSTR emethod,
            /* [in] */ float thetab,
            /* [in] */ float thetaw,
            /* [in] */ int N,
            /* [in] */ float E0_a,
            /* [in] */ BSTR SoilErosionEquation) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE calc( 
            /* [in] */ BSTR SnowModelType,
            /* [in] */ struct BSCode mBSCode,
            /* [in] */ struct Para mPara,
            /* [in] */ FLOAT *pQin,
            /* [in] */ FLOAT *pSin,
            /* [in] */ FLOAT *pWLM,
            /* [in] */ FLOAT *pWRM) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE Finalize( 
            /* [in] */ BSTR SRM) = 0;
        
    };
    
    
#else 	/* C style interface */

    typedef struct IWaterBasinVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IWaterBasin * This,
            /* [in] */ REFIID riid,
            /* [annotation][iid_is][out] */ 
            _COM_Outptr_  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IWaterBasin * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IWaterBasin * This);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *Initialize )( 
            IWaterBasin * This,
            /* [in] */ BSTR name,
            /* [in] */ long rank,
            /* [in] */ BSTR SRM,
            /* [in] */ long StatusTime,
            /* [in][in] */ BSTR user,
            /* [in] */ BSTR password,
            /* [in] */ BSTR sid,
            /* [in] */ long steps,
            /* [in] */ long HourStart,
            /* [in] */ long NumofHours,
            /* [in] */ VARIANT_BOOL isDebug,
            /* [in] */ BSTR sccd,
            /* [in] */ BSTR sRainType,
            /* [in] */ float UpInitWaterContent,
            /* [in] */ float MidInitWaterContent,
            /* [in] */ float DownInitWaterContent,
            /* [in] */ BSTR emethod,
            /* [in] */ float thetab,
            /* [in] */ float thetaw,
            /* [in] */ int N,
            /* [in] */ float E0_a,
            /* [in] */ BSTR SoilErosionEquation);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *calc )( 
            IWaterBasin * This,
            /* [in] */ BSTR SnowModelType,
            /* [in] */ struct BSCode mBSCode,
            /* [in] */ struct Para mPara,
            /* [in] */ FLOAT *pQin,
            /* [in] */ FLOAT *pSin,
            /* [in] */ FLOAT *pWLM,
            /* [in] */ FLOAT *pWRM);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *Finalize )( 
            IWaterBasin * This,
            /* [in] */ BSTR SRM);
        
        END_INTERFACE
    } IWaterBasinVtbl;

    interface IWaterBasin
    {
        CONST_VTBL struct IWaterBasinVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IWaterBasin_QueryInterface(This,riid,ppvObject)	\
    ( (This)->lpVtbl -> QueryInterface(This,riid,ppvObject) ) 

#define IWaterBasin_AddRef(This)	\
    ( (This)->lpVtbl -> AddRef(This) ) 

#define IWaterBasin_Release(This)	\
    ( (This)->lpVtbl -> Release(This) ) 


#define IWaterBasin_Initialize(This,name,rank,SRM,StatusTime,user,password,sid,steps,HourStart,NumofHours,isDebug,sccd,sRainType,UpInitWaterContent,MidInitWaterContent,DownInitWaterContent,emethod,thetab,thetaw,N,E0_a,SoilErosionEquation)	\
    ( (This)->lpVtbl -> Initialize(This,name,rank,SRM,StatusTime,user,password,sid,steps,HourStart,NumofHours,isDebug,sccd,sRainType,UpInitWaterContent,MidInitWaterContent,DownInitWaterContent,emethod,thetab,thetaw,N,E0_a,SoilErosionEquation) ) 

#define IWaterBasin_calc(This,SnowModelType,mBSCode,mPara,pQin,pSin,pWLM,pWRM)	\
    ( (This)->lpVtbl -> calc(This,SnowModelType,mBSCode,mPara,pQin,pSin,pWLM,pWRM) ) 

#define IWaterBasin_Finalize(This,SRM)	\
    ( (This)->lpVtbl -> Finalize(This,SRM) ) 

#endif /* COBJMACROS */


#endif 	/* C style interface */




#endif 	/* __IWaterBasin_INTERFACE_DEFINED__ */



#ifndef __WaterYieldLib_LIBRARY_DEFINED__
#define __WaterYieldLib_LIBRARY_DEFINED__

/* library WaterYieldLib */
/* [helpstring][version][uuid] */ 


EXTERN_C const IID LIBID_WaterYieldLib;

EXTERN_C const CLSID CLSID_WaterBasin;

#ifdef __cplusplus

class DECLSPEC_UUID("7F623663-C6B7-43E3-A6A2-765C3EAEBD00")
WaterBasin;
#endif
#endif /* __WaterYieldLib_LIBRARY_DEFINED__ */

/* Additional Prototypes for ALL interfaces */

unsigned long             __RPC_USER  BSTR_UserSize(     unsigned long *, unsigned long            , BSTR * ); 
unsigned char * __RPC_USER  BSTR_UserMarshal(  unsigned long *, unsigned char *, BSTR * ); 
unsigned char * __RPC_USER  BSTR_UserUnmarshal(unsigned long *, unsigned char *, BSTR * ); 
void                      __RPC_USER  BSTR_UserFree(     unsigned long *, BSTR * ); 

/* end of Additional Prototypes */

#ifdef __cplusplus
}
#endif

#endif


