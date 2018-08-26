#ifndef DATASTRUCT_H
#define DATASTRUCT_H
#pragma	once
#include <string>
#include <stdio.h>
#include <stdlib.h>
#include <atlstr.h>
#include <iostream>
#include <fstream>
#include <windows.h>
#include <math.h>
#include <time.h>
#include <float.h>
#include <vector>//20060327,������,Ϊ�洢������ʴ��Ϣ��
using namespace std;

#import "c:\program files\common files\system\ado\msado25.tlb" rename("EOF","EndOfFile")

////wh,Ԥ����꣬������׽���ݿ�����еĴ���
//#ifndef CATCH_ERROR
//#define CATCH_ERROR											\
//		{													\
//			CString strComError;							\
//			strComError.Format("������: %08lx\n������Ϣ: %s\n����Դ: %s\n��������: %s", \
//								e.Error(),                  \
//								e.ErrorMessage(),           \
//								(LPCSTR) e.Source(),        \
//								(LPCSTR) e.Description());  \
//			::MessageBox(NULL,strComError,"����",MB_ICONEXCLAMATION);	\
//		}
//#endif

//������Ӳ�������
#define PI 3.1415926538

//20060109,������,������������������ʱ������Ǩ������
#define YearSTD    1950
#define MonthSTD   1
#define DaySTD     1
#define HourSTD    0

//������ʴ�Ľṹ��,20060327,������,Ϊ�洢������ʴ��Ϣ��
struct GravityEvent
{
	char LR;
	long TimeStep;
	float FD;
	float FR;
	float Probability;
	float Amount;
};

class List;
class ListNode {
	friend class List;
public:
	long BSLength;
	unsigned long long BSValue;
	long SOrder;
	int tmp;
	ListNode *link;		
};

class List {	                 	
public:
	List();
	int  insert(long length,unsigned long long value,long SOrder);
	int  insert(long length,unsigned long long value,long SOrder,long rank);
	int  remove(ListNode *);
	int  find(long,unsigned long long);
	ListNode *N_find(long,unsigned long long);
	ListNode *N_find(int);
public:
	ListNode *first;
	void free(void);
};


struct BSCode 
{
	long RegionGrade;
	unsigned long long RegionIndex;
	long Length;//ToLength
	unsigned long long Value;//ToValue
};

struct Para
{
	//�ӶεĹ̶������Ľṹ�壬����ʱ�������LAI,WaterEva
	//constanat p muskingum
	float WaveCoefficient; //����ϵ��
	float x;//̹��ϵ��//�����������ӣ�����Σ�������ģ����W = K[xI+(1-x)O]�е�x��
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
	float SlopeL;//����ֵ
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

//����һ���Ӷ�
struct TreeNode 
{
	TreeNode * Parent;
	TreeNode * Boy;
	TreeNode * Girl;
	BSCode mBSCode;
	Para mPara;

	//wh�����TaskCount�Ǵ˵����λ�û����������ڼ���Ӷ����������Լ���
	int TaskCount;

	//wh�����UnderCalc����0ʱ����ʾ�úӶε����δ��ںӶβ��ڵ�ǰTreeList�У����������1����Ϊ����������ķ��룬2����Ϊ��������RegionIndex�С�
	//������������Ǳ�ʾ�úӶ��ܷ��TreeList���и����ֻ�е���0���ԡ�
	int UnderCalc;//�Ƿ����ڱ�����

	int StralherOrder;
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


struct sStatus
{
	BSCode mBSCode;
	long HourOffset;
	float E;
	float P;
	float W;
	float QRG; 
	float QRSS;
};



//2009.4.7,WH,�߱��²���ģ��(HSPUSEPARA��)
typedef struct HSParameter
{
	//���ι����������߽�
	float theta1,theta2;
	
	//ȫ������ںӶε�ƽ̲�ӿ�(GH)������
	float B0,Q0;

	//�ļ�����������Ͷ�����С�����������Ϊ��ֵ��ʹ�ú���Ĭ�Ϲ�ʽ�����򰴸ù�ʽ����
	float Emax,Emin;
	
	//����������m��Բ������Ӵ��׵���ֱ���룩
	float sDeep;

	//�Ҳ����͸ϵ��,���ҵĸ�ˮ��(0,1)
	float Kr,mu;
	
	//hType:�߳�ֵ����(relative or absolute),sShape:������״(rec,tri,par)
	string hType,sShape;
	
	//dt1:����ʱ�䲽��,dt2:����ʱ�䲽��
	float dt1,dt2;
	
	//������ͷǱ�������������С�ռ䲽��(��Ϊ�仯)
	float dx,dz;

	float sRatio;//��Բ�͸ˮ���½Ŵ���ȱ�TG/T0G
	float wRatio;//ȷ����������ˮ�͹�������ˮ��ʼˮλ��NL/HL
	float rRatio;//ȷ����ʯ��϶ˮˮλ��H1T0/OT0,λ��T0T�·�����ֵΪ��(-oo,1)

	//hspdischarge�����Ƿ񱣴�B��H��V��ֵ��false�����棬true����
	int SaveFlowPattern;

} HSParameter;



#endif