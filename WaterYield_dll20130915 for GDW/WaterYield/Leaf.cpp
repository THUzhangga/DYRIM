
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
	//����Ϊ��ʼ���ΪҶ���޴�ˮ
	//W=0.0;
	//Ҷ���ˮ�ĳ�ʼֵд������

	//����
	isDebug=false;
	LAI=myParameter->LAI;
	I0=myPara->I0;//Ҷ����ȫ����ʱ�ĳ�ˮ��
	if(Type=='S')
	{
		Area=myPara->AreaS;
		W=myParameter->WSL;//Ҷ���ˮ��//��ʼֵ��parameter�б���Ϊ0
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

	//���Ϊ�����������ڣ���WҲ��Ϊ��
	if(Area<-0.1)
	{
		W=-1;
	}

	return 1;
}


//wh���������͸��Ҷ��ľ�����
float Leaf::NextHour(float WaterAdded)//ˮ���m��
{
	float toReturn;
	toReturn=WaterAdded+W-I0*LAI;
	if(isDebug)
		cout<<"WaterAdded="<<WaterAdded<<"\tW="<<W<<"\tI0="<<I0<<"\tLAI="<<LAI<<"\ttoReturn="<<toReturn<<endl;
	if(toReturn>0)//ˮ��
	{
		W=I0*LAI;//I0*LAIΪ�����ˮ����
		return toReturn;
	}
	else//ˮ����
	{
		W+=WaterAdded;
		return 0;
	}
}