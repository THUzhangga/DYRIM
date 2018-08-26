#include "WaterInRockSnow.h"
#include <cmath>
#include <iostream>
using namespace std;

WaterInRockSnow::WaterInRockSnow(void)
:H(0.0),Kr(6e-5),mu(0.02),Hmax(0)
,SL(0),SR(0),SS(0),pL(NULL),pR(NULL),pS(NULL)
{
}

WaterInRockSnow::~WaterInRockSnow(void)
{
}

//变量初始化
void WaterInRockSnow::Initiallize(float Hmax0,float Kr0,float mu0,float SL0,float SR0,float SS0,vector<FVMunit>* pL0,vector<FVMunit>* pR0,vector<FVMunit>* pS0)
{
	Hmax=Hmax0; 
	Kr=Kr0;  
	mu=mu0;
	SL=SL0;     
	SR=SR0;  
	SS=SS0;
	pL=pL0;     
	pR=pR0;  
	pS=pS0;
}


//水量dW变化(m3)转化为水位H的变化(m),W为正代表水位上升，反之下降
void WaterInRockSnow::WtoH(float dW)
{
	if(abs(dW)<=0.01)
		return;

	float w=0;
	
	//迭代过程最小水位涨幅2cm
	float e=0.02;
	
	float H1=H;

	//最大迭代次数
	int maxNum=0;
	
	if(dW>0)
	{
		while(w<dW && H1<=Hmax)
		{
			maxNum++;

			H1+=e;
			w=this->HtoW(H,H1);

			if(w<0 || maxNum>2e4)
			{
				w=0;
				break;
			}
		}
	}
	else
	{
		dW=abs(float(dW));
		while(w<dW)
		{
			maxNum++;

			H1-=e;
			w=this->HtoW(H,H1);	
			
			//w<0是坡面上全是雪，不计算岩石水情况
			if(w<0 || maxNum>2e4)
			{
				w=0;
				break;
			}
		}
	}

	//更新当前水位
	H=H1;
}


//入口参数：H0初始水位，H1变化后水位，sType坡面类型
//功能：已知H0和H1(以x轴为基准面的相对高程m)，求单坡面两个水位之间的裂隙水总量m3
float WaterInRockSnow::HtoW(float H0,float H1,string sType)
{
	::transform(sType.begin(),sType.end(),sType.begin(),::tolower);
	
	std::vector<FVMunit>* p;
	float S;

	if(sType=="l") { p=pL; S=SL;}
	if(sType=="r") { p=pR; S=SR;}
	if(sType=="s") 
	{
		if(SS<=0){return 0;}
		p=pS; S=SS;
	}

	//size为0时说明全是雪，下面程序当然就有问题
	if(p->size()<=0)
	{
		return -1;
	}
	
	float w=0,w1=0;//m3
	float hmax=max(H0,H1),hmin=min(H0,H1);
	
	//相当于图中K点纵坐标
	const float hlast=(p->end()-1)->hdown - (p->end()-1)->dx2 * sin((p->end()-1)->beta2)/2;
	
	//hu,hd是有限体积单元包含H0和H1部分的上下边界的高程
	float hu,hd,hutmp,hdtmp;

	//最大高程低于最末有限体积单元时
	if(hmax<=hlast)
	{
		w=(hmax-hmin)*S;
	}

	//最小高程低于最末有限体积单元时
	if(hmin<=hlast)
	{
		w1=(hlast-hmin)*S;
		hmin=hlast-1e-5;
	}

	bool flag=false;
	for(int i=0;i<p->size();i++)
	{
		hutmp = (*p)[i].hdown + (*p)[i].dx2 * sin((*p)[i].beta2) / 2;
		hdtmp = (*p)[i].hdown - (*p)[i].dx2 * sin((*p)[i].beta2) / 2;

		if(hmax<=hutmp && hmax>=hdtmp)
		{
			hu=hutmp;
			flag=true;
		}
		if(hmin<=hutmp && hmin>=hdtmp)
		{
			hd=hdtmp;
			w+=(*p)[i].VH;//m3
			break;
		}
		if(flag==true)
		{
			w+=(*p)[i].VH;//m3
		}
	}
	w=w*(hmax-hmin)/(hu-hd);
	
    //注意这里要乘以给水度mu，因为H0和H1之间不是水，而是大石头中的水。
	return (w+w1)*mu;
}


//功能：得到元流域介于H0和H1之间的岩石裂隙水量
float WaterInRockSnow::HtoW(float H0,float H1)
{
	return HtoW(H0,H1,"l") + HtoW(H0,H1,"r") + HtoW(H0,H1,"s");
}
