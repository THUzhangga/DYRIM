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

//������ʼ��
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


//ˮ��dW�仯(m3)ת��ΪˮλH�ı仯(m),WΪ������ˮλ��������֮�½�
void WaterInRockSnow::WtoH(float dW)
{
	if(abs(dW)<=0.01)
		return;

	float w=0;
	
	//����������Сˮλ�Ƿ�2cm
	float e=0.02;
	
	float H1=H;

	//����������
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
			
			//w<0��������ȫ��ѩ����������ʯˮ���
			if(w<0 || maxNum>2e4)
			{
				w=0;
				break;
			}
		}
	}

	//���µ�ǰˮλ
	H=H1;
}


//��ڲ�����H0��ʼˮλ��H1�仯��ˮλ��sType��������
//���ܣ���֪H0��H1(��x��Ϊ��׼�����Ը߳�m)������������ˮλ֮�����϶ˮ����m3
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

	//sizeΪ0ʱ˵��ȫ��ѩ���������Ȼ��������
	if(p->size()<=0)
	{
		return -1;
	}
	
	float w=0,w1=0;//m3
	float hmax=max(H0,H1),hmin=min(H0,H1);
	
	//�൱��ͼ��K��������
	const float hlast=(p->end()-1)->hdown - (p->end()-1)->dx2 * sin((p->end()-1)->beta2)/2;
	
	//hu,hd�����������Ԫ����H0��H1���ֵ����±߽�ĸ߳�
	float hu,hd,hutmp,hdtmp;

	//���̵߳�����ĩ���������Ԫʱ
	if(hmax<=hlast)
	{
		w=(hmax-hmin)*S;
	}

	//��С�̵߳�����ĩ���������Ԫʱ
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
	
    //ע������Ҫ���Ը�ˮ��mu����ΪH0��H1֮�䲻��ˮ�����Ǵ�ʯͷ�е�ˮ��
	return (w+w1)*mu;
}


//���ܣ��õ�Ԫ�������H0��H1֮�����ʯ��϶ˮ��
float WaterInRockSnow::HtoW(float H0,float H1)
{
	return HtoW(H0,H1,"l") + HtoW(H0,H1,"r") + HtoW(H0,H1,"s");
}
