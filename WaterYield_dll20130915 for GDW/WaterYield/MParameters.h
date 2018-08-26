#ifndef PARAMETERS_H
#define PARAMETERS_H

#pragma once
#include "stdafx.h"
#include "datastruct.h"

//2008,自从在idl中该用自动化指针的传递，类的名字就不能使用Parameters，否则编译不通过，根据提示，应该是跟ADO中的xxx重名了，所以都改成了MParameters。
class MParameters
{
public:

	ADODB::_ConnectionPtr pTabFileCnn;//连接对象

	//20070122,xiaofc,因为要多次对连向tvarparameter的记录集操作，因此将其改为公用变量
	ADODB::_RecordsetPtr pStatRst;

	//MParameters 不随时间改变的参数
	float RouteLength;
	float Roughnessn;
	float SlopeJ;
	float Area;
	//泥沙粒径
	float SediD;
	
	float PKV0;//每小时可下渗的水量，空气进入表层土
	float PKV1;//表层土和中层土之间的垂直渗透系数
	float PKV2;//中层土和深层土之间的垂直渗透系数addbyiwish20060220

	float PKH1; //潜水层土壤水平渗透系数
	float PKH2;

	float USita1; //潜水层孔隙率的张力水部分
	float USita2; //潜水层孔隙率的自由水部分
	float MSita1;
	float MSita2;
	float DSita1;	//深层土孔隙率的张力水部分
	float DSita2;  //深层土孔隙率的自由水部分
	
	float UDepth;
	float DSDepth;  //深层土层厚度
	float ErosionK;//20051219 薛海 坡面产沙公式中的k
	float ErosionBeta;

	//David 坡面产沙模型参数
	float ErosionK1;
	float ErosionK2;
	float ErosionBeta1;
	float ErosionBeta2;

	//addbyiwish 20060222
	float au,am,ad;//基质势公式的系数项
	float bu,bm,bd;//基质势公式的指数项

	//status 随时间变化的状态
	float SoilW;//潜水层初始水量
	float MidSoilW;
	float DSW; //深层土的初始水量
	float LeafW; //叶面积水深，m
	float LAI,WaterEVA,EPI;

	//土层含水量
	//一天一个数儿，放到status表里，将来植被状况也这样写
	//L，R，S：左，右，源；U,M,D:表，中，下
	float WLU,WLM,WLD,WRU,WRM,WRD,WSU,WSM,WSD;
	float WLL,WRL,WSL;
	long lHourStart;//方案开始的时间

	//20070122,xiaofc,为保证读LAI和E0的正确性，增加日历年月变量
	short YearStart;
	short MonthStart;
	short YearEnd;
	short MonthEnd;

	//altered by weihong
	float UpInitWaterContent;
	float MidInitWaterContent;
	float DownInitWaterContent;
	CString sccd;//wh,2008.3.24

	_variant_t vtmp;//wh

	//altered by weihong
	MParameters(float upw, float midw, float downw,CString Sccd){
		UpInitWaterContent = upw;
		MidInitWaterContent = midw;
		DownInitWaterContent = downw;
		sccd = Sccd;
	}

public:
	MParameters(void);
	~MParameters(void);
	bool GetParameters(struct Para *pPara,char Position,BSCode mBSCode);
	// 把状态存入文件
	bool SaveStatusFile(void);
	//读土层含水量
	bool GetWaterInSoil(BSCode mBSCode,Para myPara);
	//读LAI、EPI
	//20070122,xiaofc，读第一个LAI,EPI记录
	void ReadLE(BSCode mBSCode);
	//20070122,xiaofc,读后续变化的LAI,EPI记录
	void ReadLE(BSCode mBSCode,short Year,short Month);

};
#endif
