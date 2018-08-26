#ifndef HIGHSLOPERUNOFF_H
#define HIGHSLOPERUNOFF_H
#pragma once
#include "DataStruct.h"
#include "YuLiang.h"
#include "YuLiangChBG.h"
#include "YuLiangCMORPH.h"//add CMORPH,shy
#include "FVMunit.h"
#include "WaterInRockSnow.h"
#include "WaterBelowChannel.h"
#include "WaterInChannel.h"
#include ".\savestatus.h"
#include <iostream>
#include <fstream>
using namespace std;

//�����������ĸ߱��²���ģ��
class HighSlopeRunoff
{
public:
	HighSlopeRunoff(void);
public:
	~HighSlopeRunoff(void);

/*********ģ��ȫ�ֱ����������в��仯*********/
public:
	CString RainType;

	//��������������(s)��������������
	int MSTEP,Steps,StatusTime;

	//ȫ����ˮ���
	float BasinArea; 

	static bool debug;
	static long HourStart;
	static long NumofHours;
	
	HSParameter Min;
	
public:
	static float E;//m3
	static float P;//m3
	
	//TOPAZ��ȡ��Ԫ�����������
	static Para* myPara;

	//�Ӷζ���������
	static BSCode mBSCode;

private:
	ADODB::_ConnectionPtr pCnn;

	//��¼������Ϣ
	ofstream myFile;

	//�վ���������γ���ꡣ
	Cyuliang PMin;
   
	//ʱ��������������ꡣ
	YuLiangChBG PMinCBG;

	//CMORPH��������γ���꣬shy
	YuLiangCMORPH PMinCMORPH;
	
	//���ͼ��Ǳ�������ˮ
	std::vector<FVMunit> fvL,fvR,fvS;

	//�ٽ�߳���
	std::vector<pair<int,int>> lv;
	
	//��������ˮ
	WaterBelowChannel Bchannel;
	
	//��������ˮ
	WaterInChannel Ichannel;

	//SaveStatus mSaveStatus;

public:
	//��ʯ��϶ˮ
	static WaterInRockSnow Rock;


private:
	//ghƽ̲�����;gmƽ̲ˮ��;efƽ̲�ӵ׿�;theta���������µ��½�(����)
	float gh,gm,ef,theta1,theta2;
	
	//ͼ�е�K,L
	float KK[2],LL[2],KS[2];

	//K,L����֮��ľ��룬���ܰ���ֱ�߶κ�Բ����
	float KL;


private:
	//����������ת����,������ʵ�����Կռ�任�ռ䣬����һ���ض��ӶΣ�ֻnewһ�Σ���Ϊû��Ҫÿһ��ʱ�䲽����newһ�Σ���deleteһ�Ρ�
	//new��delete������ʱ�俪���ģ����Ƿŵ������������ֻ�е����н����ڴ���ͷţ�������ʱ��
	//dL:����ռ䲽������,axL:�½�ţ�ٵ�����õ�����
	double *aL,*aR,*aS,*bL,*bR,*bS,*cL,*cR,*cS,*fL,*fR,*fS,*xL,*xR,*xS;
	double *dL,*dR,*dS,*afL,*afR,*afS,*axL,*axR,*axS;

	//������������(m3/s),������������(m3/s),��������ĵ�������(m2/s)
	//Qu:������֧��������ˮ�������̣�Qd:������֧���Ĺ�������ˮ��������
	float *Qu1,*Qu2,*Qd1,*Qd2,*Quout,*Qdout,*FlowB,*FlowH,*FlowV;



public:
	void Calc(BSCode mBSCode0,Para* myPara0,float* Qu10,float* Qu20,float* Qd10,float* Qd20,float* Quout0,float* Qdout0,float* FlowB0,float* FlowH0,float* FlowV0);
	void Initiallize(ADODB::_ConnectionPtr pCnn0,CString RainType0,long NumofHours0,long HourStart0,long StatusTime0,float BasinArea0,int MSTEP0,int Steps0);
		                        
private:
	//��ȡԪ���򹵵�����
	void RiverChannel(void);

	//����Ԫ�����������
	void MeshRegion(void);

	//������������
	void SlopeRunoff(string sType,float p,float dt);

	//��������ˮ�˶�����
	void SaturatedSoil(string sType,float dt);

	//��������ˮ����
	void BellowChannel(float dt);

	//ÿ���Ӷ�ִ��һ��
	void InitializeOneRiver(BSCode mBSCode0,Para* myPara0,float* Qu10,float* Qu20,float* Qd10,float* Qd20,float* Quout0,float* Qdout0,float* FlowB0,float* FlowH0,float* FlowV0);

	//ÿ���Ӷ�ִ��һ��
	void FinalizeOneRiver(void);

};



#endif
