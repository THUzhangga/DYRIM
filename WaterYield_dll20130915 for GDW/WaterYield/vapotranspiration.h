#pragma once
#include ".\Leaf.h"
#include ".\SoilWater.h"
#include ".\DeepSoil.h"
#include ".\MidSoil.h"
#include "datastruct.h"
using namespace std;
class Evapotranspiration
{
public:
	float AvgEPI; //�����������
	float ELeft; //����������ʣ��ֵ
	float E;//20070117,������,ʵ�ʵ�������������Ҷ�桢�����ȣ�����λmm
	float thetaf;//����ˮ��
	float thetas;//���ͺ�ˮ��

	//addbyiwish20060220
	//float ium,imd;//ƽ���ݶ�
	//float SitaU,SitaM,SitaD;//�������ĺ�ˮ��
	//float bu,bm,bd;//�����ƹ�ʽ��ָ����
	//float au,am,ad;//�����ƹ�ʽ��ϵ����

private:
	CString EMethod;

	//���ڸ���豹�ʽ����������������
	float theta;//��ǰ��ˮ��
	float thetab;//ë�ܶ��Ѻ�ˮ��
	float thetaw;//��ή��ˮ��
	int N;//ָ��--��������豹�ʽ���߹⻬�ĳ̶�
	float E0_a; //E0/a

	int MSTEP;//wh

	//altered by weihong
public:
	Evapotranspiration(CString em, float theta_b, float theta_w, int power, float e0a,int mstep){
		EMethod = em;
		thetab = theta_b;
		thetaw = theta_w;
		N = power;
		E0_a = e0a;
		MSTEP = mstep;
	}


public:
	Evapotranspiration(void);
	~Evapotranspiration(void);
	bool initialize(MParameters* myPara); //ÿ��ʱ�ζ���Ҫִ��
	bool NextHour(Leaf*,SoilWater*,MidSoil*,DeepSoil*,float HourOffset, float P);
};
