#pragma once
#include"MParameters.h"
#include ".\Leaf.h"
#include ".\SoilWater.h"
#include ".\DeepSoil.h"
#include "datastruct.h"
class MidSoil
{
public:
	float kh;//ˮƽ����͸ϵ��
	float kv;//�в�����������Ĵ�ֱ��͸ϵ��
	float RouteLength;
	float n1;  //��϶�ʵ�����ˮ����
	float n2;  //��϶�ʵ�����ˮ����
	float SlopeJ; //����ƽ���¶�
	float Wmax2; //���Ǳˮ��
	//20060222byiwish
	float Sita;//��������ˮ��
	//float bm;//�����ƹ�ʽ��ָ����
	//float am;//�����ƹ�ʽ��ϵ����
	float imd;//�в������������ƽ���ݶ�byiwish20060222

	bool isDebug;

public:
	float Area;
	float W;  //��ˮ��
	float Wout; //ʱ�γ���,m^3
	float Wmax1; //�������ˮ��

	int MSTEP;//wh

	//wh
	MidSoil(int timestep){
		MSTEP = timestep;
	}


public:
	MidSoil(void);
	~MidSoil(void);
	//float NextHour(float WaterAdded);
	float NextHour(float WaterAdded,MParameters myPara,float DSita,float DeepW,float DeepWmax2);//byiwish20060222 SoilW�Ǳ����ˮ������ʱ��仯��
	//// �Ƿ���Ҫ����ˮ����
	//bool NeedWater1(void);
	//// �Ƿ���Ҫ����ˮ����
	//bool NeedWater2(void);
	// ���������ı���
	float GetSRate(void);
	int initialize(MParameters);

};
