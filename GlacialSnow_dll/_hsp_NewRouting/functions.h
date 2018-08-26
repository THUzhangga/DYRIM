#ifndef FUNCTIONS_H
#define FUNCTIONS_H
#pragma once
#include "DataStruct.h"
#include "TreeList.h"
#include <iostream>
#include <stdio.h>
#include <atlstr.h>
#include <math.h>
using namespace std;
//wh,ֻ�ܷ����������嶨��ŵ�cpp�У������ͷ�ļ�ֻ�ܱ�����һ��


TreeList GetBranchInForest(TreeList* TotalList, int tmpUnitSize, int MinUnitSize, int GradeTwoCount);


bool IsEmptyForest(TreeList* TotalList, int GradeTwoCount);

//������Ԫ����0
template<class T>
void ZeroFill(T* Qs, long DataCount)
{
	for(long i=0;i<DataCount;i++)
		Qs[i]=0.0f;
}

//����ֵ�Ե�
void SwapQp(float** Q1,float** Q2);

//��˹��������ģ��
void Transform(float* QIn1, float* QIn2, float* QIn3, float* QOut,int TimeInterval,long DataCount,Para * mPara);

long GetNumOfHour(short YearS,short MonthS,short DayS,short HourS,short YearE,short MonthE,short DayE,short HourE);

short GetMonthDays(short year,short month);

void MonthAdd(short CurYear, short CurMonth, short &NextYear, short &NextMonth);

void HourToDate(float Hours,short YearStart,short MonthStart,short DayStart,float HourStart,short* pYear,short* pMonth,short* pDay,float* pHour);


//׷�Ϸ�������Խ����Է�����
template<class T>
void TDMA(int N,T *a,T *b,T *c,T *f,T *x)
{
	//N:���������,a:���¶Խ���N-1,b:�Խ���N,c:���϶Խ���N-1,f:�������Ҷ���N,x:������N
	for(int i=0;i<N-1;i++)
	{
		c[i]=c[i]/b[i];
		b[i+1]=b[i+1]-a[i]*c[i];
		if(i==0) 
		{
			f[i]=f[i]/b[i];  
		}
		else 
		{
			f[i]=(f[i]-a[i-1]*f[i-1])/b[i];
		}
	}

	x[N-1]=(f[N-1]-a[N-2]*f[N-2])/b[N-1];

	for(int i=N-2;i>=0;i--)
	{
		x[i]=f[i]-c[i]*x[i+1];
	}
}


//����2����
template<class T>
float VecDistance2(int N,T *a)
{
	float Dis=0;
	for(int i=0;i<N;i++)
	{
		Dis+=a[i]*a[i];
	}
	return sqrt(Dis);
}

//����oo����
template<class T>
float VecDistance8(int N,T *a)
{
	float Dis=0;
	for(int i=0;i<N;i++)
	{
		if(abs(a[i])>Dis)
		{
			Dis=abs(a[i]);
		}
	}
	return Dis;
}



#endif
