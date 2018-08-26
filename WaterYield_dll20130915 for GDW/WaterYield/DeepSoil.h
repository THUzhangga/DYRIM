#pragma once

#include".\MParameters.h"
#include ".\Leaf.h"
#include ".\SoilWater.h"
#include ".\MidSoil.h"
#include "datastruct.h"

class DeepSoil
{

public:
	float Area;
	float DSDepth;  //������
	float DSn1;	//��϶�ʵ�����ˮ����
	float DSn2;  //��϶�ʵ�����ˮ����
	//20060222byiwish
	float Sita;//��������ˮ��
	//float bd;//�����ƹ�ʽ��ָ����
	//float ad;//�����ƹ�ʽ��ϵ����

public:
	float DSW;  //��ǰ��ˮ��
	float Wmax1;
	float Wmax2;//addbyiwish20060225
	//20060223ע�͵�float Wout; //ʱ�γ���,m^3 addbyiwish20060222

public:
	DeepSoil(void);
	~DeepSoil(void);
	//// �Ƿ���Ҫ����ˮ����
	//bool NeedWater2(void);
	//// �Ƿ���Ҫ����ˮ����
	//bool NeedWater1(void);
	// ���ݸ�ˮ�����㣬���û�����ˮ��
	void NextHour(float WaterSupply,float imd);
	int initialize(MParameters DSPara);
};
