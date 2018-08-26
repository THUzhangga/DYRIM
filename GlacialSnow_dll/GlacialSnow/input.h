//input.h��input.cppΪ�߽����ѩ�ࡣ
#pragma once
#include "CoorTrans.h"
#include <Shlwapi.h>
#include <fstream>
#include <iostream>
#include <vector>

using namespace std;

//�úӶΣ�ĳ��ĳ��ĳ��ĳʱ����Ϣ��
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

//�úӶΣ�ĳ��ĳ��ĳ�����ߡ�������º�sca
struct dayvalue
{
	int Year;
	int Mon;
	int Day;
	float Tmax;
	float Tmin;
	
	float SCA;
};

//�úӶΣ�ĳ��ĳ�µ�������º��������
struct monvalue
{
	int Year;
	int Mon;
	float Tmax;
	float Tmin;
};

//ĳ��ĳ��ĳ����վ��������º��������
struct mon_gaugevalue
{
	monvalue monzhi;
	long id;
};


//�úӶ�(Ԫ���򣩣�ĳ��ĳ�µ�����sca����ƽ���߳̾�����
struct SCAcertain
{
	int Mon;
	int Day;
	float SCA;
};


//һ���ṹ����¶�Ӧgauge���е�һ����¼
struct QixiangZhan
{
	jwCoord jwLocation;  //����վ��λ��
	xyCoord xyLocation;
	float weight;   //Ȩ��
	long lCode;     //����վ�Ĵ���
};


struct PointInfo
{
	float dis;
	//float xiShu;
};


typedef vector<monvalue *> TSeriesType;//�Ӷε���Ϣ

typedef vector<mon_gaugevalue *>TGSeriesType;//����վ����Ϣ��

class input
{

//��ȫ�ֽӿڵ��ú��������м����ʼ��ֻ����һ��)��
public:
	void ReadElevationAndA(void);//��initialzie()����
	void initialzie();  //��ȫ�ֽӿ�OpenOracle()����
	void finalize();    //��ȫ�ֽӿ�SRMFinalize()����



//���ֲ��ӿڵ��ú�����
public:

	//���������������ֲ��ӿ�SnowInitialize()���ã�ÿ��һ���Ӷ�ִ��һ�Ρ�
	void GetDataByTS(void);
	void ReadDayValue(void);//�µ�չƽ����
	void ReadHourValue(void);//�յ�չƽ��Сʱ
	
	//����һ���������ֲ��ӿ�SnowCalc()���ã�ÿ��ʱ�䲽��ִ��һ��
	void ToWaterYield(long houroffset,float* HourRainIn);//added by wanghao
	
	//����һ���������ֲ��ӿ�ReleaseHeap()���ã�ÿ��һ���Ӷ�ִ��һ�Ρ�
	void Deleteall(void);


//����վ��ֵ��ص��м亯��
public:
	bool GetDataByDW(void);
	bool CalcuWt(jwCoord jw);
	TGSeriesType GetGageTS(long ID);
	

//ʱ��ת������
private:
	void houroffset2date(long houroffset);
	long date2houroffset(int Year,int Month,int Day,int Hour);
	int GetMonthDays(int Year, int Month);
	void MonthAdd(int CurYear, int CurMonth, int &NextYear, int &NextMonth);
	void GetNumofDay(void);
	
	

//���ݿ����
public:
	ADODB::_ConnectionPtr pConn;


//�������
private:
	vector< QixiangZhan* >  m_TargetPoints;//����վλ����Ϣ
	vector< TGSeriesType >  mySeriesArray;	
	
	hourvalue* hourhappen;
	dayvalue* dayhappen;
	TSeriesType monhappen;//�����������úӶΣ�Ԫ���򣩣�Ԫ��Ԫ��֮�����������ʱ�䣨�·ݣ���ͬ����ȻԪ�ؼ���¶�Ҳ�Ͳ�ͬ��


//��������
private:
	SCAcertain* needsca;
	long Rcount;//needsca��Ԫ�ظ�����ȫ��ֵ�����仯��
	int MonthEnd_1;
	int numofmonhappen;//int numofmonhappen = sizeof(monhappen);monhappen�ĸ���
	

	
//�Ӷα������ɲ���ģ�ʹ���
public:
	float jx;
	float wy;
	unsigned long long RegionIndex;
	long Length;
	unsigned long long Value;
	float A;
	float Havg;
	double m_CenterX;
	double m_CenterY;


//ʱ�������������ֹʱ�䣬���仯��
public:
	int YearStart;
	int MonthStart;
	int DayStart;
	int YearEnd;
	int MonthEnd;
	int DayEnd;
	long numofday;//��������
	long numofhour;
	long houroffsetstart;


};

//�úӶΣ�Ԫ���򣩣�ĳ��ĳ�µĸ���sca�����ڲ�ͬ�̣߳�
//struct SCA
//{
//	int Mon;
//	int Day;
//	float SCA1;
//	float SCA2;
//	float SCA3;
//	float SCA4;
//};

//typedef vector<float> PSeriesType;//�㲢û��ʹ��

//bool DeleteList(void);
//int numofsca = sizeof(needsca);
//SCA* allsca;
//SCA* ReadSCA(void);
//void ReadSCA(void);//modifed by wanghao
//ADODB::_Connection* OpenMDB(CString m_File);
//ADODB::_ConnectionPtr pCnn;
//ADODB::_Connection* OpenOracle(CString m_user, CString m_password, CString m_sid);