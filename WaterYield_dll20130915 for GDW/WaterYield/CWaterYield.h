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
	float* qsT;//总和

	//产沙量
	float* ssS;
	float* ssL;
	float* ssR;
	float* ssT;//总和

	//记录WLM WRM序列
	float* pWLM;
	float* pWRM;

	Cyuliang PMin;//日均雨量的处理，经纬度
	YuLiangChBG PMinCBG;//时段雨量的处理，大地坐标
	YuLiangCMORPH PMinCMORPH;//CMORPH,经纬度,shy
	CString RainType;//降雨量的类型字符串
	bool isDebug;

//altered by wh,这些参数将都由外部传进来。注：在这里类定义中的参数所有计算坡面用的都是一样的
public:
	// used to construct "MParameters"
	float UpInitWaterContent;
	float MidInitWaterContent;
	float DownInitWaterContent;	

	// used to construct "Evaptranspiration"
	//added by weihong

	CString SEEquation;//坡面产沙方程类型

	CString emethod;
	float thetab;//毛管断裂含水率
	float thetaw;//调萎含水率
	int N;//指数
	float E0_a; //E0/a
	
	//wh,added2008.3.24,used to save status table
	CString sccd;

	long NumofHours;//wh,改值不会变化，没必要每次都要从函数入口传递，影响时间
	long HourStart;//wh
	long StatusTime;//wh,20060406,增加status表存储数据的周期
	long Steps;

	int StepsInHour;//拿到这里了,wh
	int MSTEP;


public:
	//bool IsMonthStart(long HourStart,int i);
	ADODB::_ConnectionPtr pCnn;
	CString SnowModelType;//融雪模型部分
	void InitializeSRM(BSTR user,BSTR password,BSTR sid);
	void FinalizeSRM();
	

public:
	void Calc(BSCode mBSCode,Para myPara);
	int initialize(void);
	void finalize(void);
};

#endif