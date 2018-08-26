
#include "stdafx.h"
#include ".\leaf.h"
#include "datastruct.h"
using namespace std;
Leaf::Leaf(void)
{
}

Leaf::~Leaf(void)
{
}

bool Leaf::initialize(Para* myPara,MParameters* myParameter, char Type)
{
	//暂认为初始情况为叶面无存水
	//W=0.0;
	//叶面存水的初始值写入下面

	//参数
	isDebug=false;
	LAI=myParameter->LAI;
	I0=myPara->I0;//叶面完全覆盖时的持水量
	if(Type=='S')
	{
		Area=myPara->AreaS;
		W=myParameter->WSL;//叶面积水深//初始值在parameter中被设为0
	}
	else if(Type=='L')
	{
		Area=myPara->AreaL;
		W=myParameter->WLL;
	}
	else if(Type=='R')
	{
		Area=myPara->AreaR;
		W=myParameter->WRL;
	}
	else
	{
		Area=-1;
		return 0;
	}

	//面积为负，即不存在，把W也标为负
	if(Area<-0.1)
	{
		W=-1;
	}

	return 1;
}


//wh解读：返回透过叶面的净雨量
float Leaf::NextHour(float WaterAdded)//水深，以m计
{
	float toReturn;
	toReturn=WaterAdded+W-I0*LAI;
	if(isDebug)
		cout<<"WaterAdded="<<WaterAdded<<"\tW="<<W<<"\tI0="<<I0<<"\tLAI="<<LAI<<"\ttoReturn="<<toReturn<<endl;
	if(toReturn>0)//水多
	{
		W=I0*LAI;//I0*LAI为灌层蓄水能力
		return toReturn;
	}
	else//水不足
	{
		W+=WaterAdded;
		return 0;
	}
}