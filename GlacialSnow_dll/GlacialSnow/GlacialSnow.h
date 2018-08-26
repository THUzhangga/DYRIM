

/* this ALWAYS GENERATED file contains the definitions for the interfaces */


 /* File created by MIDL compiler version 6.00.0366 */
/* at Tue Dec 16 16:02:15 2008
 */
/* Compiler settings for .\GlacialSnow.idl:
    Oicf, W1, Zp8, env=Win32 (32b run)
    protocol : dce , ms_ext, c_ext
    error checks: allocation ref bounds_check enum stub_data 
    VC __declspec() decoration level: 
         __declspec(uuid()), __declspec(selectany), __declspec(novtable)
         DECLSPEC_UUID(), MIDL_INTERFACE()
*/
//@@MIDL_FILE_HEADING(  )

#pragma warning( disable: 4049 )  /* more than 64k source lines */


/* verify that the <rpcndr.h> version is high enough to compile this file*/
#ifndef __REQUIRED_RPCNDR_H_VERSION__
#define __REQUIRED_RPCNDR_H_VERSION__ 440
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

#ifndef __GlacialSnow_h__
#define __GlacialSnow_h__

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

/* Forward Declarations */ 

#ifndef __ISRM_FWD_DEFINED__
#define __ISRM_FWD_DEFINED__
typedef interface ISRM ISRM;
#endif 	/* __ISRM_FWD_DEFINED__ */


#ifndef __SRM_FWD_DEFINED__
#define __SRM_FWD_DEFINED__

#ifdef __cplusplus
typedef class SRM SRM;
#else
typedef struct SRM SRM;
#endif /* __cplusplus */

#endif 	/* __SRM_FWD_DEFINED__ */


/* header files for imported files */
#include "oaidl.h"
#include "ocidl.h"
#include "HourHappen.h"
#include "msado15.h"

#ifdef __cplusplus
extern "C"{
#endif 

void * __RPC_USER MIDL_user_allocate(size_t);
void __RPC_USER MIDL_user_free( void * ); 

#ifndef __ISRM_INTERFACE_DEFINED__
#define __ISRM_INTERFACE_DEFINED__

/* interface ISRM */
/* [unique][helpstring][nonextensible][dual][uuid][object] */ 


EXTERN_C const IID IID_ISRM;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("89745127-6161-4B13-907B-91F901F2A1C4")
    ISRM : public IDispatch
    {
    public:
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE OpenOracle( 
            /* [in] */ BSTR user,
            /* [in] */ BSTR password,
            /* [in] */ BSTR sid) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE SnowInitialize( 
            /* [in] */ FLOAT myParaX,
            /* [in] */ FLOAT myParaY,
            /* [in] */ ULONGLONG RegionIndex,
            /* [in] */ LONG Length,
            /* [in] */ ULONGLONG Value,
            FLOAT A,
            FLOAT Havg) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE SnowCalc( 
            /* [in] */ LONG Houroffset,
            /* [in] */ FLOAT *HourRain) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE SRMFinalize( void) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE ReleaseHeap( void) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct ISRMVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ISRM * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ISRM * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ISRM * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            ISRM * This,
            /* [out] */ UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            ISRM * This,
            /* [in] */ UINT iTInfo,
            /* [in] */ LCID lcid,
            /* [out] */ ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            ISRM * This,
            /* [in] */ REFIID riid,
            /* [size_is][in] */ LPOLESTR *rgszNames,
            /* [in] */ UINT cNames,
            /* [in] */ LCID lcid,
            /* [size_is][out] */ DISPID *rgDispId);
        
        /* [local] */ HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            ISRM * This,
            /* [in] */ DISPID dispIdMember,
            /* [in] */ REFIID riid,
            /* [in] */ LCID lcid,
            /* [in] */ WORD wFlags,
            /* [out][in] */ DISPPARAMS *pDispParams,
            /* [out] */ VARIANT *pVarResult,
            /* [out] */ EXCEPINFO *pExcepInfo,
            /* [out] */ UINT *puArgErr);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *OpenOracle )( 
            ISRM * This,
            /* [in] */ BSTR user,
            /* [in] */ BSTR password,
            /* [in] */ BSTR sid);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *SnowInitialize )( 
            ISRM * This,
            /* [in] */ FLOAT myParaX,
            /* [in] */ FLOAT myParaY,
            /* [in] */ ULONGLONG RegionIndex,
            /* [in] */ LONG Length,
            /* [in] */ ULONGLONG Value,
            FLOAT A,
            FLOAT Havg);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *SnowCalc )( 
            ISRM * This,
            /* [in] */ LONG Houroffset,
            /* [in] */ FLOAT *HourRain);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *SRMFinalize )( 
            ISRM * This);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *ReleaseHeap )( 
            ISRM * This);
        
        END_INTERFACE
    } ISRMVtbl;

    interface ISRM
    {
        CONST_VTBL struct ISRMVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ISRM_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ISRM_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ISRM_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ISRM_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define ISRM_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define ISRM_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define ISRM_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define ISRM_OpenOracle(This,user,password,sid)	\
    (This)->lpVtbl -> OpenOracle(This,user,password,sid)

#define ISRM_SnowInitialize(This,myParaX,myParaY,RegionIndex,Length,Value,A,Havg)	\
    (This)->lpVtbl -> SnowInitialize(This,myParaX,myParaY,RegionIndex,Length,Value,A,Havg)

#define ISRM_SnowCalc(This,Houroffset,HourRain)	\
    (This)->lpVtbl -> SnowCalc(This,Houroffset,HourRain)

#define ISRM_SRMFinalize(This)	\
    (This)->lpVtbl -> SRMFinalize(This)

#define ISRM_ReleaseHeap(This)	\
    (This)->lpVtbl -> ReleaseHeap(This)

#endif /* COBJMACROS */


#endif 	/* C style interface */



/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE ISRM_OpenOracle_Proxy( 
    ISRM * This,
    /* [in] */ BSTR user,
    /* [in] */ BSTR password,
    /* [in] */ BSTR sid);


void __RPC_STUB ISRM_OpenOracle_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE ISRM_SnowInitialize_Proxy( 
    ISRM * This,
    /* [in] */ FLOAT myParaX,
    /* [in] */ FLOAT myParaY,
    /* [in] */ ULONGLONG RegionIndex,
    /* [in] */ LONG Length,
    /* [in] */ ULONGLONG Value,
    FLOAT A,
    FLOAT Havg);


void __RPC_STUB ISRM_SnowInitialize_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE ISRM_SnowCalc_Proxy( 
    ISRM * This,
    /* [in] */ LONG Houroffset,
    /* [in] */ FLOAT *HourRain);


void __RPC_STUB ISRM_SnowCalc_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE ISRM_SRMFinalize_Proxy( 
    ISRM * This);


void __RPC_STUB ISRM_SRMFinalize_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE ISRM_ReleaseHeap_Proxy( 
    ISRM * This);


void __RPC_STUB ISRM_ReleaseHeap_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __ISRM_INTERFACE_DEFINED__ */



#ifndef __GlacialSnowLib_LIBRARY_DEFINED__
#define __GlacialSnowLib_LIBRARY_DEFINED__

/* library GlacialSnowLib */
/* [helpstring][version][uuid] */ 


EXTERN_C const IID LIBID_GlacialSnowLib;

EXTERN_C const CLSID CLSID_SRM;

#ifdef __cplusplus

class DECLSPEC_UUID("6CC60EA6-54D2-4170-90CB-7BD3D23AF27D")
SRM;
#endif
#endif /* __GlacialSnowLib_LIBRARY_DEFINED__ */

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


