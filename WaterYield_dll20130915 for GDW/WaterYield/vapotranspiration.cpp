
#include "stdafx.h"
#include ".\vapotranspiration.h"

Evapotranspiration::Evapotranspiration(void)
{
}

Evapotranspiration::~Evapotranspiration(void)
{
}

//20070301,xiaofc,将蒸发计算改为仅日间蒸发，且小时间的分配按正弦曲线
bool Evapotranspiration::initialize(MParameters* myPara/*, float HourOffset, float P*/)
{

	AvgEPI=myPara->EPI;//是每小时的蒸发能力，由tvarparameter中的watereva换算得到

	//用于傅抱璞公式
	thetaf=myPara->USita1;//田间持水量
	thetas=thetaf+myPara->USita2;//饱和含水量//wh解读：这里的各种特征含水量都是百分比，无量纲的（饱和含水量=田间持水量+自由水含水量，这里的量其实都是率）

	//altered by wh

	if(thetab<1e-5) thetab=1e-5;
	if(thetab>thetaf) thetab=thetaf-1e-5;//毛管断裂

	if(thetaw<1e-5) thetaw=1e-5;
	if(thetaw>thetab) thetaw=thetab-1e-5;//凋萎

	if(N<1) N=1;
	if(E0_a<1e-5) E0_a=1e-5;

	return 1;
}

//wh解读：蒸发是处理包括叶面以及表层土壤的蒸发，并对各个对象的土壤含水量进行实时调整。只是一层蒸发模型
//这里的nexthour其实是步长
bool Evapotranspiration::NextHour(Leaf * cLeaf,SoilWater * cSoil,MidSoil * cMid, DeepSoil * cDeep,float HourOffset, float P)
{
	//1.将平均潜在蒸发转化为相应时间步长上的正弦分布后的值
	float HourInDay;
	float EPI;//小时蒸发能力（按照正弦曲线分配后）
	HourInDay=long(floor(HourOffset))%24+HourOffset-floor(HourOffset);
	if(HourInDay<6 || HourInDay>=18 || P>0)
		EPI=0.0f;
	else
		EPI=AvgEPI*sin((HourInDay-6)*PI/12)*PI;//wh： 正弦曲线平均分配的原则，积分后在12个小时内的蒸发总量为24*AvgEPI，平均每小时为2*AvgEPI，维持蒸发总量不能变化
	
	float temp;
	temp=EPI/60*MSTEP;//addbyiwish20060304

	//2.以下开始计算
	//20070116,xiaofc,
	float e;//每一层产生的蒸发量
	E=0.0f;

	if(cLeaf->W>temp)//if(cLeaf->W>EPI)//叶面存水大于蒸发能力 iwish修改为叶面存水大于每个步长的蒸发能力20060304
	{
		//20070116,xiaofc
		e=EPI/60*MSTEP;//m/时间步长
		E+=e;

		cLeaf->W-=e;
	}
	else//叶面存水不足以蒸发，向Soil考虑
	{
		//20070116,xiaofc
		E+=cLeaf->W;

		ELeft=EPI/60*MSTEP-cLeaf->W;//剩余蒸发能力
		cLeaf->W=0.0;
		

		if(ELeft>0)
		{
			//20070702,xiaofc,增加蒸发计算方法的选择
			//CString EMethod;
			if(EMethod=="Exponential")
			{
				//20070204,xiaofc,增加风干含水率的概念，并令风干条件下的饱和程度为田间持水量下的5%
				e=ELeft*pow((cSoil->W/cSoil->Wmax2-0.05f)/0.95f,2.3f);
			}
			else if(EMethod=="FuBP")//20070702,xiaofc,傅抱璞公式
			{
				//20060704,xiaofc:
				//Soil.Sita是指当前含水量占饱和含水量的比例，要乘以总孔隙率才是m3/m3的含水量
				theta=cSoil->Sita*thetas;

				e=ELeft*theta/(thetaf-thetaw)*(thetaf/pow(pow(thetaf,N)+pow(theta,N)+pow(E0_a,N),1.0f/N)-thetaw/pow(pow(thetab,N)+pow(theta,N)+pow(E0_a,N),1.0f/N) );
				//cout<<"ELeft="<<ELeft<<"\ttheta="<<theta<<"\thetaf="<<thetaf<<"\tthetaw="<<thetaw<<"\tthetab="<<thetab<<"\t";
				//cout<<"e="<<e<<endl;
			}
			else
			{
				cout<<"Unknown type of Evaporation caculation!"<<endl;
				exit(0);
			}

			if(e<0.0f) e=0;
			
			temp=cSoil->W-e*cSoil->Area;
			
			//temp=cSoil->W-ELeft*cSoil->Area*cSoil->W/cSoil->Wmax1;
			if(temp>0)
			{
				cSoil->W=temp;

				//20070116,xiaofc
				E+=e;
			}
			else
			{
				cSoil->W=0.0;

				//20070116,xiaofc
				E+=cSoil->W/cSoil->Area;//wh解读：有多少蒸多少了，不再往中层土壤走了，相当于一层蒸发模型。
			}
		}
	
	
	}
	
	//考虑植被腾发
	//..........

	//调整各层蓄水量
	//此功能已由邓利改为根据土水势理论计算，代码在各层土类里

	//20070116,xiaofc,计算完成后给出新的含水率
	cSoil->Sita=cSoil->W/cSoil->Wmax2;//随时调整含水率20060304iwish
	//把算出来的E换为mm
	E*=1000.0f;

	return 1;
}