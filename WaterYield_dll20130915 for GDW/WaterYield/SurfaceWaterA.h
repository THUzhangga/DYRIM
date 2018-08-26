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
	float PenetrateK;//ÿСʱ��������ˮ��
	float SediD;

	//wh�������������������ˮ���õ�����ˮ��Ϊ�˽�����ˮ�������������͸�ʵ����⣨������������
	float isu;//���污��ˮ�ͱ�����������ݶ�20060302
	float Kr;//�Ǳ�����͸ϵ���뱥����͸ϵͳ�ı�ֵ
	float lambda;//����Kr�õ���ָ��//���ݷ���ά������Ĭ��ֵ

	//20051219 Ѧ�� ���Ӳ�ɳ�е�ϵ��k
	float ErosionK;
	float ErosionBeta;

	//20140526,�����������ڱ���ˮ����ʴ����Юɳ����ϵ��
	float ErosionK1;
	float ErosionK2;
	float ErosionBeta1;
	float ErosionBeta2;


public:
	
	//status
	//double WaterDepth;
	float WaterY;//������, m^3
	float WaterPenetrated; //m^3//wh������ӵر����뵽��������е�ˮ��
	bool isDebug;
	CString SEEquation;//��ɳ��ʽ

	int MSTEP;

public:
	SurfaceWaterA(void);
	SurfaceWaterA(int timestep,CString seequation);
	~SurfaceWaterA(void);
	float NextHour(float PE,MParameters myPara,float USita,float SoilW,float SoilWmax2);//20060302�޸�byiwish

	//��ɳadd by xue
	//float Slope_J,Slope_Rough,Sedi_D;
	float SedimentYield;
	void SediYield(void);
	void RevisedSediYieldXue(void);//20070823,������������Ѧ����ʽ

	//20141226,���£�1����Ϊ�����ʺ�Юɳ������ˮ��������أ�2�ǽ��³�ת��Ϊ40.4�¶�ʱ�ĵ�Ч�³���3�������ֱ�ӳ��Ա�������
	void RevisedSediYieldLi1(void); //20140526,������������Юɳ���Ķ�ˮ����ʴ������
	void RevisedSediYieldLi2(void); //20140526,������������Юɳ���Ķ�ˮ����ʴ������
	void RevisedSediYieldLi3(void); //20140526,������������Юɳ���Ķ�ˮ����ʴ������


//private:
//	double RunOff(double Depth);  //�����ǵر�������ˣ�����ֱ�ӵ����������

	int initialize(MParameters MyPara);
private:
	float integrand1(float qe,float x);  //20140526,�����������㱻������
	float integrand2(float qe,float x,float L);  //20140526,�����������㱻������
	float slopelengthadjusted(float L);  //20141225,���������³���Ч�任����
};
