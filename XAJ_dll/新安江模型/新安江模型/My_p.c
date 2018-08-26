

/* this ALWAYS GENERATED file contains the proxy stub code */


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

#if !defined(_M_IA64) && !defined(_M_AMD64)


#pragma warning( disable: 4049 )  /* more than 64k source lines */
#if _MSC_VER >= 1200
#pragma warning(push)
#endif
#pragma warning( disable: 4100 ) /* unreferenced arguments in x86 call */
#pragma warning( disable: 4211 )  /* redefine extent to static */
#pragma warning( disable: 4232 )  /* dllimport identity*/
#pragma optimize("", off ) 

#define USE_STUBLESS_PROXY


/* verify that the <rpcproxy.h> version is high enough to compile this file*/
#ifndef __REDQ_RPCPROXY_H_VERSION__
#define __REQUIRED_RPCPROXY_H_VERSION__ 440
#endif


#include "rpcproxy.h"
#ifndef __RPCPROXY_H_VERSION__
#error this stub requires an updated version of <rpcproxy.h>
#endif // __RPCPROXY_H_VERSION__


#include "My.h"

#define TYPE_FORMAT_STRING_SIZE   127                               
#define PROC_FORMAT_STRING_SIZE   187                               
#define TRANSMIT_AS_TABLE_SIZE    0            
#define WIRE_MARSHAL_TABLE_SIZE   1            

typedef struct _MIDL_TYPE_FORMAT_STRING
    {
    short          Pad;
    unsigned char  Format[ TYPE_FORMAT_STRING_SIZE ];
    } MIDL_TYPE_FORMAT_STRING;

typedef struct _MIDL_PROC_FORMAT_STRING
    {
    short          Pad;
    unsigned char  Format[ PROC_FORMAT_STRING_SIZE ];
    } MIDL_PROC_FORMAT_STRING;


static RPC_SYNTAX_IDENTIFIER  _RpcTransferSyntax = 
{{0x8A885D04,0x1CEB,0x11C9,{0x9F,0xE8,0x08,0x00,0x2B,0x10,0x48,0x60}},{2,0}};


extern const MIDL_TYPE_FORMAT_STRING __MIDL_TypeFormatString;
extern const MIDL_PROC_FORMAT_STRING __MIDL_ProcFormatString;


extern const MIDL_STUB_DESC Object_StubDesc;


extern const MIDL_SERVER_INFO IMy_ServerInfo;
extern const MIDL_STUBLESS_PROXY_INFO IMy_ProxyInfo;


extern const USER_MARSHAL_ROUTINE_QUADRUPLE UserMarshalRoutines[ WIRE_MARSHAL_TABLE_SIZE ];

#if !defined(__RPC_WIN32__)
#error  Invalid build platform for this stub.
#endif

#if !(TARGET_IS_NT40_OR_LATER)
#error You need a Windows NT 4.0 or later to run this stub because it uses these features:
#error   -Oif or -Oicf, [wire_marshal] or [user_marshal] attribute.
#error However, your C/C++ compilation flags indicate you intend to run this app on earlier systems.
#error This app will die there with the RPC_X_WRONG_STUB_VERSION error.
#endif


static const MIDL_PROC_FORMAT_STRING __MIDL_ProcFormatString =
    {
        0,
        {

	/* Procedure Initialize */

			0x33,		/* FC_AUTO_HANDLE */
			0x6c,		/* Old Flags:  object, Oi2 */
/*  2 */	NdrFcLong( 0x0 ),	/* 0 */
/*  6 */	NdrFcShort( 0x3 ),	/* 3 */
/*  8 */	NdrFcShort( 0x88 ),	/* x86 Stack size/offset = 136 */
/* 10 */	NdrFcShort( 0x90 ),	/* 144 */
/* 12 */	NdrFcShort( 0x8 ),	/* 8 */
/* 14 */	0x6,		/* Oi2 Flags:  clt must size, has return, */
			0xe,		/* 14 */

	/* Parameter SRM */

/* 16 */	NdrFcShort( 0x8b ),	/* Flags:  must size, must free, in, by val, */
/* 18 */	NdrFcShort( 0x4 ),	/* x86 Stack size/offset = 4 */
/* 20 */	NdrFcShort( 0x1a ),	/* Type Offset=26 */

	/* Parameter BasinArea */

/* 22 */	NdrFcShort( 0x48 ),	/* Flags:  in, base type, */
/* 24 */	NdrFcShort( 0x8 ),	/* x86 Stack size/offset = 8 */
/* 26 */	0xa,		/* FC_FLOAT */
			0x0,		/* 0 */

	/* Parameter XAJ */

/* 28 */	NdrFcShort( 0x8a ),	/* Flags:  must free, in, by val, */
/* 30 */	NdrFcShort( 0xc ),	/* x86 Stack size/offset = 12 */
/* 32 */	NdrFcShort( 0x24 ),	/* Type Offset=36 */

	/* Parameter sccd */

/* 34 */	NdrFcShort( 0x8b ),	/* Flags:  must size, must free, in, by val, */
/* 36 */	NdrFcShort( 0x5c ),	/* x86 Stack size/offset = 92 */
/* 38 */	NdrFcShort( 0x1a ),	/* Type Offset=26 */

	/* Parameter user */

/* 40 */	NdrFcShort( 0x8b ),	/* Flags:  must size, must free, in, by val, */
/* 42 */	NdrFcShort( 0x60 ),	/* x86 Stack size/offset = 96 */
/* 44 */	NdrFcShort( 0x1a ),	/* Type Offset=26 */

	/* Parameter password */

/* 46 */	NdrFcShort( 0x8b ),	/* Flags:  must size, must free, in, by val, */
/* 48 */	NdrFcShort( 0x64 ),	/* x86 Stack size/offset = 100 */
/* 50 */	NdrFcShort( 0x1a ),	/* Type Offset=26 */

	/* Parameter sid */

/* 52 */	NdrFcShort( 0x8b ),	/* Flags:  must size, must free, in, by val, */
/* 54 */	NdrFcShort( 0x68 ),	/* x86 Stack size/offset = 104 */
/* 56 */	NdrFcShort( 0x1a ),	/* Type Offset=26 */

	/* Parameter RainType */

/* 58 */	NdrFcShort( 0x8b ),	/* Flags:  must size, must free, in, by val, */
/* 60 */	NdrFcShort( 0x6c ),	/* x86 Stack size/offset = 108 */
/* 62 */	NdrFcShort( 0x1a ),	/* Type Offset=26 */

	/* Parameter HourStart */

/* 64 */	NdrFcShort( 0x48 ),	/* Flags:  in, base type, */
/* 66 */	NdrFcShort( 0x70 ),	/* x86 Stack size/offset = 112 */
/* 68 */	0x8,		/* FC_LONG */
			0x0,		/* 0 */

	/* Parameter NumofHours */

/* 70 */	NdrFcShort( 0x48 ),	/* Flags:  in, base type, */
/* 72 */	NdrFcShort( 0x74 ),	/* x86 Stack size/offset = 116 */
/* 74 */	0x8,		/* FC_LONG */
			0x0,		/* 0 */

	/* Parameter TimeStep */

/* 76 */	NdrFcShort( 0x48 ),	/* Flags:  in, base type, */
/* 78 */	NdrFcShort( 0x78 ),	/* x86 Stack size/offset = 120 */
/* 80 */	0x8,		/* FC_LONG */
			0x0,		/* 0 */

	/* Parameter Steps */

/* 82 */	NdrFcShort( 0x48 ),	/* Flags:  in, base type, */
/* 84 */	NdrFcShort( 0x7c ),	/* x86 Stack size/offset = 124 */
/* 86 */	0x8,		/* FC_LONG */
			0x0,		/* 0 */

	/* Parameter StatusTime */

/* 88 */	NdrFcShort( 0x48 ),	/* Flags:  in, base type, */
/* 90 */	NdrFcShort( 0x80 ),	/* x86 Stack size/offset = 128 */
/* 92 */	0x8,		/* FC_LONG */
			0x0,		/* 0 */

	/* Return value */

/* 94 */	NdrFcShort( 0x70 ),	/* Flags:  out, return, base type, */
/* 96 */	NdrFcShort( 0x84 ),	/* x86 Stack size/offset = 132 */
/* 98 */	0x8,		/* FC_LONG */
			0x0,		/* 0 */

	/* Procedure Calc */

/* 100 */	0x33,		/* FC_AUTO_HANDLE */
			0x6c,		/* Old Flags:  object, Oi2 */
/* 102 */	NdrFcLong( 0x0 ),	/* 0 */
/* 106 */	NdrFcShort( 0x4 ),	/* 4 */
/* 108 */	NdrFcShort( 0xe0 ),	/* x86 Stack size/offset = 224 */
/* 110 */	NdrFcShort( 0x13c ),	/* 316 */
/* 112 */	NdrFcShort( 0x8 ),	/* 8 */
/* 114 */	0x6,		/* Oi2 Flags:  clt must size, has return, */
			0x7,		/* 7 */

	/* Parameter SnowModelType */

/* 116 */	NdrFcShort( 0x8b ),	/* Flags:  must size, must free, in, by val, */
/* 118 */	NdrFcShort( 0x4 ),	/* x86 Stack size/offset = 4 */
/* 120 */	NdrFcShort( 0x1a ),	/* Type Offset=26 */

	/* Parameter mBSCode */

/* 122 */	NdrFcShort( 0x8a ),	/* Flags:  must free, in, by val, */
/* 124 */	NdrFcShort( 0x8 ),	/* x86 Stack size/offset = 8 */
/* 126 */	NdrFcShort( 0x3e ),	/* Type Offset=62 */

	/* Parameter mPara */

/* 128 */	NdrFcShort( 0x8a ),	/* Flags:  must free, in, by val, */
/* 130 */	NdrFcShort( 0x28 ),	/* x86 Stack size/offset = 40 */
/* 132 */	NdrFcShort( 0x4a ),	/* Type Offset=74 */

	/* Parameter pQin */

/* 134 */	NdrFcShort( 0x148 ),	/* Flags:  in, base type, simple ref, */
/* 136 */	NdrFcShort( 0xd0 ),	/* x86 Stack size/offset = 208 */
/* 138 */	0xa,		/* FC_FLOAT */
			0x0,		/* 0 */

	/* Parameter pWLM */

/* 140 */	NdrFcShort( 0x148 ),	/* Flags:  in, base type, simple ref, */
/* 142 */	NdrFcShort( 0xd4 ),	/* x86 Stack size/offset = 212 */
/* 144 */	0xa,		/* FC_FLOAT */
			0x0,		/* 0 */

	/* Parameter pWRM */

/* 146 */	NdrFcShort( 0x148 ),	/* Flags:  in, base type, simple ref, */
/* 148 */	NdrFcShort( 0xd8 ),	/* x86 Stack size/offset = 216 */
/* 150 */	0xa,		/* FC_FLOAT */
			0x0,		/* 0 */

	/* Return value */

/* 152 */	NdrFcShort( 0x70 ),	/* Flags:  out, return, base type, */
/* 154 */	NdrFcShort( 0xdc ),	/* x86 Stack size/offset = 220 */
/* 156 */	0x8,		/* FC_LONG */
			0x0,		/* 0 */

	/* Procedure Finalize */

/* 158 */	0x33,		/* FC_AUTO_HANDLE */
			0x6c,		/* Old Flags:  object, Oi2 */
/* 160 */	NdrFcLong( 0x0 ),	/* 0 */
/* 164 */	NdrFcShort( 0x5 ),	/* 5 */
/* 166 */	NdrFcShort( 0xc ),	/* x86 Stack size/offset = 12 */
/* 168 */	NdrFcShort( 0x0 ),	/* 0 */
/* 170 */	NdrFcShort( 0x8 ),	/* 8 */
/* 172 */	0x6,		/* Oi2 Flags:  clt must size, has return, */
			0x2,		/* 2 */

	/* Parameter SRM */

/* 174 */	NdrFcShort( 0x8b ),	/* Flags:  must size, must free, in, by val, */
/* 176 */	NdrFcShort( 0x4 ),	/* x86 Stack size/offset = 4 */
/* 178 */	NdrFcShort( 0x1a ),	/* Type Offset=26 */

	/* Return value */

/* 180 */	NdrFcShort( 0x70 ),	/* Flags:  out, return, base type, */
/* 182 */	NdrFcShort( 0x8 ),	/* x86 Stack size/offset = 8 */
/* 184 */	0x8,		/* FC_LONG */
			0x0,		/* 0 */

			0x0
        }
    };

static const MIDL_TYPE_FORMAT_STRING __MIDL_TypeFormatString =
    {
        0,
        {
			NdrFcShort( 0x0 ),	/* 0 */
/*  2 */	
			0x12, 0x0,	/* FC_UP */
/*  4 */	NdrFcShort( 0xc ),	/* Offset= 12 (16) */
/*  6 */	
			0x1b,		/* FC_CARRAY */
			0x1,		/* 1 */
/*  8 */	NdrFcShort( 0x2 ),	/* 2 */
/* 10 */	0x9,		/* Corr desc: FC_ULONG */
			0x0,		/*  */
/* 12 */	NdrFcShort( 0xfffc ),	/* -4 */
/* 14 */	0x6,		/* FC_SHORT */
			0x5b,		/* FC_END */
/* 16 */	
			0x17,		/* FC_CSTRUCT */
			0x3,		/* 3 */
/* 18 */	NdrFcShort( 0x8 ),	/* 8 */
/* 20 */	NdrFcShort( 0xfff2 ),	/* Offset= -14 (6) */
/* 22 */	0x8,		/* FC_LONG */
			0x8,		/* FC_LONG */
/* 24 */	0x5c,		/* FC_PAD */
			0x5b,		/* FC_END */
/* 26 */	0xb4,		/* FC_USER_MARSHAL */
			0x83,		/* 131 */
/* 28 */	NdrFcShort( 0x0 ),	/* 0 */
/* 30 */	NdrFcShort( 0x4 ),	/* 4 */
/* 32 */	NdrFcShort( 0x0 ),	/* 0 */
/* 34 */	NdrFcShort( 0xffe0 ),	/* Offset= -32 (2) */
/* 36 */	
			0x15,		/* FC_STRUCT */
			0x3,		/* 3 */
/* 38 */	NdrFcShort( 0x50 ),	/* 80 */
/* 40 */	0x8,		/* FC_LONG */
			0x8,		/* FC_LONG */
/* 42 */	0x8,		/* FC_LONG */
			0xa,		/* FC_FLOAT */
/* 44 */	0xa,		/* FC_FLOAT */
			0xa,		/* FC_FLOAT */
/* 46 */	0x8,		/* FC_LONG */
			0xa,		/* FC_FLOAT */
/* 48 */	0xa,		/* FC_FLOAT */
			0xa,		/* FC_FLOAT */
/* 50 */	0xa,		/* FC_FLOAT */
			0xa,		/* FC_FLOAT */
/* 52 */	0x8,		/* FC_LONG */
			0x8,		/* FC_LONG */
/* 54 */	0x8,		/* FC_LONG */
			0x8,		/* FC_LONG */
/* 56 */	0xa,		/* FC_FLOAT */
			0x8,		/* FC_LONG */
/* 58 */	0x8,		/* FC_LONG */
			0x8,		/* FC_LONG */
/* 60 */	0x5c,		/* FC_PAD */
			0x5b,		/* FC_END */
/* 62 */	
			0x15,		/* FC_STRUCT */
			0x7,		/* 7 */
/* 64 */	NdrFcShort( 0x20 ),	/* 32 */
/* 66 */	0x8,		/* FC_LONG */
			0x40,		/* FC_STRUCTPAD4 */
/* 68 */	0xb,		/* FC_HYPER */
			0x8,		/* FC_LONG */
/* 70 */	0x40,		/* FC_STRUCTPAD4 */
			0xb,		/* FC_HYPER */
/* 72 */	0x5c,		/* FC_PAD */
			0x5b,		/* FC_END */
/* 74 */	
			0x15,		/* FC_STRUCT */
			0x3,		/* 3 */
/* 76 */	NdrFcShort( 0xa8 ),	/* 168 */
/* 78 */	0xa,		/* FC_FLOAT */
			0xa,		/* FC_FLOAT */
/* 80 */	0xa,		/* FC_FLOAT */
			0xa,		/* FC_FLOAT */
/* 82 */	0xa,		/* FC_FLOAT */
			0xa,		/* FC_FLOAT */
/* 84 */	0xa,		/* FC_FLOAT */
			0xa,		/* FC_FLOAT */
/* 86 */	0xa,		/* FC_FLOAT */
			0xa,		/* FC_FLOAT */
/* 88 */	0xa,		/* FC_FLOAT */
			0xa,		/* FC_FLOAT */
/* 90 */	0xa,		/* FC_FLOAT */
			0x8,		/* FC_LONG */
/* 92 */	0x8,		/* FC_LONG */
			0xa,		/* FC_FLOAT */
/* 94 */	0xa,		/* FC_FLOAT */
			0xa,		/* FC_FLOAT */
/* 96 */	0xa,		/* FC_FLOAT */
			0xa,		/* FC_FLOAT */
/* 98 */	0xa,		/* FC_FLOAT */
			0xa,		/* FC_FLOAT */
/* 100 */	0xa,		/* FC_FLOAT */
			0xa,		/* FC_FLOAT */
/* 102 */	0xa,		/* FC_FLOAT */
			0xa,		/* FC_FLOAT */
/* 104 */	0xa,		/* FC_FLOAT */
			0xa,		/* FC_FLOAT */
/* 106 */	0xa,		/* FC_FLOAT */
			0xa,		/* FC_FLOAT */
/* 108 */	0xa,		/* FC_FLOAT */
			0xa,		/* FC_FLOAT */
/* 110 */	0xa,		/* FC_FLOAT */
			0xa,		/* FC_FLOAT */
/* 112 */	0xa,		/* FC_FLOAT */
			0xa,		/* FC_FLOAT */
/* 114 */	0xa,		/* FC_FLOAT */
			0xa,		/* FC_FLOAT */
/* 116 */	0xa,		/* FC_FLOAT */
			0xa,		/* FC_FLOAT */
/* 118 */	0xa,		/* FC_FLOAT */
			0xa,		/* FC_FLOAT */
/* 120 */	0x5c,		/* FC_PAD */
			0x5b,		/* FC_END */
/* 122 */	
			0x11, 0x8,	/* FC_RP [simple_pointer] */
/* 124 */	0xa,		/* FC_FLOAT */
			0x5c,		/* FC_PAD */

			0x0
        }
    };

static const USER_MARSHAL_ROUTINE_QUADRUPLE UserMarshalRoutines[ WIRE_MARSHAL_TABLE_SIZE ] = 
        {
            
            {
            BSTR_UserSize
            ,BSTR_UserMarshal
            ,BSTR_UserUnmarshal
            ,BSTR_UserFree
            }

        };



/* Object interface: IUnknown, ver. 0.0,
   GUID={0x00000000,0x0000,0x0000,{0xC0,0x00,0x00,0x00,0x00,0x00,0x00,0x46}} */


/* Object interface: IMy, ver. 0.0,
   GUID={0xB54F4697,0xCFE4,0x4A6B,{0x86,0xD5,0x89,0x7C,0x26,0xCC,0x36,0x62}} */

#pragma code_seg(".orpc")
static const unsigned short IMy_FormatStringOffsetTable[] =
    {
    0,
    100,
    158
    };

static const MIDL_STUBLESS_PROXY_INFO IMy_ProxyInfo =
    {
    &Object_StubDesc,
    __MIDL_ProcFormatString.Format,
    &IMy_FormatStringOffsetTable[-3],
    0,
    0,
    0
    };


static const MIDL_SERVER_INFO IMy_ServerInfo = 
    {
    &Object_StubDesc,
    0,
    __MIDL_ProcFormatString.Format,
    &IMy_FormatStringOffsetTable[-3],
    0,
    0,
    0,
    0};
CINTERFACE_PROXY_VTABLE(6) _IMyProxyVtbl = 
{
    &IMy_ProxyInfo,
    &IID_IMy,
    IUnknown_QueryInterface_Proxy,
    IUnknown_AddRef_Proxy,
    IUnknown_Release_Proxy ,
    (void *) (INT_PTR) -1 /* IMy::Initialize */ ,
    (void *) (INT_PTR) -1 /* IMy::Calc */ ,
    (void *) (INT_PTR) -1 /* IMy::Finalize */
};

const CInterfaceStubVtbl _IMyStubVtbl =
{
    &IID_IMy,
    &IMy_ServerInfo,
    6,
    0, /* pure interpreted */
    CStdStubBuffer_METHODS
};

static const MIDL_STUB_DESC Object_StubDesc = 
    {
    0,
    NdrOleAllocate,
    NdrOleFree,
    0,
    0,
    0,
    0,
    0,
    __MIDL_TypeFormatString.Format,
    1, /* -error bounds_check flag */
    0x20000, /* Ndr library version */
    0,
    0x600016e, /* MIDL Version 6.0.366 */
    0,
    UserMarshalRoutines,
    0,  /* notify & notify_flag routine table */
    0x1, /* MIDL flag */
    0, /* cs routines */
    0,   /* proxy/server info */
    0   /* Reserved5 */
    };

const CInterfaceProxyVtbl * _My_ProxyVtblList[] = 
{
    ( CInterfaceProxyVtbl *) &_IMyProxyVtbl,
    0
};

const CInterfaceStubVtbl * _My_StubVtblList[] = 
{
    ( CInterfaceStubVtbl *) &_IMyStubVtbl,
    0
};

PCInterfaceName const _My_InterfaceNamesList[] = 
{
    "IMy",
    0
};


#define _My_CHECK_IID(n)	IID_GENERIC_CHECK_IID( _My, pIID, n)

int __stdcall _My_IID_Lookup( const IID * pIID, int * pIndex )
{
    
    if(!_My_CHECK_IID(0))
        {
        *pIndex = 0;
        return 1;
        }

    return 0;
}

const ExtendedProxyFileInfo My_ProxyFileInfo = 
{
    (PCInterfaceProxyVtblList *) & _My_ProxyVtblList,
    (PCInterfaceStubVtblList *) & _My_StubVtblList,
    (const PCInterfaceName * ) & _My_InterfaceNamesList,
    0, // no delegation
    & _My_IID_Lookup, 
    1,
    2,
    0, /* table of [async_uuid] interfaces */
    0, /* Filler1 */
    0, /* Filler2 */
    0  /* Filler3 */
};
#pragma optimize("", on )
#if _MSC_VER >= 1200
#pragma warning(pop)
#endif


#endif /* !defined(_M_IA64) && !defined(_M_AMD64)*/

