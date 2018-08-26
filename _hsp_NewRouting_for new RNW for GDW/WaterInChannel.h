#ifndef WATERINCHANNEL_H
#define WATERINCHANNEL_H
#include "functions.h"
#include "FVMunit.h"
#include "DataStruct.h"
#include <fstream>
using namespace std;
#pragma once


//����ģ�ͣ���ֵ���һάǳˮ���̣����������������׽ROE��ʽ
class WaterInChannel
{
public:
	WaterInChannel(void);
public:
	~WaterInChannel(void);

public:
	
	//H:������GHΪ��׼������ˮͷ(�ӵ����ж����ƽ��)
	//q0:��ǰʱ���빵������ˮ������Ϊq(m2/s)
	float H,q;

private:
	//�������ٶ�
	static const float g;

	//����Ƶ��
	float MSTEP;

	//Ԫ��������������Ϣ
	Para* mPara;

	//�ӵ����ζ���(GEFH)
	float gh,gm,ef,theta1,theta2;

	//���������޵�Ԫָ��
	std::vector<FVMunit> *pL,*pR,*pS;

	//�����ָ������ʱ��Ϊ����
	float *Qu1,*Qu2,*Quout,*FlowB,*FlowH,*Flow_v;
	
	bool SaveFlowPattern;
	bool isDebug;
	ofstream* myFile;

	//������Ҫ�α���
	//��Сֵ����������J
	float nano;
	
	//�ռ䲽��
	float delta_x;

	//�ռ䲽����
	int xsteps;

	//��ǰʱ��ֵ����ΪFVM�е�Time������һ�������ﻹ��Ҫϸ��
	float Time;

	//����ʱ�䲽��dt1(s)
	int delta_t;

	//ʱ�䲽����
	int Steps;

	//Ϊ���ĵ�ƫ�ĵ�ƽ��ֵ
	float h[4];
	float u[4];	
	float B[4];
	
	//������:B=2mh,A=mh^2
	float m;
	
	//����ϵ��:ph/px������ˮ�ڵ���Чϵ��0.8,��ˮ��Ϊ1-eta=0.2
	float eta;
	
	//20070828,xiaofc,����С�����±����
	//������manningϵ��,����Ƕ�̬�ģ��仯֮���
	float n;
	
	//���Ǵ�riversegs����ֱ�Ӷ�ȡ����
	float n0;
	
	//�ӵ���m
	float L;

	//�ӵ��Ƚ�
	float S;

	//Q:nʱ�̸��Ӷνڵ������ֵ���Կռ�Ϊ����
	//Qout:n+1ʱ��,���Կռ�Ϊ����
	float *Q,*Qout;


public:
	//��ʼ����ÿ���Ӷ�ֻ��ʼ��һ��
	void Initiallize(Para* mPara0,float MSTEP0,int Steps0,float gh0,float gm0,float ef0,float theta10,float theta20,float* Qu10,float* Qu20,float* Quout0,vector<FVMunit>* pL0,vector<FVMunit>* pR0,vector<FVMunit>* pS0,float* FlowB0,float* FlowH0,float* FlowV0,bool SaveFlowPattern0,ofstream* file);
	
	//��ɢ��������һ��ʱ�䲽��
	void InChannel(void);

};



#endif
