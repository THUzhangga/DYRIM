
#include "stdafx.h"
#include ".\vapotranspiration.h"

Evapotranspiration::Evapotranspiration(void)
{
}

Evapotranspiration::~Evapotranspiration(void)
{
}

//20070301,xiaofc,�����������Ϊ���ռ���������Сʱ��ķ��䰴��������
bool Evapotranspiration::initialize(MParameters* myPara/*, float HourOffset, float P*/)
{

	AvgEPI=myPara->EPI;//��ÿСʱ��������������tvarparameter�е�watereva����õ�

	//���ڸ���豹�ʽ
	thetaf=myPara->USita1;//����ˮ��
	thetas=thetaf+myPara->USita2;//���ͺ�ˮ��//wh���������ĸ���������ˮ�����ǰٷֱȣ������ٵģ����ͺ�ˮ��=����ˮ��+����ˮ��ˮ�������������ʵ�����ʣ�

	//altered by wh

	if(thetab<1e-5) thetab=1e-5;
	if(thetab>thetaf) thetab=thetaf-1e-5;//ë�ܶ���

	if(thetaw<1e-5) thetaw=1e-5;
	if(thetaw>thetab) thetaw=thetab-1e-5;//��ή

	if(N<1) N=1;
	if(E0_a<1e-5) E0_a=1e-5;

	return 1;
}

//wh����������Ǵ������Ҷ���Լ�������������������Ը��������������ˮ������ʵʱ������ֻ��һ������ģ��
//�����nexthour��ʵ�ǲ���
bool Evapotranspiration::NextHour(Leaf * cLeaf,SoilWater * cSoil,MidSoil * cMid, DeepSoil * cDeep,float HourOffset, float P)
{
	//1.��ƽ��Ǳ������ת��Ϊ��Ӧʱ�䲽���ϵ����ҷֲ����ֵ
	float HourInDay;
	float EPI;//Сʱ���������������������߷����
	HourInDay=long(floor(HourOffset))%24+HourOffset-floor(HourOffset);
	if(HourInDay<6 || HourInDay>=18 || P>0)
		EPI=0.0f;
	else
		EPI=AvgEPI*sin((HourInDay-6)*PI/12)*PI;//wh�� ��������ƽ�������ԭ�򣬻��ֺ���12��Сʱ�ڵ���������Ϊ24*AvgEPI��ƽ��ÿСʱΪ2*AvgEPI��ά�������������ܱ仯
	
	float temp;
	temp=EPI/60*MSTEP;//addbyiwish20060304

	//2.���¿�ʼ����
	//20070116,xiaofc,
	float e;//ÿһ�������������
	E=0.0f;

	if(cLeaf->W>temp)//if(cLeaf->W>EPI)//Ҷ���ˮ������������ iwish�޸�ΪҶ���ˮ����ÿ����������������20060304
	{
		//20070116,xiaofc
		e=EPI/60*MSTEP;//m/ʱ�䲽��
		E+=e;

		cLeaf->W-=e;
	}
	else//Ҷ���ˮ��������������Soil����
	{
		//20070116,xiaofc
		E+=cLeaf->W;

		ELeft=EPI/60*MSTEP-cLeaf->W;//ʣ����������
		cLeaf->W=0.0;
		

		if(ELeft>0)
		{
			//20070702,xiaofc,�����������㷽����ѡ��
			//CString EMethod;
			if(EMethod=="Exponential")
			{
				//20070204,xiaofc,���ӷ�ɺ�ˮ�ʵĸ�������������µı��ͳ̶�Ϊ����ˮ���µ�5%
				e=ELeft*pow((cSoil->W/cSoil->Wmax2-0.05f)/0.95f,2.3f);
			}
			else if(EMethod=="FuBP")//20070702,xiaofc,����豹�ʽ
			{
				//20060704,xiaofc:
				//Soil.Sita��ָ��ǰ��ˮ��ռ���ͺ�ˮ���ı�����Ҫ�����ܿ�϶�ʲ���m3/m3�ĺ�ˮ��
				theta=cSoil->Sita*thetas;

				e=ELeft*theta/(thetaf-thetaw)*(thetaf/pow(pow(thetaf,N)+pow(theta,N)+pow(E0_a,N),1.0f/N)-thetaw/pow(pow(thetab,N)+pow(theta,N)+pow(E0_a,N),1.0f/N) );
				//cout<<"ELeft="<<ELeft<<"\ttheta="<<theta<<"\thetaf="<<thetaf<<"\tthetaw="<<thetaw<<"\tthetab="<<thetab<<"\t";
				//cout<<"e="<<e<<endl;
			}
			else
			{
				cout<<"Unknown type of Evaporation caculation!"<<endl;
				exit(0);
			}

			if(e<0.0f) e=0;
			
			temp=cSoil->W-e*cSoil->Area;
			
			//temp=cSoil->W-ELeft*cSoil->Area*cSoil->W/cSoil->Wmax1;
			if(temp>0)
			{
				cSoil->W=temp;

				//20070116,xiaofc
				E+=e;
			}
			else
			{
				cSoil->W=0.0;

				//20070116,xiaofc
				E+=cSoil->W/cSoil->Area;//wh������ж����������ˣ��������в��������ˣ��൱��һ������ģ�͡�
			}
		}
	
	
	}
	
	//����ֲ���ڷ�
	//..........

	//����������ˮ��
	//�˹������ɵ�����Ϊ������ˮ�����ۼ��㣬�����ڸ���������

	//20070116,xiaofc,������ɺ�����µĺ�ˮ��
	cSoil->Sita=cSoil->W/cSoil->Wmax2;//��ʱ������ˮ��20060304iwish
	//���������E��Ϊmm
	E*=1000.0f;

	return 1;
}