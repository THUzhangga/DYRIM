#include "FVMunit.h"
#include "math.h"
#include "HighSlopeRunoff.h"
#include "functions.h"

/******************************RichardsUnit���ʵ��******************************/
RichardsUnit::RichardsUnit(float theta0,float K0,float D0,float dz0)
:theta(theta0),K(K0),D(D0),dz(dz0)
{
}

RichardsUnit::~RichardsUnit(void)
{
}

/*********************************FVMunit���ʵ��*********************************/
//ע�⣺��̬����Ҫ����ֵ������������
float FVMunit::thetaf=0.3;   float FVMunit::thetas =0.5; float FVMunit::thetab=0.18;     float FVMunit::thetaw=0.05;     
float FVMunit::thetac=0.02;  float FVMunit::dt1=300;     float FVMunit::dt2=3600;        float FVMunit::dz=1;           
float FVMunit::E0_a=0.1;     float FVMunit::N=20;        float FVMunit::Hour=0;          float FVMunit::StartHour=0;     
double FVMunit::usD=0.01;    float FVMunit::usL=2;       float FVMunit::usf=0.5;         float FVMunit::TimeStart=0;
float FVMunit::Emax=100;     float FVMunit::Emin=40;     short FVMunit::Year=2000;       short FVMunit::Month=1;      
short FVMunit::Day=1;        short FVMunit::StartDay=1;  short FVMunit::StartYear=2000;  short FVMunit::StartMonth=1; 
ofstream* FVMunit::myFile=NULL;

float FVMunit::Betaf[]={0,0.8,7.9,3.5,3.69};   double FVMunit::fc[]={1e-4,1e-4,1e-4,1e-4,1e-4}; 
float FVMunit::thetai[]={0.4,0.3,0.3,0.3,0.3};  float FVMunit::dc[]={0.9,0.9,0.9,0.9,0.9};

//��һ�У�ɭ�֣��ڶ��У���ľ�������У��ݵأ������У�����
const float FVMunit::LAI[4][12]={{1.68,1.52,1.68,2.9,4.9,5,5,4.6,3.44,3.04,2.16,2},{2,2.25,2.95,3.85,3.75,3.5,3.55,3.2,3.3,2.85,2.6,2.2},{2,2.25,2.95,3.85,3.75,3.5,3.55,3.2,3.3,2.85,2.6,2.2},{0.05,0.02,0.05,0.25,1.5,3,4.5,5,2.5,0.5,0.05,0.02}};
const float FVMunit::fz[]={0.5,2.5,1.5,0.8,0.8};
const float FVMunit::maxLAI[4]={5,3.85,3.85,5};


FVMunit::FVMunit(float x0,float hup0,float hdown0,float hm0,float ahup0,float dx10,float dx20,float beta10,float beta20,float Width0,int landtype0,string sType0,float Dis0,float VH0)
:x(x0),hup(hup0),hdown(hdown0),hm(hm0),ahup(ahup0),dx1(dx10),dx2(dx20),beta1(beta10),beta2(beta20),Width(Width0),landtype(landtype0),ss(1e-6),sType(sType0),Dis(Dis0),VH(VH0)
,wPlant(0),EPI(0),TD(0),FED(0),qs(1e-6),ksa(1e-4),hv(0)
{
	//��������ˮ����ƽ����͸ϵ��ksa(mm/s)
	if(hm-hdown>1e-3)
	{
		ksa=fc[landtype]*(exp(-usf*(hup-hm)-exp(-usf*(hup-hdown))))/usf/(hm-hdown);
	}
	else
	{
		ksa=fc[landtype]*exp(-usf*(hup-hm));
	}

	//��������ˮ�ĸ�ˮ�Ȳ���
	mu=thetas-thetaf;

	//��ʼ���Ǳ��������Ĵ���ֲ�
	this->InitialRichardsUnit();
}


FVMunit::~FVMunit(void)
{
	if(!ri.empty())
	{
		ri.clear();//ri.swap((vector<RichardsUnit>)(ri));//�ͷ��ڴ�ռ�,���ri��ָ�������Ҫ�˾�
	}	
}


//1����ʼ������Ǳ�����������Ԫ��ִֻ��һ��
void FVMunit::InitialRichardsUnit(void)
{
	float sdh=hup-hm;
	
	//��������ˮ������������,��ʱ�����ڷǱ���������
	if(sdh<=1e-3)
	{
		if(!ri.empty()) {ri.clear();}
		hm=hup;
		return;
	}

    //rdz��r(remain),ldz:l(last)
	int num=0;
	float rdz=0,ldz=0;
	rdz=sdh-int(sdh/dz)*dz;

	if(rdz>dz/2 || sdh<dz)
	{
		num=int(sdh/dz)+1;
		ldz=rdz;
	}
	else
	{
		num=int(sdh/dz);
		ldz=rdz+dz;
	}

	thetai[landtype]=(thetai[landtype]<=thetac)?(thetac+1e-4):thetai[landtype];
	
	float zd,Se=(thetai[landtype]-thetac)/(thetas-thetac);
	double kd;
	const double dd=1e6*usD*pow(Se,usL);//mm2/s
	const float DS=3.9;//��������ά��3.8-4����
	const float lamda=0.5;
	
	//�߽�������ֵ
	thetaA=thetaB=thetai[landtype];
	DA=DB=dd;//mm2/s
	KA=1000*fc[landtype]*pow(Se,(8-2*DS)/(3-DS)+lamda);//mm/s
	KB=1000*fc[landtype]*exp(-usf*((num-1)*dz+ldz))*pow(Se,(8-2*DS)/(3-DS)+lamda);//mm/s�������dz��ά��m��λ)

	for(int i=0;i<num;i++)
	{
		if(i==num-1)
		{
			zd=i*dz+0.5*ldz;
			kd=1000*fc[landtype]*exp(-usf*zd)*pow(Se,(8-2*DS)/(3-DS)+lamda);//mm/s
			ri.push_back(RichardsUnit(thetai[landtype],kd,dd,ldz*1000));
			break;
		}
		
		zd=(i+0.5)*dz;
		kd=1000*fc[landtype]*exp(-usf*zd)*pow(Se,(8-2*DS)/(3-DS)+lamda);//mm/s
		ri.push_back(RichardsUnit(thetai[landtype],kd,dd,dz*1000));
	}

}

//2�����´���Ǳ�����������Ԫ�����������͵���ˮ������ɺ����
void FVMunit::UpdateRichardsUnit(void)
{
	float sdh=hup-hm;
	
	//��������ˮ������������,��ʱ�����ڷǱ���������
	if(sdh<=1e-3)
	{
		if(!ri.empty()) {ri.clear();}
		hm=hup;
		return;
	}

	int num=0;
	float rdz=0,ldz=0;
	rdz=sdh-int(sdh/dz)*dz;

	if(rdz>dz/2 || sdh<dz)
	{
		num=int(sdh/dz)+1;
		ldz=rdz;
	}
	else
	{
		num=int(sdh/dz);
		ldz=rdz+dz;
	}

	if(num<=0) { return;}

	int bsize=ri.size();

	//���͵���ˮλ�������1
	if(num<bsize)
	{
		ri.erase(ri.begin()+num,ri.end());
		ri[ri.size()-1].dz=ldz*1000;//mm
	}
	//���͵���ˮλ�������2
	if(num==bsize && ldz*1000<=ri[bsize-1].dz)
	{
		ri[bsize-1].dz=ldz*1000;//mm	
	}
    //���͵���ˮλ�½����1
	if(num==bsize && ldz*1000>ri[bsize-1].dz)
	{
		//���ǽ����һ������ĺ�ˮ��������ˮ��ȡ�¼�Ȩƽ����
		ri[bsize-1].theta = ri[bsize-1].theta*ri[bsize-1].dz/(ldz*1000)+thetaf*(1-ri[bsize-1].dz/(ldz*1000));

		if(ri[bsize-1].theta<=thetac) {ri[bsize-1].theta=thetac+1e-4;}
		if(ri[bsize-1].theta>thetas)  {ri[bsize-1].theta=thetas-1e-4;}

		ri[bsize-1].dz=ldz*1000;
	}
	//���͵���ˮλ�½����2����Ϊ����������߱�����ˮ����
	else
	{
		float zd,Se=(thetaf-thetac)/(thetas-thetac);
		double kd;
		const double dd=1e6*usD*pow(Se,usL);//mm2/s
		const float DS=3.9;//��������ά��3.8-4����
		const float lamda=0.5;

		for(int i=0;i<num-bsize;i++)
		{
			if(i==num-bsize-1)
			{
				zd=(bsize+i)*dz+0.5*ldz;
				kd=1000*fc[landtype]*exp(-usf*zd)*pow(Se,(8-2*DS)/(3-DS)+lamda);//mm/s
				ri.push_back(RichardsUnit(thetaf,kd,dd,ldz*1000));//mm
				break;
			}
			zd=(bsize+i+0.5)*dz;
			kd=1000*fc[landtype]*exp(-usf*zd)*pow(Se,(8-2*DS)/(3-DS)+lamda);//mm/s
			ri.push_back(RichardsUnit(thetaf,kd,dd,dz*1000));//mm
		}
	}
}


//��ڲ�����pΪdt(s)ʱ���ڵĽ���m
//���ܣ�����dtֲ��������Ľ���(m/s)
float FVMunit::Plant(float p,float dt)
{
	//��ڲ����ж�
	if(p<=0) { return 0.0;}
	HighSlopeRunoff::P += p*Width*dx1*cos(beta1);
	
	//Ϊ����LAI����������׼�������㿪ʼ֮ǰ��ˢ��ʱ��
	HourToDate(TimeStart,StartYear,StartMonth,StartDay,StartHour,&Year,&Month,&Day,&Hour);
	EPI=StepEPI(dt,ahup,3658);

	/********************************ֲ����������**********************************/
	//Kc:�ڲ����ϵ��0.1-0.2mm;dc:ֲ�����Ƕ�0-1,����ȡ0.90;lai:��ǰҶ���ָ��
	//Iv:����������;Icd:��ǰ�ڲ�ʵ�ʽ�������;Iact:�ڲ�ʵ�ʽ�����
	float Iv,Icd=0,Kc=0.00015,lai;
	float r=0;

	//����ֲ����ǰ��������m
	if(dc[landtype]<0) { dc[landtype]=0.0;}
	
	lai=FVMunit::LAI[landtype-1][Month-1];
	Iv=Kc*dc[landtype]*lai;
	if(Iv>wPlant) { Icd=Iv-wPlant;}
	
	//����ʵ��ֲ��������m
	if(p<=Icd) { wPlant+=p;   Icd-=p; r=0;     }
	else       { wPlant+=Icd; Icd=0;  r=p-Icd; }
	
	/********************************ֲ����������**********************************/
	//ֻӰ��ʣ��������������Ӱ��͸��ֲ����ˮ��
	//&&&&&&&&&&&&&&&&&&&&&&&&&&&&&vcΪ����ϵ������Ҫ�������ϣ���ͬ��ֲ���ڲ�ͬ�ļ���vcֵ�仯�ϴ󣬸�������lai�ľ�̬����&&&&&&&&&&&&&&&&&&&&&&&&&&&&
	float vc=0.25;
	if(wPlant>=vc*dc[landtype]*EPI)
	{
		HighSlopeRunoff::E += vc*dc[landtype]*EPI*Width*dx1*cos(beta1);//m3
		wPlant -= vc*dc[landtype]*EPI;
		EPI -= vc*dc[landtype]*EPI;
	}
	else
	{
		HighSlopeRunoff::E += wPlant*Width*dx1*cos(beta1);//m3
		EPI -= wPlant;
		wPlant=0;
	}

	if(HighSlopeRunoff::debug && r<0)
	{ 
		(*myFile)<<"warning:r<0,͸��ֲ������Ϊ��ֵ"<<endl;
	}

	return r/dt;//m/s
}



//��ڲ�����pΪdt(s)�ڵĽ���m,flag�ж��Ǽ�������(false)��������(true)
//���ܣ��õ������������̵�Դ����Ҳ������������ϱ߽�
void FVMunit::SlopeSourceTerm(float p,float dt)
{
	//�䵽�����ϵ�����(m/s)
	float r=Plant(p,dt);

	//��������(m/s),��һ��ָ���͹�ϵ,���������ˮ��Խ����������ԽС,��֮Խ��
	//�����������Hontonģ�͡�������ģ�͵ȣ�������Щģ��һ�㶼���ڳ�ֽ��������£�������Ϊʱ�䣬������ˮ��ģ��
	//�Ǽ�Ъ�Խ��꣬ģ�͹�ʽ�е�t���ܰ�������ȡ�����ҳ�ʼ����ϵ����ǰ�ڽ���Ӱ�죬���仰˵ģ���еĲ������Ǳ�ģ�
	//���ѿ��ǡ�������Ϊ�������߽��ͺܿ죬���ڶ�ʱ��ﵽfc���������ֱ�Ӳ�������������͸ϵ��fc��Ϊ�������б�������
	float f=fc[landtype]*exp(Betaf[landtype]*(thetas-thetaA));
	
	//��¶�����ر�������m/s
	float es=0;

	//1// whע��������ԵĻ��������������"���������ϵ"���������(���桢������ֲ����)Ӧ�ɵ�һ��һ��΢�ַ��������,����Ӱ�����ض��ڷ�������ĳ�ַ�ʽ����.��Ϊ
	           //ֻ���������ܱ�֤���й���"ͬ��"��⣬������м����Ⱥ�������ˣ���������㣩��
	//2// �����������������������,���������������֮����������������Ϊ�������������ŵ������������̵�Դ������ܻ�����ˮ�����(��������������,������Ҫ��ˮ����������������)��
	//3// ���Ҿ��ã�һ����˵����΢��ò������������Ӱ��,���Ͻ�����ˮ�϶��������������ϣ�������������������й�ʣ����������
	if(ss/dt+r<=f)
	{
		f=ss/dt+r;
		ss=0;
	}
	else
	{ 
		ss=ss+(r-f)*dt*cos(beta1);
	}

	//���뱥������ˮ����Ļع���
	if(TimeStart<TimeEnd)
	{
		ss = ss + hv*dt*cos(beta1);
	}

	//for soil
	FED += f*dt; 
	TD += dt;
	
    //���澶������
	if(ss>=EPI*(1-dc[landtype]))
	{
		ss-=EPI*(1-dc[landtype]);
		HighSlopeRunoff::E += EPI*(1-dc[landtype])*Width*dx1*cos(beta1);//m3
		EPI-=EPI*(1-dc[landtype]);
	}
	//���澶������������
	else if(ss>0 && ss<EPI*(1-dc[landtype]))
	{
		//ʵ����û��dtʱ�����������ss��������dtʱ��ȡƽ����������������
		EPI-=ss;
		
		//float theta=thetaA;
		//if(!ri.empty()) { theta=ri[0].theta;}

		//ָ��N��������豹�ʽ���߹⻬�ĳ̶�
		es=EPI*(1-dc[landtype])*thetaA/(thetaf-thetaw)*(thetaf/pow(pow(thetaf,N)+pow(thetaA,N)+pow(E0_a,N),1.0f/N)-thetaw/pow(pow(thetab,N)+pow(thetaA,N)+pow(E0_a,N),1.0f/N) );
		EPI-=es;
		
		HighSlopeRunoff::E += (ss+es)*Width*dx1*cos(beta1);//m3
	
		FED-=es;
		ss=0;
	}
	//ֻ����������
	else
	{
		//float theta=thetaA;
		//if(!ri.empty()) { theta=ri[0].theta;}

		es=EPI*(1-dc[landtype])*thetaA/(thetaf-thetaw)*(thetaf/pow(pow(thetaf,N)+pow(thetaA,N)+pow(E0_a,N),1.0f/N)-thetaw/pow(pow(thetab,N)+pow(thetaA,N)+pow(E0_a,N),1.0f/N) );
		EPI-=es;

		HighSlopeRunoff::E += es*Width*dx1*cos(beta1);//m3
		
		FED-=es;
	}
	if(EPI<0) 
	{
		EPI=0;
		cout<<"��������г�����������С��0�����"<<endl;
	}

	//������������
	float vc=0.5,Etr=0,sumEtr=0;//vc:����ϵ������ʱȡ0.5��Etr:m
	int N=int(fz[landtype]/dz+0.5);

	if(N>0 && wPlant<1e-5)
	{
		//��������ֻ�����ڷǱ���������
		for(int i=0;i<N;i++)
		{
			if(i<ri.size())
			{
				if(ri[i].theta<=thetaw) { Etr=0;}//С��ֲ����ή��ˮ��ʱ����������
				else if(ri[i].theta>=thetaf)//��������ˮ��ϵ��f(theta)ȡ1
				{
					Etr=dc[landtype]*vc*EPI*LAI[landtype-1][Month-1]/maxLAI[landtype-1]*(2*(N-i)-1)/(N*N);
				}
				else//������ˮ���ڵ�ή��ˮ������京ˮ��֮��,f(theta)��ֵ
				{
					Etr=dc[landtype]*vc*EPI*LAI[landtype-1][Month-1]/maxLAI[landtype-1]*(2*(N-i)-1)/(N*N)*(ri[i].theta-thetaw)/thetaf;
				}

				//����������ˮ��
				ri[i].theta-=Etr/ri[i].dz;
				if(ri[i].theta<thetaw)
				{
					Etr-=(thetaw-ri[i].theta)*ri[i].dz;
					ri[i].theta=thetaw;
				}
			}
			
			//�������÷����ڱ�������ˮ��
			else
			{
				Etr=dc[landtype]*vc*EPI*LAI[landtype-1][Month-1]/maxLAI[landtype-1]*(2*(N-i)-1)/(N*N);
			}

			sumEtr+=Etr;

		}//end for

		EPI-=sumEtr;
		if(EPI<0){ EPI=0; }

	}//end if(N>0)
}


//��ڲ�����dt��λs,i�Ѿ�����ʱ�䲽����,Pʱ�ν���m,Hδ֪��߳�,H0��׼վ�߳�(����ȡ3658m)
//���ܣ��õ�ÿ��ʱ�䲽������������m
float FVMunit::StepEPI(float dt,int H,int H0)
{
	float HourInDay=this->Hour;
	if( HourInDay<6 || HourInDay>=18)
	{
		return 0.0f;
	}

	float EPIday,EPIhour;
	//TOPMODEL����
	if(Emax>=0 && Emin>=0)
	{
		int j=GetNumOfHour(Year,1,1,0,Year,Month,Day,0);
		j=j/24;//�õ���ǰʱ����뱾���1��1�ŵ�d��
		EPIday=Emin+0.5*(Emax-Emin)*(1+sin(2*PI*j/365-PI/2));
		EPIhour=EPIday/24;
	}
	//ʷ�������-�̷ֲ߳���������
	else
	{
		float Ae=-11.3,Fe=1.266,Be=11.32;
		float A=-4.78,F=-5.459,B=5.227;
		float Aa=8.641,Fa=-8.319,Ba=8.031;
		float w=PI/6,we=PI/6,wa=PI/6;
		float PH=1013*(16955-H)/(16955+H);//��λmbar
		float PH0=1013*(16955-H0)/(16955+H0);
		
		float p1,p2,p3,p4,p5;
		if(Month<7) {p1=-0.005625,p2=0.09903,p3=-0.6573,p4=1.831,p5=0.625;}//�ϰ���ϵ��
		if(Month>6) {p1=0.005,p2=-0.1913,p3=2.722,p4=-17.08,p5=41.48;}//�°���ϵ��
		
		float u0=p1*pow(Month,4.0)+p2*pow(Month,3.0)+p3*pow(Month,2.0)+p4*Month+p5;//��λm/s
		float K=27,C=0.02,ka=-0.012,ke=-0.0108;
		float c1=K*(0.2+0.064*u0*pow(H/H0,0.4));
		float c2=(A*sin(w*Month+F)+B)*pow(float(PH/PH0),float(2.5));
		float Te=Ae*sin(we*Month+Fe)+Be+ke*(H-H0);
		float e1=exp( (10.286*(Te+273.15)-2148.4909)/(Te+273.15-35.85) );
		float e2=exp( 12.5633-2670.59/(Te+273.15));
		float ec=exp(C*(Aa*sin(wa*Month+Fa)+Ba+ka*(H-H0)));
		
		if(Te>0)
		{
			EPIhour=c1*(e1-c2)*ec/GetMonthDays(Year,Month)/24;	
		}
		else
		{
			EPIhour=c1*(e2-c2)*ec/GetMonthDays(Year,Month)/24;	
		}
		if(EPIhour<0)
		{
			EPIhour=0.0;
			cout<<"��������������Ϲ�ʽ���ָ�ֵ"<<endl;
		}
	}
	
	//Сʱ����������һd�ڰ��������������·���
	EPIhour=EPIhour*sin((HourInDay-6)*PI/12)*PI;
	return EPIhour*dt/3600;
}


//��ڲ�����dt:ʱ�䲽��(s);
//���ܣ��Ǳ�������ˮ�˶�
//ע:p=0ʱ,dt������ȻΪ1Сʱ,֮���ֽ���,������ڲ�����Ҫp���ж��ϱ߽��ȡ��
void FVMunit::UnsaturatedSoil(float dt)
{
	//ufeΪ�ϱ߽�������������ͨ��mm/s,p>0ʱ,dtһ����dt2���.
	const float ufe=FED/TD*1000;
	FED=TD=0;
	
	if(ri.empty()) { return;}
	int N=ri.size();

	//Richards�������������ñ���
	float Se=(thetai[landtype]-thetac)/(thetas-thetac);
	const float DS=3.9,lamda=0.5,T=(8-2*DS)/(3-DS)+lamda;
	double *a=new double[N-1],*b=new double[N],*c=new double[N-1],*x=new double[N],*f=new double[N];

	//debug������Ϣ��
	bool flag=false;//����Ƿ�������Է�����
	float sLength=0;
	if(sType=="l"||sType=="L") { sLength=HighSlopeRunoff::myPara->LengthL/cos(beta1);}
	if(sType=="r"||sType=="R") { sLength=HighSlopeRunoff::myPara->LengthR/cos(beta1);}
	if(sType=="s"||sType=="S") { sLength=HighSlopeRunoff::myPara->LengthS/cos(beta1);}
	double *aa=new double[N-1],*bb=new double[N],*cc=new double[N-1],*ff=new double[N];//�洢TDMA����֮ǰ��ԭʼ������ϵ����������Ϣ��

	
	/*************************************************�ϱ߽���������*************************************************/
	//��������ˮ��ʱ,��ΪthetaA����
	if(ss>1e-4)
	{
		thetaA=thetas;
		DA=1e6*usD*pow((thetaA-thetac)/(thetas-thetac),usL);//mm2/s
		KA=1000*fc[landtype]*pow((thetaA-thetac)/(thetas-thetac),T);//mm/s
	}
	//������ˮ��ʱͨ����������
	else
	{
		long t=0;//ţ�ٵ�������
		float aex,ex=(ri[0].theta-thetac)/(thetas-thetac);//exΪ(x-thetac)/(thetas-thetac);
		float s=1,dfex,afex=1,fex;

		//������֤��ţ���½�����ȷʵЧ��Ҫ�ߣ�����t����1000Ҳ���ܲ�����
		while(abs(afex)>0.01)
		{
			t++;
			if(t>120)
			{
				if(HighSlopeRunoff::debug)
				{
					(*myFile)<<"ţ�ٵ���������㺬ˮ��120�β�����"<<endl;
					(*myFile)<<"KA:"<<ex*(thetas-thetac)+thetac<<",ri[0].theta:"<<ri[0].theta<<endl;
				}
				break;
			}

			fex=(1000*fc[landtype]*pow(ex,T)-ufe)*ri[0].dz+2*1e6*usD*pow(ex,usL)*(ex*(thetas-thetac)+thetac-ri[0].theta);
			dfex=1000*fc[landtype]*T*pow(ex,T-1)*ri[0].dz+2*1e6*usD*pow(ex,usL-1)*(ex*(thetas-thetac)*(usL+1)+usL*(thetac-ri[0].theta));//mm,mm2/s

			//wh:����sһ��Ҫ�ָ���ֵ�����򲻶ϵı�2�������Ϊ0�ģ���ôex����Զû�����ˡ�
			s=1.0;
			for(int i=0;i<50;i++)
			{
				aex=-s*fex/dfex;//ex������
				aex=aex+ex;//�µ�ex������һ������

				afex=(1000*fc[landtype]*pow(aex,T)-ufe)*ri[0].dz+2*1e6*usD*pow(aex,usL)*(aex*(thetas-thetac)+thetac-ri[0].theta);//mm,mm2/s

				if(abs(afex)<abs(fex))
				{ 
					ex=aex;
					break;
				}
				else { s=s/2;}

				if(i==49)
				{ 
					ex=aex;
					//cout<<"�Ǳ��������ϱ߽����,ţ�ٵ�������:"<<t<<",�½�����Ҵ���:"<<i<<",û���ҵ��½���"<<endl;
				}	
			}
		}

		if(ex<0) { ex=1e-6;}//����С�ڲ��ຬˮ��
		if(ex>1) { ex=1;}//���ܴ��ڱ��ͺ�ˮ��

		//����������������exΪ��ֵ����dfex�Ŀ����������������壬���õ�����⡣
		//������м򵥴������������⣬�������0����Ϊ���ͣ�����С��0��Ϊ����һʱ��һ��
		if(!_finite(ex))
		{
			if(ufe>0){ ex=1;}
			else
			{
				ex=min(1,max((ri[0].theta-thetac)/(thetas-thetac),0));
			}
		}

		//ţ�ٵ�������ʽ���
		float a1=ex*(thetas-thetac)+thetac;
		thetaA=a1;

		//��ʽ��⣬��K[0],D[0]����KA��DA
		float a2=ri[0].theta-(ri[0].K-ufe)*ri[0].dz/2/ri[0].D;

		//�����ж�ʲôʱ��ȡa2
		if(a2>0 && a2<=thetas)
		{
			//��ʱthetaAһ��С��ri[0].theta,����ţ�ٵ���û�õ����ʽ�ʱ
			if(ri[0].K>=ufe && (a1-thetac<=1e-6 || a1-ri[0].theta>=1e-6)) 
			{
				thetaA=a2;
			}
			else if(ri[0].K<ufe && (a1<ri[0].theta || a1>=thetas)) 
			{ 
				thetaA=a2;
			}
		}

		DA=1e6*usD*pow((thetaA-thetac)/(thetas-thetac),usL);//mm2/s
		KA=1000*fc[landtype]*pow((thetaA-thetac)/(thetas-thetac),T);//mm/s
	}
	/*************************************************�ϱ߽�������*************************************************/

	/*************************************************�±߽���������*************************************************/
	//�Ǳ�����������ʯֱ�ӽӴ������
	//�Ǳ��������±߽粻�ᴦ����������������ʯ�����,��Ϊһ��������л�ˮ,���������˶��ͻᴦ������ʯ�Ľ���,���÷Ǳ������������ˡ�
	//�������Ҳ�Ͳ��ø��������±߽�����
	if(hm-hdown<1e-3)
	{
		//���汻ע�͵������������Ϊ�±߽��������������Ƽ������������ˮ���������thetas��
		//thetaB=ri[N-1].theta;
		//KB=ri[N-1].K;
		//DB=ri[N-1].D;
		
		//��ʯ����͸�Ա�������ǿ(����϶),��ΪthetaB����thetaf�Ĳ��ֶ�������ʯ,thetaBС��thetafʱ���ϲ���������
		if(HighSlopeRunoff::Rock.Kr>this->ksa)
		{
			thetaB=min(ri[N-1].theta,thetaf);
		}
		//��ʯ��͸ˮ�棬���紦����������
		else
		{
			thetaB=min(ri[N-1].theta,thetas);
		}
		Se=(thetaB-thetac)/(thetas-thetac);
		KB=1000*fc[landtype]*exp(-usf*(hup-hdown)*pow(Se,(8-2*DS)/(3-DS)+lamda));//mm/s
		DB=1e6*usD*pow(Se,usL);//mm2/s	
	}
	//�Ǳ��������뱥�������Ӵ������
	else
	{
		thetaB=thetai[landtype];
		Se=(thetaB-thetac)/(thetas-thetac);
		KB=1000*fc[landtype]*exp(-usf*(hup-hdown)*pow(Se,(8-2*DS)/(3-DS)+lamda));//mm/s
		DB=1e6*usD*pow(Se,usL);//mm2/s
	}
	/*************************************************�±߽�������*************************************************/

	/*************************************************��ʽ���Ԥ�ⲽ*************************************************/
	float zd=0;
	if(N==1)
	{
		//Ψһ����ڵ�
		x[0]=(pow(ri[0].dz,2)*ri[0].theta/dt+2*(thetaB*DB+thetaA*DA)-ri[0].dz*(KB-KA))/(pow(ri[0].dz,2)/dt+2*(DB+DA));
	}
	else
	{
		//�ϱ߽�ڵ�
		x[0]=ri[0].dz*(ri[1].theta-ri[0].theta)*(ri[0].D+ri[1].D)-ri[0].dz*(ri[0].dz+ri[1].dz)*(ri[0].K-KA)-2*(ri[0].dz+ri[1].dz)*DA*(ri[0].theta-thetaA);
		x[0]=x[0]*dt/pow(ri[0].dz,2)/(ri[0].dz+ri[1].dz)+ri[0].theta;
		
		//�м���ͨ�ڵ�
		for(int i=1;i<=N-2;i++)
		{
			x[i]=(ri[i-1].dz+ri[i].dz)*(ri[i+1].theta-ri[i].theta)*(ri[i].D+ri[i+1].D)-(ri[i+1].dz+ri[i].dz)*(ri[i].theta-ri[i-1].theta)*(ri[i].D+ri[i-1].D)-(ri[i].dz+ri[i+1].dz)*(ri[i-1].dz+ri[i].dz)*(ri[i].K-ri[i-1].K);
			x[i]=x[i]*dt/ri[i].dz/(ri[i].dz+ri[i+1].dz)/(ri[i].dz+ri[i+1].dz)+ri[i].theta;
		}

		//�±߽�ڵ�
		x[N-1]=2*(ri[N-1].dz+ri[N-2].dz)*DB*(thetaB-ri[N-1].theta)-ri[N-1].dz*(ri[N-1].theta-ri[N-2].theta)*(ri[N-1].D+ri[N-2].D)-ri[N-1].dz*(ri[N-1].dz+ri[N-2].dz)*(ri[N-1].K-ri[N-1].K);
		x[N-1]=x[N-1]*dt/pow(ri[N-1].dz,2)/(ri[N-1].dz+ri[N-2].dz)+ri[N-1].theta;
	}
	
	//��t+1ʱ�̵ĺ�ˮ������K��D���Լ��߽�����
	for(int i=0;i<N;i++)
	{
		if(x[i]<=thetac){ x[i]=thetac+1e-4;}
		if(x[i]>thetas) { x[i]=thetas-1e-4;}

		Se=(x[i]-thetac)/(thetas-thetac);
		zd+=ri[i].dz/2;

		ri[i].K=1000*fc[landtype]*exp(-usf*zd/1000)*pow(Se,(8-2*DS)/(3-DS)+lamda);//��͸ϵ��;
		ri[i].D=1e6*usD*pow(Se,usL);//��ɢϵ��;

		zd+=ri[i].dz/2;
	}
	zd=0;

	//����t+1ʱ�̵�Ԥ��ֵ���õ�t+1ʱ�̵��ϱ߽�����Ԥ��ֵ,ss>0�������Ѿ�д�ˣ����������¸�ֵ�ˡ�
	if(ss<=1e-4)
	{
		long t=0;//ţ�ٵ�������
		float aex,ex=(x[0]-thetac)/(thetas-thetac);//exΪ(x-thetac)/(thetas-thetac);
		float s=1,dfex,afex=1,fex;

		//������֤��ţ���½�����ȷʵЧ��Ҫ�ߣ�����t����1000Ҳ���ܲ�����
		while(abs(afex)>0.01)
		{
			t++;
			if(t>120)
			{
				if(HighSlopeRunoff::debug)
				{
					(*myFile)<<"ţ�ٵ���������㺬ˮ��120�β�����"<<endl;
					(*myFile)<<"KA:"<<ex*(thetas-thetac)+thetac<<",ri[0].theta:"<<ri[0].theta<<endl;
				}
				break;
			}

			fex=(1000*fc[landtype]*pow(ex,T)-ufe)*ri[0].dz+2*1e6*usD*pow(ex,usL)*(ex*(thetas-thetac)+thetac-x[0]);
			dfex=1000*fc[landtype]*T*pow(ex,T-1)*ri[0].dz+2*1e6*usD*pow(ex,usL-1)*(ex*(thetas-thetac)*(usL+1)+usL*(thetac-x[0]));//mm,mm2/s

			//wh:����sһ��Ҫ�ָ���ֵ�����򲻶ϵı�2�������Ϊ0�ģ���ôex����Զû�����ˡ�
			s=1.0;
			for(int i=0;i<50;i++)
			{
				aex=-s*fex/dfex;//ex������
				aex=aex+ex;//�µ�ex������һ������

				afex=(1000*fc[landtype]*pow(aex,T)-ufe)*ri[0].dz+2*1e6*usD*pow(aex,usL)*(aex*(thetas-thetac)+thetac-x[0]);//mm,mm2/s

				if(abs(afex)<abs(fex))
				{ 
					ex=aex;
					break;
				}
				else { s=s/2;}

				if(i==49)
				{ 
					ex=aex;
					//cout<<"�Ǳ��������ϱ߽����,ţ�ٵ�������:"<<t<<",�½�����Ҵ���:"<<i<<",û���ҵ��½���"<<endl;
				}	
			}
		}

		if(ex<0) { ex=1e-6;}//����С�ڲ��ຬˮ��
		if(ex>1) { ex=1;}//���ܴ��ڱ��ͺ�ˮ��

		thetaA=ex*(thetas-thetac)+thetac;

		DA=1e6*usD*pow((thetaA-thetac)/(thetas-thetac),usL);//mm2/s
		KA=1000*fc[landtype]*pow((thetaA-thetac)/(thetas-thetac),T);//mm/s
	}


	//����t+1ʱ�̵�Ԥ��ֵ���õ�t+1ʱ�̵��±߽�����Ԥ��ֵ��
	if(hm-hdown<1e-3)
	{
		//��ʯ����͸�Ա�������ǿ(����϶),��ΪthetaB����thetaf�Ĳ��ֶ�������ʯ,thetaBС��thetafʱ���ϲ���������
		if(HighSlopeRunoff::Rock.Kr>this->ksa)
		{
			thetaB=min(x[N-1],thetaf);
		}
		//��ʯ��͸ˮ�棬���紦����������
		else
		{
			thetaB=min(x[N-1],thetas);
		}
		Se=(thetaB-thetac)/(thetas-thetac);
		KB=1000*fc[landtype]*exp(-usf*(hup-hdown)*pow(Se,(8-2*DS)/(3-DS)+lamda));//mm/s
		DB=1e6*usD*pow(Se,usL);//mm2/s	
	}
	/************************************************��ʽ���������************************************************/

	/*************************************************��ʽ���У����*************************************************/
	//whע�����dtʱ��Ƚϳ���ͨ���ⷽ�̻ᷢ�ִ��ϵ������ɸ������ڵ�������ˮ��Ϊ��ֵ����������⣬��Ϊ��ֱ������Ӱ�죬��dt
	//�м��ĳ��ʱ��ʵ��������ˮ�Ѿ���û�ˣ��������ǿ������(��Ϊ��͸ϵ������ɢϵ���ǰ���ʱ�γ���ֵ��)����Ȼ�������ֵ��
	//���ָ�ֵ���µ�����Ҳ�õ��˱��������ڵ�ˮ��.
	float DzUp=1,DzBelow=1;
	if(N==1)
	{
		//�������ֻ��ͨ���ϱ߽���������
		//ri[0].theta=(pow(ri[0].dz,2)*ri[0].theta/2/dt + thetaB*DB - (KB-ufe)*ri[0].dz/2)/(pow(ri[0].dz,2)/2/dt+DB);
		ri[0].theta=(pow(ri[0].dz,2)*ri[0].theta/dt+2*(thetaB*DB+thetaA*DA)-ri[0].dz*(KB-KA))/(pow(ri[0].dz,2)/dt+2*(DB+DA));
		if(ri[0].theta<=thetac)
		{
			//cout<<"�����㺬ˮ��:"<<ri[0].theta<<","<<"���͵���ˮ������"<<endl;
			ri[0].theta=thetac+1e-4;
		}
		if(ri[0].theta>thetas)
		{
			ri[0].theta=thetas-1e-4;
		}
	}
	else
	{
		//�ϱ߽縳ֵ(ͨ��)
		DzBelow=(ri[0].dz+ri[1].dz)/2;

		//f[0]�к���ufe,ri[0].thetaӰ��thetaAȡֵ,thetaAӰ��ufeȡֵ,������Ӱ��ri[0].theta��ֵ
		//����һ��ֻ��ͨ���ϱ߽���������
		//b[0]=2*pow(ri[0].dz,2)*DzBelow/dt+ri[0].dz*(ri[0].D+ri[1].D);//a(A)+a(P)
		b[0]=2*pow(ri[0].dz,2)*DzBelow/dt+ri[0].dz*(ri[0].D+ri[1].D)+4*DzBelow*DA;
		c[0]=-ri[0].dz*(ri[0].D+ri[1].D);//a(E)
		
		//����һ��ֻ��ͨ���ϱ߽���������
		//f[0]=2*pow(ri[0].dz,2)*DzBelow/dt*ri[0].theta-2*ri[0].dz*DzBelow*(ri[0].K-ufe);//b1+a(A)*m
		f[0]=2*pow(ri[0].dz,2)*DzBelow/dt*ri[0].theta-2*ri[0].dz*DzBelow*(ri[0].K-KA)+4*DzBelow*DA*thetaA;

		/*if(f[0]>1000)
		{
		   cout<<"ri[0].dz:"<<ri[0].dz<<",ri[0].theta:"<<ri[0].theta<<",ri[0].K:"<<ri[0].K<<endl;
		}*/

		for(int i=1;i<=N-2;i++)
		{
			DzUp=(ri[i].dz+ri[i-1].dz)/2;
			DzBelow=(ri[i].dz+ri[i+1].dz)/2;
			a[i-1]=-(ri[i].D+ri[i-1].D)*DzBelow;//a(W)
			b[i]=2*DzUp*ri[i].dz*DzBelow/dt + DzUp*ri[i+1].D + DzBelow*ri[i-1].D + (DzUp+DzBelow)*ri[i].D;//a(P);
			c[i]=-(ri[i].D+ri[i+1].D)*DzUp;//a(E);
			f[i]=2*DzUp*ri[i].dz*DzBelow/dt*ri[i].theta - 2*DzUp*DzBelow*(ri[i].K-ri[i-1].K);//b(i)

			/*if(f[i]<0)
			{
			   cout<<"ri[i].dz:"<<ri[i].dz<<",ri[i].theta:"<<ri[i].theta<<",ri[i].K:"<<ri[i].K<<endl;
			}*/
		}

		//�±߽縳ֵ
		DzUp=(ri[N-1].dz+ri[N-2].dz)/2;
		a[N-2]=-ri[N-1].dz*(ri[N-2].D+ri[N-1].D);
		b[N-1]=2*DzUp*ri[N-1].dz*ri[N-1].dz/dt + 4*DzUp*DB + ri[N-1].dz*(ri[N-2].D+ri[N-1].D);
		f[N-1]=2*DzUp*ri[N-1].dz*ri[N-1].dz/dt*ri[N-1].theta - 2*ri[N-1].dz*DzUp*(ri[N-1].K-ri[N-2].K) + 4*DzUp*DB*thetaB;

		if(HighSlopeRunoff::debug)
		{
			for(int k=0;k<N-1;k++)
			{
				aa[k]=a[k]; bb[k]=b[k]; 
				cc[k]=c[k]; ff[k]=f[k];
			}
			bb[N-1]=b[N-1];
			ff[N-1]=f[N-1];
		}

		::TDMA(N,a,b,c,f,x);
	}

	//����ÿ��������Ԫ��K,D,theta,���������±߽�
	for(int i=0;i<N;i++)
	{
		if(N>1) { ri[i].theta = x[i];}

		if(HighSlopeRunoff::debug && ri[i].theta>thetas+1e-3)
		{
			if(flag==false)
			{
				flag=true;
				(*myFile)<<"---------------------------------------------------------------------------------------"<<endl;	
				(*myFile)<<"warning:Year:"<<Year<<",Month:"<<Month<<",Day:"<<Day<<",Hour:"<<Hour<<",sType:"<<sType<<",���泤:"<<sLength<<"(m)"<<endl;
				//cout<<"��ʱ�䲽��:"<<WholeTime/3600<<"(t),Сʱ�䲽��:"<<dt/60<<"(min),�󲽳�ʣ��ʱ��:"<<dt/60<<"(min)"<<endl;
			}
			(*myFile)<<"���¶�б�����:"<<Dis<<"(m),�����"<<i+1<<"��������Ԫ(��"<<N<<"��),����ˮ����:"<<hup-hm<<"(m)"<<endl;
			(*myFile)<<"��ǰ�����������:"<<ufe*1000<<"(mm/s),�����"<<ri[i].dz/1000<<"(m),������ˮ��:"<<ri[i].theta<<",����"<<thetas<<endl;
		}

		if(HighSlopeRunoff::debug && ri[i].theta<0)
		{
			if(flag==false)
			{
				flag=true;
				(*myFile)<<"---------------------------------------------------------------------------------------"<<endl;
				(*myFile)<<"warning:Year:"<<Year<<",Month:"<<Month<<",Day:"<<Day<<",Hour:"<<Hour<<",sType:"<<sType<<",���泤:"<<sLength<<"(m)"<<endl;
				//cout<<"��ʱ�䲽��:"<<WholeTime/3600<<"(t),Сʱ�䲽��:"<<TimeStep/60<<"(min),�󲽳�ʣ��ʱ��:"<<dt/60<<"(min)"<<endl;
			}
			(*myFile)<<"���¶�б�����:"<<Dis<<"(m),�����"<<i+1<<"��������Ԫ(��"<<N<<"��),����ˮ����:"<<hup-hm<<"(m)"<<endl;
			(*myFile)<<"��ǰ�����������:"<<ufe*1000<<"(mm/s),�����"<<ri[i].dz/1000<<"(m),������ˮ��:"<<ri[i].theta<<",С��0"<<endl;
		}

		if(HighSlopeRunoff::debug && !_finite(ri[i].theta))
		{
			(*myFile)<<"warning:�Ǳ���������ˮ���õ�INF��."<<endl;
			ri[i].theta=(thetac+thetas)/2;
		}

		//if(HighSlopeRunoff::debug && i==N-1 && flag==true)
		//{
			//flag=false;
			/*cout<<"TDMA���Է�����ֱ�Ϊ:"<<endl;
			for(int k=0;k<N;k++)
			{
			if(k==0)        { cout<<bb[k]<<"*Xp+"<<cc[k]<<"*Xe="<<ff[k]<<endl;}
			else if(k==N-1) { cout<<aa[k]<<"*Xw+"<<bb[k]<<"*Xp="<<ff[k]<<endl<<endl;}
			else            { cout<<aa[k]<<"*Xw+"<<bb[k]<<"*Xp+"<<cc[k]<<"*Xe="<<ff[k]<<endl;}
			}*/

			/*for(int k=0;k<N;k++)
			{
			if(k==0)        { cout<<b[k]<<"*Xp+"<<c[k]<<"*Xe="<<f[k]<<endl;}
			else if(k==N-1) { cout<<a[k]<<"*Xw+"<<b[k]<<"*Xp="<<f[k]<<endl<<endl;}
			else            { cout<<a[k]<<"*Xw+"<<b[k]<<"*Xp+"<<c[k]<<"*Xe="<<f[k]<<endl;}
			}*/
		//}

		if(ri[i].theta<=thetac) { ri[i].theta=thetac+1e-4;}
		if(ri[i].theta>thetas)  { ri[i].theta=thetas-1e-4;}

		Se=(ri[i].theta-thetac)/(thetas-thetac);
		zd+=ri[i].dz/2;

		ri[i].K=1000*fc[landtype]*exp(-usf*zd/1000)*pow(Se,(8-2*DS)/(3-DS)+lamda);//��͸ϵ��;
		ri[i].D=1e6*usD*pow(Se,usL);//��ɢϵ��;

		zd+=ri[i].dz/2;
	}

	if(NULL!=a)  delete[]a;  if(NULL!=b)  delete[]b;  if(NULL!=c)  delete[]c;
	if(NULL!=x)  delete[]x;  if(NULL!=f)  delete[]f;  if(NULL!=aa) delete[]aa; 
	if(NULL!=bb) delete[]bb; if(NULL!=cc) delete[]cc; if(NULL!=ff) delete[]ff;
	/************************************************��ʽ���������************************************************/
}


//����Ǳ������������ʱ�䲽������ֹ���������ˮ���򳬱��ͺ�ˮ��
//LeftTime:dt�ڻ�ʣ��ļ���ʱ��
//float FVMunit::GetTimeStep(float LeftTime,float ufe,int N)
//{
//	float maxDt=0;
//	double P=0.0;
//	if(N==1)
//	{
//		P=2*DB*(thetaB-ri[0].theta)+(ufe-KB)*ri[0].dz;
//		if(abs(P)<1e-8)
//		{
//			P=1e-8;
//		}
//		
//		if(P>0)
//		{
//			maxDt=pow(ri[0].dz,2)*(thetas-ri[0].theta)/P;
//		}
//		else
//		{
//			maxDt=pow(ri[0].dz,2)*(ri[0].theta-thetac)/abs(P);
//		}
//
//		//����һ��0.6��ȫϵ������಻����10��
//		return min(max(min(0.6*maxDt,LeftTime),min(LeftTime,dt2)),10*24*3600);
//
//	}
//	else
//	{
//		//�ϱ߽�ڵ�
//		P=ri[0].dz*(ri[1].theta-ri[0].theta)*(ri[0].D+ri[1].D)-2*ri[0].dz*ri[1].dz*(ri[0].K-ufe);
//		if(abs(P)<1e-8)
//		{
//			P=1e-8;
//		}
//		if(P>0)
//		{
//			maxDt=2*pow(ri[0].dz,2)*ri[1].dz*(thetas-ri[0].theta)/P;
//		}
//		else
//		{
//			maxDt=2*pow(ri[0].dz,2)*ri[1].dz*(ri[0].theta-thetac)/abs(P);
//		}
//		
//		//�м�ڵ�
//		if(N>2)
//		{
//			for(int i=1;i<=N-2;i++)
//			{
//				P=ri[i-1].dz*(ri[i+1].theta-ri[i].theta)*(ri[i].D+ri[i+1].D)-ri[i+1].dz*(ri[i].theta-ri[i-1].theta)*(ri[i].D+ri[i-1].D)-2*ri[i+1].dz*ri[i-1].dz*(ri[i].K-ri[i-1].K);
//
//				if(abs(P)<1e-8)
//				{
//					P=1e-8;
//				}
//				
//				//����ÿ���ڵ��и�����dt�������нڵ��dt��ѡ��һ����Сdt
//				if(P>0)
//				{
//					maxDt=min(maxDt,2*ri[i].dz*ri[i-1].dz*ri[i+1].dz*(thetas-ri[i].theta)/P);
//				}
//				else
//				{
//					maxDt=min(maxDt,2*ri[i].dz*ri[i-1].dz*ri[i+1].dz*(thetac-ri[i].theta)/P);
//				}
//			}
//		}
//
//		//�±߽�ڵ�
//		P=4*ri[N-2].dz*DB*(thetaB-ri[N-1].theta)-ri[N-1].dz*(ri[N-1].theta-ri[N-2].theta)*(ri[N-1].D+ri[N-2].D)-2*ri[N-1].dz*ri[N-2].dz*(ri[N-1].K-ri[N-1].K);
//		if(abs(P)<1e-8)
//		{
//			P=1e-8;
//		}
//		if(P>0)
//		{
//			maxDt=min(maxDt,2*pow(ri[N-1].dz,2)*ri[N-2].dz*(thetas-ri[N-1].theta)/P);
//		}
//		else
//		{
//			maxDt=min(maxDt,2*pow(ri[N-1].dz,2)*ri[N-2].dz*(ri[N-1].theta-thetac)/abs(P));
//		}
//
//	}
//	
//	//����һ��0.6��ȫϵ��
//	return min(max(min(0.6*maxDt,LeftTime),min(LeftTime,dt2)),10*24*3600);
//}
