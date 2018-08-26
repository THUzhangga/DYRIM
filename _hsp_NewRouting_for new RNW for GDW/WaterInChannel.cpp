#include "WaterInChannel.h"
#include "HighSlopeRunoff.h"

const float WaterInChannel::g=9.81;

WaterInChannel::WaterInChannel(void)
{
}

WaterInChannel::~WaterInChannel(void)
{
	delete[] Q;
	delete[] Qout;

	/*delete[] Q;
	delete[] A;
	delete[] Q2;
	delete[] A2;*/
}

//当前河段计算始末只初始化一次
void WaterInChannel::Initiallize(Para* mPara0, float MSTEP0,int Steps0,float gh0,float gm0,float ef0,float theta10,float theta20,float* Qu10,float* Qu20,float* Quout0,vector<FVMunit>* pL0,vector<FVMunit>* pR0,vector<FVMunit>* pS0,float* FlowB0,float* FlowH0,float* FlowV0,bool SaveFlowPattern0,ofstream* file)
{
	//沟道及坡面信息
	mPara=mPara0;
	gh=gh0;
	gm=gm0;
	ef=ef0; 
	theta1=theta10;
	theta2=theta20;

	//时间采样步长和总步长数
	MSTEP=MSTEP0;//s
	Steps=Steps0;

	//输入信息序列
	Qu1=Qu10;
	Qu2=Qu20;         
	
	//输出信息序列
	Quout=Quout0;
	FlowB=FlowB0;
	FlowH=FlowH0;
	Flow_v=FlowV0;

	//坡面有限体积单元指针
	pL=pL0;
	pR=pR0;
	pS=pS0;

	SaveFlowPattern=SaveFlowPattern0;
	isDebug=HighSlopeRunoff::debug;

	//报错文件句柄
	this->myFile=file;
	
	//初始水位，用于和地下水交互
	H=-gm+1e-6;
	q=0;

	//水流计算
	eta=0.75f;

	//梯形断面的平均边坡系数（边角的余切值）
	m=(1/tan(theta1)+1/tan(theta2))/2;

	n=n0=mPara->RiverManning;
	L=mPara->StreamLength;
	
	//限制河道的最小、最大坡度
	nano=0.0004f;
	S=mPara->StreamSlope;
    S=min(max(S,nano),0.4);
	
	//20060907,xiaofc,以柯朗数为1的条件设置自动空间步长
	float MaxQ=0.0f;//时段内的最大流量
	float MaxC;//对应最大流量的最大波速

	Time=FVMunit::TimeStart;
	delta_t=FVMunit::dt1;//时间步长，s

	//这个for是将两股来流混合的处理
	for(int t=0;t<=Steps;t++)
	{
		if(Qu1[t]+Qu2[t]>MaxQ) 
		{
			MaxQ=Qu1[t]+Qu2[t];
		}
	}

	//20060906,xiaofc,自动求步长数
	if(MaxQ>1.0f)
	{
		//wh解读：通过流速求波速，下面公式适用于三角形断面，经过验证是没有问题的。
		//wh这里就当三角形断面吧，如果梯形断面还得牛顿迭代，太麻烦了
		MaxC=4.0f/3.0f*pow(m/(m*m+1),0.25f)*pow(S,0.375f)*pow(MaxQ,0.25f)/sqrt(2.0)/pow(n,0.75f);
		xsteps=floor(L/MaxC/delta_t+0.5);
		if(xsteps<1) 
		{
			xsteps=1;
		}
	}
	else
	{
		xsteps=1;
	}

	//空间步长m
	delta_x=L/xsteps;

	//分配内存空间
	//wh:xsteps个网格，xsteps+1个节点
	Q=new float[xsteps+1];
	Qout=new float[xsteps+1];

	//因为在InChannel函数不会对Q[0]进行赋值，因此在这里进行
	Q[0]=Qu1[0]+Qu2[0];

	::ZeroFill(Q,xsteps+1);
	::ZeroFill(Qout,xsteps+1);
}


//精确扩散波法
void WaterInChannel::InChannel(void)
{
	Time=FVMunit::TimeStart+delta_t;
	
	//上边界条件
	//不是源坡面
	if(mPara->AreaS<0)
	{
		Qout[0]=Qu1[min(int(Time/MSTEP),Steps)]+Qu2[min(int(Time/MSTEP),Steps)];
	}
	//源河段没有上游来流
	else if(mPara->AreaS>0 && pS->size()>0)
	{
		Qout[0]=(*pS)[pS->size()-1].qs*(*pS)[pS->size()-1].Width;
	}
	else if(mPara->AreaS>0 && pS->size()<=0)
	{
		//这里是融雪补给了，先设为0
		Qout[0]=0.0;
	}

	Qout[0]=max(Qout[0],1e-10);

	//计算源汇项：加入左右坡面表层漫流对明渠的补给
	//因为坡面流的补给是在dt1时间内的，而q现有值是在包含若干个dt1时间大小的vdt时间内的,
	//而每个不同的dt1内qs值不同，因此最后要恢复为q0
	const float q0=q;
	float qL=0,qR=0;
	
	if(pL->size()>0)
	{
		qL=(*pL)[pL->size()-1].qs;
	}

	if(pR->size()>0)
	{
		qR=(*pR)[pR->size()-1].qs;
	}
	
	q=q+qL+qR;

	int N1,N2;//写入Quout中的第几项
	N1=min(int((Time-delta_t)/MSTEP),Steps);
    N2=min(int((Time)/MSTEP),Steps);

	//200512,李铁键
	//如果seglength,即本类中的L<=0,为虚拟河段
	//则不计算，直接把入口复制到出口，return
	if(L<1e-5)
	{
		//此时就一个空间步长，所以有Q[0]和Q[1]的存在
		Q[0]=Q[1]=Qout[0];
		for(int t=N1;t<=N2;t++)
		{
			Quout[t]=Qout[0];
		}
		return;
	}

	//求解出流曲线

	//河段参数
	int MaxIter=50;
	int IterCount=0;
	int IterCount2=0;

	//运算的系数
	float c[4];	
	float Q_c[4];

	//IVPMC与VPMC的区别之处
	float hh;  //平均水深
	float h_x; //pian h / pian x
	float Q_x; //pQ/px
    float J; //包括ph/px在内的能坡
	float cold;
	float cAverage;
	float BAverage;

	float K;
	float iksila;

	float C1;
	float C2;
	float C3;
	float C4;//wh,用于源汇项

	float iterQ;//每个网格点上迭代用到的Q

	//20070724,xiaofc,加一个是否从Qout[t]的循环里遇到负反应跳出来的
	bool bNegtiveQHappened=false;

	//wh:所有断面水深的平均值m
	float hAverage=0.0;

	/*if(FVMunit::TimeStart==259200)
	{
		int xxx=0;
	}*/

	//wh：因为用到j+1节点,所以xx<=xsteps-1
	for(int xx=0;xx<=xsteps-1;xx++)//空间步长循环
	{
		//Qout[xx+1]=Q[xx];//第一个时间点不进行计算
		
		//20070828,xiaofc,Q很小的时候要变糙率
		//20080401,xiaofc,改进
		//if(Q[0]<1 && n0<0.5)
		//	n=0.5-(0.5-n0)*Q[0];
		if(Q[xx]<mPara->DrainingArea/1e9*1 && n0<0.5  && mPara->DrainingArea>3e9)
		{
			n=0.5-(0.5-n0)*Q[xx]/1*1000000000/mPara->DrainingArea;
		}
		else
		{
			n=n0;
		}

        //根据流量求水深（谢才和曼宁公式） 
		//h[1]对应(i,n),h[0]对应(i,n-1)
		h[0]=pow(n*Q[xx]*1.5874f*pow(m*m+1.0f,0.33333333f)/pow(m,5.0f/3)/sqrt(S),0.375f);

		//h[3]对应(i+1,n),h[2]对应(i+1,n-1)
		h[2]=pow(n*Q[xx+1]*1.5874f*pow(m*m+1.0f,0.33333333f)/pow(m,5.0f/3)/sqrt(S),0.375f);
		
		//20060310,李铁键,h[1],h[3]的初始值不能是0,以免被除0
		if(h[0]<1e-10)
		{
			h[0]=1e-10;
		}
		if(h[2]<1e-10)
		{
			h[2]=1e-10;
		}

		h_x=0.0f; //pian h /pian x先赋0

		//最多只运行一个时间步长	
		for(int t=1;t<2;t++)
		{
		    //Qout[xx]是Q[xx]的下一时刻的值（同一个网格点）
			if(Qout[xx]<mPara->DrainingArea/1000000000*1 && n0<0.5  && mPara->DrainingArea>3000000000)
			{
				n=0.5-(0.5-n0)*Qout[xx]/1*1000000000/mPara->DrainingArea;
			}
			else
			{
				n=n0;
			}

			//20060310,李铁键,如果Q[t]<1e-5,那么直接下一时间循环
			if(Qout[xx]<1e-10)
			{
				Qout[xx+1]=Qout[xx];//0.0f;
				continue;
			}

			//Qout[t]=Qout[t-1];
			Qout[xx+1]=Qout[xx];//用Qt比Qout t-1 更容易收敛些
	

			//wh:0,1,2,3的顺序如下：0:(i,n-1),1:(i,n),2:(i+1,n-1),3:(i+1,n),0和1是当前节点i，n是当前时刻，n-1是前一时刻
			h[1]=pow(n*Qout[xx]*1.5874f*pow(m*m+1.0f,0.33333333f)/pow(m,5.0f/3)/sqrt(S),0.375f);
			h[3]=pow(n*Qout[xx+1]*1.5874f*pow(m*m+1.0f,0.33333333f)/pow(m,5.0f/3)/sqrt(S),0.375f);

			
			//求pian h / pian x
			hh=(h[0]+h[1]+h[2]+h[3])/4.0f;
			
			u[0]=(Q[xx])/h[0]/h[0]/m;//u=Q/A(A=m*h*h)
			u[1]=(Qout[xx])/h[1]/h[1]/m;
			u[2]=(Q[xx+1])/h[2]/h[2]/m;
			u[3]=(Qout[xx+1])/h[3]/h[3]/m;
			c[0]=u[0]*4.0/3.0;
			c[1]=u[1]*4.0/3.0;
			c[2]=u[2]*4.0/3.0;
			c[3]=u[3]*4.0/3.0;
			cAverage=(c[0]+c[1]+c[2]+c[3])/4;

			BAverage=4.0f*m*(Q[xx]+Qout[xx]+Q[xx+1]+Qout[xx+1])/3.0f/cAverage;
			BAverage=sqrt(BAverage);

			cold=cAverage;

			//20080811, xiaofc, 计算ph/px不采用Qout[t]点的值，那样对收敛不利
			//Q_x=(Q[t-1]+Q[t]-Qout[t-1]-Qout[t])/2.0f/delta_x;
			Q_x=((Q[xx]+Qout[xx])/2.0f-Q[xx+1])/delta_x;
			h_x=Q_x/BAverage/cAverage;


			//涨水情况
			if(Q_x>0)
			{
				J=eta*h_x+S;
			}
			//落水情况
			else
			{
				J=S;
			}
			

			//满足最小比降的限制，扩散波的坡不能太缓
			J=max(J,nano);

			//以下用更新后的J(J=J0+eta*h_x)再刷新一遍各个变量值
			u[0]=pow(m*h[0],2.0f/3)*sqrt(J)/n/1.5874f/pow(m*m+1.0f,1.0f/3);
			u[1]=pow(m*h[1],2.0f/3)*sqrt(J)/n/1.5874f/pow(m*m+1.0f,1.0f/3);
			u[2]=pow(m*h[2],2.0f/3)*sqrt(J)/n/1.5874f/pow(m*m+1.0f,1.0f/3);
			u[3]=pow(m*h[3],2.0f/3)*sqrt(J)/n/1.5874f/pow(m*m+1.0f,1.0f/3);

			c[0]=u[0]*4.0/3.0;
			c[1]=u[1]*4.0/3.0;
			c[2]=u[2]*4.0/3.0;
			c[3]=u[3]*4.0/3.0;
			cAverage=(c[0]+c[1]+c[2]+c[3])/4;

			B[0]=m*h[0]*2.0;
			B[1]=m*h[1]*2.0;
			B[2]=m*h[2]*2.0;
			B[3]=m*h[3]*2.0;

			BAverage=4.0f*m*(Q[xx]+Qout[xx]+Q[xx+1]+Qout[xx+1])/3.0f/cAverage;//正确.
			BAverage=sqrt(BAverage);

			IterCount2=0;
			//20080811, xiaofc, 优化波速的收敛条件，加快计算速度
			while(!(abs(cold-cAverage)<0.001f || abs(cold/cAverage-1.0f)<0.001f) && IterCount2<MaxIter)
			{
				IterCount2++;

				h_x=Q_x/BAverage/cAverage;

				if(Q_x>0)
				{
					J=eta*h_x+S;
				}
				else
				{
					J=S;
				}

				if(J<nano)
					J=nano;

				cold=cAverage;

				//20060317,李铁键,求C改用Newton迭代
				float fc,fcp;//F(c),F'(c)

				//正确，就是根据谢才公式，代入(c=4/3*u)，只是J中还有C一项
				fc=1.0f/4.7622f/n*pow(m*m/(m*m+1),1.0f/3)*(pow(h[0],2.0f/3)+pow(h[1],2.0f/3)+pow(h[2],2.0f/3)+pow(h[3],2.0f/3))*sqrt(J)-cold;
				fcp=-1.0f/9.5244/n*pow(m*m/(m*m+1),1.0f/3)*(pow(h[0],2.0f/3)+pow(h[1],2.0f/3)+pow(h[2],2.0f/3)+pow(h[3],2.0f/3))/sqrt(J)*h_x/cold-1.0f;
				cAverage=cold-fc/fcp;

				//20070728,xiaofc,这个牛顿迭代也不一定总收敛，如果碰到cAverage<0的时候强行退出就麻烦了
				if(cAverage<0)
				{
					cAverage=cold;
					break;
				}

				if (abs(J-nano)<1e-10) 
				{
					break;
				}

			}//end of while, get c

			BAverage=4.0f*m*(Q[xx]+Qout[xx]+Q[xx+1]+Qout[xx+1])/3.0f/cAverage;
			BAverage=sqrt(BAverage);

			//20080811, xiaofc, 用ph/px修正K容易产生负反应，取消
			K=delta_x/cAverage;
			//K=K/(1-3.0f/32*h_x/S);
			//cout<<"K= "<<K<<endl;

			Q_c[0]=Q[xx]/c[0];
			Q_c[1]=Qout[xx]/c[1];
			Q_c[2]=Q[xx+1]/c[2];
			Q_c[3]=Qout[xx+1]/c[3];

			//iksila
			iksila=(1.0f-(Q_c[0]+Q_c[1]+Q_c[2]+Q_c[3])/4/BAverage/J/delta_x)/2;

			C1=(K*iksila+0.5*delta_t)/(K*(1.0f-iksila)+0.5*delta_t);
			C2=(-K*iksila+0.5*delta_t)/(K*(1.0f-iksila)+0.5*delta_t);
			C3=(K*(1.0f-iksila)-0.5*delta_t)/(K*(1.0f-iksila)+0.5*delta_t);
			C4=(K*cAverage*delta_t)/(K*(1.0f-iksila)+0.5*delta_t);//wh added

			//新的Qout[t]
			iterQ=C1*Q[xx]+C2*Qout[xx]+C3*Q[xx+1]+C4*q;//wh modified

			//对零流量控制
			//20060310,李铁键,如果Q[t]<1e-10,那么直接下一时间循环
			//20070724,xiaofc,发生负反应直接截为0对后续计算的稳定性不利，同时需要考虑增加的保存BHV功能
			//因而水沙值采用入口t时刻值与出口t-1时刻的平均值，BHV用旧值
			//20070729,为了保持水量守衡，Qout[t]可能还是得置成极小
			if(iterQ<1e-10)
			{
				Qout[xx+1]=1e-10;//(Q[t]+Qout[t-1]/2.0f);//0.0f;
				/*if(SaveFlowPattern)
				{
					FlowB[t]=FlowB[t-1];
					FlowH[t]=FlowH[t-1];
					Flow_v[t]=Flow_v[t-1];
				}*/
				continue;
			}

			//if(isDebug)
				//myFile<<"c="<<c[0]<<"\tiksila="<<iksila<<"\tph/px="<<h_x<<"\tJ="<<J<<"\tQ="<<iterQ<<"\tIterCount:"<<IterCount2<<endl;//"\tIs DIP:"<<(Qout[t]-Qout[t-1])*(Q[t]-Q[t-1])<<endl;//float(delta_t)/K<<endl;
			

			IterCount=0;
			while( !(abs(iterQ-Qout[xx+1])<0.1 || abs(iterQ/Qout[xx+1]-1.0f)<0.005 || IterCount>MaxIter))//依上面的方法迭代，至Qout[xx+1]收敛
			{
				IterCount++;

				Qout[xx+1]=iterQ;

				h[3]=pow(n*Qout[xx+1]*1.5874f*pow(m*m+1.0f,0.33333333f)/pow(m,5.0f/3)/sqrt(S),0.375f);

				//求pian h / pian x
				hh=(h[0]+h[1]+h[2]+h[3])/4.0f;

				cold=cAverage;

				//20080811, xiaofc, 计算ph/px不采用Qout[xx+1]点的值，那样对收敛不利;
				//因为这里其他三个点的Q不更新，所以不用重算pQ/px.
				//Q_x=(Q[xx]+Qout[xx]-Q[xx+1]-Qout[xx+1])/2.0f/delta_x;
				h_x=Q_x/BAverage/cAverage;


				if(Q_x>0)
				{
					J=eta*h_x+S;
				}
				else
				{
					J=S;
				}

				J=max(J,nano);

				u[0]=pow(m*h[0],2.0f/3)*sqrt(J)/n/1.5874f/pow(m*m+1.0f,1.0f/3);
				u[1]=pow(m*h[1],2.0f/3)*sqrt(J)/n/1.5874f/pow(m*m+1.0f,1.0f/3);
				u[2]=pow(m*h[2],2.0f/3)*sqrt(J)/n/1.5874f/pow(m*m+1.0f,1.0f/3);
				u[3]=pow(m*h[3],2.0f/3)*sqrt(J)/n/1.5874f/pow(m*m+1.0f,1.0f/3);

				c[0]=u[0]*4.0/3.0;
				c[1]=u[1]*4.0/3.0;
				c[2]=u[2]*4.0/3.0;
				c[3]=u[3]*4.0/3.0;
				cAverage=(c[0]+c[1]+c[2]+c[3])/4;

				B[0]=m*h[0]*2.0;
				B[1]=m*h[1]*2.0;
				B[2]=m*h[2]*2.0;
				B[3]=m*h[3]*2.0;

				BAverage=4.0f*m*(Q[xx]+Qout[xx]+Q[xx+1]+Qout[xx+1])/3.0f/cAverage;
				BAverage=sqrt(BAverage);

				IterCount2=0;
				//20080811, xiaofc, 优化波速的收敛条件，加快计算速度
				while(!(abs(cold-cAverage)<0.001f || abs(cold/cAverage-1.0f)<0.001f) && IterCount2<MaxIter)
				{
					IterCount2++;
				
					h_x=Q_x/BAverage/cAverage;

					if(Q_x>0)
					{
						J=eta*h_x+S;
					}
					else
					{
						J=S;
					}

					J=max(J,nano);

					cold=cAverage;

					//20060317,李铁键,求C改用Newton迭代
					float fc,fcp;//F(c),F'(c)
					fc=1.0f/4.7622f/n*pow(m*m/(m*m+1),1.0f/3)*(pow(h[0],2.0f/3)+pow(h[1],2.0f/3)+pow(h[2],2.0f/3)+pow(h[3],2.0f/3))*sqrt(J)-cold;
					fcp=-1.0f/9.5244/n*pow(m*m/(m*m+1),1.0f/3)*(pow(h[0],2.0f/3)+pow(h[1],2.0f/3)+pow(h[2],2.0f/3)+pow(h[3],2.0f/3))/sqrt(J)*h_x/cold-1.0f;
					cAverage=cold-fc/fcp;
					
					//20070728,xiaofc,这个牛顿迭代也不一定总收敛，如果碰到cAverage<0的时候强行退出就麻烦了
					if(cAverage<0)
					{
						cAverage=cold;
						break;
					}

					if (abs(J-nano)<1e-10) 
					{
						break;
					}

				}//end of get c

				BAverage=4.0f*m*(Q[xx]+Qout[xx]+Q[xx+1]+Qout[xx+1])/3.0f/cAverage;
				BAverage=sqrt(BAverage);

				//20080811, xiaofc, 用ph/px修正K容易产生负反应，取消
				K=delta_x/cAverage;
				//K=K/(1-3.0f/32*h_x/S);
				//cout<<"K= "<<K<<endl;

				Q_c[0]=Q[xx]/c[0];
				Q_c[1]=Qout[xx]/c[1];
				Q_c[2]=Q[xx+1]/c[2];
				Q_c[3]=Qout[xx+1]/c[3];
				

				//iksila
				iksila=(1.0f-(Q_c[0]+Q_c[1]+Q_c[2]+Q_c[3])/4/BAverage/J/delta_x)/2;


				C1=(K*iksila+0.5*delta_t)/(K*(1.0f-iksila)+0.5*delta_t);//小
				C2=(-K*iksila+0.5*delta_t)/(K*(1.0f-iksila)+0.5*delta_t);
				C3=(K*(1.0f-iksila)-0.5*delta_t)/(K*(1.0f-iksila)+0.5*delta_t);
				C4=(K*cAverage*delta_t)/(K*(1.0f-iksila)+0.5*delta_t);//wh added
				
				iterQ=C1*Q[xx]+C2*Qout[xx]+C3*Q[xx+1]+C4*q;

				//20070724,xiaofc,发生负反应直接截为0对后续计算的稳定性不利，同时需要考虑增加的保存BHV功能
				//因而水沙值采用入口t时刻值与出口t-1时刻的平均值，BHV用旧值

				if(iterQ<1e-10)
				{
					Qout[xx+1]=1e-10;//(Qout[xx]+Q[xx+1]/2.0f);//0.0f;
					
					/*if(SaveFlowPattern)
					{
						FlowB[t]=FlowB[t-1];
						FlowH[t]=FlowH[t-1];
						Flow_v[t]=Flow_v[t-1];
					}*/

					//表示有负反应发生
					bNegtiveQHappened=true;
					break;
				}			

				//if(isDebug)
					//myFile<<"c="<<c[0]<<"\tiksila="<<iksila<<"\tph/px="<<h_x<<"\tJ="<<J<<"\tQ="<<iterQ<<"\tIterCount:"<<IterCount2<<endl;//"\tIs DIP:"<<(Qout[xx+1]-Q[xx+1])*(Qout[xx]-Q[xx])<<endl;//float(delta_t)/K<<endl;

			}//end of WHILE
			
			if(IterCount>=MaxIter && this->isDebug)
			{
				(*myFile)<<"warning: AVPM iteration overflow: "<<endl;
				//<<mBSCode.RegionIndex<<", "<<mBSCode.Value<<", "<<mBSCode.Length<<"."<<endl;
			}

			//20070722,以下负反应的处理方式会造成明显的水量的增加，因而废止
            //20060321,李铁键,恢复对负反应的控制，但改到迭代的最后，且增加对水深，从而是下一循环的流速的控制
			//if(iterQ-Q[xx+1]<0 && Qout[xx]-Q[xx]>1e-4)
			//{
			//	iterQ=Q[xx+1];
			//	if(iterQ>0)
			//		h[3]=pow(n*iterQ*1.5874f*pow(m*m+1.0f,0.33333333f)/pow(m,5.0f/3)/sqrt(S),0.375f);
			//}
			
			//20070724,xiaofc,如果在Qout[xx+1]迭代里遇见了负反应，则时间for循环continue
			if(bNegtiveQHappened)
			{
				continue;
			}

			Qout[xx+1]=iterQ;

			if(iterQ>1e6 || iterQ<-1e2 || !_finite(iterQ))
			{
				(*myFile)<<"warning:TimeStart:"<<FVMunit::TimeStart<<",(index,value,length):("<<HighSlopeRunoff::mBSCode.RegionIndex<<","<<HighSlopeRunoff::mBSCode.Value<<","<<HighSlopeRunoff::mBSCode.Length<<"):"<<"iterQ is out of range."<<endl;
				(*myFile)<<"iterQ:"<<iterQ<<",C1:"<<C1<<",Q[xx]:"<<Q[xx]<<",C2:"<<C2<<",Qout[xx]:"<<Qout[xx]<<",C3:"<<C3<<",Q[xx+1]:"<<Q[xx+1]<<",C4:"<<C4<<",q:"<<q<<endl;
			}

			//20070622, xiaofc, pass out the values of flow pattern
			/*if(SaveFlowPattern)
			{
				FlowB[t]=BAverage;
				FlowH[t]=(h[0]+h[1]+h[2]+h[3])/4.0f;
				Flow_v[t]=cAverage*0.75f;
			}*/

			//累积所有断面的水深
			hAverage+=(h[0]+h[1]+h[2]+h[3])/4.0f;

		}// end of t loop

	}//end of x loop

	//恢复源汇项q的值
	q=q0;

	//将计算值赋给Q，作为当前值
	for(int t=0;t<=xsteps;t++)
	{
		Q[t]=Qout[t];

		//cout<<"xsteps="<<t<<":"<<Q[t]<<",TimeStart:"<<FVMunit::TimeStart<<endl;
		if(!_finite(Q[t]) || Q[t]<-1e-2 || Q[t]>3e5)
		{
			if(t>0)
			{
				Q[t]=Q[t-1];
			}
		}
	}

	//输出末节点流量过程序列，以传递给下游
	for(int t=N1;t<=N2;t++)
	{
		Quout[t]=Q[xsteps];
	}

	//保存出口断面的水深、水面宽、流速值
	if(SaveFlowPattern)
	{
		for(int t=N1;t<=N2;t++)
		{
			if(Q[xsteps]<2e-10)
			{
				int tt=max(t-1,0);
				FlowB[t]=FlowB[tt];
				FlowH[t]=FlowH[tt];
				Flow_v[t]=Flow_v[tt];
				continue;
			}
			FlowB[t]=BAverage;
			FlowH[t]=(h[0]+h[1]+h[2]+h[3])/4.0f;
			Flow_v[t]=cAverage*0.75f;
		}
	}

	//得到平均断面水深及水头
	hAverage/=max(xsteps-1,1);
	H=hAverage-(gh/2/m); 
	H=max(-gm+1e-4,H);

}

	
////有限体积的ROE格式（高分辨率激波捕捉，时空二阶精度），以Q和A为变量
//void WaterInChannel::InChannel(float dt1)
//{
//	//N1,N2:写入Qout的第几项
//	int N1=0,N2=0;
//
//	//dt:时间步长,leftTime:剩余计算时间,Time:当前时间
//	float dt=dt1,leftTime=dt1,Time=FVMunit::TimeStart;
//	const float TimeStart=FVMunit::TimeStart;
//	
//	//质量守恒方程的源汇项m2/s
//	float q=0;
//
//	//当前时刻有限体积单元两侧的边界值
//	//第0个元素对应左边界，第1个元素对应右边界,每个边界都对应一个QL,QR,AL,AR
//	float QL[2],QR[2],AL[2],AR[2],BL[2],BR[2];
//	
//	//左右边界的波速,水速,水面宽等
//	float c[2],u[2],B[2],alfa1[2],alfa2[2],beta1[2],beta2[2],gama1[2],gama2[2];
//
//	//r:通量限制器的自变量,fr:通量限制器的值,a:边界过水面积,P:边界湿周,K:边界摩阻源项用到的谢才公式系数
//	float r,fr,a,P,K;
//
//	//边界通量及源汇项通量
//	float FQ[2],FA[2],SQ[2],SA[2];
//
//	//时间层循环
//	while(leftTime>0)
//	{
//		//dt=10;
//		//空间步长一定，确定时间步长
//		/*if(maxC>=30)
//		{
//			cout<<"warning:maxC="<<maxC<<"m/s,impossible?"<<endl;
//		}*/
//
//		//这里限制一下maxC，是因为如果接触maxC很大，比如无穷大，那么时间步长就无限小，程序死循环
//		/*if(maxC*leftTime>dx && maxC<30)
//		{
//			dt=0.60*dx/maxC;
//		}*/
//
//		leftTime-=dt;
//		Time+=dt;
//		
//		//cout<<"lefttime:"<<leftTime<<",dt:"<<dt<<endl;
//
//		//上边界条件
//		if(AreaS<1e-2)
//		{
//			//不是源坡面
//			Q[0]=Qu1[min(int(Time/MSTEP),Steps)]+Qu2[min(int(Time/MSTEP),Steps)];
//		}
//		else
//		{
//			//源河段没有上游来流
//			Q[0]=(*pS)[pS->size()-1].qs*(*pS)[pS->size()-1].Width;
//		}
//
//		if(Time<this->TimeEnd)
//		{
//			//沟道地下水(或者说是饱和土壤水)对明渠水的补给(m2/s)
//			q=W/(TimeEnd-TimeStart)/Length;
//
//			if(q>5)
//			{
//				int x=0;
//			}
//		}
//
//		//左右表层坡面对明渠的补给
//		q=q+(*pL)[pL->size()-1].qs+(*pR)[pR->size()-1].qs;
//
//		//空间层循环
//		for(int i=0;i<Xn;i++)
//		{
//			//*注：Xn-1个有限单元，Xn个边界，每个边界处都对应着一个L变量和R变量，也就是说用到的L和R是对应着同一个边界的，而不是有限单元的左边界和右边界
//			
//			/***********************************************计算XL[0]和XR[0]***********************************************/
//			//*注：QL[0],QR[0]等对应着i-1/2处，QL[1],QR[1]对应着i+1/2处
//			
//			if(i==0)
//			{
//				////因为用到了i-2点，所以i==0和1要特殊判断
//				//if(i<=1)
//				//{
//					QL[0]=max(Q[0],1e-10);
//					AL[0]=max(A[0],1e-7);
//				//}
//
//				////中间普通节点
//				//else
//				//{
//				//	//为了避免r分母为0，加上了1e-8
//				//	//这个实际是r(i-1)
//				//	r=(Q[i]-Q[i-1]+1e-8)/(Q[i-1]-Q[i-2]+1e-8);
//
//				//	//Van Albada通量限制器
//				//	fr=(pow(r,2)+r)/(1+pow(r,2));
//
//				//	QL[0]=Q[i-1]+0.5*fr*(Q[i-1]-Q[i-2]);
//
//				//	r=(A[i]-A[i-1]+1e-8)/(A[i-1]-A[i-2]+1e-8);
//				//	fr=(pow(r,2)+r)/(1+pow(r,2));
//				//	AL[0]=A[i-1]+0.5*fr*(A[i-1]-A[i-2]);
//				//}
//
//				//因为用到了i-1点，所以要特殊判断
//				//if(i==0)
//				//{
//					QR[0]=max(Q[0],1e-10);
//					AR[0]=max(A[0],1e-7);
//				//}
//				////因为用到了i+1点，所以要特殊判断
//				//else if(i==Xn-1)
//				//{
//				//	QR[0]=Q[Xn-1];
//				//	AR[0]=A[Xn-1];
//				//}
//				//else
//				//{
//				//	//为了避免r分母为0，加上了1e-8
//				//	//这个实际是1/r(i)
//				//	r=(Q[i]-Q[i-1]+1e-8)/(Q[i+1]-Q[i]+1e-8);
//
//				//	//Van Albada通量限制器
//				//	fr=(pow(r,2)+r)/(1+pow(r,2));
//
//				//	QR[0]=Q[i]-0.5*fr*(Q[i+1]-Q[i]);
//
//				//	r=(A[i]-A[i-1]+1e-8)/(A[i+1]-A[i]+1e-8);
//
//				//	//Van Albada通量限制器
//				//	fr=(pow(r,2)+r)/(1+pow(r,2));
//
//				//	AR[0]=A[i]-0.5*fr*(A[i+1]-A[i]);
//				//}
//				/***********************************************计算XL[0]和XR[0]***********************************************/
//
//				/************************************************计算辅助参数值************************************************/
//				//计算水面宽，体现断面形状的地方
//				if(AL[0]<=ST)
//				{
//					BL[0]=sqrt(2*max(AL[0],1e-7)*(1/tan(theta1)+1/tan(theta2))+pow(ef,2));
//				}
//				else
//				{
//					BL[0]=sqrt(2*(AL[0]-ST)*(1/tan(SlopeLA)+1/tan(SlopeRA))+pow(gh,2));//图中RS长度
//				}
//
//				if(AR[0]<=ST)
//				{
//					BR[0]=sqrt(2*max(AR[0],1e-7)*(1/tan(theta1)+1/tan(theta2))+pow(ef,2));
//				}
//				else
//				{
//					BR[0]=sqrt(2*(AR[0]-ST)*(1/tan(SlopeLA)+1/tan(SlopeRA))+pow(gh,2));
//				}
//
//				
//				if(A[0]>1e-1)
//				{
//					u[0]=(sqrt(AL[0])*AR[0]*QL[0]+sqrt(AR[0])*AL[0]*QR[0]+1e-8)/(AL[0]*AR[0]*(sqrt(AL[0])+sqrt(AR[0]))+1e-8);
//					c[0]=sqrt(9.8*cos(SlopeA)/2*(AL[0]/BL[0]+AR[0]/BR[0]));
//				}
//				else
//				{
//					c[0]=u[0]=1e-5;
//				}
//				
//				//下面的B[0]是边界处的水面宽取值，根据静水平衡条件，从表达式形式可以发现，类似于B=dA/dh
//				//B[0]=((AR[0]-AL[0])*c[0]+q*dx)/c[0]/tan(SlopeA)/dx;
//				B[0]=(BL[0]+BR[0])/2;
//
//				//界面处过水断面面积
//				a=0.5*(AL[0]/BL[0]+AR[0]/BR[0])*B[0];
//
//				//计算界面处湿周
//				if(AL[0]<=ST)
//				{
//					P=2*a/(ef+B[0])*(1/sin(theta1)+1/sin(theta2))+ef;
//				}
//				else
//				{
//					P=2*(a-ST)/(gh+B[0])*(1/sin(SlopeLA)+1/sin(SlopeRA))+gm*(1/sin(theta1)+1/sin(theta2))+ef;
//				}
//
//				//0都代表是左界面的值，1代表右界面的值
//				K=pow(a,5/3)/Manning/pow(P,2/3);
//				alfa1[0]=(QR[0]-QL[0]+(c[0]-u[0])*(AR[0]-AL[0]))/2/c[0];
//				alfa2[0]=AR[0]-AL[0]-alfa1[0];
//				gama1[0]=-pow(c[0],3)*(BR[0]-BL[0])/4/9.8/cos(SlopeA);
//				gama2[0]=-gama1[0];
//
//				//这里认为Q^=A^*u^,q^=q
//				beta1[0]=gama1[0]+B[0]*c[0]*tan(SlopeA)*dx/2-9.8*a*a*u[0]*abs(a*u[0])*dx/2/c[0]/pow(K,2)-q*dx*(u[0]-c[0])/2/c[0];
//				beta2[0]=q*dx-beta1[0];
//
//			}//end if(i==0)
//			/************************************************计算辅助参数值************************************************/
//
//			/***********************************************计算XL[1]和XR[1]***********************************************/
//			//注:下面对应的是i+1/2边界处
//			
//			//因为r用到了i-1节点，所以要特殊判断
//			if(i==0)
//			{
//				QL[1]=Q[0];
//				AL[1]=A[0];
//			}
//
//			//因为r用到了i+1节点，所以要特殊判断
//			else if(i==Xn-1)
//			{
//				QL[1]=Q[Xn-1];
//				AL[1]=A[Xn-1];
//			}
//
//			//中间普通节点
//			else
//			{
//				//对应r(i)
//				r=(Q[i+1]-Q[i]+1e-8)/(Q[i]-Q[i-1]+1e-8);
//				fr=(pow(r,2)+r)/(1+pow(r,2));
//				QL[1]=Q[i]+0.5*fr*(Q[i]-Q[i-1]);
//
//				if(QL[1]<-1 || QL[1]>1e4)
//				{
//					cout<<"i:"<<i<<","<<Q[i-1]<<","<<Q[i]<<","<<Q[i+1]<<endl;
//					int x=0;
//				}
//
//				r=(A[i+1]-A[i]+1e-8)/(A[i]-A[i-1]+1e-8);
//				fr=(pow(r,2)+r)/(1+pow(r,2));
//				AL[1]=A[i]+0.5*fr*(A[i]-A[i-1]);
//
//				if(AL[1]<-1 || AL[1]>1e4)
//				{
//					cout<<"i:"<<i<<","<<A[i-1]<<","<<A[i]<<","<<A[i+1]<<endl;
//					int x=0;
//				}
//			}
//			
//			//处理最后两个节点右边界的QR和AR，因为出现了i+2，所以对最后两个节点要特殊判断
//			if(i>=Xn-2)
//			{
//				QR[1]=max(Q[Xn-1],1e-6);
//				AR[1]=max(A[Xn-1],1e-6);
//			}
//			else
//			{
//				//对应1/r(i+1)
//				r=(Q[i+1]-Q[i]+1e-8)/(Q[i+2]-Q[i+1]+1e-8);
//				fr=(pow(r,2)+r)/(1+pow(r,2));
//				QR[1]=Q[i+1]-0.5*fr*(Q[i+2]-Q[i+1]);
//
//				if(QR[1]<-1 || QR[1]>1e4)
//				{
//					cout<<"i:"<<i<<","<<Q[i]<<","<<Q[i+1]<<","<<Q[i+2]<<endl;
//					int x=0;
//				}
//
//				r=(A[i+1]-A[i]+1e-8)/(A[i+2]-A[i+1]+1e-8);
//				fr=(pow(r,2)+r)/(1+pow(r,2));
//				AR[1]=A[i+1]-0.5*fr*(A[i+2]-A[i+1]);
//
//				if(AR[1]<-1 || AR[1]>1e4)
//				{
//					cout<<"i:"<<i<<","<<A[i]<<","<<A[i+1]<<","<<A[i+2]<<endl;
//					int x=0;
//				}
//			}
//			/***********************************************计算XL[1]和XR[1]***********************************************/
//			
//			/************************************************计算辅助参数值************************************************/
//			//计算水面宽，体现断面形状的地方
//			if(AL[1]<=ST)
//			{
//				BL[1]=sqrt(2*max(AL[1],1e-7)*(1/tan(theta1)+1/tan(theta2))+pow(ef,2));
//			}
//			else
//			{
//				BL[1]=sqrt(2*(AL[1]-ST)*(1/tan(SlopeLA)+1/tan(SlopeRA))+pow(gh,2));//图中RS长度
//			}
//
//			if(AR[1]<=ST)
//			{
//				BR[1]=sqrt(2*max(AR[1],1e-7)*(1/tan(theta1)+1/tan(theta2))+pow(ef,2));
//			}
//			else
//			{
//				BR[1]=sqrt(2*(AR[1]-ST)*(1/tan(SlopeLA)+1/tan(SlopeRA))+pow(gh,2));
//			}
//
//			//界面参变量值
//
//			//注：发现流速很难控制！
//			if(AR[1]>1e-1 && AL[1]>1e-1)
//			{
//				u[1]=(sqrt(abs(AL[1]))*AR[1]*QL[1]+sqrt(abs(AR[1]))*AL[1]*QR[1])/(AL[1]*AR[1]*(sqrt(abs(AL[1]))+sqrt(abs(AR[1]))));
//				c[1]=sqrt(abs(9.8*cos(SlopeA)/2*(AL[1]/BL[1]+AR[1]/BR[1])));
//			}
//			else
//			{
//				c[1]=u[1]=1e-5;
//			}
//			
//			if(u[1]>10)
//			{
//				int u1,u2;
//				u1=QL[1]/AL[1];
//				u2=QR[1]/AR[1];
//				cout<<"u1:"<<u1<<",u2:"<<u2<<endl;
//			}
//
//		
//			//B[1]=(abs(AR[1]-AL[1])*c[1]+q*dx)/c[1]/tan(SlopeA)/dx;
//			B[1]=(BL[1]+BR[1])/2;
//
//			//界面处过水断面面积
//			a=0.5*(AL[1]/BL[1]+AR[1]/BR[1])*B[1];
//
//			//计算界面处湿周
//			if(AL[1]<=ST)
//			{
//				P=2*a/(ef+B[1])*(1/sin(theta1)+1/sin(theta2))+ef;
//			}
//			else
//			{
//				P=2*(a-ST)/(gh+B[1])*(1/sin(SlopeLA)+1/sin(SlopeRA))+gm*(1/sin(theta1)+1/sin(theta2))+ef;
//			}
//
//			K=pow(a,5/3)/Manning/pow(P,2/3);
//			alfa1[1]=(QR[1]-QL[1]+(c[1]-u[1])*(AR[1]-AL[1]))/2/c[1];//量纲：m2
//			alfa2[1]=AR[1]-AL[1]-alfa1[1];
//			gama1[1]=-pow(c[1],3)*(BR[1]-BL[1])/4/9.8/cos(SlopeA);//量纲：m3/s
//			gama2[1]=-gama1[1];
//			beta1[1]=gama1[1]+B[1]*c[1]*tan(SlopeA)*dx/2-9.8*a*a*u[1]*abs(a*u[1])*dx/2/c[1]/pow(K,2)-q*dx*(u[1]-c[1])/2/c[1];//量纲：m3/s
//			beta2[1]=q*dx-beta1[1];
//			/************************************************计算辅助参数值************************************************/
//			
//			/**********************************************计算左右界面通量值**********************************************/
//			//左界面通量"F(i-1/2)+","S(i-1/2)+"
//			//被注释掉的FA[0]和FQ[0]与被采用的完全等价，只是代入了gama1和gama2之间的关系
//
//			//FA[0]=alfa1[0]*(u[0]+c[0]+abs(u[0]+c[0]))+alfa2[0]*(u[0]-c[0]+abs(u[0]-c[0]))+0.5*(gama1[0]+gama2[0])*(1+Sign(u[0]+c[0]));
//			FA[0]=alfa1[0]*(u[0]+c[0]+abs(u[0]+c[0]))+alfa2[0]*(u[0]-c[0]+abs(u[0]-c[0]));
//
//			//FQ[0]=alfa1[0]*(u[0]+c[0])*(u[0]+c[0]+abs(u[0]+c[0]))+alfa2[0]*(u[0]-c[0])*(u[0]-c[0]+abs(u[0]-c[0]))+0.5*(gama1[0]*(u[0]+c[0])+gama2[0]*(u[0]-c[0]))*(1+Sign(u[0]-c[0]));
//			FQ[0]=alfa1[0]*(u[0]+c[0])*(u[0]+c[0]+abs(u[0]+c[0]))+alfa2[0]*(u[0]-c[0])*(u[0]-c[0]+abs(u[0]-c[0]))+gama1[0]*c[0]*(1+Sign(u[0]-c[0]));
//
//			SA[0]=0.5*(beta1[0]*(1+Sign(u[0]+c[0]))+beta2[0]*(1+Sign(u[0]-c[0])));
//			SQ[0]=0.5*(beta1[0]*(u[0]+c[0])*(1+Sign(u[0]+c[0]))+beta2[0]*(u[0]-c[0])*(1+Sign(u[0]-c[0])));
//
//			//右界面通量"F(i+1/2)-","S(i+1/2)-"
//			//被注释掉的FA[1]和FQ[1]与被采用的完全等价，只是代入了gama1和gama2之间的关系
//
//			//FA[1]=alfa1[1]*(u[1]+c[1]-abs(u[1]+c[1]))+alfa2[1]*(u[1]-c[1]-abs(u[1]-c[1]))+0.5*(gama1[1]+gama2[1])*(1-Sign(u[1]+c[1]));
//			FA[1]=alfa1[1]*(u[1]+c[1]-abs(u[1]+c[1]))+alfa2[1]*(u[1]-c[1]-abs(u[1]-c[1]));
//
//			//FQ[1]=alfa1[1]*(u[1]+c[1])*(u[1]+c[1]-abs(u[1]+c[1]))+alfa2[1]*(u[1]-c[1])*(u[1]-c[1]-abs(u[1]-c[1]))+0.5*(gama1[1]*(u[1]+c[1])+gama2[1]*(u[1]-c[1]))*(1-Sign(u[1]-c[1]));
//			FQ[1]=alfa1[1]*(u[1]+c[1])*(u[1]+c[1]-abs(u[1]+c[1]))+alfa2[1]*(u[1]-c[1])*(u[1]-c[1]-abs(u[1]-c[1]))+gama1[1]*c[1]*(1-Sign(u[1]-c[1]));
//
//			SA[1]=0.5*(beta1[1]*(1-Sign(u[1]+c[1]))+beta2[1]*(1-Sign(u[1]-c[1])));
//			SQ[1]=0.5*(beta1[1]*(u[1]+c[1])*(1-Sign(u[1]+c[1]))+beta2[1]*(u[1]-c[1])*(1-Sign(u[1]-c[1])));
//			/**********************************************计算左右界面通量值**********************************************/
//			
//			/**********************************************得到下一时刻的变量**********************************************/
//			//注意：写到A2和Q2里面
//			A2[i]=A[i]-(FA[0]+FA[1]-SA[0]-SA[1])*dt/dx;
//			Q2[i]=Q[i]-(FQ[0]+FQ[1]-SQ[0]-SQ[1])*dt/dx;
//
//			if(Q2[i]>1e1 || A2[i]>1e1 || Q2[i]<-1e1 || A2[i]<-1e1)
//			{
//				cout<<endl<<"********************"<<endl;
//				cout<<"maxC:"<<maxC<<",dt:"<<dt<<",A:"<<A2[i]<<",Q:"<<Q2[i];
//				cout<<endl<<"********************"<<endl;
// 				int x=0;
//			}
//
//			/*if(Q2[i]>1e5 || A2[i]>1e5 || Q2[i]<-1e5 || A2[i]<-1e5)
//			{
//				cout<<endl<<"********************"<<endl;
//				cout<<"maxC:"<<maxC<<",dt:"<<dt<<",A:"<<A2[i]<<",Q:"<<Q2[i];
//				cout<<endl<<"********************"<<endl;
//				int x=0;
//			}*/
//
//			//强制赋值可能会引起数值稳定性
//			//if(A2[i]<1e-7 /*&& A2[i]>-10*/)  { A2[i]=1e-7;  }
//			//if(Q2[i]<1e-10 /*&& Q[i]>-10*/)  { Q2[i]=1e-10; }
//
//			cout<<"A:"<<A2[i]<<",Q:"<<Q2[i]<<endl;
//			if(i==Xn-1){ cout<<endl;}
//			
//			c[0]=c[1]; 
//			u[0]=u[1]; 
//			B[0]=B[1];
//
//			QL[0]=QL[1]; QR[0]=QR[1];
//			AL[0]=AL[1]; AR[0]=AR[1];
//			BL[0]=BL[1]; BR[0]=BR[1];
//			
//			alfa1[0]=alfa1[1]; alfa2[0]=alfa2[1];
//			beta1[0]=beta1[1]; beta2[0]=beta2[1];
//			gama1[0]=gama1[1]; gama2[0]=gama2[1];
//			
//	
//			//存储到上下游河段间的接口变量中
//			if(i==Xn-1)
//			{
//				N1=min(int((Time-dt)/MSTEP),Steps);
//				N2=min(int(Time/MSTEP)+1,Steps);
//				for(int n=N1;n<=N2;n++)
//				{
//					Quout[n]=Q[i];//元流域出口流量
//
//					if(Quout[n]>1e5 || Quout[n]<-10)
//					{
//						cout<<"Quout:"<<Quout[n]<<endl;
//					}
//					
//					//在要求保存的时候再计算
//					if(SaveFlowPattern)
//					{
//						if(A[i]>1e-5)
//						{
//							FlowV[n]=max(Q[i]/A[i],0);
//						}
//						else
//						{
//							FlowV[n]=0.0;
//						}
//
//						if(A[i]<=ST)
//						{
//							FlowB[n]=sqrt(2*A[i]*(1/tan(theta1)+1/tan(theta2))+pow(ef,2));
//							FlowH[n]=2*A[i]/(FlowB[n]+ef);
//						}
//						else
//						{
//							FlowB[n]=sqrt(2*(A[i]-ST)*(1/tan(SlopeLA)+1/tan(SlopeRA))+pow(gh,2));//图中RS长度
//							FlowH[n]=gm+2*(A[i]-ST)/(FlowB[n]+gh);
//						}
//					}
//				}
//			}
//
//			
//			//如果不对Q和A作限制，当Q和A都很小的时候，可能会出现maxC很大的情况，这时的时间步长dt就很小了。
//			if(Q[i]>1e-3 && A[i]>1e-1)
//			{
//				//绝对波速，用来调整时间步长
//				//当前水面宽B
//				float B=1e-6;
//
//				//根据A的大小求水面宽B
//				if(A[i]<=ST)
//				{
//					B=sqrt(2*max(A[i],1e-7)*(1/tan(theta1)+1/tan(theta2))+pow(ef,2));
//				}
//				else
//				{
//					B=sqrt(2*(A[i]-ST)*(1/tan(SlopeLA)+1/tan(SlopeRA))+pow(gh,2));//图中RS长度
//				}
//
//				//求当前最大绝对波速
//				if(A[i]<1e-5)
//				{
//					maxC=max(maxC,abs(Q[i]/A[i]));
//				}
//				else
//				{
//					maxC=max(maxC,abs(Q[i]/A[i]+sqrt(9.8*A[i]/B)));
//				}
//			}
//
//		}//end x
//
//		//用t+1时刻的值更新t时刻的值
//		for(int k=0;k<Xn;k++)
//		{
//			Q[k]=Q2[k];
//			A[k]=A2[k];
//		}
//
//	}//end t
//
//	//更新相对水头H
//	H=0;
//	for(int i=0;i<Xn;i++)
//	{
//		if(A[i]<=ST)
//		{
//			H+=2*max(A[i],1e-6)/(sqrt(2*max(A[i],1e-6)*(1/tan(theta1)+1/tan(theta2))+pow(ef,2))+ef);
//		}
//		else
//		{
//			H+=gm+2*(max(A[i],1e-6)-ST)/(sqrt(2*(max(A[i],1e-6)-ST)*(1/tan(SlopeLA)+1/tan(SlopeRA))+pow(gh,2))+gh);
//		}
//	}
//	H/=Xn;//平均水深
//
//	if(H>40 || H<1e-8)
//	{
//		int x=0;
//	}
//
//	if(H<=-1e-4)
//	{
//		cout<<FVMunit::Year<<"."<<FVMunit::Month<<"."<<FVMunit::Day<<"."<<FVMunit::Hour<<",明渠负水深:"<<H<<endl;
//		H=0;
//	}
//
//	H-=gm;//平均水头
//}

//符号函数
//int WaterInChannel::Sign(float x)
//{
//	if(x>1e-4) return 1;
//	if(x<1e-4) return -1;
//	return 0;
//}


//将李铁键的精确扩散波法改装于此，精确扩散波只有一个未知量Q，ROE格式得传递A和Q两个过程序列，而且时间步长比较受限制
