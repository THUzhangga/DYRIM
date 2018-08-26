#pragma once
#include ".\MParameters.h"
#include ".\Leaf.h"
#include ".\DeepSoil.h"
#include ".\MidSoil.h"
#include "datastruct.h"


class SoilWater //表层土
{
private:
	float lambda; //20070301,xiaofc,土壤导水系数与含水量关系中的指数

public:
	float k0;
	float RouteLength;
	float n1;  //孔隙率的张力水部分
	float n2;  //孔隙率的自由水部分
	float SlopeJ; //流域平均坡度
	float Wmax2; //最大潜水量
	float Depth;//表层土厚度，认为是耕作层，20cm=0.2m
	float kv;//垂向下渗能力，m/h，表层土入中层土的，正反向的值相同
	//20060222byiwish
	float Sita;//本层土含水率
	//float bu;//基质势公式的指数项
	//float au;//基质势公式的系数项
	float ium;//表层土和中层土的平均梯度byiwish20060222

	bool isDebug;

public:
	float Area;
	float W;  //蓄水量
	float Wout; //时段出流,m^3
	float Wmax1; //最大张力水量

	int MSTEP;//wh

	//wh
	SoilWater(int timestep){
		MSTEP = timestep;
	}

public:
	SoilWater(void);
	~SoilWater(void);
	//float NextHour(float WaterAdded,float SRate);//给定MidSoil层的SRate用于给出向下渗透的系数
	float NextHour(float WaterAdded,MParameters myPara,float MSita,float MidW,float MidWmax2);//用中层土含水率及基质势两个系数计算平均梯度，20060222byiwish
	// 是否需要张力水补给
	bool NeedWater1(void);
	// 蓄满产流的比率
	float GetSRate(void);
	int initialize(MParameters myPara);

};
