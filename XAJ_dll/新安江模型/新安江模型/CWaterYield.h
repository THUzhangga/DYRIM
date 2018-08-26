#pragma once

#include "datastruct.h"
#include ".\YuLiang.h"
#include "YuLiangChBG.h"

#ifndef WATERYIELD_H
#define WATERYIELD_H

class WaterYield
{
public:
	float* QRSS;//壤中流--存储到xajstatus表
	float* QRG;//地下径流--存储到xajstatus表
	
	float* Q;//总径流（壤中+地下+地面），对应discharge表中的Q


	//用于AVPM中的重力侵蚀，这两个参数也是产流模型与薛海重力侵蚀模型的接口
	float* pWLM;
	float* pWRM;

public:
	ADODB::_ConnectionPtr pCnn;

	Cyuliang PMin;//日均雨量，经纬度
	YuLiangChBG PMinCBG;//时段雨量，大地坐标
	
public:
	CString sccd;
	CString RainType;
	long NumofHours;
	long HourStart;
	long StatusTime;
	int MSTEP;
	long Steps;
	int StepsInHour;
	XAJParameter XAJ;//对应数据库中的XAJUSEPARA表。
	float BasinArea;//全流域面积，分配基流

private:
	int YearStart,MonthStart,YearEnd,MonthEnd;
	float AvgEPI;//小时蒸发能力
	
public:
	void initialize(void);

	void ReadLE(BSCode mBSCode);
	float StepEPI(float HourOffset, float P);//步长蒸发能力
	void Calc(BSCode mBSCode,Para myPara);
	void finalize(void);

public:
	//for SRM
	CString SnowModelType;
	void InitializeSRM(BSTR user,BSTR password,BSTR sid);
	void FinalizeSRM();

};

#endif