#include "functions.h"


//tmpUnitSize：最大粒度；MinUnitSize：最小粒度
TreeList GetBranchInForest(TreeList* TotalList, int tmpUnitSize, int MinUnitSize, int GradeTwoCount)
{
	//在森林里找一个足够大的子树
	int GradeTwoLoop=0;//这个要循环，以尽量在森林里找到够大的子树
	TreeList TreeBranch;
	TreeBranch.First=NULL;
	while( TreeBranch.First==NULL && tmpUnitSize>=MinUnitSize )
	{
		//wh，从各个RegionIndex的头节点往下找
		TreeBranch=TotalList[GradeTwoLoop].GetBranch(tmpUnitSize);//wh，实际上找是在[int((BranchSize-1)/2)，tmpUnitSize]进行，也就是说实际裁剪的任务粒度可能比MinUnitSize小。
		GradeTwoLoop++;

		//如果轮回一周还没找到，那么规模减半，重头开始
		if(GradeTwoLoop==GradeTwoCount)
		{
			GradeTwoLoop=0;
			tmpUnitSize=tmpUnitSize/2;
		}
	}

	return TreeBranch;//可能为空，此时找不到合适规模的树
}

bool IsEmptyForest(TreeList* TotalList, int GradeTwoCount)
{
	for(int i=0;i<GradeTwoCount;i++)
		if(TotalList[i].First)
			return false;
	return true;
}

//template<class T>
//void ZeroFill(T* Qs, long DataCount)
//{
//	for(long i=0;i<DataCount;i++)
//		Qs[i]=0.0f;
//}

void SwapQp(float** Q1,float** Q2)
{
	float *QTemp;
	QTemp=*Q1;
	*Q1=*Q2;
	*Q2=QTemp;
}

//wh,QIn1 is girl's,QIn2 is boy's,QIn3 is myself's wateryield,QQout is what I want.
//马斯京根汇流模型
void Transform(float* QIn1, float* QIn2, float* QIn3, float* QOut,int TimeInterval,long DataCount,Para * mPara)
{
	//Transform(Q1+Q2)+Q3=Qout
	float wavevelocity;
	//wetRadius:水力半径
	wavevelocity=mPara->WaveCoefficient*pow(mPara->WetRadius,2.0f/3)*sqrt(mPara->StreamSlope)/mPara->Manning;
	wavevelocity=4.0f;
	float k=mPara->StreamLength/wavevelocity;//k相当于洪水波在河段中的传播时间
	float c2=(0.5f*TimeInterval+k-k*mPara->x);//相当于水文学书中c0c1c2表达式的分母
	float c0=(0.5f*TimeInterval-k*mPara->x)/c2;
	float c1=(0.5f*TimeInterval+k*mPara->x)/c2;
	c2=1.0f-c0-c1;

	//cout<<c0<<c1<<c2<<endl;
	float tmp;//存QIn1[i]+QIn2[i]
	float ltmp;//存QIn1[i-1]+QIn2[i-1]
	float lnew;//存上时段新值，不含QIn3

	//第一个时间的要特殊处理
	tmp=QIn1[0]+QIn2[0];
	QOut[0]=QIn3[0]+tmp;
	ltmp=tmp;//第一时段节点入口流量
	lnew=tmp;//第一时段节点出口流量

	//以下是经典Muskingum算法
	//c0*本时段旧值+c1*上时段旧值+c2*上时段新值
	for(long i=1;i<DataCount;i++)
	{
		tmp=QIn1[i]+QIn2[i];
		//cout<<QIn1[i]<<"\t"<<QIn2[i]<<"\t"<<tmp<<"\t";
		lnew=c0*tmp+c1*ltmp+c2*lnew;//O2 = c0*I2+c1*I1+c2*O2;
		//cout<<lnew<<"\n";
		QOut[i]=QIn3[i]+lnew;//出流等于产流加上游的汇流，看来认为产流直接到出口了。
		ltmp=tmp;
		//if(i==1) cout<<QIn1[i]<<"\t"<<QIn2[i]<<"\t"<<QIn3[i]<<"\t"<<QOut[i]<<endl;
	}
	//以上是经典Muskingum算法
}


//想距多少小时
long GetNumOfHour(short YearS,short MonthS,short DayS,short HourS,short YearE,short MonthE,short DayE,short HourE)
{
	long m_iResult=0,sum=0;
	if(YearS>YearE) return -1;
	if((YearS==YearE) && (MonthS>MonthE) ) return -1;
	if((YearS==YearE) && (MonthS==MonthE) && (DayS>DayE) ) return -1;
	if((YearS==YearE) && (MonthS==MonthE) && (DayS==DayE)  && (HourS>HourE)) return -1;

	if(MonthS==MonthE && YearS==YearE)
	{
		m_iResult=(DayE-DayS)*24+(HourE-HourS);
		return m_iResult;
	}
	short CurYear,CurMonth,NextYear,NextMonth;
	CurYear=YearS;
	CurMonth=MonthS;
	sum=sum+GetMonthDays(CurYear,CurMonth);
	MonthAdd(CurYear,CurMonth,NextYear,NextMonth);
	while(!((NextYear==YearE) && (NextMonth==MonthE)))
	{
		sum=sum+GetMonthDays(NextYear,NextMonth);
		CurYear=NextYear;
		CurMonth=NextMonth;
		MonthAdd(CurYear,CurMonth,NextYear,NextMonth);
	}
	m_iResult=(sum+(DayE-DayS))*24+(HourE-HourS);
	
	return m_iResult;
}


//得到一个月有多少d。
short GetMonthDays(short year,short month)
{
	bool IsLeapYear=0;
	if(((year%4)==0 && (year%100)!=0) || (year%400)==0 )
	{
		IsLeapYear=true;
	}

	if(month==2 &&  IsLeapYear)  return 29;
	if(month==2 && !IsLeapYear)  return 28;
	if(month==8)  return 31;
	if(month<8 && month%2==0) return 30;
	if(month<8 && month%2!=0) return 31;
	if(month>8 && month%2==0) return 31;
	return 30;
}


void MonthAdd(short CurYear, short CurMonth, short &NextYear, short &NextMonth)
{
	if(CurMonth==12){
		NextYear=CurYear+1;
		NextMonth=1;
		return ;
	}
	NextYear=CurYear;
	NextMonth=CurMonth+1;
	return;
}

//通过距离YearStart,MonthStart,DayStart,HourStart的秒数(TimeStart),获得年月日并存储在*pYear,*pMonth,*pDay中
void HourToDate(float TimeStart,short YearStart,short MonthStart,short DayStart,float HourStart,short* pYear,short* pMonth,short* pDay,float* pHour)
{
	float Hours=TimeStart/3600;
	//YearStart,MonthStart,DayStart:基准年，基准月，基准日
	int DayCount=int((Hours+HourStart-1)/24)+1;  //转换小时为d

	int temp=0;
	while(temp+GetMonthDays(YearStart,MonthStart)<DayCount)
	{
		temp+=GetMonthDays(YearStart,MonthStart);
		MonthStart++;
		if(MonthStart==13)
		{
			MonthStart=1;
			YearStart++;
		}
	}
	DayStart=short(DayStart+DayCount-temp);
	temp=DayStart-GetMonthDays(YearStart,MonthStart);
	if(temp>0)
	{
		MonthStart++;
		if(MonthStart==13)
		{
			MonthStart=1;
			YearStart++;
		}
		DayStart=short(temp);
	}
	 (*pYear) = YearStart;
	(*pMonth) = MonthStart;
	  (*pDay) = DayStart;
	 (*pHour) = (Hours+HourStart)-short((Hours+HourStart)/24)*24;
}


////追赶法求解三对角线性方程组
//template<class T>
//void TDMA(int N,T *a,T *b,T *c,T *f,T *x)
//{
//	//N:方阵的行数,a:次下对角线N-1,b:对角线N,c:次上对角线N-1,f:方程组右端项N,x:计算结果N
//	for(int i=0;i<N-1;i++)
//	{
//		c[i]=c[i]/b[i];
//		b[i+1]=b[i+1]-a[i]*c[i];
//		if(i==0) 
//		{
//			f[i]=f[i]/b[i];  
//		}
//		else 
//		{
//			f[i]=(f[i]-a[i-1]*f[i-1])/b[i];
//		}
//	}
//
//	x[N-1]=(f[N-1]-a[N-2]*f[N-2])/b[N-1];
//
//	for(int i=N-2;i>=0;i--)
//	{
//		x[i]=f[i]-c[i]*x[i+1];
//	}
//}
//
////向量长
//template<class T>
//float VecDistance(int N,T *a)
//{
//	float Dis=0;
//	for(int i=0;i<N;i++)
//	{
//		Dis+=a[i]*a[i];
//	}
//	return sqrt(Dis);
//}

