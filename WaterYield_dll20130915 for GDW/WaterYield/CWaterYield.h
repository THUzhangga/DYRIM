#pragma once

#include "datastruct.h"
#include ".\YuLiang.h"
#include "YuLiangChBG.h"
#include "YuLiangCMORPH.h"

#ifndef WATERYIELD_H
#define WATERYIELD_H

class WaterYield
{
public:
	CString processor_name;
	long rank;

public:
	/*struct QSerial qsS,qsL,qsR,qsT;*/
	float* qsS;
	float* qsL;
	float* qsR;
	float* qsT;//�ܺ�

	//��ɳ��
	float* ssS;
	float* ssL;
	float* ssR;
	float* ssT;//�ܺ�

	//��¼WLM WRM����
	float* pWLM;
	float* pWRM;

	Cyuliang PMin;//�վ������Ĵ�����γ��
	YuLiangChBG PMinCBG;//ʱ�������Ĵ����������
	YuLiangCMORPH PMinCMORPH;//CMORPH,��γ��,shy
	CString RainType;//�������������ַ���
	bool isDebug;

//altered by wh,��Щ�����������ⲿ��������ע���������ඨ���еĲ������м��������õĶ���һ����
public:
	// used to construct "MParameters"
	float UpInitWaterContent;
	float MidInitWaterContent;
	float DownInitWaterContent;	

	// used to construct "Evaptranspiration"
	//added by weihong

	CString SEEquation;//�����ɳ��������

	CString emethod;
	float thetab;//ë�ܶ��Ѻ�ˮ��
	float thetaw;//��ή��ˮ��
	int N;//ָ��
	float E0_a; //E0/a
	
	//wh,added2008.3.24,used to save status table
	CString sccd;

	long NumofHours;//wh,��ֵ����仯��û��Ҫÿ�ζ�Ҫ�Ӻ�����ڴ��ݣ�Ӱ��ʱ��
	long HourStart;//wh
	long StatusTime;//wh,20060406,����status��洢���ݵ�����
	long Steps;

	int StepsInHour;//�õ�������,wh
	int MSTEP;


public:
	//bool IsMonthStart(long HourStart,int i);
	ADODB::_ConnectionPtr pCnn;
	CString SnowModelType;//��ѩģ�Ͳ���
	void InitializeSRM(BSTR user,BSTR password,BSTR sid);
	void FinalizeSRM();
	

public:
	void Calc(BSCode mBSCode,Para myPara);
	int initialize(void);
	void finalize(void);
};

#endif