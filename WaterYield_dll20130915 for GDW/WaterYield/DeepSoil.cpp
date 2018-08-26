
#include "stdafx.h"
#include ".\deepsoil.h"

DeepSoil::DeepSoil(void)
{

}

DeepSoil::~DeepSoil(void)
{
}

// 是否需要自由水补给
//bool DeepSoil::NeedWater2(void)
//{
//	if( NeedWater1() ) return false; //温饱没解决呢，就不考虑小资了
//	
//	if(DSW<Area*DSDepth*(DSn1+DSn2))
//	{
//		return true;
//	}
//	else
//	{
//		return false;
//	}
//}
//
//// 是否需要张力水补给
//bool DeepSoil::NeedWater1(void)
//{
//	if(DSW<Wmax1)
//	{
//		return true;
//	}
//	else
//	{
//		return false;
//	}
//}
void DeepSoil::NextHour(float WaterSupply,float imd)
{

	//20060227重写byiwish
	DSW+=WaterSupply;
	
	//20070117,xiaofc,邓利开始设定深层土含水量的下限是田间持水量的0.1倍
	//今改为上限是饱和，下限是含水量为0，且不必区分水分的运动方向
	//if(DSW<0.1f*Wmax1&&imd<0)
	//	DSW=Wmax1*0.1f;
	//else if(DSW>Wmax2&&imd>0)
	//	DSW=Wmax2;
	if(DSW<0)
		DSW=0;
	else if(DSW>Wmax2)
		DSW=Wmax2;

	Sita=DSW/Wmax2;//调整含水率
	//return WaterSupply;   //若不需要补水，如数奉还

}

int DeepSoil::initialize(MParameters DSPara)
{
	Area=DSPara.Area;
	DSDepth=DSPara.DSDepth;
	DSn1=DSPara.DSita1;
	DSn2=DSPara.DSita2;

	DSW=DSPara.DSW;

	Wmax1=Area*DSDepth*DSn1;
	Wmax2=Area*DSDepth*(DSn1+DSn2);//addbyiwish20060225

	//byiwish20060222
	Sita=DSW/(Area*DSDepth*(DSn1+DSn2));

	//ad=3520;
	//bd=4.21;

	return 1;
}
