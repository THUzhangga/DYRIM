#include "WaterBelowChannel.h"
#include "FVMunit.h"
#include <cmath>
#include <iostream>
using namespace std;

WaterBelowChannel::WaterBelowChannel(void)
{
	H=0.0;
	Wmax=0.0;
}

WaterBelowChannel::~WaterBelowChannel(void)
{
}


//将新增水量m3转化为水位，W可正可负
float WaterBelowChannel::WtoH(float dW)
{
	//这里还是处理一个给水度的问题，认为给水度为(饱和含水量-田间持水量)，新进来的dw填充"给水度"空间,
	//下面为了防止分母为0，又加上了else语句
	if(FVMunit::thetas-FVMunit::thetaf>=0.02)
	{
		dW=dW/(FVMunit::thetas-FVMunit::thetaf);
	}
	else
	{
		dW=dW/FVMunit::thetas;
	}

	if(abs(dW)<=0.01)
		return 0;

	//得到现有沟道地下水水量
	W=HtoW(H);

	if(dW+W>=Wmax-1e-4)
	{
		H=0;
		return dW+W-Wmax;
	}

	//迭代过程最小水位涨幅5cm
	float w=W,dH=(dW>0)?0.05:-0.05;
	
	//现有水量增加了dW
	W+=dW;
	
	//这里不限制下面死循环,w恒大于W
	W=(W>1e-4)?W:1e-4;

	if(dW>0)
	{
		while(w<W)
		{
			H+=dH;
			w=HtoW(H);
		}
	}
	else
	{
		while(w>W)
		{
			H+=dH;
			w=HtoW(H);
		}
	}

	//防止出现负水深，水深为负他就不是他了
	H=(H>Center[1]-R)?H:(Center[1]-R);

	//H最大值为0
	H=(H>0)?0:H;

	return 0.0;
}


//返回水位为H0时，和KL之间的水量m3
float WaterBelowChannel::HtoW(float H0)
{
	//S:横断面面积m2
	float S=0,x;
	
	//沟道横断面左右边界的横坐标,每个纵向剖分的上下边界纵坐标
	float left=K[0],right=L[0],top,down;

	float dx=0.1;//每10cm一个剖分
	const float KGE=(G[1]-E[1])/(G[0]-E[0]);
	const float KFH=(F[1]-HH[1])/(F[0]-HH[0]);
	
	//圆弧段非单调情况，左右边界的横坐标才有可能随着水深发生变化
	/*if(Center[0]>K[0] && Center[0]<L[0])
	{
		if(H0<K[1])
		{
			left=Center[0]-sqrt(R*R-pow(H0-Center[1],2));
		}
		if(H0<L[1])
		{
			right=Center[0]+sqrt(R*R-pow(H0-Center[1],2));
		}
	}*/
	//此时积分左边界横坐标根据下边界形状而定
	//如果H0小于K[1]，一定是过了K点，下边界处于下滑趋势，否则H0一定大于等于K[1]
	if(H0<K[1])
	{
		if(K[0]<=B[0])
		{
			left=B[0]+(H0-B[1])*(A[0]-B[0])/(A[1]-B[1]);
		}
		else
		{
			left=Center[0]-sqrt(R*R-pow(H0-Center[1],2));
		}
	}
	if(H0<L[1])
	{
		if(L[0]>=C[0])
		{
			right=C[0]+(H0-C[1])*(C[0]-D[0])/(C[1]-D[1]);
		}
		else
		{
			right=Center[0]+sqrt(R*R-pow(H0-Center[1],2));
		}
	}

	//right-left小于10cm的情况
	if(dx>right-left)
	{
		dx=right-left;
	}

	//左从到右剖分条面积不断累加
	x=left;
	while(x<right)
	{
		x+=dx;
		
		//图中GE段
		if(x<E[0])
		{
			top=min(H0,KGE*(x-G[0]));
		}
		
		//图中EF段
		if(x>=E[0] && x<=F[0])
		{
			top=min(H0,E[1]);
		}

		//图中FH段
		if(x>F[0])
		{
			top=min(H0,KFH*(x-HH[0]));
		}

		//根据下边界形状，确定下边界坐标
		//K和L位于直线AB上
		if(x<=B[0])
		{
			down=B[1]+(A[1]-B[1])/(A[0]-B[0])*(x-B[0]);
		}

		//K和L位于直线CD上
		if(x>=C[0])
		{
			down=B[1]+(A[1]-B[1])/(A[0]-B[0])*(x-B[0]);
		}

		//K和L位于圆弧上
		if(x>B[0] && x<C[0])
		{
			down=Center[1]-sqrt(R*R-pow(x-Center[0],2));
		}
		
		S+=(top-down)*dx;
	}

	return S*Length;
}
