#pragma once
#include <fstream>
#include "DataStruct.h"
using namespace std;
class AVPM
{
//public:
	//AVPM();
//public:
//	~AVPM(void);

private:
	//����
	static const float g;
	static const float gamma;
	static const float nu;
	// ������洢��ȫϵ����0.29��1.5�����ַ��0.05,0.1,0.15,0.2,0.25,0.3����ģ�����ʻ���ֵ
	static const float fuzzpro[122][7]; 
	
	ofstream myFile;
	
	long DayLoop;

public:
	AVPM(void);
	AVPM(CString ssmethod,int mstep,CString sccd);//wh
	int MSTEP;//wh
	CString sccd;//wh

public:
	//����
	float nano;//��Сֵ����������J
	long HourStart;//20080303,xiaofc,���㿪ʼʱ��
	long iTSteps;//20070611,xiaofc,��int��ʾ��Χ��С����Ϊlong
	long lTotalT;
	int delta_t;
	long t;//ʱ���±�

	ADODB::_ConnectionPtr pCnn;//20080303,xiaofc,���ݿ�����
	BSCode mBSCode; //�Ӷα��
	float delta_x;
	bool isDebug;
	bool bCalcGravity;//�Ƿ����������ʴ�ı�־

	//ˮ��
	float *Q;
	float *Qout;//�ɵ��÷�������ڴ�
	float h[4];
	float u[4];	
	float B[4];
	float m;//B=2mh,A=mh^2
	float eta;//ph/px������ˮ�ڵ���Чϵ��0.8,��ˮ��Ϊ1-eta=0.2
	
	//20070828,xiaofc,����С�����±����
	float n;//������manningϵ��,����Ƕ�̬�ģ��仯֮���
	float n0;//���ǴӲ�������ȡ����
	
	float L;
	float S;

	//��ɳ
	bool bIsDep;//���ٻ�
	float *mySin;
	float *SinL;//��
	float *SinR;//�ҵĺ�ɳ��
	float *SinMe;//���ѺӶεĲ�ɳ��
	float *Sout;
	float gammaS;//2650kg/m3
	float D50;//�о���m
	float alphaE;//��ˢʱ�Ļָ�����ϵ�� 
	float alphaD;//�ٻ�ʱ��
	float Svm;//��������Ⱥ�ɳ����ֻ��D50���

	float D90; //�ٷֱ�ռ 90�� �Ŀ���������m  heli
	CString SSMethod;//����Юɳ���ķ�����"Zhang","Fei"

	float * Qin1;
	float * Qin2;
	float * Qin3;
	
	Para mPara;

	float *WL;
	float *WR;

	//20070622,xiaofc, flow pattern
	bool SaveFlowPattern;
	float * FlowB;
	float * FlowH;
	float * Flow_v;
	
	//20080303,xiaofc, save status for avpm
	float * ChannelErosion;
	float * GravityErosion;

public: //������Ѧ����ӵ�������ʴ�����õ��ı���	

	//Ѧ ������������ʴ��Ҫ�õ��Ĳ��ֲ���
	float P2;//20060317,������,һ���Ӷε���������ڣ�����������ʴ������Χ�ĸ���
	vector <GravityEvent> * pGravityEvents;		//20060327,������,Ϊ�洢������ʴ��Ϣ��

	float Tao;   //����ˮ����Ӧ��
	float Tao_c; //�½����屻��ˢ��������Ӧ�������ƴ汾��ʽ
	float Delt_B; // �½����ڳ�ˢ��չ��ĺ���
	float Angle_beta; //���µĳ���ʧ�ȵĽǶ�
	float Angle_i;    //����dȻ�½ǣ����±ߵ�Angle_downslope���
	float Angle_down,Angle_downL,Angle_downR; 
	float Height1,Height1L,Height1R;    //�ٿ������ϵĸ߶�
	float Height_t,Height_tL,Height_tR;   //��϶���
	float Length_down,Length_downL,Length_downR;
	float T;         //��϶��ˮƽˮѹ��
	float Gama_wm;    //ĳ����ˮ���µ���������
	float Gama_d;     //���������
	float Wt;         //���»����������
	float FR,FD;      //���¿��������»���
	float Cohesive_origin,Cohesive_addi; //  ԭʼճ����������ʱ�ģ��͸���ճ����
	float Angle_Fai;     //������Ħ����
	float W_cont; //���庬ˮ��%
	float a,b;           //����ճ�����뺬ˮ����ϵ�е�ϵ����ָ��
	float F1,F2,Fs;      //ģ�����������а�ȫϵ�������½缴�䱾��
	float H_Average;         //���ڰ�С�ɳ������ĸ�ˮ��ƽ������Ϊ�ҵ�����
	float Hc;      //С�ɳ�Ĺ�����ĳ���߶�
	float k;        //��϶���¸ߵı�
	float Probability;  //�ȶ�����
	float SlideAmount;  // ������
	//************* ������Щ����ҪС�ɳ�ӿ���ȡ������
	float mL,mR;  //���¶��µı���ֵ
	float Height,HeightL,HeightR;     //���¸߶�
	float Angle_slope; //�Һ����������ǣ�����ֵ��
	float Slope_A,Slope_L,Slope_AL,Slope_LL,Slope_AR,Slope_LR;    //���������볤��
	float Length_all;// �����ϱ� ,����ǼҺ�������泤��
	float Stream_J;      //�������¶�
	float mytemp;

	//�����Ǻ���

	void InitGravityPara();
	void ConvertTao(float H_avg);   //��С�ɳ�Ŀ�ǳ����ת�����϶��Ĺ���
	void InitGullyShap(Para mPara);       //���Һ�����ļ�����ת��Ϊһ�����º�һ�����£�ȷ���������µ��¶ȣ��³��ȡ�
	void CalFs(int LR);     //���㹵���ȶ��ԣ��õ���ȫϵ��
	void CalFuzzProbi(float Cigema);  //�����Ӧ��ĳһ��Fs��ģ�����ʡ���һ������Ϊ��ȫϵ�����ڶ���ϵ��Ϊ����
	inline float GenRand(float min,float max);  //����һ��[min,max]�������
	void HowMuchFailed(int LR);                //���ݼ������ʧ�ȸ��ʼ��㻬�µ�����

	//Ѧ***


public:
	void Calc(void);//�����㺯��
	float GetSS(float Sv,float h,float U,float *omega);//����Юɳ��,���һ�������������ظ�����
	void SediCalc(void); //��ƽ����ɳ����,����Calc��ʱ��ѭ�������
	void initialize(Para mPara);
	void finalize(void);

	float GetSSFei(float Sv,float h,float U,float *omega);//����Юɳ��,���һ�������������ظ����� //heli

	// ���浽status�������
	void SaveStatus(void);
};
