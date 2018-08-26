
#include ".\soilwater.h"
using namespace std;

SoilWater::SoilWater(void)
{	
}

SoilWater::~SoilWater(void)
{
}

float SoilWater::NextHour(float WaterAdded,MParameters myPara,float MSita,float MidW,float MidWmax2)//float SoilWater::NextHour(float WaterAdded,float SRate)//ˮ����m^3,SRate����һ�����ı�����
{
	float toReturn;

	Wout=0;

	//��������byiwish20060220
	W=W+WaterAdded;
	
	//������Ӧ���ں�ˮ������֮ǰ����20060301
	float tempf;
	tempf=3600.0f/60*MSTEP;
	//cout<<"======================k0="<<k0<<"============================"<<endl;
	//ˮƽ��������
	if(W>Wmax1)
	{	
		Wout=SlopeJ*k0*(W-Wmax1)*tempf/2.0f/(tempf/4.0f*SlopeJ*k0+n2*RouteLength);//K0��pkh1

		W-=Wout;

		if(W<Wmax1)//wh�����������ʣ�����СҪΪWmax1����Ϊ��Ϊֻ�г�������ˮ���Ĳų�������if��������
		{
			Wout=Wout-(Wmax1-W);
			W=Wmax1;
		}
		if(W>Wmax2)
		{
			Wout=Wout+(W-Wmax2);
			W=Wmax2;
		}
	}
	else
	{
		Wout=0.0f;
	}

	//��������20060301
	float B;//һ��ϵ�������⹫ʽ�߳�
	Sita=W/Wmax2;
	B=2.0f/(2.0f*SlopeJ*RouteLength)*log(1.0f+(2.0f*SlopeJ*RouteLength)/Depth);//(2*SlopeJ*RouteLength)���в����ĺ��hm��SlopeJ��RouteLength�����в�����

	//������������ʱ������Ϊ0 20060419byiwish
	//wh��iumΪ���е�ƽ����ˮ���ݶ�
	if(Sita<1&&MSita<1)
		ium=1.0f-B*(myPara.au*pow(Sita,(-myPara.bu))-myPara.am*pow(MSita,(-myPara.bm)));//(2*SlopeJ*RouteLength)���в����ĺ�ȣ�SlopeJ��RouteLength�����в�����
	else if(Sita>=1&&MSita<1)
		ium=1.0f-B*(0-myPara.am*pow(MSita,(-myPara.bm)));
	else if(Sita<1&&MSita>=1)
		ium=1.0f-B*(myPara.au*pow(Sita,(-myPara.bu))-0);
	else
		ium=1;


	toReturn=kv*pow(0.5f*(Sita+MSita),lambda)*ium*Area/60*MSTEP;//m3//20070228,xiaofc,kv�޸�Ϊ�溬ˮ���仯=kv*(sita/sitas)^b

	float wtemp;
	if(toReturn>=0)
	{
		if(toReturn<=W)//���������ˮ��С�ڱ�����ˮ�����򰴼��������������������ˮ��Ϊ1������ȫ������
		{
			if((MidW+toReturn)<MidWmax2)//����ˮ������ԭ��ˮ���������в����������ˮ����������������
			{
				W=W-toReturn;
			}
			else//�����в�������������������������ˮ��Ϊֹ
			{
				toReturn=MidWmax2-MidW;
				W-=toReturn;
			}
		}
		else
		{
			wtemp=W-1.0f;
			if((MidW+wtemp)<MidWmax2)
			{
				toReturn=wtemp;
				W=1.0f;
			}
			else
			{
				toReturn=MidWmax2-MidW;
				W-=toReturn;
			}
		}
	}
	else
	{
		wtemp=-toReturn;
		if(wtemp<MidW)//�в���ˮ���㹻����
		{
			wtemp=W+wtemp;
			if(wtemp<Wmax2)//����ˮ�ֺ󲻳���������������ˮ��
			{
				W=wtemp;
			}
			else//����󳬹������ˮ�����ˮ�����������ˮ��
			{
				toReturn=-(Wmax2-W);
				W=Wmax2;
			}
		}
		else//�в���ˮ�ֲ����������������в���ˮ��Ϊ1������ȫ����������
		{
			wtemp=MidW-1.0f;
			if((W+wtemp)<Wmax2)
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

	//20060313,���в����Ľ�����������
	float LoopToReturn=0;
	int LoopCount=0;
	float DeltaToReturn;//20080304,xiaofc,��������������ˮ���ı仯��
	while ( LoopCount<20) 
	{
		LoopCount++;
		LoopToReturn=toReturn;
		Sita=W/Wmax2;//������ˮ��
		MidW+=LoopToReturn;
		MSita=MidW/MidWmax2;
	
		//ţ�ٵ�����
		toReturn=LoopToReturn-(toReturn-LoopToReturn)/(-kv*pow(0.5f*(Sita+MSita),lambda)*Area*B*(myPara.au*myPara.bu*pow((W-LoopToReturn),(-myPara.bu-1))/pow(Wmax2,(-myPara.bu))+myPara.am*myPara.bm*pow((MidW+LoopToReturn),(-myPara.bm-1))/pow(MidWmax2,(-myPara.bm)))-1);//20070228,xiaofc,kv�޸�Ϊ�溬ˮ���仯=kv*(sita/sitas)^b
		DeltaToReturn=toReturn-LoopToReturn;;//20080304,xiaofc,��������������ˮ���ı仯��
		
		if(DeltaToReturn/*toReturn*/>=0)
		{
			if(DeltaToReturn<=W)//���������ˮ��С�ڱ�����ˮ�����򰴼��������������������ˮ��Ϊ1������ȫ������
			{
				if((MidW+DeltaToReturn)<MidWmax2)//����ˮ������ԭ��ˮ���������в����������ˮ����������������
				{
					W-=DeltaToReturn;
				}
				else//�����в�������������������������ˮ��Ϊֹ
				{
					DeltaToReturn=MidWmax2-MidW;
					toReturn=DeltaToReturn+LoopToReturn;//20080304,xiaofc,��������������ˮ���ı仯��
					W-=DeltaToReturn;
				}
			}
			else
			{
				wtemp=W-1.0f;
				if((MidW+wtemp)<MidWmax2)
				{
					DeltaToReturn=wtemp;
					toReturn=DeltaToReturn+LoopToReturn;//20080304,xiaofc,��������������ˮ���ı仯��
					W=1.0f;
				}
				else
				{
					DeltaToReturn=MidWmax2-MidW;
					toReturn=DeltaToReturn+LoopToReturn;//20080304,xiaofc,��������������ˮ���ı仯��
					W-=DeltaToReturn;
				}
			}
		}
		else
		{
			wtemp=-DeltaToReturn;
			if(wtemp<MidW)//�в���ˮ���㹻����
			{
				wtemp=W+wtemp;//�����в���ˮ�ֵ�����֮��Ľ��
				if(wtemp<Wmax2)//����ˮ�ֺ󲻳���������������ˮ��
				{
					W=wtemp;
				}
				else//����󳬹������ˮ�����ˮ�����������ˮ��
				{
					DeltaToReturn=-(Wmax2-W);
					toReturn=DeltaToReturn+LoopToReturn;
					W=Wmax2;
				}
			}
			else//�в���ˮ�ֲ����������������в���ˮ��Ϊ1������ȫ����������
			{
				wtemp=MidW-1.0f;//���ܳ�Ϊ-deltatoreturn
				if((W+wtemp)<Wmax2)
				{
					DeltaToReturn=-wtemp;
					toReturn=DeltaToReturn+LoopToReturn;
					W+=wtemp;
				}
				else
				{
					DeltaToReturn=-(Wmax2-W);
					toReturn=DeltaToReturn+LoopToReturn;
					W=Wmax2;
				}
			}
		}
		
		//20080304,xiaofc,�µ���������
		if(abs(DeltaToReturn)<1e-3) break; 
	}//end of while
	
	return toReturn;
}
bool SoilWater::NeedWater1(void)
{	

	if(W<Wmax1)
	{
		return true;
	}
	else
	{
		return false;
	}
}


// ���������ı���
float SoilWater::GetSRate(void)
{
	//20051212 ������
	//�����ǰ��ˮ����������ˮ�����������ظ�ֵ
	//�Դ�ʹ��ˮ����������ˮ��������£��ر�����ϵ����������ˮ���µĲο���������

	//if( NeedWater1() )
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

int SoilWater::initialize(MParameters myPara)
{
	Area=myPara.Area;
	k0=myPara.PKH1;
	RouteLength=myPara.RouteLength;
	n1=myPara.USita1;//��������ˮ���ǲ���������ʱ��20060220
	n2=myPara.USita2;
	SlopeJ=myPara.SlopeJ;
	kv=myPara.PKV1;//��������������������ҪС����������������
	Depth=myPara.UDepth;//�������

	Wmax1=Area*Depth*n1;   //Ш�Σ�û��RouteLength*2
	Wmax2=Area*Depth*(n1+n2); //20041016�޸�
	
	W=myPara.SoilW;//�����ɱ���ʼ���������ˮ��ת����

	isDebug=false;
	
	//byiwish20060222
	Sita=W/Wmax2;//wh����ʵ�ʺ�ˮ���/��������϶�����=ʵ�ʺ�ˮ��/���ͺ�ˮ��

	lambda=6.0f;

	//david,��ʼ��д������
    Wout=0.0f;

	return 1;
}
