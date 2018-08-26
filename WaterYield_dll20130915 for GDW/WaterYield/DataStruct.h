
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
#include <vector>//20060327,������,Ϊ�洢������ʴ��Ϣ��

//2009.9.10:wh����������������stdafx.h�����ã���releaseģʽ�»���ֺܶ��޷�ʶ��ADODB�ı������
#include "stdafx.h"

//#import "c:\program files\common files\system\ado\msado25.tlb" rename("EOF","EndOfFile")//���renameǰ��д��nonamespaces������ᱨ�������󣬹��ƿ϶��������ͻ�ˡ�


//20060109,������,������������������ʱ������Ǩ������
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

	//David �����ɳģ�Ͳ���
	float ErosionK1;
	float ErosionK2;
	float ErosionBeta1;
	float ErosionBeta2;


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
	float P;//20070202,xiaofc,��¼ÿ����Ԫ���ս�����,����: mm

	float SlopeErosion; ////20080303, xiaofc, ÿ����Ԫ��������ʴ��

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