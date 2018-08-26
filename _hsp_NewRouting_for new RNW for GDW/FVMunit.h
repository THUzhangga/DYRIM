#ifndef FVMUNIT_H
#define FVMUNIT_H
#pragma once
#include <vector>
#include <string>
#include <fstream>
using namespace std;

//注：这里所有的单位都是国际单位，方便计算，单位转化过程都在程序最开始时完成

//一个非饱和土壤单元
class RichardsUnit
{
public:
	RichardsUnit(float theta0,float K0,float D0,float dz0);
public:
	~RichardsUnit(void);

public:
	//当前实际土壤含水量
	float theta;
	
	//这里将变量转化为mm级，为了减少浮点数，可能会增加计算速度
	//渗透系数mm/s,扩散率mm2/s
	double K,D;

	//空间步长mm，为土壤不均匀分层预留
	float dz;
};


//地下水运动的有限体积单元
class FVMunit
{
public:
	//初始化后就不会再变
	FVMunit(float x0,float hup0,float hdown0,float hm0,float ahup0,float dx10,float dx20,float beta10,float beta20,float Width0,int landtype0,string sType0,float Dis0,float VH);
public:
	~FVMunit(void);

public:
	//涉及高程的值，均是以T0G为基准面的相对高程,ahup除外。
	//hup,hdown上下边界中点的相对高程,ahup:绝对高程，蒸发能力输入
	float hup,hdown,ahup;

	//dx1:上边界长m;dx2:下边界长m;Width坡面宽m
	float dx1,dx2,Width;

	//上边界和不透水层坡度
	float beta1,beta2;

	//对应水平方向单元的岩石裂隙水体积
	float VH;

	//有限单元的横坐标
	float x;

	//SlopeType:L,R,S
	string sType;
	
	//距离坡顶的距离m(沿着坡面方向)
	float Dis;


public:
	//田间持水量thetaf:不同土壤类型差别较大，细砂4.5%左右，粘土20%左右,用在非饱和和饱和土壤水间交互
	//毛管断裂含水量thetab:thetab=(0.6~0.7)*thetaf;
	//植被凋萎含水量thetaw:受土壤类型和植被类型控制（两篇中文文献)，但主要土壤控制，对于壤土一般5%左右量级
	//土壤残余含水量thetac:求解Richards方程用到,敏感度较低，取较小值0-0.08
	//饱和含水量thetas:0.4-0.6
	static float thetaf,thetas,thetab,thetaw,thetac;
	
	//N决定傅抱璞公式曲线光滑的程度一般在10到30之间，E0_a一般可以忽略补给，可以取较小的值，比如0.1
	static float N,E0_a;
	
	//Richards方程的参数,usD:(1-20)cm2/min,这里已转化成国际单位计算;usL:(1-10)耗散系数；usf为饱和K的垂向衰减系数，如果认为不衰减为0
	static float usL,usf;
	static double usD;
	
	//非饱和土壤垂向空间步长m
	static float dz;
	
	static short StartYear,StartMonth,StartDay,Year,Month,Day;

	//dt1,dt2坡面上下的时间步长(s),TimeStart执行到现在已经经历的秒数
	static float dt1,dt2,StartHour,Hour;

	//程序已经模拟的时间(s)
	static float TimeStart;

	//夏季最大日蒸发和冬季最小日蒸发m
	static float Emax,Emin;

	//高程敏感的参数：
	//dc植被覆盖率0-1，fc坡面饱和渗透系数m/d(fc给一些经验值)这里已转化成国际单位,thetai:初始非饱和土壤含水量
	//Betaf:坡面入渗能力衰减系数(Inx/thetas),x为最大入渗能力与稳定入渗之比。
	//Betaf文献取值: 耕地:3.69,草地:3.5,灌木林:7.95,针叶林:0.80,阔叶林:1.98,竹林:2.23
	static float dc[5],Betaf[5],thetai[5];
	static double fc[5];

	//不同植被类型的叶面积覆盖指数
	const static float LAI[4][12],maxLAI[4];

	//蒸腾作用影响的最大深度m
	const static float fz[5];

	//报错文件句柄指针
	static ofstream* myFile;


public:
	//坡面上的植被覆盖类型：雪0，林1，灌2，草3，耕4
	int landtype;	

	//坡面实际水深m，坡面实际单宽流量m2/s
	float ss,qs;

	//hv:回归流m/s,hm:饱和土壤水水位m,ksa:饱和土壤的平均渗透系数m/s,mu:给水度(0,1)
	float hv,hm,mu;
	double ksa;

	//大于TimeStart时表示有回归流产生
	float TimeEnd;
	
	//非饱和土壤的网格节点
	std::vector<RichardsUnit> ri;

private:
	//dt1植被实际蓄水量m,dt1剩余蒸发能力m
	float wPlant,EPI;

	//非饱和土壤运动的上边界通量,FED:m,TD:s
	float TD,FED;

	//非饱和土壤运动的上下边界
	float thetaA,thetaB;
	double KA,DA,KB,DB;

	
public:
	//更新垂向非饱和土壤网格单元数目
	void UpdateRichardsUnit(void);

    //坡面产流
	void SlopeSourceTerm(float p,float dt);

	//非饱和土壤水运动
	void UnsaturatedSoil(float dt);

	
private:
    //初始化垂向非饱和土壤网格单元数目
	void InitialRichardsUnit(void);

	//估算非饱和土壤计算的时间步长，防止计算出负含水量或超饱和含水量
	//float GetTimeStep(float LeftTime,float ufe,int N);

	//植被截留:p:降雨,houroffset:已经历的小时数,返回穿过净雨量
	float Plant(float p,float dt);

	float StepEPI(float dt,int H,int H0);

};

#endif
