
#ifndef DATASTRUCT_H
#define DATASTRUCT_H

#pragma	once
#include <stdio.h>
#include <stdlib.h>
#include <atlstr.h>
#include <iostream>
#include <windows.h>
#include <math.h>
#include <time.h>
#include <float.h>

#import "c:\program files\common files\system\ado\msado25.tlb" rename("EOF","EndOfFile")

#define YearSTD    1950
#define MonthSTD   1
#define DaySTD     1
#define HourSTD    0

#define PI 3.1415926538

struct BSCode 
{
	long RegionGrade;
	unsigned long long RegionIndex;
	long Length;
	unsigned long long Value;
};

//2008.11.5,WH,新安江模型
typedef struct XAJParameter
{
	int WUM;
	int WLM;
	int WDM;
	float C;
	float B;
	float IMP;
	int SM;
	float EX;
	float KG;
	float KSS;
	float KKG;
	float KKSS;

	int WU0;
	int WL0;
	int WD0;
	int S0;
	int QRS0;
	int QRSS0;
	int QRG0;

}XAJParameter;


//河段的固定参数的结构体，不含时变参数，LAI,WaterEva
struct Para
{
	//constanat p muskingum
	float WaveCoefficient; //波速系数
	float x;//坦化系数
	float WetRadius; //湿周

	//Middle X
	float X;
	//Middle Y
	float Y;

	//沟道
	float StreamSlope; //坡度
	float StreamLength; //长
	float Manning; //n
	float RiverManning;//20051218 李铁键 沟道的糙率
	float m;//边坡系数,最初取4

	//20080306,xiaofc,增加沟坡坡度字段，专门用于计算重力侵蚀
	float GullySlope; //角度
	float DrainingArea;//汇水面积

	//土壤土质
	float D50;
	int LandUse;
	int SoilType;
	
	//需要通过矩阵转换的
	float Sita1,Sita2,MSita1,MSita2,DSita1,DSita2;//孔隙率
	float UDepth,DDepth;
	float PKH1,PKH2,PKV0,PKV1,PKV2;//渗透力
	float I0;
	float ErosionK;
	float ErosionBeta;

	//左
	float SlopeL;
	float LengthL;
	float AreaL;

	//右
	float SlopeR;
	float LengthR;
	float AreaR;

	//源
	float SlopeS;
	float LengthS;
	float AreaS;

	float A;
	float UElevation;
	float DElevation;
};

//对应xajstatus表
struct sStatus
{
	BSCode StatusBSCode;
	long HourOffset;
	float E;
	float P;
	float W;
	float QRG; 
	float QRSS;
};

#endif