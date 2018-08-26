#include "stdafx.h"
#include ".\surfacewatera.h"

SurfaceWaterA::SurfaceWaterA(void)
{

}

//wh
SurfaceWaterA::SurfaceWaterA(int timestep,CString seequation)
{
	this->MSTEP = timestep;
	this->SEEquation = seequation;
}

SurfaceWaterA::~SurfaceWaterA(void)
{
}

//wh��USita��ǰ��ˮ��ռ���ͺ�ˮ�ʵı�������Ϊ����qzu�Ĺ�ʽ��Ҫ��ֵ��Ϊ�˷���
float SurfaceWaterA::NextHour(float PE,MParameters myPara,float USita,float SoilW,float SoilWmax2)
//PE:�䵽�ر�ľ�����
//USita�Ǳ��ͳ̶ȣ�W/Wmax2
{
	//float PenetCan=PenetrateK*(1-SRate)/60*MSTEP;
	
	WaterY=0;
	WaterPenetrated=0;

	//������
	//WaterY=SRate*PE*Area;

	//��������
	//if(PE>PenetCan)
	//{
	//	WaterY=(PE-PenetCan)*Area;
	//	WaterPenetrated=PenetCan*Area;
	//}
	//else
	//{
	//	WaterPenetrated=PE*Area;
	//}

	//wh������������к�ɫ�ı��ʽ�õ��˻����ƺʹ������ɵĸ��
	isu=1+2*myPara.au*pow(USita,-(myPara.bu))/myPara.UDepth;//�����ݶ�

	//xiaofc,20070720,�Ľ��ԷǱ�������͸ϵ�������Ĵ���Kr=Usita^lambda
	//lambda��������϶�ķ���ά��D��������D=2ʱ��lambda=5;D=3ʱ, lambda=+oo
	Kr=pow((0.5f+USita/2),lambda);//lambdaĬ��ֵΪ5

	//WaterPenetrated=myPara.PKV0*isu*Area*MSTEP/60;//m3
	WaterPenetrated=Kr*myPara.PKV0*isu*Area*MSTEP/60;//m3//wh��PKV0�ĵ�λ��m/Сʱ

	float wtemp;
	wtemp=PE*Area;

	//wh��������µ�if��else�ǵõ��ر������������Ĵ���ʽ�����������ĸ���ܿ��ڱ���������ˮ����
	if(WaterPenetrated<wtemp)//����ˮ��С�ھ�����
	{
		wtemp=WaterPenetrated+SoilW;
		
		if(wtemp>SoilWmax2)//�����������ˮ������������ֻ���������ˮ��
			WaterPenetrated=SoilWmax2-SoilW;

		WaterY=PE*Area-WaterPenetrated;
	}
	else//�������������������򰴾�����������������û�в���
	{
		wtemp=wtemp+SoilW;

		if(wtemp<SoilWmax2)
			WaterPenetrated=PE*Area;
		else
		{
			WaterPenetrated=SoilWmax2-SoilW;
			WaterY=PE*Area-WaterPenetrated;
		}

	}

	if(WaterY<0)
		WaterY=0;

	SedimentYield=0;

	if( WaterY>0 && SlopeJ>1e-5 && abs(Area+1)>1e-5) 
		if(SEEquation=="Xue")
			SediYield();
		else if(SEEquation=="RevisedXue")
			RevisedSediYieldXue();
		else if(SEEquation=="Guo1")
			RevisedSediYieldLi1();
		else if(SEEquation=="Guo2")
			RevisedSediYieldLi2();
		else if(SEEquation=="Guo3")
			RevisedSediYieldLi3();
		else if(SEEquation=="NONE")
			;
		else
			cout<<"Unrecognized Soil Erosion Equation Type "<<SEEquation<<" !"<<endl;

	return WaterPenetrated;
};

//��ɳ���㣬add by Ѧ��
void SurfaceWaterA::SediYield(void)
{   
	//20051219 ����ErosionK�����������ݿ�Parameter����
	float k=ErosionK;
	
	float beta=0.3f;   //��ʱ���������
	float WYield;//=WYield/3600.0f;  //�ر������λ��Ϊm3/s
	WYield=WaterY/(60*MSTEP);
	if( WYield>0 && SlopeJ>1e-5 && abs(Area+1)>1e-5) 
	{
		//SedimentYield=5.0f/42.0f*0.8f*3.14f*Area*k*pow(2650.0f,beta+1)*pow(9.8f,beta)*pow(WYield/(Area/RouteLength),0.6f*beta)*pow(Roughnessn,double(0.6f*beta-0.6f))*pow(RouteLength,0.4f)*SediD*pow(WYield/Area,0.4f)*pow(SlopeJ,double(1.3f*beta+0.3f)); //��ɳ ��λ kg/s
		//SedimentYield=0.2991993f*Area*k*pow(2650.0f,beta+1)*pow(9.8f,beta)*pow(WYield/(Area/RouteLength),0.6f*beta)*pow(Roughnessn,double(0.6f*beta-0.6f))*pow(RouteLength,0.4f)*SediD/1000.0f*pow(WYield/Area,0.4f)*pow(SlopeJ,double(1.3f*beta+0.3f)); //��ɳ ��λ kg/s
		//20051219 Ѧ�� ���J��ָ��1.3f*beta=>0.7f*beta
		SedimentYield=0.2991993f*Area*k*pow(2650.0f,beta+1)*pow(9.8f,beta)*pow(WYield/(Area/RouteLength),0.6f*beta)*pow(Roughnessn,double(0.6f*beta-0.6f))*pow(RouteLength,0.4f)*SediD/1000.0f*pow(WYield/Area,0.4f)*pow(SlopeJ,double(0.7f*beta+0.3f)); //��ɳ ��λ kg/s
		//cout<<"SediY: "<<SedimentYield<<"\t"<<SediD<<"\t"<<WYield<<"\t"<<Area<<endl;
	}
	else 
		SedimentYield=0;

	if(!_finite (SedimentYield))
	{
		cout<<"INFINITE Sediment Yield Result in SurfaceWater:"<<endl;
		cout<<"Area="<<Area<<"\tk="<<k<<"\tbeta="<<beta<<"\tWYield="<<WYield<<endl;
		cout<<"Length="<<RouteLength<<"\tManning_n="<<Roughnessn<<"\tSediD="<<SediD<<"\tSlope="<<SlopeJ<<endl;
	}
}

void SurfaceWaterA::RevisedSediYieldXue(void)
{
	float L=RouteLength*2;//Topaz�������³��ǰ볤
	float B=Area/L;//����Ŀ��
	float ql;//��ʾ�½Ŵ���λ��ȵ�����
	ql=WaterY/(60*MSTEP);//�Ƚ��ر������λ��Ϊm3/s
	ql/=B;//m2/s
	float GammaM;//��ˮ���أ����½ŵ�Юɳ�����㣬���鿡��ʽ,���ת��Ϊ��ˮ�������½Ż�ˮ���ص�ƽ��ֵ
	float h=pow(ql*Roughnessn/sqrt(SlopeJ),0.6);//�½Ŵ�ˮ��
	float v=ql/h;//����
	float miu0=0.001; //��ˮ����ճ��ϵ�� 0.001��N*s��/��m^2��
	//���������ٶ�Ӧ����m
	float D50=SediD/1000;
	float D90=D50*3;
	//D90ƫ�󣬶��ڲ�͹�0.10mm���ʣ����D50ȡ0.05mm ����ʽD90Ϊ0.15mmƫ��
	D90=0.10/1000.0f;
	float omegaS;//��ˮ����
	omegaS=(sqrt(10.99f*D90*D90*D90+36.0f*miu0*miu0/1000000)-6.0f*miu0/1000)/D90;
	float ks=2.0*D90;
	//float Re=4.0*h*v*1000/9.81/miu0;
    float Re=4.0*h*v*1000/miu0; //���������ٶ�
	float ff=0.11*pow((ks/4.0/h+68.0/Re),double(0.25) );
	float temp1=v/omegaS*sqrt(ff/8.0);
	float temp2=D90/4.0/h;
	float SSfei;
	//David,��Ȼȡ��ֵ��GammaM���ֻ�ܵ�(2650+1000)/2
	SSfei=0.0068*pow(temp1,1.5f)*pow(temp2,1.0f/6.0f) ;
	//SSfei=0.3;
	GammaM=SSfei*1650.0f/2+1000;
	//if(GammaM>=1825)	GammaM=1825;
	//20070827,xiaofc,ʽ�в�����gammaM,��ͳһΪRouM
	//GammaM*=9.8f;

	//��ɳ��,kg/s
	SedimentYield=5.0f*PI/6.0/(3.0*ErosionBeta+7.0)*ErosionK*pow(D50,-ErosionBeta)*2650.0*pow(GammaM/(2650.0f-GammaM),ErosionBeta)*pow(Roughnessn,0.6*(ErosionBeta-1.0f))*pow(SlopeJ,0.7*ErosionBeta+0.3f)*pow(ql,0.6f*ErosionBeta+0.4f)*Area;
	//������ɳ�ʺ��ټ��һ���Ƿ񳬹�������ɳ��
	//if (SedimentYield>SSfei*2650.0f*ql*B)
	//{
	//	SedimentYield=SSfei*2650.0f*ql*B;
	//}
	//	if (SedimentYield>800*ql*B)
	//{
	//	SedimentYield=800*ql*B;
	//}
	//	SedimentYield=100*ql*B;
	//cout<<"SedimentYield per unit width="<<SedimentYield/B<<endl;
		if(!_finite (SedimentYield))
	{
		cout<<"INFINITE Sediment Yield Result in SurfaceWater:"<<endl;
		cout<<"Area="<<Area<<"\tk="<<ErosionK<<"\tbeta="<<ErosionBeta<<"\tql="<<ql<<endl;
		cout<<"Length="<<L<<"\tManning_n="<<Roughnessn<<"\tSediD="<<SediD<<"\tSlope="<<SlopeJ<<endl;
	}
	
}

void SurfaceWaterA::RevisedSediYieldLi1(void)
{
	//cout<<"this is an improved soil erosion module: GUO1"<<endl;
	float ql=WaterY/(60*MSTEP)/Area;
	//����40.4%�¶ȶ�Ӧ�ĵ�Ч�³�
	float L=2.0f*RouteLength;
	//���¿���������ʽ
	float Tn=0.0f;
	float T2n=0.0f;
	float T4n=0.0f;
	float T8n=0.0f;
	float Sn=0.0f;
	float S2n=0.0f;
	float S4n=0.0f;
	float Cn=0.0f;
	float C2n=0.0f;
	float Rn=0.0f;
	float Rnb=0.0f;
	float TOL=1e-3;
	float eps=1.0f+TOL;
	int i;
	int n=1024;
	float dh=L/n;
	for (i=1;i<=n;i++)
	{
		Tn=Tn+dh/2.0f*(integrand1(ql,(i-1)*dh)+integrand1(ql,i*dh));
		T2n=T2n+dh/2.0f*integrand1(ql,(2*i-1)*dh/2.0f);
		T4n=T4n+dh/4.0f*(integrand1(ql,(4*i-3)*dh/4.0f)+integrand1(ql,(4*i-1)*dh/4.0f));
		T8n=T8n+dh/8.0f*(integrand1(ql,(8*i-7)*dh/8.0f)+integrand1(ql,(8*i-5)*dh/8.0f)+integrand1(ql,(8*i-3)*dh/8.0f)+integrand1(ql,(8*i-1)*dh/8.0f));
	}
    T2n=T2n+0.5*Tn;
	T4n=T4n+0.5*T2n;
	T8n=T8n+0.5*T4n;
	Sn=(4*T2n-Tn)/3.0f;
	S2n=(4*T4n-T2n)/3.0f;
	S4n=(4*T8n-T4n)/3.0f;
	Cn=(16*S2n-Sn)/15.0f;
	C2n=(16*S4n-S2n)/15.0f;
	Rn=(64*C2n-Cn)/63.0f;
	n=n*4;
	int circlenumber=0;
	while (eps>TOL && circlenumber<=10)
	{
		
		Rnb=Rn;
		n=n*2;
		dh=L/n;
		Tn=T2n;
		T2n=T4n;
		T4n=T8n;
		Sn=S2n;
		S2n=S4n;
		Cn=C2n;
		T8n=0.0f;
		for (i=1;i<=n;i++)
		{
			T8n=T8n+dh/2.0f*integrand1(ql,(2*i-1)*dh/2.0f);
		}
		T8n=T8n+0.5*T4n;
		S4n=(4*T8n-T4n)/3.0f;
		C2n=(16*S4n-S2n)/15.0f;
		Rn=(64*C2n-Cn)/63.0f;
		eps=2*abs(Rn-Rnb)/(Rn+Rnb);
		circlenumber=circlenumber+1;
		//cout<<"Rn="<<Rn<<"\teps="<<eps<<"\tn="<<n<<endl;
	}
	SedimentYield=Rn*Area/2.0f/RouteLength; //������ɳ��ת��Ϊȫ������ɳ��
	//SedimentYield=0.0f;
	//cout<<"SedimentYield="<<SedimentYield<<"\tTotal circles "<<circlenumber<<endl;
		if(!_finite (SedimentYield))
	{
		cout<<"INFINITE Sediment Yield Result(Guo Module) in SurfaceWater:"<<endl;
		cout<<"Area="<<Area<<"\tk="<<ErosionK1<<"\tbeta="<<ErosionBeta1<<"\tql="<<ql<<endl;
		cout<<"Length="<<L<<"\tManning_n="<<Roughnessn<<"\tSediD="<<SediD<<"\tSlope="<<SlopeJ<<endl;
	}
}
void SurfaceWaterA::RevisedSediYieldLi2(void)
{
	//cout<<"this is an improved soil erosion module: GUO2"<<endl;
	float ql=WaterY/(60*MSTEP)/Area;
	//����40.4%�¶ȶ�Ӧ�ĵ�Ч�³�
	float L=slopelengthadjusted(2.0f*RouteLength);
	//cout<<"Original Slope: "<<SlopeJ<<"Original Slopelength: "<<2.0f*RouteLength<<"Adjusted Slopelength: "<<L<<endl;
	//���¿���������ʽ
	float Tn=0.0f;
	float T2n=0.0f;
	float T4n=0.0f;
	float T8n=0.0f;
	float Sn=0.0f;
	float S2n=0.0f;
	float S4n=0.0f;
	float Cn=0.0f;
	float C2n=0.0f;
	float Rn=0.0f;
	float Rnb=0.0f;
	float TOL=1e-3;
	float eps=1.0f+TOL;
	int i;
	int n=1024;
	float dh=L/n;
	for (i=1;i<=n;i++)
	{
		Tn=Tn+dh/2.0f*(integrand2(ql,(i-1)*dh,L)+integrand2(ql,i*dh,L));
		T2n=T2n+dh/2.0f*integrand2(ql,(2*i-1)*dh/2.0f,L);
		T4n=T4n+dh/4.0f*(integrand2(ql,(4*i-3)*dh/4.0f,L)+integrand2(ql,(4*i-1)*dh/4.0f,L));
		T8n=T8n+dh/8.0f*(integrand2(ql,(8*i-7)*dh/8.0f,L)+integrand2(ql,(8*i-5)*dh/8.0f,L)+integrand2(ql,(8*i-3)*dh/8.0f,L)+integrand2(ql,(8*i-1)*dh/8.0f,L));
	}
    T2n=T2n+0.5*Tn;
	T4n=T4n+0.5*T2n;
	T8n=T8n+0.5*T4n;
	Sn=(4*T2n-Tn)/3.0f;
	S2n=(4*T4n-T2n)/3.0f;
	S4n=(4*T8n-T4n)/3.0f;
	Cn=(16*S2n-Sn)/15.0f;
	C2n=(16*S4n-S2n)/15.0f;
	Rn=(64*C2n-Cn)/63.0f;
	n=n*4;
	int circlenumber=0;
	while (eps>TOL && circlenumber<=10)
	{
		
		Rnb=Rn;
		n=n*2;
		dh=L/n;
		Tn=T2n;
		T2n=T4n;
		T4n=T8n;
		Sn=S2n;
		S2n=S4n;
		Cn=C2n;
		T8n=0.0f;
		for (i=1;i<=n;i++)
		{
			T8n=T8n+dh/2.0f*integrand2(ql,(2*i-1)*dh/2.0f,L);
		}
		T8n=T8n+0.5*T4n;
		S4n=(4*T8n-T4n)/3.0f;
		C2n=(16*S4n-S2n)/15.0f;
		Rn=(64*C2n-Cn)/63.0f;
		eps=2*abs(Rn-Rnb)/(Rn+Rnb);
		circlenumber=circlenumber+1;
		//cout<<"Rn="<<Rn<<"\teps="<<eps<<"\tn="<<n<<endl;
	}
	SedimentYield=Rn*Area/2.0f/RouteLength; //������ɳ��ת��Ϊȫ������ɳ��
	//SedimentYield=0.0f;
	//cout<<"SedimentYield="<<SedimentYield<<"\tTotal circles "<<circlenumber<<endl;
		if(!_finite (SedimentYield))
	{
		cout<<"INFINITE Sediment Yield Result(Guo Module) in SurfaceWater:"<<endl;
		cout<<"Area="<<Area<<"\tk="<<ErosionK1<<"\tbeta="<<ErosionBeta1<<"\tql="<<ql<<endl;
		cout<<"Length="<<L<<"\tManning_n="<<Roughnessn<<"\tSediD="<<SediD<<"\tSlope="<<SlopeJ<<endl;
	}
}
void SurfaceWaterA::RevisedSediYieldLi3(void)
{
	//cout<<"this is an improved soil erosion module: GUO3"<<endl;
	float ql=WaterY/(60*MSTEP)/Area;
	//����40.4%�¶ȶ�Ӧ�ĵ�Ч�³�
	//float L=slopelengthadjusted(2.0f*RouteLength);
	float L=2.0f*RouteLength;
	//���¿���������ʽ
	float Tn=0.0f;
	float T2n=0.0f;
	float T4n=0.0f;
	float T8n=0.0f;
	float Sn=0.0f;
	float S2n=0.0f;
	float S4n=0.0f;
	float Cn=0.0f;
	float C2n=0.0f;
	float Rn=0.0f;
	float Rnb=0.0f;
	float TOL=1e-3;
	float eps=1.0f+TOL;
	int i;
	int n=1024;
	float dh=L/n;
	for (i=1;i<=n;i++)
	{
		Tn=Tn+dh/2.0f*(integrand2(ql,(i-1)*dh,L)+integrand2(ql,i*dh,L));
		T2n=T2n+dh/2.0f*integrand2(ql,(2*i-1)*dh/2.0f,L);
		T4n=T4n+dh/4.0f*(integrand2(ql,(4*i-3)*dh/4.0f,L)+integrand2(ql,(4*i-1)*dh/4.0f,L));
		T8n=T8n+dh/8.0f*(integrand2(ql,(8*i-7)*dh/8.0f,L)+integrand2(ql,(8*i-5)*dh/8.0f,L)+integrand2(ql,(8*i-3)*dh/8.0f,L)+integrand2(ql,(8*i-1)*dh/8.0f,L));
	}
    T2n=T2n+0.5*Tn;
	T4n=T4n+0.5*T2n;
	T8n=T8n+0.5*T4n;
	Sn=(4*T2n-Tn)/3.0f;
	S2n=(4*T4n-T2n)/3.0f;
	S4n=(4*T8n-T4n)/3.0f;
	Cn=(16*S2n-Sn)/15.0f;
	C2n=(16*S4n-S2n)/15.0f;
	Rn=(64*C2n-Cn)/63.0f;
	n=n*4;
	int circlenumber=0;
	while (eps>TOL && circlenumber<=10)
	{
		
		Rnb=Rn;
		n=n*2;
		dh=L/n;
		Tn=T2n;
		T2n=T4n;
		T4n=T8n;
		Sn=S2n;
		S2n=S4n;
		Cn=C2n;
		T8n=0.0f;
		for (i=1;i<=n;i++)
		{
			T8n=T8n+dh/2.0f*integrand2(ql,(2*i-1)*dh/2.0f,L);
		}
		T8n=T8n+0.5*T4n;
		S4n=(4*T8n-T4n)/3.0f;
		C2n=(16*S4n-S2n)/15.0f;
		Rn=(64*C2n-Cn)/63.0f;
		eps=2*abs(Rn-Rnb)/(Rn+Rnb);
		circlenumber=circlenumber+1;
		//cout<<"Rn="<<Rn<<"\teps="<<eps<<"\tn="<<n<<endl;
	}
	SedimentYield=Rn*Area/2.0f/RouteLength*pow(slopelengthadjusted(L)/L,0.5); //������ɳ��ת��Ϊȫ������ɳ��
	//SedimentYield=0.0f;
	//cout<<"slopelengthadjusted(2.0f*RouteLength)/L="<<slopelengthadjusted(2.0f*RouteLength)/L<<"\tSedimentYield="<<SedimentYield<<"\tTotal circles "<<circlenumber<<endl;
		if(!_finite (SedimentYield))
	{
		cout<<"INFINITE Sediment Yield Result(Guo Module) in SurfaceWater:"<<endl;
		cout<<"Area="<<Area<<"\tk="<<ErosionK1<<"\tbeta="<<ErosionBeta1<<"\tql="<<ql<<endl;
		cout<<"Length="<<L<<"\tManning_n="<<Roughnessn<<"\tSediD="<<SediD<<"\tSlope="<<SlopeJ<<endl;
	}
}
/*double SurfaceWaterA::RunOff(double Depth)
{
	//�뷨�ǶԵģ�SlopeӦ������ƽ���¶�SurfaceSlope+WaterDepth/2/RouteLength

	if(Depth!=0)	
	{
		WaterY=pow(Depth,5.0/3.0)*sqrt(SlopeJ)*Area/2.0/RouteLength/Roughnessn*3600.0;
		if(WaterY>WaterDepth*Area)
		{WaterY=WaterDepth*Area;}
		return WaterY;
	}
	}*/
int SurfaceWaterA::initialize(MParameters MyPara)
{
	Area=MyPara.Area;
	RouteLength=MyPara.RouteLength;
	Roughnessn=MyPara.Roughnessn;//riversegs�е�manning
	SlopeJ=MyPara.SlopeJ;
	PenetrateK=MyPara.PKV0;//�ɵ�����������ϵ��
	SediD=MyPara.SediD;//��ֵ����soild50
	
	lambda=8.0f;

	//20051219 Ѧ�� ����k
	ErosionK=MyPara.ErosionK;
	ErosionBeta=MyPara.ErosionBeta;

	//David,�²�ɳĮ�Ͳ���
	ErosionK1=MyPara.ErosionK1;
	ErosionK2=MyPara.ErosionK2;
	ErosionBeta1=MyPara.ErosionBeta1;
	ErosionBeta2=MyPara.ErosionBeta2;

	//david,��ʼ��д������
	WaterY=0.0f;
	SedimentYield=0.0f;

	isDebug=false;

	return 1;
}

float SurfaceWaterA::integrand1(float qe,float x)
{
	//��һ�ַ��������Ľ���ʽ��Ϊˮ�����ʵĺ����������¶ȱ����������Ҫ�Բ���k����
	float Slopef=sinf(atanf(SlopeJ))/sinf(atanf(0.404));
	float ErosionK1r=ErosionK1*pow(Slopef,ErosionBeta1);
	float ErosionK2r=ErosionK2*pow(Slopef,ErosionBeta2);
	float dbeta=ErosionBeta1-ErosionBeta2;
	float kr=ErosionK1r/ErosionK2r/(dbeta+1.0f);
	float temp=kr*pow(qe,dbeta)*(pow(x,dbeta+1.0f)-pow(2.0f*RouteLength,dbeta+1.0f));
	return ErosionK1r*pow(qe*x,ErosionBeta1)*exp(temp);
}
float SurfaceWaterA::integrand2(float qe,float x,float L)
{
	//�ڶ��ַ���������USLE���е�Чת��������40.4%�¶ȵĵ�Ч�³�����˲�����������k
	float dbeta=ErosionBeta1-ErosionBeta2;
	float kr=ErosionK1/ErosionK2/(dbeta+1.0f);
	float temp=kr*pow(qe,dbeta)*(pow(x,dbeta+1.0f)-pow(L,dbeta+1.0f));
	return ErosionK1*pow(qe*x,ErosionBeta1)*exp(temp);
}
float SurfaceWaterA::slopelengthadjusted(float L)
{
	//cout<<"Original Slope: "<<SlopeJ<<endl;
	float lf_m=0.0f; 
    if (SlopeJ>=0.05)
		lf_m=0.5;
	else if (SlopeJ>=0.045)
		lf_m=0.4+20*(SlopeJ-0.045);
	else if (SlopeJ>=0.035)
		lf_m=0.4;
	else if (SlopeJ>=0.03)
		lf_m=0.3+20*(SlopeJ-0.03);
	else if (SlopeJ>=0.01)
		lf_m=0.3;
	else
		lf_m=0.2;
	float sinslope=sinf(atanf(SlopeJ));
	float sf_s=0.0f;                   //�¶�����
	if (sinslope>=(0.46/5.11))
		sf_s=21.91*sinslope-0.96;
	else if (sinslope>=(0.53/6.0f))
		sf_s=16.8*sinslope-0.5;
	else
		sf_s=10.8*sinslope+0.03;
	float lf_m404=0.5f;
	float sf_s404=21.91*sinf(atanf(0.404))-0.96;
	return pow(L/22.13,lf_m/lf_m404)*pow(sf_s/sf_s404,1.0f/lf_m404)*22.13;
}