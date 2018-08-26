#pragma once
#include "MParameters.h"

class Leaf
{
private:
	float I0; //年内最大截流能力，m^3
	float Area;//子流域面积
	float LAI; //Leaf Area Index
public:
	float W; //存水量,m
	bool isDebug;

public:
	Leaf(void);
	~Leaf(void);
	float NextHour(float WaterAdded); //返回落地雨，水深，m，供给SurfaceWater
	bool initialize(Para *,MParameters *,char); //不再像以前几个类那样通过MParameters类得到参数，而直接从MetaBasins类里去取了
};
