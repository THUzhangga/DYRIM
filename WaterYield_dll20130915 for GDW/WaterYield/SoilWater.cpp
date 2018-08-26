
#include ".\soilwater.h"
using namespace std;

SoilWater::SoilWater(void)
{	
}

SoilWater::~SoilWater(void)
{
}

float SoilWater::NextHour(float WaterAdded,MParameters myPara,float MSita,float MidW,float MidWmax2)//float SoilWater::NextHour(float WaterAdded,float SRate)//水量，m^3,SRate是下一层土的饱和率
{
	float toReturn;

	Wout=0;

	//接受下渗byiwish20060220
	W=W+WaterAdded;
	
	//产流，应该在含水量调整之前计算20060301
	float tempf;
	tempf=3600.0f/60*MSTEP;
	//cout<<"======================k0="<<k0<<"============================"<<endl;
	//水平出流处理
	if(W>Wmax1)
	{	
		Wout=SlopeJ*k0*(W-Wmax1)*tempf/2.0f/(tempf/4.0f*SlopeJ*k0+n2*RouteLength);//K0即pkh1

		W-=Wout;

		if(W<Wmax1)//wh解读：出流后，剩余的最小要为Wmax1，因为认为只有超过田间持水量的才出流，该if控制下限
		{
			Wout=Wout-(Wmax1-W);
			W=Wmax1;
		}
		if(W>Wmax2)
		{
			Wout=Wout+(W-Wmax2);
			W=Wmax2;
		}
	}
	else
	{
		Wout=0.0f;
	}

	//本层下渗20060301
	float B;//一个系数，避免公式冗长
	Sita=W/Wmax2;
	B=2.0f/(2.0f*SlopeJ*RouteLength)*log(1.0f+(2.0f*SlopeJ*RouteLength)/Depth);//(2*SlopeJ*RouteLength)是中层土的厚度hm，SlopeJ和RouteLength都是中层土的

	//控制土壤饱和时基质势为0 20060419byiwish
	//wh：ium为表到中的平均土水势梯度
	if(Sita<1&&MSita<1)
		ium=1.0f-B*(myPara.au*pow(Sita,(-myPara.bu))-myPara.am*pow(MSita,(-myPara.bm)));//(2*SlopeJ*RouteLength)是中层土的厚度，SlopeJ和RouteLength都是中层土的
	else if(Sita>=1&&MSita<1)
		ium=1.0f-B*(0-myPara.am*pow(MSita,(-myPara.bm)));
	else if(Sita<1&&MSita>=1)
		ium=1.0f-B*(myPara.au*pow(Sita,(-myPara.bu))-0);
	else
		ium=1;


	toReturn=kv*pow(0.5f*(Sita+MSita),lambda)*ium*Area/60*MSTEP;//m3//20070228,xiaofc,kv修改为随含水量变化=kv*(sita/sitas)^b

	float wtemp;
	if(toReturn>=0)
	{
		if(toReturn<=W)//如果下渗的水量小于本层土水量，则按计算量下渗；否则令本层土水量为1，其余全部下渗
		{
			if((MidW+toReturn)<MidWmax2)//下渗水量加上原来水量不超过中层土的最大蓄水量，按计算量下渗
			{
				W=W-toReturn;
			}
			else//超过中层土最大蓄量，下渗补到最大蓄水量为止
			{
				toReturn=MidWmax2-MidW;
				W-=toReturn;
			}
		}
		else
		{
			wtemp=W-1.0f;
			if((MidW+wtemp)<MidWmax2)
			{
				toReturn=wtemp;
				W=1.0f;
			}
			else
			{
				toReturn=MidWmax2-MidW;
				W-=toReturn;
			}
		}
	}
	else
	{
		wtemp=-toReturn;
		if(wtemp<MidW)//中层土水分足够补充
		{
			wtemp=W+wtemp;
			if(wtemp<Wmax2)//补充水分后不超过表层土的最大蓄水量
			{
				W=wtemp;
			}
			else//补充后超过最大蓄水量，令含水量等于最大蓄水量
			{
				toReturn=-(Wmax2-W);
				W=Wmax2;
			}
		}
		else//中层土水分不够补充表层土，令中层土水量为1，其余全部补充表层土
		{
			wtemp=MidW-1.0f;
			if((W+wtemp)<Wmax2)
			{
				toReturn=-wtemp;
				W+=wtemp;
			}
			else
			{
				toReturn=-(Wmax2-W);
				W=Wmax2;
			}
		}
	}

	//20060313,与中层土的交换迭代计算
	float LoopToReturn=0;
	int LoopCount=0;
	float DeltaToReturn;//20080304,xiaofc,迭代过程中下渗水量的变化量
	while ( LoopCount<20) 
	{
		LoopCount++;
		LoopToReturn=toReturn;
		Sita=W/Wmax2;//调整含水率
		MidW+=LoopToReturn;
		MSita=MidW/MidWmax2;
	
		//牛顿迭代法
		toReturn=LoopToReturn-(toReturn-LoopToReturn)/(-kv*pow(0.5f*(Sita+MSita),lambda)*Area*B*(myPara.au*myPara.bu*pow((W-LoopToReturn),(-myPara.bu-1))/pow(Wmax2,(-myPara.bu))+myPara.am*myPara.bm*pow((MidW+LoopToReturn),(-myPara.bm-1))/pow(MidWmax2,(-myPara.bm)))-1);//20070228,xiaofc,kv修改为随含水量变化=kv*(sita/sitas)^b
		DeltaToReturn=toReturn-LoopToReturn;;//20080304,xiaofc,迭代过程中下渗水量的变化量
		
		if(DeltaToReturn/*toReturn*/>=0)
		{
			if(DeltaToReturn<=W)//如果下渗的水量小于本层土水量，则按计算量下渗；否则令本层土水量为1，其余全部下渗
			{
				if((MidW+DeltaToReturn)<MidWmax2)//下渗水量加上原来水量不超过中层土的最大蓄水量，按计算量下渗
				{
					W-=DeltaToReturn;
				}
				else//超过中层土最大蓄量，下渗补到最大蓄水量为止
				{
					DeltaToReturn=MidWmax2-MidW;
					toReturn=DeltaToReturn+LoopToReturn;//20080304,xiaofc,迭代过程中下渗水量的变化量
					W-=DeltaToReturn;
				}
			}
			else
			{
				wtemp=W-1.0f;
				if((MidW+wtemp)<MidWmax2)
				{
					DeltaToReturn=wtemp;
					toReturn=DeltaToReturn+LoopToReturn;//20080304,xiaofc,迭代过程中下渗水量的变化量
					W=1.0f;
				}
				else
				{
					DeltaToReturn=MidWmax2-MidW;
					toReturn=DeltaToReturn+LoopToReturn;//20080304,xiaofc,迭代过程中下渗水量的变化量
					W-=DeltaToReturn;
				}
			}
		}
		else
		{
			wtemp=-DeltaToReturn;
			if(wtemp<MidW)//中层土水分足够补充
			{
				wtemp=W+wtemp;//假设中层土水分调上来之后的结果
				if(wtemp<Wmax2)//补充水分后不超过表层土的最大蓄水量
				{
					W=wtemp;
				}
				else//补充后超过最大蓄水量，令含水量等于最大蓄水量
				{
					DeltaToReturn=-(Wmax2-W);
					toReturn=DeltaToReturn+LoopToReturn;
					W=Wmax2;
				}
			}
			else//中层土水分不够补充表层土，令中层土水量为1，其余全部补充表层土
			{
				wtemp=MidW-1.0f;//可能成为-deltatoreturn
				if((W+wtemp)<Wmax2)
				{
					DeltaToReturn=-wtemp;
					toReturn=DeltaToReturn+LoopToReturn;
					W+=wtemp;
				}
				else
				{
					DeltaToReturn=-(Wmax2-W);
					toReturn=DeltaToReturn+LoopToReturn;
					W=Wmax2;
				}
			}
		}
		
		//20080304,xiaofc,新的跳出条件
		if(abs(DeltaToReturn)<1e-3) break; 
	}//end of while
	
	return toReturn;
}
bool SoilWater::NeedWater1(void)
{	

	if(W<Wmax1)
	{
		return true;
	}
	else
	{
		return false;
	}
}


// 蓄满产流的比率
float SoilWater::GetSRate(void)
{
	//20051212 李铁键
	//如果当前含水量不足田间持水量，正常返回负值
	//以此使含水量不足田间持水量的情况下，地表下渗系数大于田间持水量下的参考下渗能力

	//if( NeedWater1() )
	//{
	//	return 0.0;
	//}
	//else
	//{
		if(abs(Wmax2-Wmax1)<1e-5)
			return 0.0;

		return (W-Wmax1)/(Wmax2-Wmax1);
	//}
}

int SoilWater::initialize(MParameters myPara)
{
	Area=myPara.Area;
	k0=myPara.PKH1;
	RouteLength=myPara.RouteLength;
	n1=myPara.USita1;//这两个含水率是参数不是适时的20060220
	n2=myPara.USita2;
	SlopeJ=myPara.SlopeJ;
	kv=myPara.PKV1;//表层土入深层土的下渗力要小于入表层土的下渗率
	Depth=myPara.UDepth;//耕作深度

	Wmax1=Area*Depth*n1;   //楔形，没有RouteLength*2
	Wmax2=Area*Depth*(n1+n2); //20041016修改
	
	W=myPara.SoilW;//就是由表层初始土壤体积含水量转换的

	isDebug=false;
	
	//byiwish20060222
	Sita=W/Wmax2;//wh：（实际含水体积/总土壤空隙体积）=实际含水量/饱和含水量

	lambda=6.0f;

	//david,初始化写到这里
    Wout=0.0f;

	return 1;
}
