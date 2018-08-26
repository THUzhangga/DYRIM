#pragma once
#include ".\MParameters.h"
#include ".\Leaf.h"
#include ".\DeepSoil.h"
#include ".\MidSoil.h"
#include "datastruct.h"


class SoilWater //�����
{
private:
	float lambda; //20070301,xiaofc,������ˮϵ���뺬ˮ����ϵ�е�ָ��

public:
	float k0;
	float RouteLength;
	float n1;  //��϶�ʵ�����ˮ����
	float n2;  //��϶�ʵ�����ˮ����
	float SlopeJ; //����ƽ���¶�
	float Wmax2; //���Ǳˮ��
	float Depth;//�������ȣ���Ϊ�Ǹ����㣬20cm=0.2m
	float kv;//��������������m/h����������в����ģ��������ֵ��ͬ
	//20060222byiwish
	float Sita;//��������ˮ��
	//float bu;//�����ƹ�ʽ��ָ����
	//float au;//�����ƹ�ʽ��ϵ����
	float ium;//��������в�����ƽ���ݶ�byiwish20060222

	bool isDebug;

public:
	float Area;
	float W;  //��ˮ��
	float Wout; //ʱ�γ���,m^3
	float Wmax1; //�������ˮ��

	int MSTEP;//wh

	//wh
	SoilWater(int timestep){
		MSTEP = timestep;
	}

public:
	SoilWater(void);
	~SoilWater(void);
	//float NextHour(float WaterAdded,float SRate);//����MidSoil���SRate���ڸ���������͸��ϵ��
	float NextHour(float WaterAdded,MParameters myPara,float MSita,float MidW,float MidWmax2);//���в�����ˮ�ʼ�����������ϵ������ƽ���ݶȣ�20060222byiwish
	// �Ƿ���Ҫ����ˮ����
	bool NeedWater1(void);
	// ���������ı���
	float GetSRate(void);
	int initialize(MParameters myPara);

};
