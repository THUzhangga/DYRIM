
#pragma	once

#ifndef DATASTRUCT_H
#define DATASTRUCT_H

#include <stdio.h>
#include <stdlib.h>
#include <atlstr.h>
#include <iostream>
#include <windows.h>
#include <math.h>
#include <time.h>
#include <float.h>
#include <vector>//20060327,李铁键,为存储重力侵蚀信息用

//2009.9.10:wh，如果不加上下面的stdafx.h的引用，在release模式下会出现很多无法识别ADODB的编译错误
#include "stdafx.h"

//#import "c:\program files\common files\system\ado\msado25.tlb" rename("EOF","EndOfFile")//如果rename前面写上nonamespaces，程序会报无数错误，估计肯定和哪里冲突了。


//20060109,李铁键,把两个雨量类里的这个时间起点宏迁到这里
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

	//David 坡面产沙模型参数
	float ErosionK1;
	float ErosionK2;
	float ErosionBeta1;
	float ErosionBeta2;


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

struct sStatus
{
	BSCode StatusBSCode;
	long HourOffset;
	float WLL;
	float WLU;
	float WLM;
	float WLD;
	float WRL;
	float WRU;
	float WRM;
	float WRD;
	float WSL;
	float WSU;
	float WSM;
	float WSD;
	float E;//20070116,xiaofc
	float P;//20070202,xiaofc,记录每个单元的日降雨量,量纲: mm

	float SlopeErosion; ////20080303, xiaofc, 每个单元的坡面侵蚀量

};


//2008,wh,for srm
struct hourvalue
{
	long houroffset;
	int Year;
	int Mon;
	int Day;
	int Hour;
	float SCA;
	float Tmin;
	float Tmax;
	float Thour;
	float P;
	float A;
};


#endif