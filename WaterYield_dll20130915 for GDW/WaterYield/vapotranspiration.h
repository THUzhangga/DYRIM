#pragma once
#include ".\Leaf.h"
#include ".\SoilWater.h"
#include ".\DeepSoil.h"
#include ".\MidSoil.h"
#include "datastruct.h"
using namespace std;
class Evapotranspiration
{
public:
	float AvgEPI; //最大蒸发能力
	float ELeft; //蒸发能力的剩余值
	float E;//20070117,李铁键,实际的蒸发量（包括叶面、土壤等），单位mm
	float thetaf;//田间持水量
	float thetas;//饱和含水量

	//addbyiwish20060220
	//float ium,imd;//平均梯度
	//float SitaU,SitaM,SitaD;//三层土的含水率
	//float bu,bm,bd;//基质势公式的指数项
	//float au,am,ad;//基质势公式的系数项

private:
	CString EMethod;

	//用于傅抱璞公式（用于土壤蒸发）
	float theta;//当前含水率
	float thetab;//毛管断裂含水率
	float thetaw;//调萎含水率
	int N;//指数--决定傅抱璞公式曲线光滑的程度
	float E0_a; //E0/a

	int MSTEP;//wh

	//altered by weihong
public:
	Evapotranspiration(CString em, float theta_b, float theta_w, int power, float e0a,int mstep){
		EMethod = em;
		thetab = theta_b;
		thetaw = theta_w;
		N = power;
		E0_a = e0a;
		MSTEP = mstep;
	}


public:
	Evapotranspiration(void);
	~Evapotranspiration(void);
	bool initialize(MParameters* myPara); //每个时段都需要执行
	bool NextHour(Leaf*,SoilWater*,MidSoil*,DeepSoil*,float HourOffset, float P);
};
