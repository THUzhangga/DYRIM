#ifndef WATERINCHANNEL_H
#define WATERINCHANNEL_H
#include "functions.h"
#include "FVMunit.h"
#include "DataStruct.h"
#include <fstream>
using namespace std;
#pragma once


//汇流模型，数值求解一维浅水方程，有限体积法激波捕捉ROE格式
class WaterInChannel
{
public:
	WaterInChannel(void);
public:
	~WaterInChannel(void);

public:
	
	//H:仍是以GH为基准面的相对水头(河道所有断面的平均)
	//q0:当前时刻与沟道地下水交互量为q(m2/s)
	float H,q;

private:
	//重力加速度
	static const float g;

	//采样频率
	float MSTEP;

	//元流域坡面拓扑信息
	Para* mPara;

	//河道梯形断面(GEFH)
	float gh,gm,ef,theta1,theta2;

	//各坡面有限单元指针
	std::vector<FVMunit> *pL,*pR,*pS;

	//这里的指针是以时间为变量
	float *Qu1,*Qu2,*Quout,*FlowB,*FlowH,*Flow_v;
	
	bool SaveFlowPattern;
	bool isDebug;
	ofstream* myFile;

	//计算需要参变量
	//极小值，用于能坡J
	float nano;
	
	//空间步长
	float delta_x;

	//空间步长数
	int xsteps;

	//当前时刻值，因为FVM中的Time里面在一个步长里还需要细分
	float Time;

	//计算时间步长dt1(s)
	int delta_t;

	//时间步长数
	int Steps;

	//为求四点偏心的平均值
	float h[4];
	float u[4];	
	float B[4];
	
	//三角形:B=2mh,A=mh^2
	float m;
	
	//经验系数:ph/px项在涨水期的有效系数0.8,退水期为1-eta=0.2
	float eta;
	
	//20070828,xiaofc,考虑小流量下变糙率
	//沟道的manning系数,这个是动态的，变化之后的
	float n;
	
	//这是从riversegs表里直接读取来的
	float n0;
	
	//河道长m
	float L;

	//河道比降
	float S;

	//Q:n时刻各河段节点的流量值，以空间为变量
	//Qout:n+1时刻,，以空间为变量
	float *Q,*Qout;


public:
	//初始化，每条河段只初始化一次
	void Initiallize(Para* mPara0,float MSTEP0,int Steps0,float gh0,float gm0,float ef0,float theta10,float theta20,float* Qu10,float* Qu20,float* Quout0,vector<FVMunit>* pL0,vector<FVMunit>* pR0,vector<FVMunit>* pS0,float* FlowB0,float* FlowH0,float* FlowV0,bool SaveFlowPattern0,ofstream* file);
	
	//扩散波法计算一个时间步长
	void InChannel(void);

};



#endif
