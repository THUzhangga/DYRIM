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


//������ˮ��m3ת��Ϊˮλ��W�����ɸ�
float WaterBelowChannel::WtoH(float dW)
{
	//���ﻹ�Ǵ���һ����ˮ�ȵ����⣬��Ϊ��ˮ��Ϊ(���ͺ�ˮ��-����ˮ��)���½�����dw���"��ˮ��"�ռ�,
	//����Ϊ�˷�ֹ��ĸΪ0���ּ�����else���
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

	//�õ����й�������ˮˮ��
	W=HtoW(H);

	if(dW+W>=Wmax-1e-4)
	{
		H=0;
		return dW+W-Wmax;
	}

	//����������Сˮλ�Ƿ�5cm
	float w=W,dH=(dW>0)?0.05:-0.05;
	
	//����ˮ��������dW
	W+=dW;
	
	//���ﲻ����������ѭ��,w�����W
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

	//��ֹ���ָ�ˮ�ˮ��Ϊ�����Ͳ�������
	H=(H>Center[1]-R)?H:(Center[1]-R);

	//H���ֵΪ0
	H=(H>0)?0:H;

	return 0.0;
}


//����ˮλΪH0ʱ����KL֮���ˮ��m3
float WaterBelowChannel::HtoW(float H0)
{
	//S:��������m2
	float S=0,x;
	
	//������������ұ߽�ĺ�����,ÿ�������ʷֵ����±߽�������
	float left=K[0],right=L[0],top,down;

	float dx=0.1;//ÿ10cmһ���ʷ�
	const float KGE=(G[1]-E[1])/(G[0]-E[0]);
	const float KFH=(F[1]-HH[1])/(F[0]-HH[0]);
	
	//Բ���ηǵ�����������ұ߽�ĺ�������п�������ˮ����仯
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
	//��ʱ������߽����������±߽���״����
	//���H0С��K[1]��һ���ǹ���K�㣬�±߽紦���»����ƣ�����H0һ�����ڵ���K[1]
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

	//right-leftС��10cm�����
	if(dx>right-left)
	{
		dx=right-left;
	}

	//��ӵ����ʷ�����������ۼ�
	x=left;
	while(x<right)
	{
		x+=dx;
		
		//ͼ��GE��
		if(x<E[0])
		{
			top=min(H0,KGE*(x-G[0]));
		}
		
		//ͼ��EF��
		if(x>=E[0] && x<=F[0])
		{
			top=min(H0,E[1]);
		}

		//ͼ��FH��
		if(x>F[0])
		{
			top=min(H0,KFH*(x-HH[0]));
		}

		//�����±߽���״��ȷ���±߽�����
		//K��Lλ��ֱ��AB��
		if(x<=B[0])
		{
			down=B[1]+(A[1]-B[1])/(A[0]-B[0])*(x-B[0]);
		}

		//K��Lλ��ֱ��CD��
		if(x>=C[0])
		{
			down=B[1]+(A[1]-B[1])/(A[0]-B[0])*(x-B[0]);
		}

		//K��Lλ��Բ����
		if(x>B[0] && x<C[0])
		{
			down=Center[1]-sqrt(R*R-pow(x-Center[0],2));
		}
		
		S+=(top-down)*dx;
	}

	return S*Length;
}
