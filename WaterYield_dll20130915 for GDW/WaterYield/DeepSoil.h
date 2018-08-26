#pragma once

#include".\MParameters.h"
#include ".\Leaf.h"
#include ".\SoilWater.h"
#include ".\MidSoil.h"
#include "datastruct.h"

class DeepSoil
{

public:
	float Area;
	float DSDepth;  //土层厚度
	float DSn1;	//孔隙率的张力水部分
	float DSn2;  //孔隙率的自由水部分
	//20060222byiwish
	float Sita;//本层土含水率
	//float bd;//基质势公式的指数项
	//float ad;//基质势公式的系数项

public:
	float DSW;  //当前蓄水量
	float Wmax1;
	float Wmax2;//addbyiwish20060225
	//20060223注释掉float Wout; //时段出流,m^3 addbyiwish20060222

public:
	DeepSoil(void);
	~DeepSoil(void);
	//// 是否需要自由水补给
	//bool NeedWater2(void);
	//// 是否需要张力水补给
	//bool NeedWater1(void);
	// 根据给水量计算，输出没用完的水量
	void NextHour(float WaterSupply,float imd);
	int initialize(MParameters DSPara);
};
