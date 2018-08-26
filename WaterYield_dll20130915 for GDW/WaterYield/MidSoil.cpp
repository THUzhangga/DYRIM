
#include "stdafx.h"
#include ".\midsoil.h"
using namespace std;

MidSoil::MidSoil(void)
{
}

MidSoil::~MidSoil(void)
{
}

float MidSoil::NextHour(float WaterAdded,MParameters myPara,float DSita,float DeepW,float DeepWmax2)//水量，m^3
{
	
	float wslope;
	float toReturn;

	Wout=0;

	//byiwish20060222

	W=W+WaterAdded;
	
	//产流，应该在含水量调整之前计算20060301
	if(W>Wmax1)//byiwish20060222
	{
		//20050408,增加分钟步长，原来的3600常数成为可变数
		//20060313,因为W前面已经加上了WaterAdded,所以下面的公式最后用-WaterAdded/2.0而不用+
		wslope=W-Wmax1-WaterAdded/2.0f;
		float tempf;
		tempf=3600.0f/60*MSTEP;
		Wout=tempf/2.0f*kh*wslope*wslope/(Area*RouteLength*RouteLength*n2*n2+tempf/2.0f*kh*wslope);

		W-=Wout;

		if(W<=Wmax1)
		{
			//cout<<"W<=Wmax1"<<endl;
			Wout-=Wmax1-W;
			W=Wmax1;
		}

		if(W>Wmax2)
		{
			//cout<<"W>Wmax2"<<"\t";
			Wout+=W-Wmax2;
			//cout<<"midW=\t"<<W<<"\t"<<"midWmax2=\t"<<Wmax2<<"\t";
			//cout<<"midout=\t"<<Wout<<endl;
			W=Wmax2;
		}	
	}
	else
	{
		Wout=0.0f;
	}

	
	
	float wtemp;
	float B;//一个系数，避免公式冗长
	Sita=W/Wmax2;
	B=2/(2*SlopeJ*RouteLength)*log(1+(2*SlopeJ*RouteLength)/myPara.DSDepth);//(2*SlopeJ*RouteLength)是中层土的厚度
	//控制土壤饱和时基质势为0 20060419byiwish	
	if(Sita<1&&DSita<1)
		imd=1-B*(myPara.am*pow(Sita,(-myPara.bm))-myPara.ad*pow(DSita,(-myPara.bd)));
	else if(Sita>=1&&DSita<1)
		imd=1-B*(0-myPara.ad*pow(DSita,(-myPara.bd)));
	else if(Sita<1&&DSita>=1)
		imd=1-B*(myPara.am*pow(Sita,(-myPara.bm))-0);
	else
		imd=1;
	toReturn=kv*imd*Area/60*MSTEP;
	
	//牛顿迭代法第一步，令R=0
	toReturn=0.0f-(toReturn-0.0f)/(-kv*Area*B*(myPara.am*myPara.bm*pow((W-0.0f),(-myPara.bm-1))/pow(Wmax2,(-myPara.bm))+myPara.ad*myPara.bd*pow((DeepW+0.0f),(-myPara.bd-1))/pow(DeepWmax2,(-myPara.bd)))-1);

	if(toReturn>=0)
	{
		if(toReturn<W)//如果下渗的水量小于本层土水量，则按计算量下渗；否则令本层土水量为1，其余全部下渗
		{

			W-=toReturn;
		}
		else
		{
			wtemp=W-1.0f;
			toReturn=wtemp;
			W=1.0f;
		}
	}
	else
	{
		wtemp=-toReturn;
		if(wtemp<DeepW)
		{
			wtemp=W+wtemp;
			if(wtemp<Wmax2)
			{
				W=wtemp;
			}
			else
			{
				toReturn=-(Wmax2-W);
				W=Wmax2;
			}
		}
		else
		{
			wtemp=DeepW-1.0f;
			if((wtemp+W)<Wmax2)
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

	//cout<<"imd=\t"<<imd<<"\t"<<"toReturn=\t"<<toReturn<<"\t";


	//20060313,与深层土的交换迭代计算
	float LoopToReturn=0;
	int LoopCount=0;
	while (abs(LoopToReturn-toReturn)>1.0f && LoopCount<20) 
	{
		LoopCount++;
		LoopToReturn=toReturn;
		DeepW+=LoopToReturn;
		DSita=DeepW/DeepWmax2;
		Sita=W/Wmax2;//调整含水率
		if(Sita<1&&DSita<1)
			imd=1-B*(myPara.am*pow(Sita,(-myPara.bm))-myPara.ad*pow(DSita,(-myPara.bd)));
		else if(Sita>=1&&DSita<1)
			imd=1-B*(0-myPara.ad*pow(DSita,(-myPara.bd)));
		else if(Sita<1&&DSita>=1)
			imd=1-B*(myPara.am*pow(Sita,(-myPara.bm))-0);
		else
			imd=1;
		toReturn=kv*imd*Area/60*MSTEP;

		//以下是牛顿迭代法
		toReturn=LoopToReturn-(toReturn-LoopToReturn)/(-kv*Area*B*(myPara.am*myPara.bm*pow((W-LoopToReturn),(-myPara.bm-1))/pow(Wmax2,(-myPara.bm))+myPara.ad*myPara.bd*pow((DeepW+LoopToReturn),(-myPara.bd-1))/pow(DeepWmax2,(-myPara.bd)))-1);

		if(toReturn>=0)
		{
			if(toReturn<W)//如果下渗的水量小于本层土水量，则按计算量下渗；否则令本层土水量为1，其余全部下渗
				W-=toReturn;
			else
			{
				wtemp=W-1.0f;
				toReturn=wtemp;
				W=1.0f;
			}
		}
		else
		{
			wtemp=-toReturn;
			if(wtemp<DeepW)
			{
				wtemp=W+wtemp;
				if(wtemp<Wmax2)
				{
					W=wtemp;
				}
				else
				{
					toReturn=-(Wmax2-W);
					W=Wmax2;
				}
			}
			else
			{
				wtemp=DeepW-1.0f;
				if((wtemp+W)<Wmax2)
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
	}//end of while

	//return 0.0f;不要了？20060222
	//byiwish20060222
	return toReturn;
}
//bool MidSoil::NeedWater1(void)
//{	
//
//	if(W<Wmax1)
//	{
//		return true;
//	}
//	else
//	{
//		return false;
//	}
//}
//
//// 是否需要自由水补给
//bool MidSoil::NeedWater2(void)
//{
//	if( NeedWater1() ) return false;
//	
//	
//	if(W<Wmax2)
//	{
//		return true;
//	}
//	else
//	{
//		return false;
//	}
//}

// 蓄满产流的比率
float MidSoil::GetSRate(void) 
//20041018:SRate,饱和率，如果越饱和，比率越高，蓄满为1,如果还不足田间持水量，则为0
{
	//20051212 李铁键
	//如果当前含水量不足田间持水量，正常返回负值
	//以此使含水量不足田间持水量的情况下，地表下渗系数大于田间持水量下的参考下渗能力

	//if(NeedWater1())
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

int MidSoil::initialize(MParameters MyPara)
{
	Area=MyPara.Area;
	RouteLength=MyPara.RouteLength;

	kh=MyPara.PKH2;//中层土的水平渗透速率
	n1=MyPara.MSita1;//中层土田间持水量孔隙率
	n2=MyPara.MSita2;//中层土自由含水量的孔隙率
	SlopeJ=MyPara.SlopeJ;
	kv=MyPara.PKV2;//add by iwish 20060220

	if(SlopeJ<1.1e-5)//20110927,xiaofc,坡面坡度太小，中层土体积过小，计算失败
	{
		Wmax1=Area*MyPara.UDepth*n1;   //楔形，没有RouteLength*2
		Wmax2=Area*MyPara.UDepth*(n1+n2); //20041016修改
	}
	else
	{
		Wmax1=Area*RouteLength*SlopeJ*n1;   //楔形，没有RouteLength*2
		Wmax2=Area*RouteLength*SlopeJ*(n1+n2); //20041016修改
	}
	
	W=MyPara.MidSoilW;//将来应该从数据库里读//lifangmi
	//W=Wmax1+(Wmax2-Wmax1)/100.0;  

	//byiwish20060222
	Sita=W/Wmax2;

	isDebug=false;

	//am=3520;
	//bm=4.21;

	//david,初始化写到这里
    Wout=0.0f;

	return 1;
}
