#ifndef FVMUNIT_H
#define FVMUNIT_H
#pragma once
#include <vector>
#include <string>
#include <fstream>
using namespace std;

//ע���������еĵ�λ���ǹ��ʵ�λ��������㣬��λת�����̶��ڳ����ʼʱ���

//һ���Ǳ���������Ԫ
class RichardsUnit
{
public:
	RichardsUnit(float theta0,float K0,float D0,float dz0);
public:
	~RichardsUnit(void);

public:
	//��ǰʵ��������ˮ��
	float theta;
	
	//���ｫ����ת��Ϊmm����Ϊ�˼��ٸ����������ܻ����Ӽ����ٶ�
	//��͸ϵ��mm/s,��ɢ��mm2/s
	double K,D;

	//�ռ䲽��mm��Ϊ���������ȷֲ�Ԥ��
	float dz;
};


//����ˮ�˶������������Ԫ
class FVMunit
{
public:
	//��ʼ����Ͳ����ٱ�
	FVMunit(float x0,float hup0,float hdown0,float hm0,float ahup0,float dx10,float dx20,float beta10,float beta20,float Width0,int landtype0,string sType0,float Dis0,float VH);
public:
	~FVMunit(void);

public:
	//�漰�̵߳�ֵ��������T0GΪ��׼�����Ը߳�,ahup���⡣
	//hup,hdown���±߽��е����Ը߳�,ahup:���Ը̣߳�������������
	float hup,hdown,ahup;

	//dx1:�ϱ߽糤m;dx2:�±߽糤m;Width�����m
	float dx1,dx2,Width;

	//�ϱ߽�Ͳ�͸ˮ���¶�
	float beta1,beta2;

	//��Ӧˮƽ����Ԫ����ʯ��϶ˮ���
	float VH;

	//���޵�Ԫ�ĺ�����
	float x;

	//SlopeType:L,R,S
	string sType;
	
	//�����¶��ľ���m(�������淽��)
	float Dis;


public:
	//����ˮ��thetaf:��ͬ�������Ͳ��ϴ�ϸɰ4.5%���ң�ճ��20%����,���ڷǱ��ͺͱ�������ˮ�佻��
	//ë�ܶ��Ѻ�ˮ��thetab:thetab=(0.6~0.7)*thetaf;
	//ֲ����ή��ˮ��thetaw:���������ͺ�ֲ�����Ϳ��ƣ���ƪ��������)������Ҫ�������ƣ���������һ��5%��������
	//�������ຬˮ��thetac:���Richards�����õ�,���жȽϵͣ�ȡ��Сֵ0-0.08
	//���ͺ�ˮ��thetas:0.4-0.6
	static float thetaf,thetas,thetab,thetaw,thetac;
	
	//N��������豹�ʽ���߹⻬�ĳ̶�һ����10��30֮�䣬E0_aһ����Ժ��Բ���������ȡ��С��ֵ������0.1
	static float N,E0_a;
	
	//Richards���̵Ĳ���,usD:(1-20)cm2/min,������ת���ɹ��ʵ�λ����;usL:(1-10)��ɢϵ����usfΪ����K�Ĵ���˥��ϵ���������Ϊ��˥��Ϊ0
	static float usL,usf;
	static double usD;
	
	//�Ǳ�����������ռ䲽��m
	static float dz;
	
	static short StartYear,StartMonth,StartDay,Year,Month,Day;

	//dt1,dt2�������µ�ʱ�䲽��(s),TimeStartִ�е������Ѿ�����������
	static float dt1,dt2,StartHour,Hour;

	//�����Ѿ�ģ���ʱ��(s)
	static float TimeStart;

	//�ļ�����������Ͷ�����С������m
	static float Emax,Emin;

	//�߳����еĲ�����
	//dcֲ��������0-1��fc���汥����͸ϵ��m/d(fc��һЩ����ֵ)������ת���ɹ��ʵ�λ,thetai:��ʼ�Ǳ���������ˮ��
	//Betaf:������������˥��ϵ��(Inx/thetas),xΪ��������������ȶ�����֮�ȡ�
	//Betaf����ȡֵ: ����:3.69,�ݵ�:3.5,��ľ��:7.95,��Ҷ��:0.80,��Ҷ��:1.98,����:2.23
	static float dc[5],Betaf[5],thetai[5];
	static double fc[5];

	//��ֲͬ�����͵�Ҷ�������ָ��
	const static float LAI[4][12],maxLAI[4];

	//��������Ӱ���������m
	const static float fz[5];

	//�����ļ����ָ��
	static ofstream* myFile;


public:
	//�����ϵ�ֲ���������ͣ�ѩ0����1����2����3����4
	int landtype;	

	//����ʵ��ˮ��m������ʵ�ʵ�������m2/s
	float ss,qs;

	//hv:�ع���m/s,hm:��������ˮˮλm,ksa:����������ƽ����͸ϵ��m/s,mu:��ˮ��(0,1)
	float hv,hm,mu;
	double ksa;

	//����TimeStartʱ��ʾ�лع�������
	float TimeEnd;
	
	//�Ǳ�������������ڵ�
	std::vector<RichardsUnit> ri;

private:
	//dt1ֲ��ʵ����ˮ��m,dt1ʣ����������m
	float wPlant,EPI;

	//�Ǳ��������˶����ϱ߽�ͨ��,FED:m,TD:s
	float TD,FED;

	//�Ǳ��������˶������±߽�
	float thetaA,thetaB;
	double KA,DA,KB,DB;

	
public:
	//���´���Ǳ�����������Ԫ��Ŀ
	void UpdateRichardsUnit(void);

    //�������
	void SlopeSourceTerm(float p,float dt);

	//�Ǳ�������ˮ�˶�
	void UnsaturatedSoil(float dt);

	
private:
    //��ʼ������Ǳ�����������Ԫ��Ŀ
	void InitialRichardsUnit(void);

	//����Ǳ������������ʱ�䲽������ֹ���������ˮ���򳬱��ͺ�ˮ��
	//float GetTimeStep(float LeftTime,float ufe,int N);

	//ֲ������:p:����,houroffset:�Ѿ�����Сʱ��,���ش���������
	float Plant(float p,float dt);

	float StepEPI(float dt,int H,int H0);

};

#endif
