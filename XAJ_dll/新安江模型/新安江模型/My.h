

/* this ALWAYS GENERATED file contains the definitions for the interfaces */


 /* File created by MIDL compiler version 6.00.0366 */
/* at Wed Aug 12 15:18:59 2009
 */
/* Compiler settings for .\My.idl:
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

#ifndef __My_h__
#define __My_h__

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

/* Forward Declarations */ 

#ifndef __IMy_FWD_DEFINED__
#define __IMy_FWD_DEFINED__
typedef interface IMy IMy;
#endif 	/* __IMy_FWD_DEFINED__ */


#ifndef __My_FWD_DEFINED__
#define __My_FWD_DEFINED__

#ifdef __cplusplus
typedef class My My;
#else
typedef struct My My;
#endif /* __cplusplus */

#endif 	/* __My_FWD_DEFINED__ */


/* header files for imported files */
#include "oaidl.h"
#include "ocidl.h"
#include "InterfaceStruct.h"

#ifdef __cplusplus
extern "C"{
#endif 

void * __RPC_USER MIDL_user_allocate(size_t);
void __RPC_USER MIDL_user_free( void * ); 

#ifndef __IMy_INTERFACE_DEFINED__
#define __IMy_INTERFACE_DEFINED__

/* interface IMy */
/* [unique][helpstring][uuid][object] */ 


EXTERN_C const IID IID_IMy;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("B54F4697-CFE4-4A6B-86D5-897C26CC3662")
    IMy : public IUnknown
    {
    public:
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE Initialize( 
            /* [in] */ BSTR SRM,
            /* [in] */ FLOAT BasinArea,
            /* [in] */ struct XAJParameter XAJ,
            /* [in] */ BSTR sccd,
            /* [in] */ BSTR user,
            /* [in] */ BSTR password,
            /* [in] */ BSTR sid,
            /* [in] */ BSTR RainType,
            /* [in] */ LONG HourStart,
            /* [in] */ LONG NumofHours,
            /* [in] */ LONG TimeStep,
            /* [in] */ LONG Steps,
            /* [in] */ LONG StatusTime) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE Calc( 
            /* [in] */ BSTR SnowModelType,
            /* [in] */ struct BSCode mBSCode,
            /* [in] */ struct Para mPara,
            /* [in] */ FLOAT *pQin,
            /* [in] */ FLOAT *pWLM,
            /* [in] */ FLOAT *pWRM) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE Finalize( 
            /* [in] */ BSTR SRM) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IMyVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IMy * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IMy * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IMy * This);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE *Initialize )( 
            IMy * This,
            /* [in] */ BSTR SRM,
            /* [in] */ FLOAT BasinArea,
            /* [in] */ struct XAJParameter XAJ,
            /* [in] */ BSTR sccd,
            /* [in] */ BSTR user,
            /* [in] */ BSTR password,
            /* [in] */ BSTR sid,
            /* [in] */ BSTR RainType,
            /* [in] */ LONG HourStart,
            /* [in] */ LONG NumofHours,
            /* [in] */ LONG TimeStep,
            /* [in] */ LONG Steps,
            /* [in] */ LONG StatusTime);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE *Calc )( 
            IMy * This,
            /* [in] */ BSTR SnowModelType,
            /* [in] */ struct BSCode mBSCode,
            /* [in] */ struct Para mPara,
            /* [in] */ FLOAT *pQin,
            /* [in] */ FLOAT *pWLM,
            /* [in] */ FLOAT *pWRM);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE *Finalize )( 
            IMy * This,
            /* [in] */ BSTR SRM);
        
        END_INTERFACE
    } IMyVtbl;

    interface IMy
    {
        CONST_VTBL struct IMyVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IMy_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IMy_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IMy_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IMy_Initialize(This,SRM,BasinArea,XAJ,sccd,user,password,sid,RainType,HourStart,NumofHours,TimeStep,Steps,StatusTime)	\
    (This)->lpVtbl -> Initialize(This,SRM,BasinArea,XAJ,sccd,user,password,sid,RainType,HourStart,NumofHours,TimeStep,Steps,StatusTime)

#define IMy_Calc(This,SnowModelType,mBSCode,mPara,pQin,pWLM,pWRM)	\
    (This)->lpVtbl -> Calc(This,SnowModelType,mBSCode,mPara,pQin,pWLM,pWRM)

#define IMy_Finalize(This,SRM)	\
    (This)->lpVtbl -> Finalize(This,SRM)

#endif /* COBJMACROS */


#endif 	/* C style interface */



/* [helpstring] */ HRESULT STDMETHODCALLTYPE IMy_Initialize_Proxy( 
    IMy * This,
    /* [in] */ BSTR SRM,
    /* [in] */ FLOAT BasinArea,
    /* [in] */ struct XAJParameter XAJ,
    /* [in] */ BSTR sccd,
    /* [in] */ BSTR user,
    /* [in] */ BSTR password,
    /* [in] */ BSTR sid,
    /* [in] */ BSTR RainType,
    /* [in] */ LONG HourStart,
    /* [in] */ LONG NumofHours,
    /* [in] */ LONG TimeStep,
    /* [in] */ LONG Steps,
    /* [in] */ LONG StatusTime);


void __RPC_STUB IMy_Initialize_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring] */ HRESULT STDMETHODCALLTYPE IMy_Calc_Proxy( 
    IMy * This,
    /* [in] */ BSTR SnowModelType,
    /* [in] */ struct BSCode mBSCode,
    /* [in] */ struct Para mPara,
    /* [in] */ FLOAT *pQin,
    /* [in] */ FLOAT *pWLM,
    /* [in] */ FLOAT *pWRM);


void __RPC_STUB IMy_Calc_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring] */ HRESULT STDMETHODCALLTYPE IMy_Finalize_Proxy( 
    IMy * This,
    /* [in] */ BSTR SRM);


void __RPC_STUB IMy_Finalize_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __IMy_INTERFACE_DEFINED__ */



#ifndef __新安江模型Lib_LIBRARY_DEFINED__
#define __新安江模型Lib_LIBRARY_DEFINED__

/* library 新安江模型Lib */
/* [helpstring][version][uuid] */ 


EXTERN_C const IID LIBID_新安江模型Lib;

EXTERN_C const CLSID CLSID_My;

#ifdef __cplusplus

class DECLSPEC_UUID("4C5338FF-E85A-49FC-930D-D3FCC6A1B6C2")
My;
#endif
#endif /* __新安江模型Lib_LIBRARY_DEFINED__ */

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


