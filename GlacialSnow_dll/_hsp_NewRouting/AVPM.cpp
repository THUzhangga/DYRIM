#include <math.h>
#include <iostream>
#include ".\avpm.h"
using namespace std;

//静态变量
const float AVPM::g=9.81;
const float AVPM::gamma=1000;
const float AVPM::nu=0.000001;
const float AVPM::fuzzpro[122][7]= { 0.29	,	1	,	1	,	1	,	1	,	1	,	0.99999	,
0.3	,	1	,	1	,	1	,	1	,	1	,	0.99999	,
0.31	,	1	,	1	,	1	,	1	,	1	,	0.99998	,
0.32	,	1	,	1	,	1	,	1	,	1	,	0.99997	,
0.33	,	1	,	1	,	1	,	1	,	1	,	0.99995	,
0.34	,	1	,	1	,	1	,	1	,	1	,	0.99993	,
0.35	,	1	,	1	,	1	,	1	,	0.99999	,	0.99989	,
0.36	,	1	,	1	,	1	,	1	,	0.99999	,	0.99985	,
0.37	,	1	,	1	,	1	,	1	,	0.99998	,	0.99979	,
0.38	,	1	,	1	,	1	,	1	,	0.99998	,	0.99971	,
0.39	,	1	,	1	,	1	,	1	,	0.99996	,	0.99961	,
0.4	,	1	,	1	,	1	,	1	,	0.99995	,	0.99947	,
0.41	,	1	,	1	,	1	,	1	,	0.99992	,	0.9993	,
0.42	,	1	,	1	,	1	,	1	,	0.99989	,	0.99909	,
0.43	,	1	,	1	,	1	,	0.99999	,	0.99984	,	0.99883	,
0.44	,	1	,	1	,	1	,	0.99999	,	0.99978	,	0.9985	,
0.45	,	1	,	1	,	1	,	0.99998	,	0.99969	,	0.99811	,
0.46	,	1	,	1	,	1	,	0.99998	,	0.99958	,	0.99764	,
0.47	,	1	,	1	,	1	,	0.99996	,	0.99944	,	0.99707	,
0.48	,	1	,	1	,	1	,	0.99995	,	0.99926	,	0.99641	,
0.49	,	1	,	1	,	1	,	0.99992	,	0.99904	,	0.99563	,
0.5	,	1	,	1	,	1	,	0.99989	,	0.99875	,	0.99472	,
0.51	,	1	,	1	,	1	,	0.99984	,	0.9984	,	0.99367	,
0.52	,	1	,	1	,	1	,	0.99977	,	0.99798	,	0.99246	,
0.53	,	1	,	1	,	0.99999	,	0.99968	,	0.99746	,	0.99109	,
0.54	,	1	,	1	,	0.99999	,	0.99956	,	0.99684	,	0.98953	,
0.55	,	1	,	1	,	0.99998	,	0.99941	,	0.99611	,	0.98777	,
0.56	,	1	,	1	,	0.99997	,	0.99921	,	0.99524	,	0.9858	,
0.57	,	1	,	1	,	0.99995	,	0.99895	,	0.99422	,	0.98361	,
0.58	,	1	,	1	,	0.99993	,	0.99863	,	0.99303	,	0.98118	,
0.59	,	1	,	1	,	0.99989	,	0.99823	,	0.99166	,	0.9785	,
0.6	,	1	,	1	,	0.99984	,	0.99773	,	0.99009	,	0.97556	,
0.61	,	1	,	1	,	0.99976	,	0.99713	,	0.9883	,	0.97234	,
0.62	,	1	,	1	,	0.99966	,	0.99639	,	0.98627	,	0.96884	,
0.63	,	1	,	0.99999	,	0.99953	,	0.9955	,	0.98398	,	0.96505	,
0.64	,	1	,	0.99999	,	0.99935	,	0.99444	,	0.98141	,	0.96095	,
0.65	,	1	,	0.99998	,	0.99911	,	0.99319	,	0.97856	,	0.95654	,
0.66	,	1	,	0.99997	,	0.99879	,	0.99172	,	0.97539	,	0.95181	,
0.67	,	1	,	0.99995	,	0.99839	,	0.99001	,	0.97189	,	0.94676	,
0.68	,	1	,	0.99992	,	0.99788	,	0.98802	,	0.96805	,	0.94139	,
0.69	,	1	,	0.99988	,	0.99723	,	0.98574	,	0.96386	,	0.93569	,
0.7	,	1	,	0.99982	,	0.99642	,	0.98314	,	0.95929	,	0.92965	,
0.71	,	1	,	0.99972	,	0.99542	,	0.98018	,	0.95434	,	0.92329	,
0.72	,	1	,	0.99958	,	0.9942	,	0.97685	,	0.94899	,	0.91659	,
0.73	,	1	,	0.99939	,	0.99272	,	0.97311	,	0.94325	,	0.90956	,
0.74	,	1	,	0.99912	,	0.99095	,	0.96894	,	0.93709	,	0.90221	,
0.75	,	0.99999	,	0.99875	,	0.98883	,	0.96431	,	0.93052	,	0.89454	,
0.76	,	0.99998	,	0.99825	,	0.98634	,	0.95921	,	0.92353	,	0.88655	,
0.77	,	0.99995	,	0.99759	,	0.98341	,	0.95361	,	0.91611	,	0.87825	,
0.78	,	0.99991	,	0.99673	,	0.98002	,	0.94749	,	0.90828	,	0.86965	,
0.79	,	0.99983	,	0.99562	,	0.97611	,	0.94084	,	0.90003	,	0.86076	,
0.8	,	0.99971	,	0.9942	,	0.97164	,	0.93364	,	0.89136	,	0.85158	,
0.81	,	0.9995	,	0.99241	,	0.96655	,	0.92588	,	0.88229	,	0.84212	,
0.82	,	0.99919	,	0.99018	,	0.96082	,	0.91755	,	0.87281	,	0.83241	,
0.83	,	0.99872	,	0.98744	,	0.95438	,	0.90866	,	0.86293	,	0.82244	,
0.84	,	0.99804	,	0.9841	,	0.94722	,	0.89918	,	0.85267	,	0.81223	,
0.85	,	0.99708	,	0.98008	,	0.93929	,	0.88914	,	0.84204	,	0.80179	,
0.86	,	0.99575	,	0.97528	,	0.93056	,	0.87853	,	0.83106	,	0.79114	,
0.87	,	0.99398	,	0.96962	,	0.92102	,	0.86736	,	0.81972	,	0.78029	,
0.88	,	0.99163	,	0.96299	,	0.91064	,	0.85564	,	0.80806	,	0.76925	,
0.89	,	0.98859	,	0.95531	,	0.8994	,	0.84338	,	0.79609	,	0.75803	,
0.9	,	0.98473	,	0.94649	,	0.88731	,	0.83061	,	0.78383	,	0.74666	,
0.91	,	0.97988	,	0.93645	,	0.87437	,	0.81734	,	0.7713	,	0.73514	,
0.92	,	0.97388	,	0.92512	,	0.86058	,	0.8036	,	0.75851	,	0.7235	,
0.93	,	0.96654	,	0.91245	,	0.84596	,	0.78941	,	0.74548	,	0.71173	,
0.94	,	0.95768	,	0.89837	,	0.83053	,	0.77479	,	0.73224	,	0.69987	,
0.95	,	0.9471	,	0.88288	,	0.81432	,	0.75978	,	0.71881	,	0.68792	,
0.96	,	0.93458	,	0.86595	,	0.79737	,	0.74441	,	0.70521	,	0.6759	,
0.97	,	0.91992	,	0.8476	,	0.77972	,	0.72871	,	0.69146	,	0.66382	,
0.98	,	0.90293	,	0.82784	,	0.76141	,	0.7127	,	0.67758	,	0.65169	,
0.99	,	0.88344	,	0.80674	,	0.7425	,	0.69644	,	0.66359	,	0.63953	,
1	,	0.86131	,	0.78435	,	0.72304	,	0.67995	,	0.64952	,	0.62736	,
1.01	,	0.83646	,	0.76076	,	0.7031	,	0.66327	,	0.63538	,	0.61518	,
1.02	,	0.80889	,	0.73607	,	0.68273	,	0.64642	,	0.6212	,	0.603	,
1.03	,	0.77864	,	0.7104	,	0.662	,	0.62946	,	0.607	,	0.59085	,
1.04	,	0.74589	,	0.68389	,	0.64098	,	0.61241	,	0.59279	,	0.57872	,
1.05	,	0.71089	,	0.65667	,	0.61974	,	0.59531	,	0.5786	,	0.56664	,
1.06	,	0.67397	,	0.6289	,	0.59834	,	0.57819	,	0.56444	,	0.55461	,
1.07	,	0.63555	,	0.60072	,	0.57685	,	0.56108	,	0.55033	,	0.54265	,
1.08	,	0.5961	,	0.5723	,	0.55533	,	0.54403	,	0.53629	,	0.53076	,
1.09	,	0.55613	,	0.5438	,	0.53386	,	0.52705	,	0.52233	,	0.51895	,
1.1	,	0.51614	,	0.51535	,	0.5125	,	0.51018	,	0.50848	,	0.50723	,
1.11	,	0.47661	,	0.48712	,	0.4913	,	0.49344	,	0.49474	,	0.49561	,
1.12	,	0.438	,	0.45924	,	0.47031	,	0.47687	,	0.48113	,	0.4841	,
1.13	,	0.40067	,	0.43184	,	0.44961	,	0.46049	,	0.46767	,	0.47271	,
1.14	,	0.36494	,	0.40504	,	0.42922	,	0.44432	,	0.45436	,	0.46144	,
1.15	,	0.33104	,	0.37894	,	0.40921	,	0.42838	,	0.44122	,	0.4503	,
1.16	,	0.29914	,	0.35365	,	0.3896	,	0.4127	,	0.42826	,	0.43929	,
1.17	,	0.26933	,	0.32923	,	0.37044	,	0.39729	,	0.41548	,	0.42842	,
1.18	,	0.24164	,	0.30576	,	0.35177	,	0.38216	,	0.4029	,	0.4177	,
1.19	,	0.21606	,	0.28328	,	0.3336	,	0.36735	,	0.39053	,	0.40714	,
1.2	,	0.19256	,	0.26184	,	0.31598	,	0.35285	,	0.37836	,	0.39672	,
1.21	,	0.17104	,	0.24147	,	0.2989	,	0.33868	,	0.36642	,	0.38647	,
1.22	,	0.15143	,	0.22218	,	0.2824	,	0.32485	,	0.3547	,	0.37637	,
1.23	,	0.13362	,	0.20398	,	0.26649	,	0.31137	,	0.34322	,	0.36644	,
1.24	,	0.1175	,	0.18686	,	0.25117	,	0.29825	,	0.33197	,	0.35669	,
1.25	,	0.10297	,	0.17081	,	0.23645	,	0.28549	,	0.32095	,	0.3471	,
1.26	,	0.0899	,	0.15581	,	0.22234	,	0.27309	,	0.31018	,	0.33768	,
1.27	,	0.078196	,	0.14184	,	0.20883	,	0.26106	,	0.29965	,	0.32844	,
1.28	,	0.067749	,	0.12885	,	0.19592	,	0.2494	,	0.28937	,	0.31937	,
1.29	,	0.058459	,	0.11682	,	0.18361	,	0.23812	,	0.27934	,	0.31048	,
1.3	,	0.05023	,	0.10571	,	0.17189	,	0.2272	,	0.26955	,	0.30176	,
1.31	,	0.042969	,	0.095468	,	0.16074	,	0.21666	,	0.26001	,	0.29323	,
1.32	,	0.036589	,	0.086054	,	0.15016	,	0.20648	,	0.25072	,	0.28487	,
1.33	,	0.031009	,	0.077423	,	0.14013	,	0.19666	,	0.24168	,	0.27668	,
1.34	,	0.02615	,	0.069529	,	0.13064	,	0.18721	,	0.23289	,	0.26868	,
1.35	,	0.021939	,	0.062326	,	0.12167	,	0.17811	,	0.22433	,	0.26085	,
1.36	,	0.018309	,	0.055768	,	0.1132	,	0.16935	,	0.21602	,	0.2532	,
1.37	,	0.015195	,	0.049812	,	0.10523	,	0.16095	,	0.20796	,	0.24572	,
1.38	,	0.01254	,	0.044413	,	0.097726	,	0.15287	,	0.20012	,	0.23841	,
1.39	,	0.010288	,	0.03953	,	0.090674	,	0.14513	,	0.19253	,	0.23128	,
1.4	,	0.0083889	,	0.035124	,	0.084055	,	0.13771	,	0.18516	,	0.22432	,
1.41	,	0.006798	,	0.031155	,	0.07785	,	0.1306	,	0.17803	,	0.21753	,
1.42	,	0.0054736	,	0.027588	,	0.072041	,	0.1238	,	0.17112	,	0.2109	,
1.43	,	0.0043782	,	0.024389	,	0.066609	,	0.11729	,	0.16443	,	0.20444	,
1.44	,	0.0034784	,	0.021524	,	0.061535	,	0.11108	,	0.15795	,	0.19814	,
1.45	,	0.0027445	,	0.018965	,	0.056801	,	0.10515	,	0.15169	,	0.19201	,
1.46	,	0.0021501	,	0.016682	,	0.052389	,	0.099483	,	0.14564	,	0.18603	,
1.47	,	0.0016724	,	0.014651	,	0.048282	,	0.094084	,	0.13979	,	0.18021	,
1.48	,	0.0012912	,	0.012845	,	0.044463	,	0.088939	,	0.13415	,	0.17454	,
1.49	,	0.00098953	,	0.011245	,	0.040914	,	0.084039	,	0.12869	,	0.16902	,
1.5	,	0.00075257	,	0.0098276	,	0.037621	,	0.079376	,	0.12343	,	0.16366	
}; 
//薛
const float pi=3.14159f;

AVPM::AVPM(void)
{
}

//wh,20080802
AVPM::AVPM(CString ssmethod,int mstep,CString sccd)
{
	this->SSMethod = ssmethod;
	this->MSTEP = mstep;
	this->sccd = sccd;
}

void AVPM::Calc(void)
{

	//20060907,xiaofc,以柯朗数为1的条件设置自动空间步长
	float MaxQ=0.0f;//时段内的最大流量
	float MaxC;//对应最大流量的最大波速

	bool bIterOvrflwWarn;
	bIterOvrflwWarn=false;

	//这个for是将两股来流混合的处理
	for(t=0;t<iTSteps;t++)
	{
		//wh解读：Q[t]存储的是当前河段的上边界条件。
		Q[t]=Qin1[t]+Qin2[t];//先将上游的两股来流相加，给Q[]

		if(Q[t]>MaxQ) MaxQ=Q[t];//20060906,xiaofc,统计最大Q

		
		//20051218 李铁键 增加对本河段产沙上限的检验
		//wh:SinL,SinR:上游两支流的来沙量
		if(SinL[t]>gammaS*(1.0f-mPara.Sita1-mPara.Sita2)+gamma*(mPara.Sita1+mPara.Sita2))
			SinL[t]=gammaS*(1.0f-mPara.Sita1-mPara.Sita2)+gamma*(mPara.Sita1+mPara.Sita2);
		if(SinR[t]>gammaS*(1.0f-mPara.Sita1-mPara.Sita2)+gamma*(mPara.Sita1+mPara.Sita2))
			SinR[t]=gammaS*(1.0f-mPara.Sita1-mPara.Sita2)+gamma*(mPara.Sita1+mPara.Sita2);

		if(Q[t]>0)
			mySin[t]=(SinL[t]*Qin1[t]+SinR[t]*Qin2[t])/Q[t];
		else
			mySin[t]=0;
		
		if(mySin[t]<0) 
			mySin[t]=0;
		else if(mySin[t]>Svm*gammaS)//20051218 李铁键 增加对入流泥沙上限的检验
			mySin[t]=Svm*gammaS;
		
		//if(isDebug)

			//if(t>=5430 && t<=6150)
			//	cout<<Q[t]<<"\t"<<Qin1[t]<<"\t"<<Qin2[t]<<"\t"<<Qin3[t]<<endl;
	}

	//200512,李铁键
	//如果seglength,即本类中的L<=0,为虚拟河段
	//则不计算，直接把入口复制到出口，return
	if(L<1e-5)
	{
		for(t=0;t<iTSteps;t++)
		{
			Qout[t]=Q[t];
			Sout[t]=mySin[t];
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

	//空间步长数
	int xsteps;

	//20060906,xiaofc,自动求步长数
	if(MaxQ>1.0f)
	{
		//wh解读：通过流速求波速，下面公式适用于三角形断面
		MaxC=4.0f/3.0f*pow(m/(m*m+1),0.25f)*pow(S,0.375f)*pow(MaxQ,0.25f)/sqrt(2.0)/pow(n,0.75f);
		xsteps=floor(L/MaxC/delta_t+0.5);
		if(xsteps<1) xsteps=1;
	}
	else
		xsteps=1;

	//cout<<"xsteps is "<<xsteps<<endl;

	//delta_x:空间步长(m)
	delta_x=L/xsteps;

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

	
	float iterQ;//每个网格点上迭代用到的Q

	for(int xx=0;xx<xsteps;xx++)//空间步长循环
	{

		if(xx)//把非第0空间层新的Qout给Q
			for(t=0;t<iTSteps;t++)
				Q[t]=Qout[t];
		else
			GravityErosion[0]=0;

		Qout[0]=Q[0];//第一个时间点不进行计算
		mySin[0]=0.0f;
		Sout[0]=mySin[0];
		
		//20070828,xiaofc,Q很小的时候要变糙率
		//20080401,xiaofc,改进
		//if(Q[0]<1 && n0<0.5)
		//	n=0.5-(0.5-n0)*Q[0];
		if(Q[0]<mPara.DrainingArea/1000000000*1 && n0<0.5  && mPara.DrainingArea>3000000000)
			n=0.5-(0.5-n0)*Q[0]/1*1000000000/mPara.DrainingArea;
		else
			n=n0;

        h[1]=pow(n*Q[0]*1.5874f*pow(m*m+1.0f,0.33333333f)/pow(m,5.0f/3)/sqrt(S),0.375f);
		h[3]=pow(n*Qout[0]*1.5874f*pow(m*m+1.0f,0.33333333f)/pow(m,5.0f/3)/sqrt(S),0.375f);
		
		//20060310,李铁键,h[1],h[3]的初始值不能是0,以免被除0
		if(h[1]<1e-10)
			h[1]=1e-10;
		if(h[3]<1e-10)
			h[3]=1e-10;

		h_x=0.0f; //pian h /pian x先赋0

		for(t=1;t<iTSteps;t++)//时间循环
		{


			//if(t==510)
			//	t=510;

			//20080306,xiaofc,重力侵蚀入水量状态的初始化
			//必须放在前面，因为有些时间循环是不计算直接continue的
			DayLoop=t*MSTEP/60/24;
			if ( (t*MSTEP) % (60*24)==0 && xx==0)
				GravityErosion[DayLoop]=0.0f;

			//20070828,xiaofc,Q很小的时候要变糙率
			//20080401,xiaofc,改进
			//if(Q[t]<1 && n0<0.5)
			//	n=0.5-(0.5-n0)*Q[t];
			if(Q[t]<mPara.DrainingArea/1000000000*1 && n0<0.5  && mPara.DrainingArea>3000000000)
				n=0.5-(0.5-n0)*Q[t]/1*1000000000/mPara.DrainingArea;
			else
				n=n0;

			//20070724,xiaofc,加一个是否从Qout[t]的循环里遇到负反应跳出来的
			bool bNegtiveQHappened=false;

			//运用manning公式求水深
			//cout<<"Time Step "<<t<<endl;
			if(isDebug)
				myFile<<"Time Step "<<t<<"\tQ0="<<Q[t]<<endl;

			//20060310,李铁键,如果Q[t]<1e-5,那么直接下一时间循环
			if(Q[t]<1e-5)
			{
				Qout[t]=Q[t];//0.0f;
				Sout[t]=mySin[t];//0.0f;
				continue;
			}

			//Qout[t]=Qout[t-1];
			Qout[t]=Q[t];//用Qt比Qout t-1 更容易收敛些
	

			h[0]=h[1];
			h[2]=h[3];

			h[1]=pow(n*Q[t]*1.5874f*pow(m*m+1.0f,0.33333333f)/pow(m,5.0f/3)/sqrt(S),0.375f);
			h[3]=pow(n*Qout[t]*1.5874f*pow(m*m+1.0f,0.33333333f)/pow(m,5.0f/3)/sqrt(S),0.375f);

			
			//求pian h / pian x
			hh=(h[0]+h[1]+h[2]+h[3])/4.0f;
			
			u[0]=(Q[t-1])/h[0]/h[0]/m;
			u[1]=(Q[t])/h[1]/h[1]/m;
			u[2]=(Qout[t-1])/h[2]/h[2]/m;
			u[3]=(Qout[t])/h[3]/h[3]/m;
			c[0]=u[0]*4.0/3.0;
			c[1]=u[1]*4.0/3.0;
			c[2]=u[2]*4.0/3.0;
			c[3]=u[3]*4.0/3.0;
			cAverage=(c[0]+c[1]+c[2]+c[3])/4;

			BAverage=4.0f*m*(Q[t-1]+Q[t]+Qout[t-1]+Qout[t])/3.0f/cAverage;
			BAverage=sqrt(BAverage);

			cold=cAverage;

			//20080811, xiaofc, 计算ph/px不采用Qout[t]点的值，那样对收敛不利
			//Q_x=(Q[t-1]+Q[t]-Qout[t-1]-Qout[t])/2.0f/delta_x;
			Q_x=((Q[t-1]+Q[t])/2.0f-Qout[t-1])/delta_x;
			h_x=Q_x/BAverage/cAverage;


			if(Q_x>0)
				J=eta*h_x+S;
			else
				J=S;
			
			//cout<<"1";

			if(J>nano)
			{
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

				BAverage=4.0f*m*(Q[t-1]+Q[t]+Qout[t-1]+Qout[t])/3.0f/cAverage;
				BAverage=sqrt(BAverage);

			}
			else
			{
				J=nano;
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

				BAverage=4.0f*m*(Q[t-1]+Q[t]+Qout[t-1]+Qout[t])/3.0f/cAverage;
				BAverage=sqrt(BAverage);
			}
			//cout<<"2";
			IterCount2=0;
			//20080811, xiaofc, 优化波速的收敛条件，加快计算速度
			while(!(abs(cold-cAverage)<0.001f || abs(cold/cAverage-1.0f)<0.001f) && IterCount2<MaxIter)
			{
				IterCount2++;

				h_x=Q_x/BAverage/cAverage;

				if(Q_x>0)
					J=eta*h_x+S;
				else
					J=S;

				if(J<nano)
					J=nano;

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

				if (abs(J-nano)<1e-10) break;

			}//end of while, get c

			BAverage=4.0f*m*(Q[t-1]+Q[t]+Qout[t-1]+Qout[t])/3.0f/cAverage;
			BAverage=sqrt(BAverage);

			//20080811, xiaofc, 用ph/px修正K容易产生负反应，取消
			K=delta_x/cAverage;
			//K=K/(1-3.0f/32*h_x/S);
			//cout<<"K= "<<K<<endl;

			Q_c[0]=Q[t-1]/c[0];
			Q_c[1]=Q[t]/c[1];
			Q_c[2]=Qout[t-1]/c[2];
			Q_c[3]=Qout[t]/c[3];


			//iksila
			iksila=(1.0f-(Q_c[0]+Q_c[1]+Q_c[2]+Q_c[3])/4/BAverage/J/delta_x)/2;

			C1=(K*iksila+0.5*delta_t)/(K*(1.0f-iksila)+0.5*delta_t);
			C2=(-K*iksila+0.5*delta_t)/(K*(1.0f-iksila)+0.5*delta_t);
			C3=(K*(1.0f-iksila)-0.5*delta_t)/(K*(1.0f-iksila)+0.5*delta_t);

			//新的Qout[t]
			iterQ=C1*Q[t-1]+C2*Q[t]+C3*Qout[t-1];

			//对零流量控制
			//20060310,李铁键,如果Q[t]<1e-10,那么直接下一时间循环
			//20070724,xiaofc,发生负反应直接截为0对后续计算的稳定性不利，同时需要考虑增加的保存BHV功能
			//因而水沙值采用入口t时刻值与出口t-1时刻的平均值，BHV用旧值
			//20070729,为了保持水量守衡，Qout[t]可能还是得置成极小
			if(iterQ<1e-10)
			{
				Qout[t]=1e-10;//(Q[t]+Qout[t-1]/2.0f);//0.0f;
				Sout[t]=(mySin[t]+Sout[t-1])/2.0f;//0.0f;
				if(SaveFlowPattern)
				{
					FlowB[t]=FlowB[t-1];
					FlowH[t]=FlowH[t-1];
					Flow_v[t]=Flow_v[t-1];
				}
				continue;
			}

			if(isDebug)
				myFile<<"c="<<c[0]<<"\tiksila="<<iksila<<"\tph/px="<<h_x<<"\tJ="<<J<<"\tQ="<<iterQ<<"\tIterCount:"<<IterCount2<<endl;//"\tIs DIP:"<<(Qout[t]-Qout[t-1])*(Q[t]-Q[t-1])<<endl;//float(delta_t)/K<<endl;
			

			IterCount=0;
			while( !(abs(iterQ-Qout[t])<0.1 || abs(iterQ/Qout[t]-1.0f)<0.005 || IterCount>MaxIter))//依上面的方法迭代，至Qout[t]收敛
			{
				IterCount++;

				Qout[t]=iterQ;

				h[3]=pow(n*Qout[t]*1.5874f*pow(m*m+1.0f,0.33333333f)/pow(m,5.0f/3)/sqrt(S),0.375f);

				//求pian h / pian x
				hh=(h[0]+h[1]+h[2]+h[3])/4.0f;


				cold=cAverage;

				//20080811, xiaofc, 计算ph/px不采用Qout[t]点的值，那样对收敛不利;
				//							因为这里其他三个点的Q不更新，所以不用重算pQ/px.
				//Q_x=(Q[t-1]+Q[t]-Qout[t-1]-Qout[t])/2.0f/delta_x;
				h_x=Q_x/BAverage/cAverage;


				if(Q_x>0)
					J=eta*h_x+S;
				else
					J=S;


				if(J>nano)
				{
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

					BAverage=4.0f*m*(Q[t-1]+Q[t]+Qout[t-1]+Qout[t])/3.0f/cAverage;
					BAverage=sqrt(BAverage);

				}
				else
				{
					J=nano;
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

					BAverage=4.0f*m*(Q[t-1]+Q[t]+Qout[t-1]+Qout[t])/3.0f/cAverage;
					BAverage=sqrt(BAverage);
				}
				IterCount2=0;
				//20080811, xiaofc, 优化波速的收敛条件，加快计算速度
				while(!(abs(cold-cAverage)<0.001f || abs(cold/cAverage-1.0f)<0.001f) && IterCount2<MaxIter)
				{
					IterCount2++;
				
					h_x=Q_x/BAverage/cAverage;

					if(Q_x>0)
						J=eta*h_x+S;
					else
						J=S;

					if(J<nano)
						J=nano;

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

					if (abs(J-nano)<1e-10) break;

				}//end of get c
				BAverage=4.0f*m*(Q[t-1]+Q[t]+Qout[t-1]+Qout[t])/3.0f/cAverage;
				BAverage=sqrt(BAverage);

				//20080811, xiaofc, 用ph/px修正K容易产生负反应，取消
				K=delta_x/cAverage;
				//K=K/(1-3.0f/32*h_x/S);
				//cout<<"K= "<<K<<endl;

				Q_c[0]=Q[t-1]/c[0];
				Q_c[1]=Q[t]/c[1];
				Q_c[2]=Qout[t-1]/c[2];
				Q_c[3]=Qout[t]/c[3];
				

				//iksila
				iksila=(1.0f-(Q_c[0]+Q_c[1]+Q_c[2]+Q_c[3])/4/BAverage/J/delta_x)/2;


				C1=(K*iksila+0.5*delta_t)/(K*(1.0f-iksila)+0.5*delta_t);//小
				C2=(-K*iksila+0.5*delta_t)/(K*(1.0f-iksila)+0.5*delta_t);
				C3=(K*(1.0f-iksila)-0.5*delta_t)/(K*(1.0f-iksila)+0.5*delta_t);
				
				iterQ=C1*Q[t-1]+C2*Q[t]+C3*Qout[t-1];

				//20070724,xiaofc,发生负反应直接截为0对后续计算的稳定性不利，同时需要考虑增加的保存BHV功能
				//因而水沙值采用入口t时刻值与出口t-1时刻的平均值，BHV用旧值
				if(iterQ<1e-10)
				{
					Qout[t]=1e-10;//(Q[t]+Qout[t-1]/2.0f);//0.0f;
					Sout[t]=(mySin[t]+Sout[t-1])/2.0f;//0.0f;
					if(SaveFlowPattern)
					{
						FlowB[t]=FlowB[t-1];
						FlowH[t]=FlowH[t-1];
						Flow_v[t]=Flow_v[t-1];
					}
					bNegtiveQHappened=true;
					break;
				}			

				if(isDebug)
					myFile<<"c="<<c[0]<<"\tiksila="<<iksila<<"\tph/px="<<h_x<<"\tJ="<<J<<"\tQ="<<iterQ<<"\tIterCount:"<<IterCount2<<endl;//"\tIs DIP:"<<(Qout[t]-Qout[t-1])*(Q[t]-Q[t-1])<<endl;//float(delta_t)/K<<endl;

			}//end of WHILE
			
			if(IterCount>=MaxIter && !bIterOvrflwWarn)
			{
				cout<<"#Warning, AVPM iteration overflow: "<<mBSCode.RegionIndex<<", "<<mBSCode.Value<<", "<<mBSCode.Length<<"."<<endl;
				bIterOvrflwWarn=true;
			}
			//20070722,以下负反应的处理方式会造成明显的水量的增加，因而废止
            //20060321,李铁键,恢复对负反应的控制，但改到迭代的最后，且增加对水深，从而是下一循环的流速的控制
			//if(iterQ-Qout[t-1]<0 && Q[t]-Q[t-1]>1e-4)
			//{
			//	iterQ=Qout[t-1];
			//	if(iterQ>0)
			//		h[3]=pow(n*iterQ*1.5874f*pow(m*m+1.0f,0.33333333f)/pow(m,5.0f/3)/sqrt(S),0.375f);
			//}
			
			//20070724,xiaofc,如果在Qout[t]迭代里遇见了负反应，则时间for循环continue
			if(bNegtiveQHappened)
				continue;

			Qout[t]=iterQ;

			//20070622, xiaofc, pass out the values of flow pattern
			if(SaveFlowPattern)
			{
				FlowB[t]=BAverage;
				FlowH[t]=(h[0]+h[1]+h[2]+h[3])/4.0f;
				Flow_v[t]=cAverage*0.75f;
			}

			//////////////////////////////////////////////////////////////////////////
			//重力侵蚀
			//////////////////////////////////////////////////////////////////////////
			//20051212 薛海
			//增加是否计算重力侵蚀的判断：
			//外界直接控制bCalcGravity和左右全坡平均坡度>15度，即tan()>0.268
			//20080411,xiaofc,无定河专用,重力侵蚀发生与否不在受左右坡面坡度限制
			//if(bCalcGravity && (mPara.SlopeL>0.268f || mPara.SlopeR>0.268f))
			if(bCalcGravity /*&& (mPara.SlopeL>0.268f || mPara.SlopeR>0.268f)*/)
			{
				//薛  我的函数调用在此开始
				//if(isDebug)
				//	if(t>=5430 && t<=6150)
				//		cout<<SinL[t]<<"\t"<<Qin1[t]<<"\t"<<SinR[t]<<"\t"<<Qin2[t]<<endl;
				W_cont=(WL[t]+WR[t])*0.5f;
				Gama_wm=(1.0f+W_cont/100.0f)*Gama_d;       //此处想给出沟坡土体在某一个含水量下的容重， kg/m3
			
				ConvertTao(hh);
				CalFs(0);
				CalFuzzProbi(0.1);
				HowMuchFailed(0);
				CalFs(1);
				CalFuzzProbi(0.1);
				HowMuchFailed(1);

				//薛 我的函数调用结束
			}

			SediCalc();
			
		}//end of timeloop

	}//end of x loop

	float tmp;

	for(t=0;t<iTSteps;t++)
	{
		//将上游两个河段的水沙演进到出口后
		//再把本单元流域产的水沙直接叠加到出口

		//20080303,xiaofc,处理保存沟道侵蚀量的过程
		//if(mBSCode.Value==1 && mBSCode.Length==187 && t==25680)
		//	t=t;
		DayLoop=t*MSTEP/60/24;
		//20080306,xiaofc,一日开始的时刻将河道冲刷置为淤积重力侵蚀带走的量
		//如此经过一d时间步长的累加之后，实为河道总冲淤量减去重力侵蚀带走的量，就是纯河道冲刷量
		if ( (t*MSTEP) % (60*24)==0)
			ChannelErosion[DayLoop]=-GravityErosion[DayLoop];

		//ChannelErosion[DayLoop]+=(Sout[t]*Qout[t]-mySin[t]*(Qin1[t]+Qin2[t]))*60*MSTEP;
		//20080306, xiaofc, 河段出入沙量的差减去重力侵蚀进入量成为沟道冲淤变化量
		ChannelErosion[DayLoop]+=(Sout[t]*Qout[t]-mySin[t]*(Qin1[t]+Qin2[t]))*60*MSTEP;

		//20051218 李铁键 增加对本河段产沙上限的检验
		if(SinMe[t]>gammaS*(1.0f-mPara.Sita1-mPara.Sita2)+gamma*(mPara.Sita1+mPara.Sita2))
			SinMe[t]=gammaS*(1.0f-mPara.Sita1-mPara.Sita2)+gamma*(mPara.Sita1+mPara.Sita2);

		tmp=Qout[t];
		Qout[t]=Qout[t]+Qin3[t];

		if(Qout[t]<1e-5)
			Sout[t]=0.0f;
		else
            Sout[t]=(Sout[t]*tmp+SinMe[t]*Qin3[t])/Qout[t];

		if(Sout[t]<0)
			Sout[t]=0;
		else if(Sout[t]>gammaS*Svm)//20051218 李铁键 增加对沙上限的检验
			Sout[t]=gammaS*Svm;
	
		if(!_finite (Sout[t]))
		{
			cout<<"INFINITE AVPM.Sout[t] Result:"<<endl;
			//cout<<"RegionIndex="<< mBSCode.RegionIndex<<"\tBSValue="<<mBSCode.Value<<"\tBSLength="<<mBSCode.Length<<endl;
			cout<<"t="<<t<<endl;
			exit(0);
		}
		//cout<<"Qout=\y"<<Qout[t]<<endl;

		//if(isDebug)
		//	if(t>=5430 && t<=6150)
		//		cout<<"Qout[t]="<<Qout[t]<<"Sout[t]="<<Sout[t]<<"\ttmp="<<tmp<<"\tSinMe[t]="<<SinMe[t]<<endl;
	}

}
float AVPM::GetSS(float Sv,float h,float U,float * omega)
{
	float SS;
	float omega0;//清水沉速
	float omegaS;//浑水沉速
	float kappa;//k
	float gammam;
	float tmp;
	
	if(Sv>Svm)
		Sv=Svm;
	else if(Sv<1e-10)
		Sv=1e-10;

	//清水沉速
	tmp=13.95f*nu/D50;
	omega0=sqrt(tmp*tmp+1.09f*(gammaS-gamma)/gamma*g*D50)-tmp;
	
	if(Sv/2.25f/sqrt(D50*1000.0f)>=0.9f)
		omegaS=0.001f*omega0;
	else
		omegaS=omega0*(1.0f-1.25*Sv)*pow(1.0f-Sv/2.25f/sqrt(D50*1000.0f),3.5f);//此式中D50单位为mm,黄河高含沙洪水模型的相似律,pp25
	
	gammam=gammaS*Sv+(1.0f-Sv)*gamma;
	
	kappa=0.4f-1.68*(0.365-Sv)*sqrt(Sv);
	
	//20051221 李铁键 张红武公式的ln(h/6/D50)部分是沿垂线积分后的部分，
	//D50是从推移质层厚中出来的,h是从水深中出来的
	//实际h应为水力半径R,只有在宽浅的冲积河流中，h取水深
	//本方法中，h应该是水力半径，R=(m/sqrt(m*m+1))*(h/2)
	//tmp=R/6/D50=m/sqrt(m*m+1)*h/12/D50
	tmp=h/12.0f/D50*m/sqrt(m*m+1.0f);
	if(tmp<1.00001f)
		SS=0.0f;
	else
		SS=2.5*pow((0.0022f+Sv)*U*U*U/kappa/((gammaS-gammam)/gammam)/g/h/omegaS*log(tmp),0.62f);
	
	//20051217,李铁键：增加含沙量异常时的调试信息
	if(isDebug && SS>2650)
		myFile<<"SS="<<SS<<"\ttmp="<<tmp<<"\tSv="<<Sv<<"\tKappa="<<kappa<<"\tOmegaS="<<omegaS<<"\tU="<<U<<"\tgammam="<<gammam<<endl;
	*omega=omegaS;
	

	return SS;
}

float AVPM::GetSSFei(float Sv,float h,float U,float * omega)  //heli
{
	//Sv:	体积比含沙量
	//RR:	水力半径 
	//本方法中，h应该是水力半径，RR=(m/sqrt(m*m+1))*(h/2)
	float RR=0.5f*m/sqrt(m*m+1)*h;

	float ks=2.0*D90;

	//initialize里已计算Svm,且应是log10()不是log()
	//float ss0=1.0/D90/1000.0;
	//float Svm=0.92-0.2*log(ss0);//界限体积比含沙量

	//20070724,xiaofc,Sv不能大于Svm
	if(Sv>0.9f*Svm)
		Sv=0.9f*Svm;

	float Rm=gamma+(gammaS-gamma)*Sv;//浑水密度 

	float tt=1.0-(Sv/Svm);
	float miu0=0.001; //清水动力粘滞系数 0.001（N*s）/（m^2）
	float miu=miu0*pow(double(tt),double(-2.0));

	//20070724,xiaofc,采用费祥俊的公式计算浑水沉速
	float omegaS;
	omegaS=(sqrt(10.99f*D90*D90*D90+36.0f*miu*miu/Rm/Rm)-6.0f*miu/Rm)/D90;
	*omega=omegaS;

	float Re=4.0*RR*U*Rm/9.81/miu;

	float ff=0.11*pow((ks/4.0/RR+68.0/Re),double(0.25) );

	float temp1=U/omegaS*sqrt(ff/8.0);
	float temp2=D90/4.0/RR;
	float SSfei;
	SSfei=0.0068*pow(double(temp1),double(1.5))*pow(double(temp2),double(1.0/6.0) );

	//如果计算的挟沙力大于极限含沙量，限制其值
	if(SSfei>0.99f*Svm)
		SSfei=0.99f*Svm;

	//返回质量含沙量
	return SSfei*gammaS;
}

void AVPM::SediCalc(void)
{
	//挟沙力
	float SSin;
	float SSout;
	float omega;//沉速
	float tmp;

	if((Q[t-1]+Q[t])<1e-5 || (Q[t-1]+Q[t]+Qout[t-1]+Qout[t])<1e-5)
	{
		Sout[t]=0.0f;
		return;
	}

	
	////20060317,李铁键，不必非要冲刷时重力侵蚀才发生作用
	//if(bCalcGravity && SlideAmount>1e-5)// && SSin>(mySin[t-1]+mySin[t])/2.0)//冲刷
	//{
	//	//把重力侵蚀的结果弄进来
	//	//20051128时用挟沙力作重力侵蚀量转为悬移质获得的浓度
	//	//SlideAmount-=(SSin-mySin[t])*delta_t*(Q[t-1]+Q[t])/2.0f;
	//	//mySin[t]=SSin;
	//	
	//	//20051206,以费祥俊极限含沙量作为重力侵蚀量转为悬移质获得的浓度
	//	//[费祥俊 黄河中下游含沙水流粘度的计算模型  泥沙研究 1991 (2)]
	//	//Svm=0.92-0.2*lg(1/D50) D50的量纲为mm
	//	SlideAmount-=(Svm*gammaS-mySin[t])*delta_t*(Q[t-1]+Q[t])/2.0f;
	//	mySin[t]=Svm*gammaS;	

	//	if(SlideAmount<0)
	//	{
	//		mySin[t]=mySin[t]+SlideAmount/delta_t/(Q[t-1]+Q[t])*2.0f;
	//		SlideAmount=0;
	//	}
	//	
	//	//20060321,李铁键,将计算挟沙力改到重力侵蚀计算之后，所以关掉了以下两句
	//	//SSin=GetSS((mySin[t-1]+mySin[t])/2.0/gammaS,(h[0]+h[1])/2.0,(u[0]+u[1])/2.0,&tmp);
	//	//omega=(tmp1+tmp)/2.0f;
	//}

	//20080306,xiaofc,重写重力侵蚀进入沟道的方法
	//改为slideamount>0时，alphaE=1, 一样按不平衡输沙计算
	float NowAlphaE;
	if (bCalcGravity &&SlideAmount>0)
		NowAlphaE=3.0f*alphaE;//1.0f;20080414,xiaofc,
	else
		NowAlphaE=alphaE;

	if(SSMethod=="Zhang")
	{
		SSin=GetSS((mySin[t-1]+mySin[t])/2.0/gammaS,(h[0]+h[1])/2.0,(u[0]+u[1])/2.0,&tmp);
		SSout=GetSS(Sout[t-1]/gammaS,h[2],u[2],&omega);
	}
	else if(SSMethod=="Fei")
	{	
		SSin=GetSSFei((mySin[t-1]+mySin[t])/2.0/gammaS,(h[0]+h[1])/2.0,(u[0]+u[1])/2.0,&tmp);  //heli
		SSout=GetSSFei(Sout[t-1]/gammaS,h[2],u[2],&omega);
	}
	else
		cout<<"Error of SSMethod"<<endl;


	//求上下游断面的挟沙力，用旧时间层
	omega=(omega+tmp)/2.0f;
	if(SSout>Sout[t-1])//冲刷
	{
		bIsDep=0;
		//tmp=alphaE*omega*delta_x*(B[0]+B[1]+B[2]+B[3])/(Q[t-1]+Q[t]+Qout[t-1]+Qout[t]);//a*w*L/q
		tmp=NowAlphaE*omega*delta_x*(B[0]+B[1]+B[2]+B[3])/(Q[t-1]+Q[t]+Qout[t-1]+Qout[t]);//a*w*L/q
	}
	else
	{
		bIsDep=1;
		tmp=alphaD*omega*delta_x*(B[0]+B[1]+B[2]+B[3])/(Q[t-1]+Q[t]+Qout[t-1]+Qout[t]);//a*w*L/q
	}

	//恒定不平衡输沙方程的积分式
	Sout[t]=SSout+((mySin[t-1]+mySin[t])/2.0-SSin)*exp(-tmp)+(SSin-SSout)/tmp*(1.0f-exp(-tmp));
	
	//20051217,李铁键：增加含沙量异常时的调试信息
	if(isDebug)
		myFile<<"t="<<t<<"\tSout[t]="<<Sout[t]<<"\tSSout="<<SSout<<"\tSSin="<<SSin<<"\tmySin[t-1]="<<mySin[t-1]<<"\tmySin[t]="<<mySin[t]<<"\ttmp="<<tmp<<endl;

	if(Sout[t]<0.0001f)
		Sout[t]=0.0001f;
	else if(Sout[t]>gammaS*Svm)
		Sout[t]=gammaS*Svm-1.0f;

	//20080306,xiaofc,处理slideamount的变化
	float DeltaSlideAmount;
	if (bCalcGravity &&SlideAmount>0&&bIsDep==0)
	{
		
		DeltaSlideAmount=(Sout[t]*Qout[t]-Q[t]*mySin[t])*delta_t;
		
		if(DeltaSlideAmount<0)
			return;

		if(DeltaSlideAmount>SlideAmount)
		{
			Sout[t]-=(DeltaSlideAmount-SlideAmount)/delta_t/Qout[t];
			DeltaSlideAmount=SlideAmount;
		}

		SlideAmount-=DeltaSlideAmount;
		GravityErosion[DayLoop]+=DeltaSlideAmount;
	}	
}

void AVPM::initialize(Para myPara)
{	
	//初始化变量
	////////////
	mPara=myPara;
	//计算
	//isDebug=1;
	nano=0.0002f;
	//delta_t=3600;
	//lTotalT=140*3600;
	//iTSteps=lTotalT/delta_t;

	//水流
	eta=0.75f;
	m=mPara.m;
	//if(mPara.DrainingArea>10000000000)
	//	m=mPara.DrainingArea/10000000000*m;
	//20051218 李铁键 沟道的糙率与坡面糙率区分 为RiverManning
	n0=mPara.RiverManning;
	L=mPara.StreamLength;
	//20080411,xiaofc,无定河专用
	S=mPara.StreamSlope;
	if (mPara.SoilType==5)
		//if (S<0.20)
			S=S*1.93f;
    if(S<nano)
		S=nano;
	if(S>0.4f)
		S=0.4f;

	//泥沙
	//wh,将下面变为注释
	/*char path[MAX_PATH];
	::GetModuleFileName(NULL,path,MAX_PATH);
	CString Str(path);
	int i=Str.ReverseFind('\\');
	CString m_FilePath=Str.Left(i+1);

	CString m_strFilePath=m_FilePath+"FilePath.ini";
	char temp[100];

	::GetPrivateProfileString("MODEL","SediTransCapF","Unknown",temp,sizeof(temp),m_strFilePath);
	SSMethod.Format("%s",temp);
	ZeroMemory(temp,sizeof(temp));*/
	
	gammaS=2650.0f;//kg/m3
	D50=mPara.D50/1000;//0.02*1e-3;量纲为m
	if(mPara.SoilType==5)
		D90=0.07*1e-3; //m
	else//无定河专用
		D90=0.12*1e-3;//m

	
	//分配内存空间
	Q=new float[iTSteps];
	//Qout=new float[iTSteps];
	mySin=new float[iTSteps];
	//Sout=new float[iTSteps];
	
	//20080303,xiaofc,沟道侵蚀量
	long DayCount;
	DayCount=iTSteps*MSTEP/60/24;
	
	//cout<<"DayCount:"<<DayCount<<endl;

	//20080629,wanghao将下面的DayCount都加上了1,否则delete时不对
	ChannelErosion=new float[DayCount+1];
	GravityErosion=new float[DayCount+1];
	
	InitGravityPara();
	InitGullyShap(mPara); 
	
	SlideAmount=0.0f;
	//极限体积比含沙量
	Svm=0.92f-0.2f*log10(1.0f/D50/1000.0f);
	if(!_finite (Svm))
	{
		cout<<"INFINITE Maximum Sediment Concentration in AVPM.initialize():"<<endl;
		cout<<"Regionindex="<<mBSCode.RegionIndex<<"\tBSValue="<<mBSCode.Value <<"\tBSLength="<<mBSCode.Length<<endl;
		cout<<"SediD="<<D50<<endl;
		exit(0);
	}
	
	if(isDebug)
	{
		myFile.open("Out.txt",ios_base::out | ios_base::app);
	}
}

void AVPM::finalize(void)
{
	//回收内存空间
	delete[] Q;
	//delete[] Qout;
	delete[] mySin;
	//delete[] Sout;
	
    delete [] ChannelErosion; //20080303,xiaofc,沟道侵蚀量
	delete [] GravityErosion;

	Q=NULL;
	//Qout=NULL;
	mySin=NULL;
	//Sout=NULL;

	if(isDebug)
		myFile.close();

}
//薛 我的函数定义在此开始

void AVPM::InitGravityPara()
{
	Hc=1;
	//mR=??;
	Angle_Fai=21.0f*pi/180.0f;
	//Angle_Fai=29.0f*3.1415f/180.0f;

	k=0.3f;
	//Water_cont=??;
	a=462.27f;       //凝聚力和含水量公式中的两个系数,适用于Q2黄土
	b=-2.5283f;
	
	//20060328,量纲错误,且要修正为Q2黄土
	//Cohesive_origin=12.0f;  //Q3黄土的初始凝聚力。单位 KN/m2
	Cohesive_origin=12000.0f;//Q2黄土的初始凝聚力,单位N/m2,要跟FR的N/m量纲相配合

	//薛


}

void AVPM::InitGullyShap(Para mPara)              //同时给左坡右坡初始化，为了后边调整水位
{    //用于把一个简单的坡变成两个坡，一缓一陡
	float cos_allL,cos_allR,cos_up;//,cos_down; 
	float SlopeScaling=3.0f;
	//无定河专用
	//cos_allL=1.0f/sqrt(1.0f+mPara.SlopeL*mPara.SlopeL);
	//cos_allR=1.0f/sqrt(1.0f+mPara.SlopeR*mPara.SlopeR);
	cos_allL=1.0f/sqrt(1.0f+SlopeScaling*SlopeScaling*mPara.SlopeL*mPara.SlopeL);
	cos_allR=1.0f/sqrt(1.0f+SlopeScaling*SlopeScaling*mPara.SlopeR*mPara.SlopeR);

	//20080306,xiaofc,沟坡区坡度改为提前设置的参数值，不再根据坡面坡度计算
	//if ((mPara.SlopeL<0.0f)||(abs(mPara.SlopeL)<1e-6) )
 //        Angle_downL=10.0f*pi/180.0f;
	//else 
	//{
	//   Angle_downL=2.0f*atan(mPara.SlopeL);   
	//   if(Angle_downL>=65.0f*pi/180.0f)
 //	      Angle_downL=65.0f*pi/180.0f;

	//} 
	
	Angle_downL=mPara.GullySlope*pi/180.0f;
	Angle_downR=Angle_downL;

	//if ((mPara.SlopeR<0.0f)||(abs(mPara.SlopeR)<1e-6) )
	//	Angle_downR=10.0f*pi/180.0f;
	//else 
	//{
	//	Angle_downR=2.0f*atan(mPara.SlopeR);   
	//	if(Angle_downR>=65.0f*pi/180.0f)
	//		Angle_downR=65.0f*pi/180.0f;

	//} 
	


	mL=1.0f/tan(Angle_downL);
	mR=1.0f/tan(Angle_downR);
	//Length_up=2.0f/3.0f*Slope_L*cos_all/cos(Angle_up);
	//无定河专用
	//Length_downL=1.0f/2.0f*mPara.SlopeL*cos_allL/cos(Angle_downL);
	//Length_downR=1.0f/2.0f*mPara.SlopeR*cos_allR/cos(Angle_downR);
	Length_downL=1.0f/2.0f*mPara.SlopeL*SlopeScaling*cos_allL/cos(Angle_downL);
	Length_downR=1.0f/2.0f*mPara.SlopeR*SlopeScaling*cos_allR/cos(Angle_downR);
	
	//20080306,xiaofc,沟坡区高度计算有误
	//HeightL=1.0f/2.0f*mPara.SlopeL*cos_allL*tan(Angle_downL);    	//沟坡高度
	//HeightR=1.0f/2.0f*mPara.SlopeR*cos_allR*tan(Angle_downR); 
	//无定河专用
	//HeightL=0.3f*mPara.SlopeL*mPara.LengthL;
	//HeightR=0.3f*mPara.SlopeR*mPara.LengthR;
	HeightL=0.3f*mPara.SlopeL*SlopeScaling*mPara.LengthL;
	HeightR=0.3f*mPara.SlopeR*SlopeScaling*mPara.LengthR;
	
	Height1L=HeightL;     //开始让竖直面高度为0，也就是没有侧向冲刷
	Height1R=HeightR;
	Height_tL=HeightL*k;
	Height_tR=HeightR*k;
	Slope_AL=mPara.AreaL;
	Slope_AR=mPara.AreaR;
	Slope_LL=mPara.LengthL;
	Slope_LR=mPara.LengthR;
	Gama_d=gammaS/(1.0f-mPara.MSita1-mPara.MSita2);
	// Gama_wm=(1.0f+W_cont/100.0f)*Gama_d;       //此处想给出沟坡土体在某一个含水量下的容重，怎么给？  N/m2


} 
void AVPM::ConvertTao(float H_avg)
{
	float h_add; //由于沟坡比小飞虫的陡而导致的水深增量
	float B1;//,B2; //见沟坡转换图中的标识
	float delt_A; // 附加的面积
	float temp_Tao; //按小飞虫的水深计算的侧向切应力

	delt_A=1.0f/2.0f*pow((H_avg-Hc),2.0f)*(2.0f*m-mL-mR); 
	B1=2.0f*Hc*m+(H_avg-Hc)*(mL+mR);
	h_add=(-2.0f*B1+sqrt(4.0f*B1*B1+8.0*(mL+mR)*delt_A))/(2.0f*(mL+mR));
	temp_Tao=1000.0f*9.8f*m*H_avg/(2.0f*sqrt(1.0f+m*m));                        //  N/m3
	Tao=temp_Tao*(2.0f*H_avg*sqrt(1.0+m*m)/(2.0f*Hc*sqrt(1.0+m*m)+sqrt(1.0+mL*mL)*(H_avg+h_add-Hc)+sqrt(1.0+mR*mR)*(H_avg+h_add-Hc)));

}

inline float AVPM::GenRand(float min,float max)
{
	srand( (unsigned)time( NULL ) );
	return  min+(max-min)*rand()/RAND_MAX;
}



void AVPM::CalFs(int LR)   //LR 0:左，1:右
{
	switch(LR){
	   case 0:{
		   Height=HeightL;
		   Height1=Height1L;
		   Height_t=Height_tL;
		   Length_down=Length_downL;
		   Angle_down=Angle_downL;
		   break;
			  }
	   case 1: {

		   Height=HeightR;
		   Height1=Height1R;
		   Height_t=Height_tR;
		   Length_down=Length_downR;
		   Angle_down=Angle_downR;
		   break;

			   }
	}

	Tao_c=6680.0f*D50+3.67e-6/D50;   // N/m2 D50量纲为m

	if(Tao>Tao_c)
	{

		Delt_B=3.64e-4*(Tao-Tao_c)*exp(-1.3f*Tao_c)*6.0f;  //计算横向冲刷宽度,后边的6是6分钟时间间隔

	}
	else
	{
		Delt_B=0.0f;

	}

	//20060328,要累计Height1的变化
	//Height1=Height-Delt_B*tan(Angle_down);
	Height1-=Delt_B*tan(Angle_down);
	if(Height1<0)
		Height1=0;
	if(LR==0)
		Height1L=Height1;
	else
		Height1R=Height1;

	Angle_beta=0.5f*(atan(Height/Height1*(1.0f-k*k)*tan(Angle_down))+Angle_Fai);   //此处Angle_Fai应为弧度
	
	//20080306,xiaofc,如果坡坏角>沟坡坡度，显然不发生滑坡
	if(Angle_beta>=Angle_down)
	{	
		Fs=999;
		return;
	}

	T=0.5f*1000.0f*g*pow(Height_t,2.0f);          //N/m

	//20060328,修正量纲的问题
	Wt=Gama_wm*0.5f*((Height*Height-Height_t*Height_t)/tan(Angle_beta)-Height1*Height1/tan(Angle_down));   // kg/m
	Wt=Wt*g;//N/m

	FD=Wt*sin(Angle_beta)+T*cos(Angle_beta);                  //下滑力,N/m

	Cohesive_addi=a*pow(W_cont,b)*g*10000;                            //这个a和b，在init里初始化，严格意义上各坡面应该有差别,这里的含水量单位是％,Cohesive_addi的单位是N/m2
	FR=(Cohesive_origin+Cohesive_addi)*(Height-Height_t)/sin(Angle_beta)+Wt*cos(Angle_beta)*tan(Angle_Fai);//N/m
	Fs=FR/FD;


}

void AVPM::CalFuzzProbi(float Cigema)
{ 
	//本函数根据计算的FS及规定的方差计算失稳可能性
	int row=0,col=0;
	int i=0,j=0;

	if (Cigema==0.05f) 
	{
		col=1;
	}
	else if (Cigema==0.1f)
	{
		col=2;
	}
	else if (Cigema==0.15f)
	{
		col=3;
	}

	else if (Cigema==0.2f)
	{
		col=4;
	}
	else if (Cigema==0.25f)
	{
		col=5;
	}

	else if (Cigema==0.3f)
	{
		col=6;
	}

	if (Fs>1.5f)
		Probability=0.0f;
	else if (Fs<0.29f) 
		Probability=1.0f;
	else 
	{
		for(i=0;i<=128;i++)
		{   if (Fs==fuzzpro[i][0]) 
		Probability=fuzzpro[i][col]; 
		if ((Fs>fuzzpro[i][0]) && (Fs<fuzzpro[i+1][0]))        //如果介于某两个FS之间 
		{
			Probability=1.0f/2.0f*(fuzzpro[i][col]+fuzzpro[i+1][col]);
			break;
		}
		else
			continue;

		}
	}
}


void AVPM::HowMuchFailed(int LR)
{
	float randnum;      //定义一个随机数，用它模拟垮塌与否
	float ThisTimeAmount;
	randnum=GenRand(0,1);
	switch(LR){
	   case 0:{
		   Slope_A= Slope_AL;
		   Slope_L=Slope_LL;
		   Height=HeightL;
		   Height1=Height1L;
		   Height_t=Height_tL;
		   Angle_down=Angle_downL;
		   break;
			  }
	   case 1:{
		   Slope_A= Slope_AR;
		   Slope_L=Slope_LR;
		   Height=HeightR;
		   Height1=Height1R;
		   Height_t=Height_tR;
		   Angle_down=Angle_downR;
		   break;
			  }
	}

	if (SlideAmount<0.01f && randnum<Probability) 
	{
		ThisTimeAmount=0.5f*((Height*Height-Height_t*Height_t)/tan(Angle_beta)-Height1*Height1/tan(Angle_down))*Slope_A/Slope_L*P2;  //用坡体失稳面积乘以坡的平均宽度
		
		//20060328,算出坍塌体积之后,修正水流冲刷引起的直立面高度
		//mytemp=Height1*Height1+2.0f*tan(Angle_down)*ThisTimeAmount;
		//mytemp=sqrt(Height1*Height1+2.0f*tan(Angle_down)*ThisTimeAmount);
		Height1=sqrt(Height1*Height1+2.0f*tan(Angle_down)*ThisTimeAmount);
		if(LR==0)
			Height1L=Height1;
		else
			Height1R=Height1;
		
		//把体积转化成泥沙质量
		ThisTimeAmount*=(1.0f-mPara.MSita1-mPara.MSita2)*gammaS;
	}
	else
	{
		ThisTimeAmount=0.0f;

	}
	//switch(LR){
	//   case 0:{
	//	   SlideAmountL += SlideAmount;
	//	   break;  
	//		  }
	//   case 1:{
	//	   SlideAmountR += SlideAmount;
	//		  }
	//}
	
	SlideAmount+=ThisTimeAmount;

	//20060327,李铁键,记录重力侵蚀事件
	GravityEvent myGEvent;
	if(pGravityEvents && ThisTimeAmount>0)
	{
		//cout<<"Added to Vec"<<endl;
		myGEvent.TimeStep=t;
		if(LR==0)
			myGEvent.LR='L';
		else
			myGEvent.LR='R';
		myGEvent.FD=FD;
		myGEvent.FR=FR;
		//myGEvent.Probability=Probability;
		//myGEvent.Probability=W_cont;
        myGEvent.Probability=Probability;
		//myGEvent.Amount=ThisTimeAmount;
		 myGEvent.Amount=ThisTimeAmount;

		pGravityEvents->push_back(myGEvent);
	}
    
}



// 保存到status表的数据
void AVPM::SaveStatus(void)
{
	//处理虚河段
	if(L<1e-5) return;

	CString SQL;
	_bstr_t bSQL;
	ADODB::_RecordsetPtr pRstStat;
	pRstStat.CreateInstance(__uuidof(ADODB::Recordset));
	
	//wh,增加了sccd
	SQL.Format("SELECT sccd,Regionindex,bsvalue,bslength,houroffset,channelero,gravityero from Status where sccd=%s and RegionIndex=%I64u and BSValue=%I64u and BSLength=%d and HourOffset>%d and HourOffset<=%d ORDER BY HourOffset",sccd,mBSCode.RegionIndex,mBSCode.Value,mBSCode.Length,HourStart,HourStart+iTSteps*MSTEP/60);
	bSQL=SQL.GetString();
	pRstStat->Open(bSQL,(ADODB::_Connection*)pCnn,ADODB:: adOpenKeyset,ADODB::adLockOptimistic,ADODB::adCmdText);
	pRstStat->MoveFirst();
	pCnn->BeginTrans();
	long i;
	try
	{
		for( i=0;i<pRstStat->RecordCount;i++)
		{
			pRstStat->Fields->Item["ChannelEro"]->Value=ChannelErosion[i];//20080303,xiaofc,沟道侵蚀量
			pRstStat->Fields->Item["GravityEro"]->Value=GravityErosion[i];
			pRstStat->MoveNext();
		}
		pRstStat->UpdateBatch(ADODB::adAffectAll);
	}
	catch (_com_error &e) 
	{
		cout<<"Error Occured  While Writing AVPM Output to DB."<<endl;
		cout<<"RegionIndex="<<mBSCode.RegionIndex<<"\tBSValue="<<mBSCode.Value<<"\tBSLength="<<mBSCode.Length<<endl;
		cout<<"Error Occured in the Day Begin at the HourOffset of "<<HourStart+i*24<<endl;
		cout<<"ChannelErosion="<<ChannelErosion[i]<<"\tGravityErosion="<<GravityErosion[i]<<endl;
		cout<<"SQL: "<<bSQL<<endl;

		_bstr_t eTem;
		long el;
		el=e.Error();
		eTem=e.Description();
		cout<<"HRESULT="<<el<<"\tErrMessage="<<eTem<<endl;

		exit(0);
	}

	pCnn->CommitTrans();
	pRstStat->Close();
}
