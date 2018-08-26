#pragma once

#include "datastruct.h"
#include ".\YuLiang.h"
#include "YuLiangChBG.h"

#ifndef WATERYIELD_H
#define WATERYIELD_H

class WaterYield
{
public:
	float* QRSS;//������--�洢��xajstatus��
	float* QRG;//���¾���--�洢��xajstatus��
	
	float* Q;//�ܾ���������+����+���棩����Ӧdischarge���е�Q


	//����AVPM�е�������ʴ������������Ҳ�ǲ���ģ����Ѧ��������ʴģ�͵Ľӿ�
	float* pWLM;
	float* pWRM;

public:
	ADODB::_ConnectionPtr pCnn;

	Cyuliang PMin;//�վ���������γ��
	YuLiangChBG PMinCBG;//ʱ���������������
	
public:
	CString sccd;
	CString RainType;
	long NumofHours;
	long HourStart;
	long StatusTime;
	int MSTEP;
	long Steps;
	int StepsInHour;
	XAJParameter XAJ;//��Ӧ���ݿ��е�XAJUSEPARA��
	float BasinArea;//ȫ����������������

private:
	int YearStart,MonthStart,YearEnd,MonthEnd;
	float AvgEPI;//Сʱ��������
	
public:
	void initialize(void);

	void ReadLE(BSCode mBSCode);
	float StepEPI(float HourOffset, float P);//������������
	void Calc(BSCode mBSCode,Para myPara);
	void finalize(void);

public:
	//for SRM
	CString SnowModelType;
	void InitializeSRM(BSTR user,BSTR password,BSTR sid);
	void FinalizeSRM();

};

#endif