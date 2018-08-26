
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

//2008.11.5,WH,�°���ģ��
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


//�ӶεĹ̶������Ľṹ�壬����ʱ�������LAI,WaterEva
struct Para
{
	//constanat p muskingum
	float WaveCoefficient; //����ϵ��
	float x;//̹��ϵ��
	float WetRadius; //ʪ��

	//Middle X
	float X;
	//Middle Y
	float Y;

	//����
	float StreamSlope; //�¶�
	float StreamLength; //��
	float Manning; //n
	float RiverManning;//20051218 ������ �����Ĳ���
	float m;//����ϵ��,���ȡ4

	//20080306,xiaofc,���ӹ����¶��ֶΣ�ר�����ڼ���������ʴ
	float GullySlope; //�Ƕ�
	float DrainingArea;//��ˮ���

	//��������
	float D50;
	int LandUse;
	int SoilType;
	
	//��Ҫͨ������ת����
	float Sita1,Sita2,MSita1,MSita2,DSita1,DSita2;//��϶��
	float UDepth,DDepth;
	float PKH1,PKH2,PKV0,PKV1,PKV2;//��͸��
	float I0;
	float ErosionK;
	float ErosionBeta;

	//��
	float SlopeL;
	float LengthL;
	float AreaL;

	//��
	float SlopeR;
	float LengthR;
	float AreaR;

	//Դ
	float SlopeS;
	float LengthS;
	float AreaS;

	float A;
	float UElevation;
	float DElevation;
};

//��Ӧxajstatus��
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