#include "FVMunit.h"
#include "math.h"
#include "HighSlopeRunoff.h"
#include "functions.h"

/******************************RichardsUnit类的实现******************************/
RichardsUnit::RichardsUnit(float theta0,float K0,float D0,float dz0)
:theta(theta0),K(K0),D(D0),dz(dz0)
{
}

RichardsUnit::~RichardsUnit(void)
{
}

/*********************************FVMunit类的实现*********************************/
//注意：静态变量要赋初值，否则编译出错。
float FVMunit::thetaf=0.3;   float FVMunit::thetas =0.5; float FVMunit::thetab=0.18;     float FVMunit::thetaw=0.05;     
float FVMunit::thetac=0.02;  float FVMunit::dt1=300;     float FVMunit::dt2=3600;        float FVMunit::dz=1;           
float FVMunit::E0_a=0.1;     float FVMunit::N=20;        float FVMunit::Hour=0;          float FVMunit::StartHour=0;     
double FVMunit::usD=0.01;    float FVMunit::usL=2;       float FVMunit::usf=0.5;         float FVMunit::TimeStart=0;
float FVMunit::Emax=100;     float FVMunit::Emin=40;     short FVMunit::Year=2000;       short FVMunit::Month=1;      
short FVMunit::Day=1;        short FVMunit::StartDay=1;  short FVMunit::StartYear=2000;  short FVMunit::StartMonth=1; 
ofstream* FVMunit::myFile=NULL;

float FVMunit::Betaf[]={0,0.8,7.9,3.5,3.69};   double FVMunit::fc[]={1e-4,1e-4,1e-4,1e-4,1e-4}; 
float FVMunit::thetai[]={0.4,0.3,0.3,0.3,0.3};  float FVMunit::dc[]={0.9,0.9,0.9,0.9,0.9};

//第一行：森林；第二行：灌木；第三行：草地；第四行：耕地
const float FVMunit::LAI[4][12]={{1.68,1.52,1.68,2.9,4.9,5,5,4.6,3.44,3.04,2.16,2},{2,2.25,2.95,3.85,3.75,3.5,3.55,3.2,3.3,2.85,2.6,2.2},{2,2.25,2.95,3.85,3.75,3.5,3.55,3.2,3.3,2.85,2.6,2.2},{0.05,0.02,0.05,0.25,1.5,3,4.5,5,2.5,0.5,0.05,0.02}};
const float FVMunit::fz[]={0.5,2.5,1.5,0.8,0.8};
const float FVMunit::maxLAI[4]={5,3.85,3.85,5};


FVMunit::FVMunit(float x0,float hup0,float hdown0,float hm0,float ahup0,float dx10,float dx20,float beta10,float beta20,float Width0,int landtype0,string sType0,float Dis0,float VH0)
:x(x0),hup(hup0),hdown(hdown0),hm(hm0),ahup(ahup0),dx1(dx10),dx2(dx20),beta1(beta10),beta2(beta20),Width(Width0),landtype(landtype0),ss(1e-6),sType(sType0),Dis(Dis0),VH(VH0)
,wPlant(0),EPI(0),TD(0),FED(0),qs(1e-6),ksa(1e-4),hv(0)
{
	//饱和土壤水断面平均渗透系数ksa(mm/s)
	if(hm-hdown>1e-3)
	{
		ksa=fc[landtype]*(exp(-usf*(hup-hm)-exp(-usf*(hup-hdown))))/usf/(hm-hdown);
	}
	else
	{
		ksa=fc[landtype]*exp(-usf*(hup-hm));
	}

	//饱和土壤水的给水度参数
	mu=thetas-thetaf;

	//初始化非饱和土壤的垂向分层
	this->InitialRichardsUnit();
}


FVMunit::~FVMunit(void)
{
	if(!ri.empty())
	{
		ri.clear();//ri.swap((vector<RichardsUnit>)(ri));//释放内存空间,如果ri是指针变量需要此句
	}	
}


//1、初始化垂向非饱和土壤网格单元，只执行一次
void FVMunit::InitialRichardsUnit(void)
{
	float sdh=hup-hm;
	
	//饱和土壤水出流坡面的情况,此时不存在非饱和土壤。
	if(sdh<=1e-3)
	{
		if(!ri.empty()) {ri.clear();}
		hm=hup;
		return;
	}

    //rdz：r(remain),ldz:l(last)
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
	const float DS=3.9;//土壤分形维数3.8-4左右
	const float lamda=0.5;
	
	//边界条件赋值
	thetaA=thetaB=thetai[landtype];
	DA=DB=dd;//mm2/s
	KA=1000*fc[landtype]*pow(Se,(8-2*DS)/(3-DS)+lamda);//mm/s
	KB=1000*fc[landtype]*exp(-usf*((num-1)*dz+ldz))*pow(Se,(8-2*DS)/(3-DS)+lamda);//mm/s（这里的dz先维持m单位)

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

//2、更新垂向非饱和土壤网格单元的数量，饱和地下水计算完成后调用
void FVMunit::UpdateRichardsUnit(void)
{
	float sdh=hup-hm;
	
	//饱和土壤水出流坡面的情况,此时不存在非饱和土壤。
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

	//饱和地下水位上升情况1
	if(num<bsize)
	{
		ri.erase(ri.begin()+num,ri.end());
		ri[ri.size()-1].dz=ldz*1000;//mm
	}
	//饱和地下水位上升情况2
	if(num==bsize && ldz*1000<=ri[bsize-1].dz)
	{
		ri[bsize-1].dz=ldz*1000;//mm	
	}
    //饱和地下水位下降情况1
	if(num==bsize && ldz*1000>ri[bsize-1].dz)
	{
		//就是将最后一个网格的含水量和田间持水量取下加权平均。
		ri[bsize-1].theta = ri[bsize-1].theta*ri[bsize-1].dz/(ldz*1000)+thetaf*(1-ri[bsize-1].dz/(ldz*1000));

		if(ri[bsize-1].theta<=thetac) {ri[bsize-1].theta=thetac+1e-4;}
		if(ri[bsize-1].theta>thetas)  {ri[bsize-1].theta=thetas-1e-4;}

		ri[bsize-1].dz=ldz*1000;
	}
	//饱和地下水位下降情况2（认为新增的网格具备田间持水量）
	else
	{
		float zd,Se=(thetaf-thetac)/(thetas-thetac);
		double kd;
		const double dd=1e6*usD*pow(Se,usL);//mm2/s
		const float DS=3.9;//土壤分形维数3.8-4左右
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


//入口参数：p为dt(s)时间内的降雨m
//功能：经过dt植被截留后的降雨(m/s)
float FVMunit::Plant(float p,float dt)
{
	//入口参数判断
	if(p<=0) { return 0.0;}
	HighSlopeRunoff::P += p*Width*dx1*cos(beta1);
	
	//为计算LAI和蒸发能力准备，计算开始之前先刷新时间
	HourToDate(TimeStart,StartYear,StartMonth,StartDay,StartHour,&Year,&Month,&Day,&Hour);
	EPI=StepEPI(dt,ahup,3658);

	/********************************植被截留计算**********************************/
	//Kc:冠层截留系数0.1-0.2mm;dc:植被覆盖度0-1,这里取0.90;lai:当前叶面积指数
	//Iv:最大截留能力;Icd:当前冠层实际截留能力;Iact:冠层实际截留量
	float Iv,Icd=0,Kc=0.00015,lai;
	float r=0;

	//计算植被当前截留能力m
	if(dc[landtype]<0) { dc[landtype]=0.0;}
	
	lai=FVMunit::LAI[landtype-1][Month-1];
	Iv=Kc*dc[landtype]*lai;
	if(Iv>wPlant) { Icd=Iv-wPlant;}
	
	//计算实际植被截留量m
	if(p<=Icd) { wPlant+=p;   Icd-=p; r=0;     }
	else       { wPlant+=Icd; Icd=0;  r=p-Icd; }
	
	/********************************植被截留蒸发**********************************/
	//只影响剩余蒸发能力，不影响透过植被的水量
	//&&&&&&&&&&&&&&&&&&&&&&&&&&&&&vc为作物系数，需要查阅资料，不同的植被在不同的季节vc值变化较大，给出类似lai的静态变量&&&&&&&&&&&&&&&&&&&&&&&&&&&&
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
		(*myFile)<<"warning:r<0,透过植被雨量为负值"<<endl;
	}

	return r/dt;//m/s
}



//入口参数：p为dt(s)内的降雨m,flag判断是计算坡面(false)还是土壤(true)
//功能：得到坡面漫流过程的源汇项也是土壤计算的上边界
void FVMunit::SlopeSourceTerm(float p,float dt)
{
	//落到坡面上的雨量(m/s)
	float r=Plant(p,dt);

	//下渗能力(m/s),给一个指数型关系,表层土壤含水量越大入渗能力越小,反之越大。
	//如果考虑引入Honton模型、飞利浦模型等，但是这些模型一般都是在充分降雨条件下，横坐标为时间，而流域水文模拟
	//是间歇性降雨，模型公式中的t不能按照连续取，而且初始下渗系数受前期降雨影响，换句话说模型中的参数都是变的，
	//很难考虑。可以认为下渗曲线降低很快，能在短时间达到fc，因此这里直接采用土壤饱和渗透系数fc作为入渗的判别条件。
	float f=fc[landtype]*exp(Betaf[landtype]*(thetas-thetaA));
	
	//裸露土壤地表蒸发率m/s
	float es=0;

	//1// wh注：如果绝对的基于物理机理，所有"互相间有联系"的物理过程(坡面、土壤、植被等)应由单一的一个微分方程来表达,所有影响因素都在方程中以某种方式存在.因为
	           //只有这样才能保证所有过程"同步"求解，否则就有计算先后的问题了（即解耦计算）。
	//2// 这里就是先算入渗再算蒸发,最后再坡面漫流。之所以这样处理，是因为将入渗和蒸发放到坡面漫流方程的源汇项，可能会解出负水深情况(比如入渗超级快,坡面需要负水深来满足入渗需求)。
	//3// 但我觉得：一般来说由于微地貌、土壤吸附等影响,天上降的雨水肯定先滞留在坡面上，入渗和蒸发过后如果有过剩的再流动。
	if(ss/dt+r<=f)
	{
		f=ss/dt+r;
		ss=0;
	}
	else
	{ 
		ss=ss+(r-f)*dt*cos(beta1);
	}

	//加入饱和土壤水补充的回归流
	if(TimeStart<TimeEnd)
	{
		ss = ss + hv*dt*cos(beta1);
	}

	//for soil
	FED += f*dt; 
	TD += dt;
	
    //坡面径流蒸发
	if(ss>=EPI*(1-dc[landtype]))
	{
		ss-=EPI*(1-dc[landtype]);
		HighSlopeRunoff::E += EPI*(1-dc[landtype])*Width*dx1*cos(beta1);//m3
		EPI-=EPI*(1-dc[landtype]);
	}
	//坡面径流和土壤蒸发
	else if(ss>0 && ss<EPI*(1-dc[landtype]))
	{
		//实际上没到dt时间就蒸发完了ss，这里在dt时间取平均进行坡面流计算
		EPI-=ss;
		
		//float theta=thetaA;
		//if(!ri.empty()) { theta=ri[0].theta;}

		//指数N决定傅抱璞公式曲线光滑的程度
		es=EPI*(1-dc[landtype])*thetaA/(thetaf-thetaw)*(thetaf/pow(pow(thetaf,N)+pow(thetaA,N)+pow(E0_a,N),1.0f/N)-thetaw/pow(pow(thetab,N)+pow(thetaA,N)+pow(E0_a,N),1.0f/N) );
		EPI-=es;
		
		HighSlopeRunoff::E += (ss+es)*Width*dx1*cos(beta1);//m3
	
		FED-=es;
		ss=0;
	}
	//只有土壤蒸发
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
		cout<<"计算过程中出现蒸发能力小于0的情况"<<endl;
	}

	//计算蒸腾作用
	float vc=0.5,Etr=0,sumEtr=0;//vc:作物系数，暂时取0.5，Etr:m
	int N=int(fz[landtype]/dz+0.5);

	if(N>0 && wPlant<1e-5)
	{
		//蒸腾作用只发生在非饱和土壤中
		for(int i=0;i<N;i++)
		{
			if(i<ri.size())
			{
				if(ri[i].theta<=thetaw) { Etr=0;}//小于植被凋萎含水量时不考虑蒸腾
				else if(ri[i].theta>=thetaf)//大于田间持水量系数f(theta)取1
				{
					Etr=dc[landtype]*vc*EPI*LAI[landtype-1][Month-1]/maxLAI[landtype-1]*(2*(N-i)-1)/(N*N);
				}
				else//土壤含水量在凋萎含水量和田间含水量之间,f(theta)插值
				{
					Etr=dc[landtype]*vc*EPI*LAI[landtype-1][Month-1]/maxLAI[landtype-1]*(2*(N-i)-1)/(N*N)*(ri[i].theta-thetaw)/thetaf;
				}

				//调整土壤含水量
				ri[i].theta-=Etr/ri[i].dz;
				if(ri[i].theta<thetaw)
				{
					Etr-=(thetaw-ri[i].theta)*ri[i].dz;
					ri[i].theta=thetaw;
				}
			}
			
			//蒸腾作用发生在饱和土壤水中
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


//入口参数：dt单位s,i已经历的时间步长数,P时段降雨m,H未知点高程,H0基准站高程(这里取3658m)
//功能：得到每个时间步长的蒸发能力m
float FVMunit::StepEPI(float dt,int H,int H0)
{
	float HourInDay=this->Hour;
	if( HourInDay<6 || HourInDay>=18)
	{
		return 0.0f;
	}

	float EPIday,EPIhour;
	//TOPMODEL方法
	if(Emax>=0 && Emin>=0)
	{
		int j=GetNumOfHour(Year,1,1,0,Year,Month,Day,0);
		j=j/24;//得到当前时间距离本年度1月1号的d数
		EPIday=Emin+0.5*(Emax-Emin)*(1+sin(2*PI*j/365-PI/2));
		EPIhour=EPIday/24;
	}
	//史海匀拟合-高程分布蒸发能力
	else
	{
		float Ae=-11.3,Fe=1.266,Be=11.32;
		float A=-4.78,F=-5.459,B=5.227;
		float Aa=8.641,Fa=-8.319,Ba=8.031;
		float w=PI/6,we=PI/6,wa=PI/6;
		float PH=1013*(16955-H)/(16955+H);//单位mbar
		float PH0=1013*(16955-H0)/(16955+H0);
		
		float p1,p2,p3,p4,p5;
		if(Month<7) {p1=-0.005625,p2=0.09903,p3=-0.6573,p4=1.831,p5=0.625;}//上半年系数
		if(Month>6) {p1=0.005,p2=-0.1913,p3=2.722,p4=-17.08,p5=41.48;}//下半年系数
		
		float u0=p1*pow(Month,4.0)+p2*pow(Month,3.0)+p3*pow(Month,2.0)+p4*Month+p5;//单位m/s
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
			cout<<"海匀蒸发能力拟合公式出现负值"<<endl;
		}
	}
	
	//小时蒸发能力在一d内按照正弦曲线重新分配
	EPIhour=EPIhour*sin((HourInDay-6)*PI/12)*PI;
	return EPIhour*dt/3600;
}


//入口参数：dt:时间步长(s);
//功能：非饱和土壤水运动
//注:p=0时,dt可能仍然为1小时,之后又降雨,所以入口参数需要p来判断上边界的取法
void FVMunit::UnsaturatedSoil(float dt)
{
	//ufe为上边界入渗或者蒸发通量mm/s,p>0时,dt一定和dt2相等.
	const float ufe=FED/TD*1000;
	FED=TD=0;
	
	if(ri.empty()) { return;}
	int N=ri.size();

	//Richards方程求解过程中用变量
	float Se=(thetai[landtype]-thetac)/(thetas-thetac);
	const float DS=3.9,lamda=0.5,T=(8-2*DS)/(3-DS)+lamda;
	double *a=new double[N-1],*b=new double[N],*c=new double[N-1],*x=new double[N],*f=new double[N];

	//debug调试信息用
	bool flag=false;//标记是否输出线性方程组
	float sLength=0;
	if(sType=="l"||sType=="L") { sLength=HighSlopeRunoff::myPara->LengthL/cos(beta1);}
	if(sType=="r"||sType=="R") { sLength=HighSlopeRunoff::myPara->LengthR/cos(beta1);}
	if(sType=="s"||sType=="S") { sLength=HighSlopeRunoff::myPara->LengthS/cos(beta1);}
	double *aa=new double[N-1],*bb=new double[N],*cc=new double[N-1],*ff=new double[N];//存储TDMA计算之前的原始方程组系数，调试信息用

	
	/*************************************************上边界条件更新*************************************************/
	//当坡面有水深时,认为thetaA饱和
	if(ss>1e-4)
	{
		thetaA=thetas;
		DA=1e6*usD*pow((thetaA-thetac)/(thetas-thetac),usL);//mm2/s
		KA=1000*fc[landtype]*pow((thetaA-thetac)/(thetas-thetac),T);//mm/s
	}
	//坡面无水深时通量条件限制
	else
	{
		long t=0;//牛顿迭代次数
		float aex,ex=(ri[0].theta-thetac)/(thetas-thetac);//ex为(x-thetac)/(thetas-thetac);
		float s=1,dfex,afex=1,fex;

		//经过验证，牛顿下降迭代确实效率要高，否则t超过1000也可能不收敛
		while(abs(afex)>0.01)
		{
			t++;
			if(t>120)
			{
				if(HighSlopeRunoff::debug)
				{
					(*myFile)<<"牛顿迭代土壤表层含水量120次不收敛"<<endl;
					(*myFile)<<"KA:"<<ex*(thetas-thetac)+thetac<<",ri[0].theta:"<<ri[0].theta<<endl;
				}
				break;
			}

			fex=(1000*fc[landtype]*pow(ex,T)-ufe)*ri[0].dz+2*1e6*usD*pow(ex,usL)*(ex*(thetas-thetac)+thetac-ri[0].theta);
			dfex=1000*fc[landtype]*T*pow(ex,T-1)*ri[0].dz+2*1e6*usD*pow(ex,usL-1)*(ex*(thetas-thetac)*(usL+1)+usL*(thetac-ri[0].theta));//mm,mm2/s

			//wh:这里s一定要恢复初值，否则不断的被2除，会变为0的，那么ex就永远没增长了。
			s=1.0;
			for(int i=0;i<50;i++)
			{
				aex=-s*fex/dfex;//ex的增量
				aex=aex+ex;//新的ex，但不一定采用

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
					//cout<<"非饱和土壤上边界计算,牛顿迭代次数:"<<t<<",下降解查找次数:"<<i<<",没有找到下降解"<<endl;
				}	
			}
		}

		if(ex<0) { ex=1e-6;}//不能小于残余含水量
		if(ex>1) { ex=1;}//不能大于饱和含水量

		//上面迭代过程中如果ex为负值，则dfex的开方操作可能无意义，最后得到无穷解。
		//这里进行简单处理，如果无意义解，降雨大于0则认为饱和，降雨小于0认为和上一时刻一样
		if(!_finite(ex))
		{
			if(ufe>0){ ex=1;}
			else
			{
				ex=min(1,max((ri[0].theta-thetac)/(thetas-thetac),0));
			}
		}

		//牛顿迭代法隐式求解
		float a1=ex*(thetas-thetac)+thetac;
		thetaA=a1;

		//显式求解，用K[0],D[0]代替KA和DA
		float a2=ri[0].theta-(ri[0].K-ufe)*ri[0].dz/2/ri[0].D;

		//下面判断什么时候取a2
		if(a2>0 && a2<=thetas)
		{
			//此时thetaA一定小于ri[0].theta,并且牛顿迭代没得到合适解时
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
	/*************************************************上边界更新完成*************************************************/

	/*************************************************下边界条件更新*************************************************/
	//非饱和土壤与岩石直接接触的情况
	//非饱和土壤下边界不会处于真正入渗补给岩石的情况,因为一旦交界层有积水,饱和土壤运动就会处理与岩石的交互,不用非饱和土壤操心了。
	//因此这里也就不用给入渗的下边界条件
	if(hm-hdown<1e-3)
	{
		//下面被注释掉的三行如果作为下边界条件不利于抑制计算出的土壤含水量（会大于thetas）
		//thetaB=ri[N-1].theta;
		//KB=ri[N-1].K;
		//DB=ri[N-1].D;
		
		//岩石的渗透性比土壤的强(大裂隙),认为thetaB超过thetaf的部分都渗入岩石,thetaB小于thetaf时受上层土壤控制
		if(HighSlopeRunoff::Rock.Kr>this->ksa)
		{
			thetaB=min(ri[N-1].theta,thetaf);
		}
		//岩石不透水面，交界处受土壤控制
		else
		{
			thetaB=min(ri[N-1].theta,thetas);
		}
		Se=(thetaB-thetac)/(thetas-thetac);
		KB=1000*fc[landtype]*exp(-usf*(hup-hdown)*pow(Se,(8-2*DS)/(3-DS)+lamda));//mm/s
		DB=1e6*usD*pow(Se,usL);//mm2/s	
	}
	//非饱和土壤与饱和土壤接触的情况
	else
	{
		thetaB=thetai[landtype];
		Se=(thetaB-thetac)/(thetas-thetac);
		KB=1000*fc[landtype]*exp(-usf*(hup-hdown)*pow(Se,(8-2*DS)/(3-DS)+lamda));//mm/s
		DB=1e6*usD*pow(Se,usL);//mm2/s
	}
	/*************************************************下边界更新完成*************************************************/

	/*************************************************显式求解预测步*************************************************/
	float zd=0;
	if(N==1)
	{
		//唯一网格节点
		x[0]=(pow(ri[0].dz,2)*ri[0].theta/dt+2*(thetaB*DB+thetaA*DA)-ri[0].dz*(KB-KA))/(pow(ri[0].dz,2)/dt+2*(DB+DA));
	}
	else
	{
		//上边界节点
		x[0]=ri[0].dz*(ri[1].theta-ri[0].theta)*(ri[0].D+ri[1].D)-ri[0].dz*(ri[0].dz+ri[1].dz)*(ri[0].K-KA)-2*(ri[0].dz+ri[1].dz)*DA*(ri[0].theta-thetaA);
		x[0]=x[0]*dt/pow(ri[0].dz,2)/(ri[0].dz+ri[1].dz)+ri[0].theta;
		
		//中间普通节点
		for(int i=1;i<=N-2;i++)
		{
			x[i]=(ri[i-1].dz+ri[i].dz)*(ri[i+1].theta-ri[i].theta)*(ri[i].D+ri[i+1].D)-(ri[i+1].dz+ri[i].dz)*(ri[i].theta-ri[i-1].theta)*(ri[i].D+ri[i-1].D)-(ri[i].dz+ri[i+1].dz)*(ri[i-1].dz+ri[i].dz)*(ri[i].K-ri[i-1].K);
			x[i]=x[i]*dt/ri[i].dz/(ri[i].dz+ri[i+1].dz)/(ri[i].dz+ri[i+1].dz)+ri[i].theta;
		}

		//下边界节点
		x[N-1]=2*(ri[N-1].dz+ri[N-2].dz)*DB*(thetaB-ri[N-1].theta)-ri[N-1].dz*(ri[N-1].theta-ri[N-2].theta)*(ri[N-1].D+ri[N-2].D)-ri[N-1].dz*(ri[N-1].dz+ri[N-2].dz)*(ri[N-1].K-ri[N-1].K);
		x[N-1]=x[N-1]*dt/pow(ri[N-1].dz,2)/(ri[N-1].dz+ri[N-2].dz)+ri[N-1].theta;
	}
	
	//用t+1时刻的含水量更新K和D，以及边界条件
	for(int i=0;i<N;i++)
	{
		if(x[i]<=thetac){ x[i]=thetac+1e-4;}
		if(x[i]>thetas) { x[i]=thetas-1e-4;}

		Se=(x[i]-thetac)/(thetas-thetac);
		zd+=ri[i].dz/2;

		ri[i].K=1000*fc[landtype]*exp(-usf*zd/1000)*pow(Se,(8-2*DS)/(3-DS)+lamda);//渗透系数;
		ri[i].D=1e6*usD*pow(Se,usL);//扩散系数;

		zd+=ri[i].dz/2;
	}
	zd=0;

	//根据t+1时刻的预测值，得到t+1时刻的上边界条件预测值,ss>0是上面已经写了，不用再重新赋值了。
	if(ss<=1e-4)
	{
		long t=0;//牛顿迭代次数
		float aex,ex=(x[0]-thetac)/(thetas-thetac);//ex为(x-thetac)/(thetas-thetac);
		float s=1,dfex,afex=1,fex;

		//经过验证，牛顿下降迭代确实效率要高，否则t超过1000也可能不收敛
		while(abs(afex)>0.01)
		{
			t++;
			if(t>120)
			{
				if(HighSlopeRunoff::debug)
				{
					(*myFile)<<"牛顿迭代土壤表层含水量120次不收敛"<<endl;
					(*myFile)<<"KA:"<<ex*(thetas-thetac)+thetac<<",ri[0].theta:"<<ri[0].theta<<endl;
				}
				break;
			}

			fex=(1000*fc[landtype]*pow(ex,T)-ufe)*ri[0].dz+2*1e6*usD*pow(ex,usL)*(ex*(thetas-thetac)+thetac-x[0]);
			dfex=1000*fc[landtype]*T*pow(ex,T-1)*ri[0].dz+2*1e6*usD*pow(ex,usL-1)*(ex*(thetas-thetac)*(usL+1)+usL*(thetac-x[0]));//mm,mm2/s

			//wh:这里s一定要恢复初值，否则不断的被2除，会变为0的，那么ex就永远没增长了。
			s=1.0;
			for(int i=0;i<50;i++)
			{
				aex=-s*fex/dfex;//ex的增量
				aex=aex+ex;//新的ex，但不一定采用

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
					//cout<<"非饱和土壤上边界计算,牛顿迭代次数:"<<t<<",下降解查找次数:"<<i<<",没有找到下降解"<<endl;
				}	
			}
		}

		if(ex<0) { ex=1e-6;}//不能小于残余含水量
		if(ex>1) { ex=1;}//不能大于饱和含水量

		thetaA=ex*(thetas-thetac)+thetac;

		DA=1e6*usD*pow((thetaA-thetac)/(thetas-thetac),usL);//mm2/s
		KA=1000*fc[landtype]*pow((thetaA-thetac)/(thetas-thetac),T);//mm/s
	}


	//根据t+1时刻的预测值，得到t+1时刻的下边界条件预测值。
	if(hm-hdown<1e-3)
	{
		//岩石的渗透性比土壤的强(大裂隙),认为thetaB超过thetaf的部分都渗入岩石,thetaB小于thetaf时受上层土壤控制
		if(HighSlopeRunoff::Rock.Kr>this->ksa)
		{
			thetaB=min(x[N-1],thetaf);
		}
		//岩石不透水面，交界处受土壤控制
		else
		{
			thetaB=min(x[N-1],thetas);
		}
		Se=(thetaB-thetac)/(thetas-thetac);
		KB=1000*fc[landtype]*exp(-usf*(hup-hdown)*pow(Se,(8-2*DS)/(3-DS)+lamda));//mm/s
		DB=1e6*usD*pow(Se,usL);//mm2/s	
	}
	/************************************************显式求解更新完成************************************************/

	/*************************************************隐式求解校正步*************************************************/
	//wh注：如果dt时间比较长，通过解方程会发现从上到下若干个网格内的土壤含水量为负值，这容易理解，因为垂直入渗的影响，在dt
	//中间的某个时刻实际上土壤水已经渗没了，如果继续强迫其渗(因为渗透系数、扩散系数是按照时段初赋值的)，自然会求出负值，
	//出现负值以下的土壤也得到了本来不存在的水量.
	float DzUp=1,DzBelow=1;
	if(N==1)
	{
		//下面这句只对通量上边界条件适用
		//ri[0].theta=(pow(ri[0].dz,2)*ri[0].theta/2/dt + thetaB*DB - (KB-ufe)*ri[0].dz/2)/(pow(ri[0].dz,2)/2/dt+DB);
		ri[0].theta=(pow(ri[0].dz,2)*ri[0].theta/dt+2*(thetaB*DB+thetaA*DA)-ri[0].dz*(KB-KA))/(pow(ri[0].dz,2)/dt+2*(DB+DA));
		if(ri[0].theta<=thetac)
		{
			//cout<<"坡面表层含水率:"<<ri[0].theta<<","<<"饱和地下水有蒸发"<<endl;
			ri[0].theta=thetac+1e-4;
		}
		if(ri[0].theta>thetas)
		{
			ri[0].theta=thetas-1e-4;
		}
	}
	else
	{
		//上边界赋值(通量)
		DzBelow=(ri[0].dz+ri[1].dz)/2;

		//f[0]中含有ufe,ri[0].theta影响thetaA取值,thetaA影响ufe取值,进而又影响ri[0].theta的值
		//以下一行只对通量上边界条件适用
		//b[0]=2*pow(ri[0].dz,2)*DzBelow/dt+ri[0].dz*(ri[0].D+ri[1].D);//a(A)+a(P)
		b[0]=2*pow(ri[0].dz,2)*DzBelow/dt+ri[0].dz*(ri[0].D+ri[1].D)+4*DzBelow*DA;
		c[0]=-ri[0].dz*(ri[0].D+ri[1].D);//a(E)
		
		//以下一行只对通量上边界条件适用
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

		//下边界赋值
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

	//更新每个土壤单元的K,D,theta,不包括上下边界
	for(int i=0;i<N;i++)
	{
		if(N>1) { ri[i].theta = x[i];}

		if(HighSlopeRunoff::debug && ri[i].theta>thetas+1e-3)
		{
			if(flag==false)
			{
				flag=true;
				(*myFile)<<"---------------------------------------------------------------------------------------"<<endl;	
				(*myFile)<<"warning:Year:"<<Year<<",Month:"<<Month<<",Day:"<<Day<<",Hour:"<<Hour<<",sType:"<<sType<<",坡面长:"<<sLength<<"(m)"<<endl;
				//cout<<"大时间步长:"<<WholeTime/3600<<"(t),小时间步长:"<<dt/60<<"(min),大步长剩余时间:"<<dt/60<<"(min)"<<endl;
			}
			(*myFile)<<"距坡顶斜向距离:"<<Dis<<"(m),垂向第"<<i+1<<"个土壤单元(共"<<N<<"个),地下水埋深:"<<hup-hm<<"(m)"<<endl;
			(*myFile)<<"当前土壤表层蒸发:"<<ufe*1000<<"(mm/s),网格厚"<<ri[i].dz/1000<<"(m),土壤含水量:"<<ri[i].theta<<",大于"<<thetas<<endl;
		}

		if(HighSlopeRunoff::debug && ri[i].theta<0)
		{
			if(flag==false)
			{
				flag=true;
				(*myFile)<<"---------------------------------------------------------------------------------------"<<endl;
				(*myFile)<<"warning:Year:"<<Year<<",Month:"<<Month<<",Day:"<<Day<<",Hour:"<<Hour<<",sType:"<<sType<<",坡面长:"<<sLength<<"(m)"<<endl;
				//cout<<"大时间步长:"<<WholeTime/3600<<"(t),小时间步长:"<<TimeStep/60<<"(min),大步长剩余时间:"<<dt/60<<"(min)"<<endl;
			}
			(*myFile)<<"距坡顶斜向距离:"<<Dis<<"(m),垂向第"<<i+1<<"个土壤单元(共"<<N<<"个),地下水埋深:"<<hup-hm<<"(m)"<<endl;
			(*myFile)<<"当前土壤表层蒸发:"<<ufe*1000<<"(mm/s),网格厚"<<ri[i].dz/1000<<"(m),土壤含水量:"<<ri[i].theta<<",小于0"<<endl;
		}

		if(HighSlopeRunoff::debug && !_finite(ri[i].theta))
		{
			(*myFile)<<"warning:非饱和土壤含水量得到INF解."<<endl;
			ri[i].theta=(thetac+thetas)/2;
		}

		//if(HighSlopeRunoff::debug && i==N-1 && flag==true)
		//{
			//flag=false;
			/*cout<<"TDMA线性方程组分别为:"<<endl;
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

		ri[i].K=1000*fc[landtype]*exp(-usf*zd/1000)*pow(Se,(8-2*DS)/(3-DS)+lamda);//渗透系数;
		ri[i].D=1e6*usD*pow(Se,usL);//扩散系数;

		zd+=ri[i].dz/2;
	}

	if(NULL!=a)  delete[]a;  if(NULL!=b)  delete[]b;  if(NULL!=c)  delete[]c;
	if(NULL!=x)  delete[]x;  if(NULL!=f)  delete[]f;  if(NULL!=aa) delete[]aa; 
	if(NULL!=bb) delete[]bb; if(NULL!=cc) delete[]cc; if(NULL!=ff) delete[]ff;
	/************************************************隐式求解更新完成************************************************/
}


//估算非饱和土壤计算的时间步长，防止计算出负含水量或超饱和含水量
//LeftTime:dt内还剩余的计算时间
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
//		//乘以一个0.6安全系数，最多不超过10天
//		return min(max(min(0.6*maxDt,LeftTime),min(LeftTime,dt2)),10*24*3600);
//
//	}
//	else
//	{
//		//上边界节点
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
//		//中间节点
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
//				//对于每个节点有个最大的dt，在所有节点的dt中选择一个最小dt
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
//		//下边界节点
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
//	//乘以一个0.6安全系数
//	return min(max(min(0.6*maxDt,LeftTime),min(LeftTime,dt2)),10*24*3600);
//}
