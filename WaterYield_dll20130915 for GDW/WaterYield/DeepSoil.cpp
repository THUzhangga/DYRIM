
#include "stdafx.h"
#include ".\deepsoil.h"

DeepSoil::DeepSoil(void)
{

}

DeepSoil::~DeepSoil(void)
{
}

// �Ƿ���Ҫ����ˮ����
//bool DeepSoil::NeedWater2(void)
//{
//	if( NeedWater1() ) return false; //�±�û����أ��Ͳ�����С����
//	
//	if(DSW<Area*DSDepth*(DSn1+DSn2))
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
//bool DeepSoil::NeedWater1(void)
//{
//	if(DSW<Wmax1)
//	{
//		return true;
//	}
//	else
//	{
//		return false;
//	}
//}
void DeepSoil::NextHour(float WaterSupply,float imd)
{

	//20060227��дbyiwish
	DSW+=WaterSupply;
	
	//20070117,xiaofc,������ʼ�趨�������ˮ��������������ˮ����0.1��
	//���Ϊ�����Ǳ��ͣ������Ǻ�ˮ��Ϊ0���Ҳ�������ˮ�ֵ��˶�����
	//if(DSW<0.1f*Wmax1&&imd<0)
	//	DSW=Wmax1*0.1f;
	//else if(DSW>Wmax2&&imd>0)
	//	DSW=Wmax2;
	if(DSW<0)
		DSW=0;
	else if(DSW>Wmax2)
		DSW=Wmax2;

	Sita=DSW/Wmax2;//������ˮ��
	//return WaterSupply;   //������Ҫ��ˮ�������

}

int DeepSoil::initialize(MParameters DSPara)
{
	Area=DSPara.Area;
	DSDepth=DSPara.DSDepth;
	DSn1=DSPara.DSita1;
	DSn2=DSPara.DSita2;

	DSW=DSPara.DSW;

	Wmax1=Area*DSDepth*DSn1;
	Wmax2=Area*DSDepth*(DSn1+DSn2);//addbyiwish20060225

	//byiwish20060222
	Sita=DSW/(Area*DSDepth*(DSn1+DSn2));

	//ad=3520;
	//bd=4.21;

	return 1;
}
