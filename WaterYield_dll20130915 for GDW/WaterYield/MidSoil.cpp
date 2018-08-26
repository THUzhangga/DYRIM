
#include "stdafx.h"
#include ".\midsoil.h"
using namespace std;

MidSoil::MidSoil(void)
{
}

MidSoil::~MidSoil(void)
{
}

float MidSoil::NextHour(float WaterAdded,MParameters myPara,float DSita,float DeepW,float DeepWmax2)//ˮ����m^3
{
	
	float wslope;
	float toReturn;

	Wout=0;

	//byiwish20060222

	W=W+WaterAdded;
	
	//������Ӧ���ں�ˮ������֮ǰ����20060301
	if(W>Wmax1)//byiwish20060222
	{
		//20050408,���ӷ��Ӳ�����ԭ����3600������Ϊ�ɱ���
		//20060313,��ΪWǰ���Ѿ�������WaterAdded,��������Ĺ�ʽ�����-WaterAdded/2.0������+
		wslope=W-Wmax1-WaterAdded/2.0f;
		float tempf;
		tempf=3600.0f/60*MSTEP;
		Wout=tempf/2.0f*kh*wslope*wslope/(Area*RouteLength*RouteLength*n2*n2+tempf/2.0f*kh*wslope);

		W-=Wout;

		if(W<=Wmax1)
		{
			//cout<<"W<=Wmax1"<<endl;
			Wout-=Wmax1-W;
			W=Wmax1;
		}

		if(W>Wmax2)
		{
			//cout<<"W>Wmax2"<<"\t";
			Wout+=W-Wmax2;
			//cout<<"midW=\t"<<W<<"\t"<<"midWmax2=\t"<<Wmax2<<"\t";
			//cout<<"midout=\t"<<Wout<<endl;
			W=Wmax2;
		}	
	}
	else
	{
		Wout=0.0f;
	}

	
	
	float wtemp;
	float B;//һ��ϵ�������⹫ʽ�߳�
	Sita=W/Wmax2;
	B=2/(2*SlopeJ*RouteLength)*log(1+(2*SlopeJ*RouteLength)/myPara.DSDepth);//(2*SlopeJ*RouteLength)���в����ĺ��
	//������������ʱ������Ϊ0 20060419byiwish	
	if(Sita<1&&DSita<1)
		imd=1-B*(myPara.am*pow(Sita,(-myPara.bm))-myPara.ad*pow(DSita,(-myPara.bd)));
	else if(Sita>=1&&DSita<1)
		imd=1-B*(0-myPara.ad*pow(DSita,(-myPara.bd)));
	else if(Sita<1&&DSita>=1)
		imd=1-B*(myPara.am*pow(Sita,(-myPara.bm))-0);
	else
		imd=1;
	toReturn=kv*imd*Area/60*MSTEP;
	
	//ţ�ٵ�������һ������R=0
	toReturn=0.0f-(toReturn-0.0f)/(-kv*Area*B*(myPara.am*myPara.bm*pow((W-0.0f),(-myPara.bm-1))/pow(Wmax2,(-myPara.bm))+myPara.ad*myPara.bd*pow((DeepW+0.0f),(-myPara.bd-1))/pow(DeepWmax2,(-myPara.bd)))-1);

	if(toReturn>=0)
	{
		if(toReturn<W)//���������ˮ��С�ڱ�����ˮ�����򰴼��������������������ˮ��Ϊ1������ȫ������
		{

			W-=toReturn;
		}
		else
		{
			wtemp=W-1.0f;
			toReturn=wtemp;
			W=1.0f;
		}
	}
	else
	{
		wtemp=-toReturn;
		if(wtemp<DeepW)
		{
			wtemp=W+wtemp;
			if(wtemp<Wmax2)
			{
				W=wtemp;
			}
			else
			{
				toReturn=-(Wmax2-W);
				W=Wmax2;
			}
		}
		else
		{
			wtemp=DeepW-1.0f;
			if((wtemp+W)<Wmax2)
			{
				toReturn=-wtemp;
				W+=wtemp;
			}
			else
			{
				toReturn=-(Wmax2-W);
				W=Wmax2;
			}
		}
	}

	//cout<<"imd=\t"<<imd<<"\t"<<"toReturn=\t"<<toReturn<<"\t";


	//20060313,��������Ľ�����������
	float LoopToReturn=0;
	int LoopCount=0;
	while (abs(LoopToReturn-toReturn)>1.0f && LoopCount<20) 
	{
		LoopCount++;
		LoopToReturn=toReturn;
		DeepW+=LoopToReturn;
		DSita=DeepW/DeepWmax2;
		Sita=W/Wmax2;//������ˮ��
		if(Sita<1&&DSita<1)
			imd=1-B*(myPara.am*pow(Sita,(-myPara.bm))-myPara.ad*pow(DSita,(-myPara.bd)));
		else if(Sita>=1&&DSita<1)
			imd=1-B*(0-myPara.ad*pow(DSita,(-myPara.bd)));
		else if(Sita<1&&DSita>=1)
			imd=1-B*(myPara.am*pow(Sita,(-myPara.bm))-0);
		else
			imd=1;
		toReturn=kv*imd*Area/60*MSTEP;

		//������ţ�ٵ�����
		toReturn=LoopToReturn-(toReturn-LoopToReturn)/(-kv*Area*B*(myPara.am*myPara.bm*pow((W-LoopToReturn),(-myPara.bm-1))/pow(Wmax2,(-myPara.bm))+myPara.ad*myPara.bd*pow((DeepW+LoopToReturn),(-myPara.bd-1))/pow(DeepWmax2,(-myPara.bd)))-1);

		if(toReturn>=0)
		{
			if(toReturn<W)//���������ˮ��С�ڱ�����ˮ�����򰴼��������������������ˮ��Ϊ1������ȫ������
				W-=toReturn;
			else
			{
				wtemp=W-1.0f;
				toReturn=wtemp;
				W=1.0f;
			}
		}
		else
		{
			wtemp=-toReturn;
			if(wtemp<DeepW)
			{
				wtemp=W+wtemp;
				if(wtemp<Wmax2)
				{
					W=wtemp;
				}
				else
				{
					toReturn=-(Wmax2-W);
					W=Wmax2;
				}
			}
			else
			{
				wtemp=DeepW-1.0f;
				if((wtemp+W)<Wmax2)
				{
					toReturn=-wtemp;
					W+=wtemp;
				}
				else
				{
					toReturn=-(Wmax2-W);
					W=Wmax2;
				}
			}
		}
	}//end of while

	//return 0.0f;��Ҫ�ˣ�20060222
	//byiwish20060222
	return toReturn;
}
//bool MidSoil::NeedWater1(void)
//{	
//
//	if(W<Wmax1)
//	{
//		return true;
//	}
//	else
//	{
//		return false;
//	}
//}
//
//// �Ƿ���Ҫ����ˮ����
//bool MidSoil::NeedWater2(void)
//{
//	if( NeedWater1() ) return false;
//	
//	
//	if(W<Wmax2)
//	{
//		return true;
//	}
//	else
//	{
//		return false;
//	}
//}

// ���������ı���
float MidSoil::GetSRate(void) 
//20041018:SRate,�����ʣ����Խ���ͣ�����Խ�ߣ�����Ϊ1,�������������ˮ������Ϊ0
{
	//20051212 ������
	//�����ǰ��ˮ����������ˮ�����������ظ�ֵ
	//�Դ�ʹ��ˮ����������ˮ��������£��ر�����ϵ����������ˮ���µĲο���������

	//if(NeedWater1())
	//{
	//	return 0.0;
	//}
	//else
	//{
		if(abs(Wmax2-Wmax1)<1e-5)
			return 0.0;
		return (W-Wmax1)/(Wmax2-Wmax1);
	//}
}

int MidSoil::initialize(MParameters MyPara)
{
	Area=MyPara.Area;
	RouteLength=MyPara.RouteLength;

	kh=MyPara.PKH2;//�в�����ˮƽ��͸����
	n1=MyPara.MSita1;//�в�������ˮ����϶��
	n2=MyPara.MSita2;//�в������ɺ�ˮ���Ŀ�϶��
	SlopeJ=MyPara.SlopeJ;
	kv=MyPara.PKV2;//add by iwish 20060220

	if(SlopeJ<1.1e-5)//20110927,xiaofc,�����¶�̫С���в��������С������ʧ��
	{
		Wmax1=Area*MyPara.UDepth*n1;   //Ш�Σ�û��RouteLength*2
		Wmax2=Area*MyPara.UDepth*(n1+n2); //20041016�޸�
	}
	else
	{
		Wmax1=Area*RouteLength*SlopeJ*n1;   //Ш�Σ�û��RouteLength*2
		Wmax2=Area*RouteLength*SlopeJ*(n1+n2); //20041016�޸�
	}
	
	W=MyPara.MidSoilW;//����Ӧ�ô����ݿ����//lifangmi
	//W=Wmax1+(Wmax2-Wmax1)/100.0;  

	//byiwish20060222
	Sita=W/Wmax2;

	isDebug=false;

	//am=3520;
	//bm=4.21;

	//david,��ʼ��д������
    Wout=0.0f;

	return 1;
}
