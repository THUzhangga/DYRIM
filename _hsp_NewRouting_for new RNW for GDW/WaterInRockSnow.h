#ifndef WATERINROCKSNOW_H
#define WATERINROCKSNOW_H
#pragma once
#include <string>
#include "FVMunit.h"
#include <algorithm>//transform函数需要该头文件
using namespace std;

//岩石裂隙水和融雪径流
//注：岩石水不可能出流到坡面，因为平衡态时，饱和土壤水中水深大于0的起点和岩石裂隙水相平。
class WaterInRockSnow
{
public:
	WaterInRockSnow(void);
public:
	~WaterInRockSnow(void);

public:
	//水位m，相对高程(T0T为基准面)
	float H;
	
	//渗透系数Kr:影响土壤对岩石的补给
	double Kr;

public:
	void Initiallize(float Hmax0,float Kr0,float mu0,float SL0,float SR0,float SS0,vector<FVMunit>* pL0,vector<FVMunit>* pR0,vector<FVMunit>* pS0);
	
	//水量W变化(m3)转化为水位H的变化(m),W可正可负
	void WtoH(float dW);
	
private:
	//A点的纵坐标
	float Hmax;

	//给水度,影响岩石裂隙水的水位升降快慢
	float mu;
	
	//圆弧下部的水平投影面积m2，用来计算水位和水量转化
	float SL,SR,SS;

	std::vector<FVMunit> *pL,*pR,*pS;

private:
	float HtoW(float H0,float H1,string sType);
	float HtoW(float H0,float H1);

};

#endif

//《地下水动力学》 农业出版社 李佩成主编
//表1：
//土壤渗透系数Kr：
//1,透水性很好的砂砾和含粗砂的砾石，强烈溶蚀的石灰岩和裂隙很发育的岩层：100-1000(米/日)
//2,粗砂;干净的中砂；冲洗干净的砂砾或含细砂的干净砾石，溶蚀较强的岩层：10-1000(米/日)
//3,被细砂或少量粘土充填的砂砾或者砾石；中细砂；弱溶蚀和裂隙较少的岩层；1-10(米/日)
//4,弱透水的细砂；亚粘土；裂隙少的岩层；0.1-1(米/日)
//5,渗透性很弱的亚粘土：0.001-0.1(米/日)
//6，几乎不透水的粘土；密实的泥灰岩；以及其他渗透性很差的厚实的岩层；小于0.001(米/日)

//表2:
//土壤给水度参考值mu:
//1,砂性粘壤土：0.005-0.05
//2,砂壤土：0.05-0.1
//3,粉砂：0.1-0.15
//4,细砂：0.15-0.2
//5,中砂：0.2-0.25
//6,粗砂及砂砾石：0.25-0.35
//7,裂隙灰岩：0.001-0.1
//8,裂隙砂岩：0.02-0.03


/*
//表3：给水度的经验值：
卵砾石：0.35-0.30
粗砂：0.30-0.25
中砂：0.25-0.20
细砂：0.20-0.15
极细砂：0.15-0.10
泥质砂：0.03-0.19
亚粘土：0.03-0.12
粘土：0-0.05
强裂隙岩石：0.01-0.002
裂隙岩石：0.002-0.0002
强岩溶化：0.15-0.05
岩溶化：0.05-0.01
弱岩溶化：0.01-0.005
*/



