

/* this ALWAYS GENERATED file contains the proxy stub code */


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

#if defined(_M_AMD64)


#pragma warning( disable: 4049 )  /* more than 64k source lines */
#if _MSC_VER >= 1200
#pragma warning(push)
#endif

#pragma warning( disable: 4211 )  /* redefine extern to static */
#pragma warning( disable: 4232 )  /* dllimport identity*/
#pragma warning( disable: 4024 )  /* array to pointer mapping*/
#pragma warning( disable: 4152 )  /* function/data pointer conversion in expression */

#define USE_STUBLESS_PROXY


/* verify that the <rpcproxy.h> version is high enough to compile this file*/
#ifndef __REDQ_RPCPROXY_H_VERSION__
#define __REQUIRED_RPCPROXY_H_VERSION__ 475
#endif


#include "rpcproxy.h"
#ifndef __RPCPROXY_H_VERSION__
#error this stub requires an updated version of <rpcproxy.h>
#endif /* __RPCPROXY_H_VERSION__ */


#include "WaterYield.h"

#define TYPE_FORMAT_STRING_SIZE   115                               
#define PROC_FORMAT_STRING_SIZE   277                               
#define EXPR_FORMAT_STRING_SIZE   1                                 
#define TRANSMIT_AS_TABLE_SIZE    0            
#define WIRE_MARSHAL_TABLE_SIZE   1            

typedef struct _WaterYield_MIDL_TYPE_FORMAT_STRING
    {
    short          Pad;
    unsigned char  Format[ TYPE_FORMAT_STRING_SIZE ];
    } WaterYield_MIDL_TYPE_FORMAT_STRING;

typedef struct _WaterYield_MIDL_PROC_FORMAT_STRING
    {
    short          Pad;
    unsigned char  Format[ PROC_FORMAT_STRING_SIZE ];
    } WaterYield_MIDL_PROC_FORMAT_STRING;

typedef struct _WaterYield_MIDL_EXPR_FORMAT_STRING
    {
    long          Pad;
    unsigned char  Format[ EXPR_FORMAT_STRING_SIZE ];
    } WaterYield_MIDL_EXPR_FORMAT_STRING;


static const RPC_SYNTAX_IDENTIFIER  _RpcTransferSyntax = 
{{0x8A885D04,0x1CEB,0x11C9,{0x9F,0xE8,0x08,0x00,0x2B,0x10,0x48,0x60}},{2,0}};


extern const WaterYield_MIDL_TYPE_FORMAT_STRING WaterYield__MIDL_TypeFormatString;
extern const WaterYield_MIDL_PROC_FORMAT_STRING WaterYield__MIDL_ProcFormatString;
extern const WaterYield_MIDL_EXPR_FORMAT_STRING WaterYield__MIDL_ExprFormatString;


extern const MIDL_STUB_DESC Object_StubDesc;


extern const MIDL_SERVER_INFO IWaterBasin_ServerInfo;
extern const MIDL_STUBLESS_PROXY_INFO IWaterBasin_ProxyInfo;


extern const USER_MARSHAL_ROUTINE_QUADRUPLE UserMarshalRoutines[ WIRE_MARSHAL_TABLE_SIZE ];

#if !defined(__RPC_WIN64__)
#error  Invalid build platform for this stub.
#endif

static const WaterYield_MIDL_PROC_FORMAT_STRING WaterYield__MIDL_ProcFormatString =
    {
        0,
        {

	/* Procedure Initialize */

			0x33,		/* FC_AUTO_HANDLE */
			0x6c,		/* Old Flags:  object, Oi2 */
/*  2 */	NdrFcLong( 0x0 ),	/* 0 */
/*  6 */	NdrFcShort( 0x3 ),	/* 3 */
/*  8 */	NdrFcShort( 0xc0 ),	/* X64 Stack size/offset = 192 */
/* 10 */	NdrFcShort( 0x66 ),	/* 102 */
/* 12 */	NdrFcShort( 0x8 ),	/* 8 */
/* 14 */	0x46,		/* Oi2 Flags:  clt must size, has return, has ext, */
			0x17,		/* 23 */
/* 16 */	0xa,		/* 10 */
			0x5,		/* Ext Flags:  new corr desc, srv corr check, */
/* 18 */	NdrFcShort( 0x0 ),	/* 0 */
/* 20 */	NdrFcShort( 0x1 ),	/* 1 */
/* 22 */	NdrFcShort( 0x0 ),	/* 0 */
/* 24 */	NdrFcShort( 0x0 ),	/* 0 */

	/* Parameter name */

/* 26 */	NdrFcShort( 0x8b ),	/* Flags:  must size, must free, in, by val, */
/* 28 */	NdrFcShort( 0x8 ),	/* X64 Stack size/offset = 8 */
/* 30 */	NdrFcShort( 0x1c ),	/* Type Offset=28 */

	/* Parameter rank */

/* 32 */	NdrFcShort( 0x48 ),	/* Flags:  in, base type, */
/* 34 */	NdrFcShort( 0x10 ),	/* X64 Stack size/offset = 16 */
/* 36 */	0x8,		/* FC_LONG */
			0x0,		/* 0 */

	/* Parameter SRM */

/* 38 */	NdrFcShort( 0x8b ),	/* Flags:  must size, must free, in, by val, */
/* 40 */	NdrFcShort( 0x18 ),	/* X64 Stack size/offset = 24 */
/* 42 */	NdrFcShort( 0x1c ),	/* Type Offset=28 */

	/* Parameter StatusTime */

/* 44 */	NdrFcShort( 0x48 ),	/* Flags:  in, base type, */
/* 46 */	NdrFcShort( 0x20 ),	/* X64 Stack size/offset = 32 */
/* 48 */	0x8,		/* FC_LONG */
			0x0,		/* 0 */

	/* Parameter user */

/* 50 */	NdrFcShort( 0x8b ),	/* Flags:  must size, must free, in, by val, */
/* 52 */	NdrFcShort( 0x28 ),	/* X64 Stack size/offset = 40 */
/* 54 */	NdrFcShort( 0x1c ),	/* Type Offset=28 */

	/* Parameter password */

/* 56 */	NdrFcShort( 0x8b ),	/* Flags:  must size, must free, in, by val, */
/* 58 */	NdrFcShort( 0x30 ),	/* X64 Stack size/offset = 48 */
/* 60 */	NdrFcShort( 0x1c ),	/* Type Offset=28 */

	/* Parameter sid */

/* 62 */	NdrFcShort( 0x8b ),	/* Flags:  must size, must free, in, by val, */
/* 64 */	NdrFcShort( 0x38 ),	/* X64 Stack size/offset = 56 */
/* 66 */	NdrFcShort( 0x1c ),	/* Type Offset=28 */

	/* Parameter steps */

/* 68 */	NdrFcShort( 0x48 ),	/* Flags:  in, base type, */
/* 70 */	NdrFcShort( 0x40 ),	/* X64 Stack size/offset = 64 */
/* 72 */	0x8,		/* FC_LONG */
			0x0,		/* 0 */

	/* Parameter HourStart */

/* 74 */	NdrFcShort( 0x48 ),	/* Flags:  in, base type, */
/* 76 */	NdrFcShort( 0x48 ),	/* X64 Stack size/offset = 72 */
/* 78 */	0x8,		/* FC_LONG */
			0x0,		/* 0 */

	/* Parameter NumofHours */

/* 80 */	NdrFcShort( 0x48 ),	/* Flags:  in, base type, */
/* 82 */	NdrFcShort( 0x50 ),	/* X64 Stack size/offset = 80 */
/* 84 */	0x8,		/* FC_LONG */
			0x0,		/* 0 */

	/* Parameter isDebug */

/* 86 */	NdrFcShort( 0x48 ),	/* Flags:  in, base type, */
/* 88 */	NdrFcShort( 0x58 ),	/* X64 Stack size/offset = 88 */
/* 90 */	0x6,		/* FC_SHORT */
			0x0,		/* 0 */

	/* Parameter sccd */

/* 92 */	NdrFcShort( 0x8b ),	/* Flags:  must size, must free, in, by val, */
/* 94 */	NdrFcShort( 0x60 ),	/* X64 Stack size/offset = 96 */
/* 96 */	NdrFcShort( 0x1c ),	/* Type Offset=28 */

	/* Parameter sRainType */

/* 98 */	NdrFcShort( 0x8b ),	/* Flags:  must size, must free, in, by val, */
/* 100 */	NdrFcShort( 0x68 ),	/* X64 Stack size/offset = 104 */
/* 102 */	NdrFcShort( 0x1c ),	/* Type Offset=28 */

	/* Parameter UpInitWaterContent */

/* 104 */	NdrFcShort( 0x48 ),	/* Flags:  in, base type, */
/* 106 */	NdrFcShort( 0x70 ),	/* X64 Stack size/offset = 112 */
/* 108 */	0xa,		/* FC_FLOAT */
			0x0,		/* 0 */

	/* Parameter MidInitWaterContent */

/* 110 */	NdrFcShort( 0x48 ),	/* Flags:  in, base type, */
/* 112 */	NdrFcShort( 0x78 ),	/* X64 Stack size/offset = 120 */
/* 114 */	0xa,		/* FC_FLOAT */
			0x0,		/* 0 */

	/* Parameter DownInitWaterContent */

/* 116 */	NdrFcShort( 0x48 ),	/* Flags:  in, base type, */
/* 118 */	NdrFcShort( 0x80 ),	/* X64 Stack size/offset = 128 */
/* 120 */	0xa,		/* FC_FLOAT */
			0x0,		/* 0 */

	/* Parameter emethod */

/* 122 */	NdrFcShort( 0x8b ),	/* Flags:  must size, must free, in, by val, */
/* 124 */	NdrFcShort( 0x88 ),	/* X64 Stack size/offset = 136 */
/* 126 */	NdrFcShort( 0x1c ),	/* Type Offset=28 */

	/* Parameter thetab */

/* 128 */	NdrFcShort( 0x48 ),	/* Flags:  in, base type, */
/* 130 */	NdrFcShort( 0x90 ),	/* X64 Stack size/offset = 144 */
/* 132 */	0xa,		/* FC_FLOAT */
			0x0,		/* 0 */

	/* Parameter thetaw */

/* 134 */	NdrFcShort( 0x48 ),	/* Flags:  in, base type, */
/* 136 */	NdrFcShort( 0x98 ),	/* X64 Stack size/offset = 152 */
/* 138 */	0xa,		/* FC_FLOAT */
			0x0,		/* 0 */

	/* Parameter N */

/* 140 */	NdrFcShort( 0x48 ),	/* Flags:  in, base type, */
/* 142 */	NdrFcShort( 0xa0 ),	/* X64 Stack size/offset = 160 */
/* 144 */	0x8,		/* FC_LONG */
			0x0,		/* 0 */

	/* Parameter E0_a */

/* 146 */	NdrFcShort( 0x48 ),	/* Flags:  in, base type, */
/* 148 */	NdrFcShort( 0xa8 ),	/* X64 Stack size/offset = 168 */
/* 150 */	0xa,		/* FC_FLOAT */
			0x0,		/* 0 */

	/* Parameter SoilErosionEquation */

/* 152 */	NdrFcShort( 0x8b ),	/* Flags:  must size, must free, in, by val, */
/* 154 */	NdrFcShort( 0xb0 ),	/* X64 Stack size/offset = 176 */
/* 156 */	NdrFcShort( 0x1c ),	/* Type Offset=28 */

	/* Return value */

/* 158 */	NdrFcShort( 0x70 ),	/* Flags:  out, return, base type, */
/* 160 */	NdrFcShort( 0xb8 ),	/* X64 Stack size/offset = 184 */
/* 162 */	0x8,		/* FC_LONG */
			0x0,		/* 0 */

	/* Procedure calc */

/* 164 */	0x33,		/* FC_AUTO_HANDLE */
			0x6c,		/* Old Flags:  object, Oi2 */
/* 166 */	NdrFcLong( 0x0 ),	/* 0 */
/* 170 */	NdrFcShort( 0x4 ),	/* 4 */
/* 172 */	NdrFcShort( 0x48 ),	/* X64 Stack size/offset = 72 */
/* 174 */	NdrFcShort( 0x190 ),	/* 400 */
/* 176 */	NdrFcShort( 0x8 ),	/* 8 */
/* 178 */	0x46,		/* Oi2 Flags:  clt must size, has return, has ext, */
			0x8,		/* 8 */
/* 180 */	0xa,		/* 10 */
			0x85,		/* Ext Flags:  new corr desc, srv corr check, has big amd64 byval param */
/* 182 */	NdrFcShort( 0x0 ),	/* 0 */
/* 184 */	NdrFcShort( 0x1 ),	/* 1 */
/* 186 */	NdrFcShort( 0x0 ),	/* 0 */
/* 188 */	NdrFcShort( 0x0 ),	/* 0 */

	/* Parameter SnowModelType */

/* 190 */	NdrFcShort( 0x8b ),	/* Flags:  must size, must free, in, by val, */
/* 192 */	NdrFcShort( 0x8 ),	/* X64 Stack size/offset = 8 */
/* 194 */	NdrFcShort( 0x1c ),	/* Type Offset=28 */

	/* Parameter mBSCode */

/* 196 */	NdrFcShort( 0x10a ),	/* Flags:  must free, in, simple ref, */
/* 198 */	NdrFcShort( 0x10 ),	/* X64 Stack size/offset = 16 */
/* 200 */	NdrFcShort( 0x2a ),	/* Type Offset=42 */

	/* Parameter mPara */

/* 202 */	NdrFcShort( 0x10a ),	/* Flags:  must free, in, simple ref, */
/* 204 */	NdrFcShort( 0x18 ),	/* X64 Stack size/offset = 24 */
/* 206 */	NdrFcShort( 0x3a ),	/* Type Offset=58 */

	/* Parameter pQin */

/* 208 */	NdrFcShort( 0x148 ),	/* Flags:  in, base type, simple ref, */
/* 210 */	NdrFcShort( 0x20 ),	/* X64 Stack size/offset = 32 */
/* 212 */	0xa,		/* FC_FLOAT */
			0x0,		/* 0 */

	/* Parameter pSin */

/* 214 */	NdrFcShort( 0x148 ),	/* Flags:  in, base type, simple ref, */
/* 216 */	NdrFcShort( 0x28 ),	/* X64 Stack size/offset = 40 */
/* 218 */	0xa,		/* FC_FLOAT */
			0x0,		/* 0 */

	/* Parameter pWLM */

/* 220 */	NdrFcShort( 0x148 ),	/* Flags:  in, base type, simple ref, */
/* 222 */	NdrFcShort( 0x30 ),	/* X64 Stack size/offset = 48 */
/* 224 */	0xa,		/* FC_FLOAT */
			0x0,		/* 0 */

	/* Parameter pWRM */

/* 226 */	NdrFcShort( 0x148 ),	/* Flags:  in, base type, simple ref, */
/* 228 */	NdrFcShort( 0x38 ),	/* X64 Stack size/offset = 56 */
/* 230 */	0xa,		/* FC_FLOAT */
			0x0,		/* 0 */

	/* Return value */

/* 232 */	NdrFcShort( 0x70 ),	/* Flags:  out, return, base type, */
/* 234 */	NdrFcShort( 0x40 ),	/* X64 Stack size/offset = 64 */
/* 236 */	0x8,		/* FC_LONG */
			0x0,		/* 0 */

	/* Procedure Finalize */

/* 238 */	0x33,		/* FC_AUTO_HANDLE */
			0x6c,		/* Old Flags:  object, Oi2 */
/* 240 */	NdrFcLong( 0x0 ),	/* 0 */
/* 244 */	NdrFcShort( 0x5 ),	/* 5 */
/* 246 */	NdrFcShort( 0x18 ),	/* X64 Stack size/offset = 24 */
/* 248 */	NdrFcShort( 0x0 ),	/* 0 */
/* 250 */	NdrFcShort( 0x8 ),	/* 8 */
/* 252 */	0x46,		/* Oi2 Flags:  clt must size, has return, has ext, */
			0x2,		/* 2 */
/* 254 */	0xa,		/* 10 */
			0x5,		/* Ext Flags:  new corr desc, srv corr check, */
/* 256 */	NdrFcShort( 0x0 ),	/* 0 */
/* 258 */	NdrFcShort( 0x1 ),	/* 1 */
/* 260 */	NdrFcShort( 0x0 ),	/* 0 */
/* 262 */	NdrFcShort( 0x0 ),	/* 0 */

	/* Parameter SRM */

/* 264 */	NdrFcShort( 0x8b ),	/* Flags:  must size, must free, in, by val, */
/* 266 */	NdrFcShort( 0x8 ),	/* X64 Stack size/offset = 8 */
/* 268 */	NdrFcShort( 0x1c ),	/* Type Offset=28 */

	/* Return value */

/* 270 */	NdrFcShort( 0x70 ),	/* Flags:  out, return, base type, */
/* 272 */	NdrFcShort( 0x10 ),	/* X64 Stack size/offset = 16 */
/* 274 */	0x8,		/* FC_LONG */
			0x0,		/* 0 */

			0x0
        }
    };

static const WaterYield_MIDL_TYPE_FORMAT_STRING WaterYield__MIDL_TypeFormatString =
    {
        0,
        {
			NdrFcShort( 0x0 ),	/* 0 */
/*  2 */	
			0x12, 0x0,	/* FC_UP */
/*  4 */	NdrFcShort( 0xe ),	/* Offset= 14 (18) */
/*  6 */	
			0x1b,		/* FC_CARRAY */
			0x1,		/* 1 */
/*  8 */	NdrFcShort( 0x2 ),	/* 2 */
/* 10 */	0x9,		/* Corr desc: FC_ULONG */
			0x0,		/*  */
/* 12 */	NdrFcShort( 0xfffc ),	/* -4 */
/* 14 */	NdrFcShort( 0x1 ),	/* Corr flags:  early, */
/* 16 */	0x6,		/* FC_SHORT */
			0x5b,		/* FC_END */
/* 18 */	
			0x17,		/* FC_CSTRUCT */
			0x3,		/* 3 */
/* 20 */	NdrFcShort( 0x8 ),	/* 8 */
/* 22 */	NdrFcShort( 0xfff0 ),	/* Offset= -16 (6) */
/* 24 */	0x8,		/* FC_LONG */
			0x8,		/* FC_LONG */
/* 26 */	0x5c,		/* FC_PAD */
			0x5b,		/* FC_END */
/* 28 */	0xb4,		/* FC_USER_MARSHAL */
			0x83,		/* 131 */
/* 30 */	NdrFcShort( 0x0 ),	/* 0 */
/* 32 */	NdrFcShort( 0x8 ),	/* 8 */
/* 34 */	NdrFcShort( 0x0 ),	/* 0 */
/* 36 */	NdrFcShort( 0xffde ),	/* Offset= -34 (2) */
/* 38 */	
			0x11, 0x0,	/* FC_RP */
/* 40 */	NdrFcShort( 0x2 ),	/* Offset= 2 (42) */
/* 42 */	
			0x15,		/* FC_STRUCT */
			0x7,		/* 7 */
/* 44 */	NdrFcShort( 0x20 ),	/* 32 */
/* 46 */	0x8,		/* FC_LONG */
			0x40,		/* FC_STRUCTPAD4 */
/* 48 */	0xb,		/* FC_HYPER */
			0x8,		/* FC_LONG */
/* 50 */	0x40,		/* FC_STRUCTPAD4 */
			0xb,		/* FC_HYPER */
/* 52 */	0x5c,		/* FC_PAD */
			0x5b,		/* FC_END */
/* 54 */	
			0x11, 0x0,	/* FC_RP */
/* 56 */	NdrFcShort( 0x2 ),	/* Offset= 2 (58) */
/* 58 */	
			0x15,		/* FC_STRUCT */
			0x3,		/* 3 */
/* 60 */	NdrFcShort( 0xb8 ),	/* 184 */
/* 62 */	0xa,		/* FC_FLOAT */
			0xa,		/* FC_FLOAT */
/* 64 */	0xa,		/* FC_FLOAT */
			0xa,		/* FC_FLOAT */
/* 66 */	0xa,		/* FC_FLOAT */
			0xa,		/* FC_FLOAT */
/* 68 */	0xa,		/* FC_FLOAT */
			0xa,		/* FC_FLOAT */
/* 70 */	0xa,		/* FC_FLOAT */
			0xa,		/* FC_FLOAT */
/* 72 */	0xa,		/* FC_FLOAT */
			0xa,		/* FC_FLOAT */
/* 74 */	0xa,		/* FC_FLOAT */
			0x8,		/* FC_LONG */
/* 76 */	0x8,		/* FC_LONG */
			0xa,		/* FC_FLOAT */
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
			0xa,		/* FC_FLOAT */
/* 92 */	0xa,		/* FC_FLOAT */
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
/* 108 */	0x5c,		/* FC_PAD */
			0x5b,		/* FC_END */
/* 110 */	
			0x11, 0x8,	/* FC_RP [simple_pointer] */
/* 112 */	0xa,		/* FC_FLOAT */
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



/* Standard interface: __MIDL_itf_WaterYield_0000_0000, ver. 0.0,
   GUID={0x00000000,0x0000,0x0000,{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00}} */


/* Object interface: IUnknown, ver. 0.0,
   GUID={0x00000000,0x0000,0x0000,{0xC0,0x00,0x00,0x00,0x00,0x00,0x00,0x46}} */


/* Object interface: IWaterBasin, ver. 0.0,
   GUID={0xF7013BFF,0x2387,0x43C7,{0x85,0xEB,0xA6,0xF2,0xDB,0x04,0xB3,0x09}} */

#pragma code_seg(".orpc")
static const unsigned short IWaterBasin_FormatStringOffsetTable[] =
    {
    0,
    164,
    238
    };

static const MIDL_STUBLESS_PROXY_INFO IWaterBasin_ProxyInfo =
    {
    &Object_StubDesc,
    WaterYield__MIDL_ProcFormatString.Format,
    &IWaterBasin_FormatStringOffsetTable[-3],
    0,
    0,
    0
    };


static const MIDL_SERVER_INFO IWaterBasin_ServerInfo = 
    {
    &Object_StubDesc,
    0,
    WaterYield__MIDL_ProcFormatString.Format,
    &IWaterBasin_FormatStringOffsetTable[-3],
    0,
    0,
    0,
    0};
CINTERFACE_PROXY_VTABLE(6) _IWaterBasinProxyVtbl = 
{
    &IWaterBasin_ProxyInfo,
    &IID_IWaterBasin,
    IUnknown_QueryInterface_Proxy,
    IUnknown_AddRef_Proxy,
    IUnknown_Release_Proxy ,
    (void *) (INT_PTR) -1 /* IWaterBasin::Initialize */ ,
    (void *) (INT_PTR) -1 /* IWaterBasin::calc */ ,
    (void *) (INT_PTR) -1 /* IWaterBasin::Finalize */
};

const CInterfaceStubVtbl _IWaterBasinStubVtbl =
{
    &IID_IWaterBasin,
    &IWaterBasin_ServerInfo,
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
    WaterYield__MIDL_TypeFormatString.Format,
    1, /* -error bounds_check flag */
    0x50002, /* Ndr library version */
    0,
    0x8000253, /* MIDL Version 8.0.595 */
    0,
    UserMarshalRoutines,
    0,  /* notify & notify_flag routine table */
    0x1, /* MIDL flag */
    0, /* cs routines */
    0,   /* proxy/server info */
    0
    };

const CInterfaceProxyVtbl * const _WaterYield_ProxyVtblList[] = 
{
    ( CInterfaceProxyVtbl *) &_IWaterBasinProxyVtbl,
    0
};

const CInterfaceStubVtbl * const _WaterYield_StubVtblList[] = 
{
    ( CInterfaceStubVtbl *) &_IWaterBasinStubVtbl,
    0
};

PCInterfaceName const _WaterYield_InterfaceNamesList[] = 
{
    "IWaterBasin",
    0
};


#define _WaterYield_CHECK_IID(n)	IID_GENERIC_CHECK_IID( _WaterYield, pIID, n)

int __stdcall _WaterYield_IID_Lookup( const IID * pIID, int * pIndex )
{
    
    if(!_WaterYield_CHECK_IID(0))
        {
        *pIndex = 0;
        return 1;
        }

    return 0;
}

const ExtendedProxyFileInfo WaterYield_ProxyFileInfo = 
{
    (PCInterfaceProxyVtblList *) & _WaterYield_ProxyVtblList,
    (PCInterfaceStubVtblList *) & _WaterYield_StubVtblList,
    (const PCInterfaceName * ) & _WaterYield_InterfaceNamesList,
    0, /* no delegation */
    & _WaterYield_IID_Lookup, 
    1,
    2,
    0, /* table of [async_uuid] interfaces */
    0, /* Filler1 */
    0, /* Filler2 */
    0  /* Filler3 */
};
#if _MSC_VER >= 1200
#pragma warning(pop)
#endif


#endif /* defined(_M_AMD64)*/

