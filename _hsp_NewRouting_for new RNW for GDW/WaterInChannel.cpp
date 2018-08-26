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

//��ǰ�Ӷμ���ʼĩֻ��ʼ��һ��
void WaterInChannel::Initiallize(Para* mPara0, float MSTEP0,int Steps0,float gh0,float gm0,float ef0,float theta10,float theta20,float* Qu10,float* Qu20,float* Quout0,vector<FVMunit>* pL0,vector<FVMunit>* pR0,vector<FVMunit>* pS0,float* FlowB0,float* FlowH0,float* FlowV0,bool SaveFlowPattern0,ofstream* file)
{
	//������������Ϣ
	mPara=mPara0;
	gh=gh0;
	gm=gm0;
	ef=ef0; 
	theta1=theta10;
	theta2=theta20;

	//ʱ������������ܲ�����
	MSTEP=MSTEP0;//s
	Steps=Steps0;

	//������Ϣ����
	Qu1=Qu10;
	Qu2=Qu20;         
	
	//�����Ϣ����
	Quout=Quout0;
	FlowB=FlowB0;
	FlowH=FlowH0;
	Flow_v=FlowV0;

	//�������������Ԫָ��
	pL=pL0;
	pR=pR0;
	pS=pS0;

	SaveFlowPattern=SaveFlowPattern0;
	isDebug=HighSlopeRunoff::debug;

	//�����ļ����
	this->myFile=file;
	
	//��ʼˮλ�����ں͵���ˮ����
	H=-gm+1e-6;
	q=0;

	//ˮ������
	eta=0.75f;

	//���ζ����ƽ������ϵ�����߽ǵ�����ֵ��
	m=(1/tan(theta1)+1/tan(theta2))/2;

	n=n0=mPara->RiverManning;
	L=mPara->StreamLength;
	
	//���ƺӵ�����С������¶�
	nano=0.0004f;
	S=mPara->StreamSlope;
    S=min(max(S,nano),0.4);
	
	//20060907,xiaofc,�Կ�����Ϊ1�����������Զ��ռ䲽��
	float MaxQ=0.0f;//ʱ���ڵ��������
	float MaxC;//��Ӧ��������������

	Time=FVMunit::TimeStart;
	delta_t=FVMunit::dt1;//ʱ�䲽����s

	//���for�ǽ�����������ϵĴ���
	for(int t=0;t<=Steps;t++)
	{
		if(Qu1[t]+Qu2[t]>MaxQ) 
		{
			MaxQ=Qu1[t]+Qu2[t];
		}
	}

	//20060906,xiaofc,�Զ��󲽳���
	if(MaxQ>1.0f)
	{
		//wh�����ͨ���������٣����湫ʽ�����������ζ��棬������֤��û������ġ�
		//wh����͵������ζ���ɣ�������ζ��滹��ţ�ٵ�����̫�鷳��
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

	//�ռ䲽��m
	delta_x=L/xsteps;

	//�����ڴ�ռ�
	//wh:xsteps������xsteps+1���ڵ�
	Q=new float[xsteps+1];
	Qout=new float[xsteps+1];

	//��Ϊ��InChannel���������Q[0]���и�ֵ��������������
	Q[0]=Qu1[0]+Qu2[0];

	::ZeroFill(Q,xsteps+1);
	::ZeroFill(Qout,xsteps+1);
}


//��ȷ��ɢ����
void WaterInChannel::InChannel(void)
{
	Time=FVMunit::TimeStart+delta_t;
	
	//�ϱ߽�����
	//����Դ����
	if(mPara->AreaS<0)
	{
		Qout[0]=Qu1[min(int(Time/MSTEP),Steps)]+Qu2[min(int(Time/MSTEP),Steps)];
	}
	//Դ�Ӷ�û����������
	else if(mPara->AreaS>0 && pS->size()>0)
	{
		Qout[0]=(*pS)[pS->size()-1].qs*(*pS)[pS->size()-1].Width;
	}
	else if(mPara->AreaS>0 && pS->size()<=0)
	{
		//��������ѩ�����ˣ�����Ϊ0
		Qout[0]=0.0;
	}

	Qout[0]=max(Qout[0],1e-10);

	//����Դ����������������������������Ĳ���
	//��Ϊ�������Ĳ�������dt1ʱ���ڵģ���q����ֵ���ڰ������ɸ�dt1ʱ���С��vdtʱ���ڵ�,
	//��ÿ����ͬ��dt1��qsֵ��ͬ��������Ҫ�ָ�Ϊq0
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

	int N1,N2;//д��Quout�еĵڼ���
	N1=min(int((Time-delta_t)/MSTEP),Steps);
    N2=min(int((Time)/MSTEP),Steps);

	//200512,������
	//���seglength,�������е�L<=0,Ϊ����Ӷ�
	//�򲻼��㣬ֱ�Ӱ���ڸ��Ƶ����ڣ�return
	if(L<1e-5)
	{
		//��ʱ��һ���ռ䲽����������Q[0]��Q[1]�Ĵ���
		Q[0]=Q[1]=Qout[0];
		for(int t=N1;t<=N2;t++)
		{
			Quout[t]=Qout[0];
		}
		return;
	}

	//����������

	//�Ӷβ���
	int MaxIter=50;
	int IterCount=0;
	int IterCount2=0;

	//�����ϵ��
	float c[4];	
	float Q_c[4];

	//IVPMC��VPMC������֮��
	float hh;  //ƽ��ˮ��
	float h_x; //pian h / pian x
	float Q_x; //pQ/px
    float J; //����ph/px���ڵ�����
	float cold;
	float cAverage;
	float BAverage;

	float K;
	float iksila;

	float C1;
	float C2;
	float C3;
	float C4;//wh,����Դ����

	float iterQ;//ÿ��������ϵ����õ���Q

	//20070724,xiaofc,��һ���Ƿ��Qout[t]��ѭ������������Ӧ��������
	bool bNegtiveQHappened=false;

	//wh:���ж���ˮ���ƽ��ֵm
	float hAverage=0.0;

	/*if(FVMunit::TimeStart==259200)
	{
		int xxx=0;
	}*/

	//wh����Ϊ�õ�j+1�ڵ�,����xx<=xsteps-1
	for(int xx=0;xx<=xsteps-1;xx++)//�ռ䲽��ѭ��
	{
		//Qout[xx+1]=Q[xx];//��һ��ʱ��㲻���м���
		
		//20070828,xiaofc,Q��С��ʱ��Ҫ�����
		//20080401,xiaofc,�Ľ�
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

        //����������ˮ�л�ź�������ʽ�� 
		//h[1]��Ӧ(i,n),h[0]��Ӧ(i,n-1)
		h[0]=pow(n*Q[xx]*1.5874f*pow(m*m+1.0f,0.33333333f)/pow(m,5.0f/3)/sqrt(S),0.375f);

		//h[3]��Ӧ(i+1,n),h[2]��Ӧ(i+1,n-1)
		h[2]=pow(n*Q[xx+1]*1.5874f*pow(m*m+1.0f,0.33333333f)/pow(m,5.0f/3)/sqrt(S),0.375f);
		
		//20060310,������,h[1],h[3]�ĳ�ʼֵ������0,���ⱻ��0
		if(h[0]<1e-10)
		{
			h[0]=1e-10;
		}
		if(h[2]<1e-10)
		{
			h[2]=1e-10;
		}

		h_x=0.0f; //pian h /pian x�ȸ�0

		//���ֻ����һ��ʱ�䲽��	
		for(int t=1;t<2;t++)
		{
		    //Qout[xx]��Q[xx]����һʱ�̵�ֵ��ͬһ������㣩
			if(Qout[xx]<mPara->DrainingArea/1000000000*1 && n0<0.5  && mPara->DrainingArea>3000000000)
			{
				n=0.5-(0.5-n0)*Qout[xx]/1*1000000000/mPara->DrainingArea;
			}
			else
			{
				n=n0;
			}

			//20060310,������,���Q[t]<1e-5,��ôֱ����һʱ��ѭ��
			if(Qout[xx]<1e-10)
			{
				Qout[xx+1]=Qout[xx];//0.0f;
				continue;
			}

			//Qout[t]=Qout[t-1];
			Qout[xx+1]=Qout[xx];//��Qt��Qout t-1 ����������Щ
	

			//wh:0,1,2,3��˳�����£�0:(i,n-1),1:(i,n),2:(i+1,n-1),3:(i+1,n),0��1�ǵ�ǰ�ڵ�i��n�ǵ�ǰʱ�̣�n-1��ǰһʱ��
			h[1]=pow(n*Qout[xx]*1.5874f*pow(m*m+1.0f,0.33333333f)/pow(m,5.0f/3)/sqrt(S),0.375f);
			h[3]=pow(n*Qout[xx+1]*1.5874f*pow(m*m+1.0f,0.33333333f)/pow(m,5.0f/3)/sqrt(S),0.375f);

			
			//��pian h / pian x
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

			//20080811, xiaofc, ����ph/px������Qout[t]���ֵ����������������
			//Q_x=(Q[t-1]+Q[t]-Qout[t-1]-Qout[t])/2.0f/delta_x;
			Q_x=((Q[xx]+Qout[xx])/2.0f-Q[xx+1])/delta_x;
			h_x=Q_x/BAverage/cAverage;


			//��ˮ���
			if(Q_x>0)
			{
				J=eta*h_x+S;
			}
			//��ˮ���
			else
			{
				J=S;
			}
			

			//������С�Ƚ������ƣ���ɢ�����²���̫��
			J=max(J,nano);

			//�����ø��º��J(J=J0+eta*h_x)��ˢ��һ���������ֵ
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

			BAverage=4.0f*m*(Q[xx]+Qout[xx]+Q[xx+1]+Qout[xx+1])/3.0f/cAverage;//��ȷ.
			BAverage=sqrt(BAverage);

			IterCount2=0;
			//20080811, xiaofc, �Ż����ٵ������������ӿ�����ٶ�
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

				//20060317,������,��C����Newton����
				float fc,fcp;//F(c),F'(c)

				//��ȷ�����Ǹ���л�Ź�ʽ������(c=4/3*u)��ֻ��J�л���Cһ��
				fc=1.0f/4.7622f/n*pow(m*m/(m*m+1),1.0f/3)*(pow(h[0],2.0f/3)+pow(h[1],2.0f/3)+pow(h[2],2.0f/3)+pow(h[3],2.0f/3))*sqrt(J)-cold;
				fcp=-1.0f/9.5244/n*pow(m*m/(m*m+1),1.0f/3)*(pow(h[0],2.0f/3)+pow(h[1],2.0f/3)+pow(h[2],2.0f/3)+pow(h[3],2.0f/3))/sqrt(J)*h_x/cold-1.0f;
				cAverage=cold-fc/fcp;

				//20070728,xiaofc,���ţ�ٵ���Ҳ��һ�����������������cAverage<0��ʱ��ǿ���˳����鷳��
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

			//20080811, xiaofc, ��ph/px����K���ײ�������Ӧ��ȡ��
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

			//�µ�Qout[t]
			iterQ=C1*Q[xx]+C2*Qout[xx]+C3*Q[xx+1]+C4*q;//wh modified

			//������������
			//20060310,������,���Q[t]<1e-10,��ôֱ����һʱ��ѭ��
			//20070724,xiaofc,��������Ӧֱ�ӽ�Ϊ0�Ժ���������ȶ��Բ�����ͬʱ��Ҫ�������ӵı���BHV����
			//���ˮɳֵ�������tʱ��ֵ�����t-1ʱ�̵�ƽ��ֵ��BHV�þ�ֵ
			//20070729,Ϊ�˱���ˮ���غ⣬Qout[t]���ܻ��ǵ��óɼ�С
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
			while( !(abs(iterQ-Qout[xx+1])<0.1 || abs(iterQ/Qout[xx+1]-1.0f)<0.005 || IterCount>MaxIter))//������ķ�����������Qout[xx+1]����
			{
				IterCount++;

				Qout[xx+1]=iterQ;

				h[3]=pow(n*Qout[xx+1]*1.5874f*pow(m*m+1.0f,0.33333333f)/pow(m,5.0f/3)/sqrt(S),0.375f);

				//��pian h / pian x
				hh=(h[0]+h[1]+h[2]+h[3])/4.0f;

				cold=cAverage;

				//20080811, xiaofc, ����ph/px������Qout[xx+1]���ֵ����������������;
				//��Ϊ���������������Q�����£����Բ�������pQ/px.
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
				//20080811, xiaofc, �Ż����ٵ������������ӿ�����ٶ�
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

					//20060317,������,��C����Newton����
					float fc,fcp;//F(c),F'(c)
					fc=1.0f/4.7622f/n*pow(m*m/(m*m+1),1.0f/3)*(pow(h[0],2.0f/3)+pow(h[1],2.0f/3)+pow(h[2],2.0f/3)+pow(h[3],2.0f/3))*sqrt(J)-cold;
					fcp=-1.0f/9.5244/n*pow(m*m/(m*m+1),1.0f/3)*(pow(h[0],2.0f/3)+pow(h[1],2.0f/3)+pow(h[2],2.0f/3)+pow(h[3],2.0f/3))/sqrt(J)*h_x/cold-1.0f;
					cAverage=cold-fc/fcp;
					
					//20070728,xiaofc,���ţ�ٵ���Ҳ��һ�����������������cAverage<0��ʱ��ǿ���˳����鷳��
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

				//20080811, xiaofc, ��ph/px����K���ײ�������Ӧ��ȡ��
				K=delta_x/cAverage;
				//K=K/(1-3.0f/32*h_x/S);
				//cout<<"K= "<<K<<endl;

				Q_c[0]=Q[xx]/c[0];
				Q_c[1]=Qout[xx]/c[1];
				Q_c[2]=Q[xx+1]/c[2];
				Q_c[3]=Qout[xx+1]/c[3];
				

				//iksila
				iksila=(1.0f-(Q_c[0]+Q_c[1]+Q_c[2]+Q_c[3])/4/BAverage/J/delta_x)/2;


				C1=(K*iksila+0.5*delta_t)/(K*(1.0f-iksila)+0.5*delta_t);//С
				C2=(-K*iksila+0.5*delta_t)/(K*(1.0f-iksila)+0.5*delta_t);
				C3=(K*(1.0f-iksila)-0.5*delta_t)/(K*(1.0f-iksila)+0.5*delta_t);
				C4=(K*cAverage*delta_t)/(K*(1.0f-iksila)+0.5*delta_t);//wh added
				
				iterQ=C1*Q[xx]+C2*Qout[xx]+C3*Q[xx+1]+C4*q;

				//20070724,xiaofc,��������Ӧֱ�ӽ�Ϊ0�Ժ���������ȶ��Բ�����ͬʱ��Ҫ�������ӵı���BHV����
				//���ˮɳֵ�������tʱ��ֵ�����t-1ʱ�̵�ƽ��ֵ��BHV�þ�ֵ

				if(iterQ<1e-10)
				{
					Qout[xx+1]=1e-10;//(Qout[xx]+Q[xx+1]/2.0f);//0.0f;
					
					/*if(SaveFlowPattern)
					{
						FlowB[t]=FlowB[t-1];
						FlowH[t]=FlowH[t-1];
						Flow_v[t]=Flow_v[t-1];
					}*/

					//��ʾ�и���Ӧ����
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

			//20070722,���¸���Ӧ�Ĵ���ʽ��������Ե�ˮ�������ӣ������ֹ
            //20060321,������,�ָ��Ը���Ӧ�Ŀ��ƣ����ĵ���������������Ӷ�ˮ��Ӷ�����һѭ�������ٵĿ���
			//if(iterQ-Q[xx+1]<0 && Qout[xx]-Q[xx]>1e-4)
			//{
			//	iterQ=Q[xx+1];
			//	if(iterQ>0)
			//		h[3]=pow(n*iterQ*1.5874f*pow(m*m+1.0f,0.33333333f)/pow(m,5.0f/3)/sqrt(S),0.375f);
			//}
			
			//20070724,xiaofc,�����Qout[xx+1]�����������˸���Ӧ����ʱ��forѭ��continue
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

			//�ۻ����ж����ˮ��
			hAverage+=(h[0]+h[1]+h[2]+h[3])/4.0f;

		}// end of t loop

	}//end of x loop

	//�ָ�Դ����q��ֵ
	q=q0;

	//������ֵ����Q����Ϊ��ǰֵ
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

	//���ĩ�ڵ������������У��Դ��ݸ�����
	for(int t=N1;t<=N2;t++)
	{
		Quout[t]=Q[xsteps];
	}

	//������ڶ����ˮ�ˮ�������ֵ
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

	//�õ�ƽ������ˮ�ˮͷ
	hAverage/=max(xsteps-1,1);
	H=hAverage-(gh/2/m); 
	H=max(-gm+1e-4,H);

}

	
////���������ROE��ʽ���߷ֱ��ʼ�����׽��ʱ�ն��׾��ȣ�����Q��AΪ����
//void WaterInChannel::InChannel(float dt1)
//{
//	//N1,N2:д��Qout�ĵڼ���
//	int N1=0,N2=0;
//
//	//dt:ʱ�䲽��,leftTime:ʣ�����ʱ��,Time:��ǰʱ��
//	float dt=dt1,leftTime=dt1,Time=FVMunit::TimeStart;
//	const float TimeStart=FVMunit::TimeStart;
//	
//	//�����غ㷽�̵�Դ����m2/s
//	float q=0;
//
//	//��ǰʱ�����������Ԫ����ı߽�ֵ
//	//��0��Ԫ�ض�Ӧ��߽磬��1��Ԫ�ض�Ӧ�ұ߽�,ÿ���߽綼��Ӧһ��QL,QR,AL,AR
//	float QL[2],QR[2],AL[2],AR[2],BL[2],BR[2];
//	
//	//���ұ߽�Ĳ���,ˮ��,ˮ����
//	float c[2],u[2],B[2],alfa1[2],alfa2[2],beta1[2],beta2[2],gama1[2],gama2[2];
//
//	//r:ͨ�����������Ա���,fr:ͨ����������ֵ,a:�߽��ˮ���,P:�߽�ʪ��,K:�߽�Ħ��Դ���õ���л�Ź�ʽϵ��
//	float r,fr,a,P,K;
//
//	//�߽�ͨ����Դ����ͨ��
//	float FQ[2],FA[2],SQ[2],SA[2];
//
//	//ʱ���ѭ��
//	while(leftTime>0)
//	{
//		//dt=10;
//		//�ռ䲽��һ����ȷ��ʱ�䲽��
//		/*if(maxC>=30)
//		{
//			cout<<"warning:maxC="<<maxC<<"m/s,impossible?"<<endl;
//		}*/
//
//		//��������һ��maxC������Ϊ����Ӵ�maxC�ܴ󣬱����������ôʱ�䲽��������С��������ѭ��
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
//		//�ϱ߽�����
//		if(AreaS<1e-2)
//		{
//			//����Դ����
//			Q[0]=Qu1[min(int(Time/MSTEP),Steps)]+Qu2[min(int(Time/MSTEP),Steps)];
//		}
//		else
//		{
//			//Դ�Ӷ�û����������
//			Q[0]=(*pS)[pS->size()-1].qs*(*pS)[pS->size()-1].Width;
//		}
//
//		if(Time<this->TimeEnd)
//		{
//			//��������ˮ(����˵�Ǳ�������ˮ)������ˮ�Ĳ���(m2/s)
//			q=W/(TimeEnd-TimeStart)/Length;
//
//			if(q>5)
//			{
//				int x=0;
//			}
//		}
//
//		//���ұ������������Ĳ���
//		q=q+(*pL)[pL->size()-1].qs+(*pR)[pR->size()-1].qs;
//
//		//�ռ��ѭ��
//		for(int i=0;i<Xn;i++)
//		{
//			//*ע��Xn-1�����޵�Ԫ��Xn���߽磬ÿ���߽紦����Ӧ��һ��L������R������Ҳ����˵�õ���L��R�Ƕ�Ӧ��ͬһ���߽�ģ����������޵�Ԫ����߽���ұ߽�
//			
//			/***********************************************����XL[0]��XR[0]***********************************************/
//			//*ע��QL[0],QR[0]�ȶ�Ӧ��i-1/2����QL[1],QR[1]��Ӧ��i+1/2��
//			
//			if(i==0)
//			{
//				////��Ϊ�õ���i-2�㣬����i==0��1Ҫ�����ж�
//				//if(i<=1)
//				//{
//					QL[0]=max(Q[0],1e-10);
//					AL[0]=max(A[0],1e-7);
//				//}
//
//				////�м���ͨ�ڵ�
//				//else
//				//{
//				//	//Ϊ�˱���r��ĸΪ0��������1e-8
//				//	//���ʵ����r(i-1)
//				//	r=(Q[i]-Q[i-1]+1e-8)/(Q[i-1]-Q[i-2]+1e-8);
//
//				//	//Van Albadaͨ��������
//				//	fr=(pow(r,2)+r)/(1+pow(r,2));
//
//				//	QL[0]=Q[i-1]+0.5*fr*(Q[i-1]-Q[i-2]);
//
//				//	r=(A[i]-A[i-1]+1e-8)/(A[i-1]-A[i-2]+1e-8);
//				//	fr=(pow(r,2)+r)/(1+pow(r,2));
//				//	AL[0]=A[i-1]+0.5*fr*(A[i-1]-A[i-2]);
//				//}
//
//				//��Ϊ�õ���i-1�㣬����Ҫ�����ж�
//				//if(i==0)
//				//{
//					QR[0]=max(Q[0],1e-10);
//					AR[0]=max(A[0],1e-7);
//				//}
//				////��Ϊ�õ���i+1�㣬����Ҫ�����ж�
//				//else if(i==Xn-1)
//				//{
//				//	QR[0]=Q[Xn-1];
//				//	AR[0]=A[Xn-1];
//				//}
//				//else
//				//{
//				//	//Ϊ�˱���r��ĸΪ0��������1e-8
//				//	//���ʵ����1/r(i)
//				//	r=(Q[i]-Q[i-1]+1e-8)/(Q[i+1]-Q[i]+1e-8);
//
//				//	//Van Albadaͨ��������
//				//	fr=(pow(r,2)+r)/(1+pow(r,2));
//
//				//	QR[0]=Q[i]-0.5*fr*(Q[i+1]-Q[i]);
//
//				//	r=(A[i]-A[i-1]+1e-8)/(A[i+1]-A[i]+1e-8);
//
//				//	//Van Albadaͨ��������
//				//	fr=(pow(r,2)+r)/(1+pow(r,2));
//
//				//	AR[0]=A[i]-0.5*fr*(A[i+1]-A[i]);
//				//}
//				/***********************************************����XL[0]��XR[0]***********************************************/
//
//				/************************************************���㸨������ֵ************************************************/
//				//����ˮ������ֶ�����״�ĵط�
//				if(AL[0]<=ST)
//				{
//					BL[0]=sqrt(2*max(AL[0],1e-7)*(1/tan(theta1)+1/tan(theta2))+pow(ef,2));
//				}
//				else
//				{
//					BL[0]=sqrt(2*(AL[0]-ST)*(1/tan(SlopeLA)+1/tan(SlopeRA))+pow(gh,2));//ͼ��RS����
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
//				//�����B[0]�Ǳ߽紦��ˮ���ȡֵ�����ݾ�ˮƽ���������ӱ��ʽ��ʽ���Է��֣�������B=dA/dh
//				//B[0]=((AR[0]-AL[0])*c[0]+q*dx)/c[0]/tan(SlopeA)/dx;
//				B[0]=(BL[0]+BR[0])/2;
//
//				//���洦��ˮ�������
//				a=0.5*(AL[0]/BL[0]+AR[0]/BR[0])*B[0];
//
//				//������洦ʪ��
//				if(AL[0]<=ST)
//				{
//					P=2*a/(ef+B[0])*(1/sin(theta1)+1/sin(theta2))+ef;
//				}
//				else
//				{
//					P=2*(a-ST)/(gh+B[0])*(1/sin(SlopeLA)+1/sin(SlopeRA))+gm*(1/sin(theta1)+1/sin(theta2))+ef;
//				}
//
//				//0��������������ֵ��1�����ҽ����ֵ
//				K=pow(a,5/3)/Manning/pow(P,2/3);
//				alfa1[0]=(QR[0]-QL[0]+(c[0]-u[0])*(AR[0]-AL[0]))/2/c[0];
//				alfa2[0]=AR[0]-AL[0]-alfa1[0];
//				gama1[0]=-pow(c[0],3)*(BR[0]-BL[0])/4/9.8/cos(SlopeA);
//				gama2[0]=-gama1[0];
//
//				//������ΪQ^=A^*u^,q^=q
//				beta1[0]=gama1[0]+B[0]*c[0]*tan(SlopeA)*dx/2-9.8*a*a*u[0]*abs(a*u[0])*dx/2/c[0]/pow(K,2)-q*dx*(u[0]-c[0])/2/c[0];
//				beta2[0]=q*dx-beta1[0];
//
//			}//end if(i==0)
//			/************************************************���㸨������ֵ************************************************/
//
//			/***********************************************����XL[1]��XR[1]***********************************************/
//			//ע:�����Ӧ����i+1/2�߽紦
//			
//			//��Ϊr�õ���i-1�ڵ㣬����Ҫ�����ж�
//			if(i==0)
//			{
//				QL[1]=Q[0];
//				AL[1]=A[0];
//			}
//
//			//��Ϊr�õ���i+1�ڵ㣬����Ҫ�����ж�
//			else if(i==Xn-1)
//			{
//				QL[1]=Q[Xn-1];
//				AL[1]=A[Xn-1];
//			}
//
//			//�м���ͨ�ڵ�
//			else
//			{
//				//��Ӧr(i)
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
//			//������������ڵ��ұ߽��QR��AR����Ϊ������i+2�����Զ���������ڵ�Ҫ�����ж�
//			if(i>=Xn-2)
//			{
//				QR[1]=max(Q[Xn-1],1e-6);
//				AR[1]=max(A[Xn-1],1e-6);
//			}
//			else
//			{
//				//��Ӧ1/r(i+1)
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
//			/***********************************************����XL[1]��XR[1]***********************************************/
//			
//			/************************************************���㸨������ֵ************************************************/
//			//����ˮ������ֶ�����״�ĵط�
//			if(AL[1]<=ST)
//			{
//				BL[1]=sqrt(2*max(AL[1],1e-7)*(1/tan(theta1)+1/tan(theta2))+pow(ef,2));
//			}
//			else
//			{
//				BL[1]=sqrt(2*(AL[1]-ST)*(1/tan(SlopeLA)+1/tan(SlopeRA))+pow(gh,2));//ͼ��RS����
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
//			//����α���ֵ
//
//			//ע���������ٺ��ѿ��ƣ�
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
//			//���洦��ˮ�������
//			a=0.5*(AL[1]/BL[1]+AR[1]/BR[1])*B[1];
//
//			//������洦ʪ��
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
//			alfa1[1]=(QR[1]-QL[1]+(c[1]-u[1])*(AR[1]-AL[1]))/2/c[1];//���٣�m2
//			alfa2[1]=AR[1]-AL[1]-alfa1[1];
//			gama1[1]=-pow(c[1],3)*(BR[1]-BL[1])/4/9.8/cos(SlopeA);//���٣�m3/s
//			gama2[1]=-gama1[1];
//			beta1[1]=gama1[1]+B[1]*c[1]*tan(SlopeA)*dx/2-9.8*a*a*u[1]*abs(a*u[1])*dx/2/c[1]/pow(K,2)-q*dx*(u[1]-c[1])/2/c[1];//���٣�m3/s
//			beta2[1]=q*dx-beta1[1];
//			/************************************************���㸨������ֵ************************************************/
//			
//			/**********************************************�������ҽ���ͨ��ֵ**********************************************/
//			//�����ͨ��"F(i-1/2)+","S(i-1/2)+"
//			//��ע�͵���FA[0]��FQ[0]�뱻���õ���ȫ�ȼۣ�ֻ�Ǵ�����gama1��gama2֮��Ĺ�ϵ
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
//			//�ҽ���ͨ��"F(i+1/2)-","S(i+1/2)-"
//			//��ע�͵���FA[1]��FQ[1]�뱻���õ���ȫ�ȼۣ�ֻ�Ǵ�����gama1��gama2֮��Ĺ�ϵ
//
//			//FA[1]=alfa1[1]*(u[1]+c[1]-abs(u[1]+c[1]))+alfa2[1]*(u[1]-c[1]-abs(u[1]-c[1]))+0.5*(gama1[1]+gama2[1])*(1-Sign(u[1]+c[1]));
//			FA[1]=alfa1[1]*(u[1]+c[1]-abs(u[1]+c[1]))+alfa2[1]*(u[1]-c[1]-abs(u[1]-c[1]));
//
//			//FQ[1]=alfa1[1]*(u[1]+c[1])*(u[1]+c[1]-abs(u[1]+c[1]))+alfa2[1]*(u[1]-c[1])*(u[1]-c[1]-abs(u[1]-c[1]))+0.5*(gama1[1]*(u[1]+c[1])+gama2[1]*(u[1]-c[1]))*(1-Sign(u[1]-c[1]));
//			FQ[1]=alfa1[1]*(u[1]+c[1])*(u[1]+c[1]-abs(u[1]+c[1]))+alfa2[1]*(u[1]-c[1])*(u[1]-c[1]-abs(u[1]-c[1]))+gama1[1]*c[1]*(1-Sign(u[1]-c[1]));
//
//			SA[1]=0.5*(beta1[1]*(1-Sign(u[1]+c[1]))+beta2[1]*(1-Sign(u[1]-c[1])));
//			SQ[1]=0.5*(beta1[1]*(u[1]+c[1])*(1-Sign(u[1]+c[1]))+beta2[1]*(u[1]-c[1])*(1-Sign(u[1]-c[1])));
//			/**********************************************�������ҽ���ͨ��ֵ**********************************************/
//			
//			/**********************************************�õ���һʱ�̵ı���**********************************************/
//			//ע�⣺д��A2��Q2����
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
//			//ǿ�Ƹ�ֵ���ܻ�������ֵ�ȶ���
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
//			//�洢�������κӶμ�Ľӿڱ�����
//			if(i==Xn-1)
//			{
//				N1=min(int((Time-dt)/MSTEP),Steps);
//				N2=min(int(Time/MSTEP)+1,Steps);
//				for(int n=N1;n<=N2;n++)
//				{
//					Quout[n]=Q[i];//Ԫ�����������
//
//					if(Quout[n]>1e5 || Quout[n]<-10)
//					{
//						cout<<"Quout:"<<Quout[n]<<endl;
//					}
//					
//					//��Ҫ�󱣴��ʱ���ټ���
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
//							FlowB[n]=sqrt(2*(A[i]-ST)*(1/tan(SlopeLA)+1/tan(SlopeRA))+pow(gh,2));//ͼ��RS����
//							FlowH[n]=gm+2*(A[i]-ST)/(FlowB[n]+gh);
//						}
//					}
//				}
//			}
//
//			
//			//�������Q��A�����ƣ���Q��A����С��ʱ�򣬿��ܻ����maxC�ܴ���������ʱ��ʱ�䲽��dt�ͺ�С�ˡ�
//			if(Q[i]>1e-3 && A[i]>1e-1)
//			{
//				//���Բ��٣���������ʱ�䲽��
//				//��ǰˮ���B
//				float B=1e-6;
//
//				//����A�Ĵ�С��ˮ���B
//				if(A[i]<=ST)
//				{
//					B=sqrt(2*max(A[i],1e-7)*(1/tan(theta1)+1/tan(theta2))+pow(ef,2));
//				}
//				else
//				{
//					B=sqrt(2*(A[i]-ST)*(1/tan(SlopeLA)+1/tan(SlopeRA))+pow(gh,2));//ͼ��RS����
//				}
//
//				//��ǰ�����Բ���
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
//		//��t+1ʱ�̵�ֵ����tʱ�̵�ֵ
//		for(int k=0;k<Xn;k++)
//		{
//			Q[k]=Q2[k];
//			A[k]=A2[k];
//		}
//
//	}//end t
//
//	//�������ˮͷH
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
//	H/=Xn;//ƽ��ˮ��
//
//	if(H>40 || H<1e-8)
//	{
//		int x=0;
//	}
//
//	if(H<=-1e-4)
//	{
//		cout<<FVMunit::Year<<"."<<FVMunit::Month<<"."<<FVMunit::Day<<"."<<FVMunit::Hour<<",������ˮ��:"<<H<<endl;
//		H=0;
//	}
//
//	H-=gm;//ƽ��ˮͷ
//}

//���ź���
//int WaterInChannel::Sign(float x)
//{
//	if(x>1e-4) return 1;
//	if(x<1e-4) return -1;
//	return 0;
//}


//���������ľ�ȷ��ɢ������װ�ڴˣ���ȷ��ɢ��ֻ��һ��δ֪��Q��ROE��ʽ�ô���A��Q�����������У�����ʱ�䲽���Ƚ�������
