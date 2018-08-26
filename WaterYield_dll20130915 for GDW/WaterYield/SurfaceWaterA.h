#pragma once
#include<math.h>
#include"MParameters.h"
using namespace std;
class SurfaceWaterA
{
private:
	//MParameters
	float RouteLength;
	double Roughnessn;
	double SlopeJ;
	float Area;
	float PenetrateK;//每小时可下渗的水量
	float SediD;

	//wh解读：以下三个参数土水势用到，土水势为了解决表层水流到表层土的渗透率的问题（超渗产流）。
	float isu;//表面薄层水和表层土的下渗梯度20060302
	float Kr;//非饱和渗透系数与饱和渗透系统的比值
	float lambda;//计算Kr用到的指数//根据分形维数，有默认值

	//20051219 薛海 增加产沙中的系数k
	float ErosionK;
	float ErosionBeta;

	//20140526,郭大卫，用于表征水流侵蚀力与挟沙力的系数
	float ErosionK1;
	float ErosionK2;
	float ErosionBeta1;
	float ErosionBeta2;


public:
	
	//status
	//double WaterDepth;
	float WaterY;//产流量, m^3
	float WaterPenetrated; //m^3//wh解读：从地表渗入到表层土壤中的水量
	bool isDebug;
	CString SEEquation;//产沙公式

	int MSTEP;

public:
	SurfaceWaterA(void);
	SurfaceWaterA(int timestep,CString seequation);
	~SurfaceWaterA(void);
	float NextHour(float PE,MParameters myPara,float USita,float SoilW,float SoilWmax2);//20060302修改byiwish

	//产沙add by xue
	//float Slope_J,Slope_Rough,Sedi_D;
	float SedimentYield;
	void SediYield(void);
	void RevisedSediYieldXue(void);//20070823,李铁键，修正薛海公式

	//20141226,以下：1是认为剥离率和挟沙力都与水流功率相关；2是将坡长转换为40.4坡度时的等效坡长；3是算完后直接乘以比例因子
	void RevisedSediYieldLi1(void); //20140526,郭大卫，考虑挟沙力的对水流侵蚀的限制
	void RevisedSediYieldLi2(void); //20140526,郭大卫，考虑挟沙力的对水流侵蚀的限制
	void RevisedSediYieldLi3(void); //20140526,郭大卫，考虑挟沙力的对水流侵蚀的限制


//private:
//	double RunOff(double Depth);  //不考虑地表的滞蓄了，产流直接到子流域出口

	int initialize(MParameters MyPara);
private:
	float integrand1(float qe,float x);  //20140526,郭大卫，计算被积函数
	float integrand2(float qe,float x,float L);  //20140526,郭大卫，计算被积函数
	float slopelengthadjusted(float L);  //20141225,郭大卫，坡长等效变换函数
};
