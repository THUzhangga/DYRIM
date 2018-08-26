#pragma once
#include"MParameters.h"
#include ".\Leaf.h"
#include ".\SoilWater.h"
#include ".\DeepSoil.h"
#include "datastruct.h"
class MidSoil
{
public:
	float kh;//水平向渗透系数
	float kv;//中层土入深层土的垂直渗透系数
	float RouteLength;
	float n1;  //孔隙率的张力水部分
	float n2;  //孔隙率的自由水部分
	float SlopeJ; //流域平均坡度
	float Wmax2; //最大潜水量
	//20060222byiwish
	float Sita;//本层土含水率
	//float bm;//基质势公式的指数项
	//float am;//基质势公式的系数项
	float imd;//中层土和深层土的平均梯度byiwish20060222

	bool isDebug;

public:
	float Area;
	float W;  //蓄水量
	float Wout; //时段出流,m^3
	float Wmax1; //最大张力水量

	int MSTEP;//wh

	//wh
	MidSoil(int timestep){
		MSTEP = timestep;
	}


public:
	MidSoil(void);
	~MidSoil(void);
	//float NextHour(float WaterAdded);
	float NextHour(float WaterAdded,MParameters myPara,float DSita,float DeepW,float DeepWmax2);//byiwish20060222 SoilW是表层土水量，随时间变化的
	//// 是否需要张力水补给
	//bool NeedWater1(void);
	//// 是否需要自由水补给
	//bool NeedWater2(void);
	// 蓄满产流的比率
	float GetSRate(void);
	int initialize(MParameters);

};
