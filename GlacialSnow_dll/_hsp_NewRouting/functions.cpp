#include "functions.h"


//tmpUnitSize��������ȣ�MinUnitSize����С����
TreeList GetBranchInForest(TreeList* TotalList, int tmpUnitSize, int MinUnitSize, int GradeTwoCount)
{
	//��ɭ������һ���㹻�������
	int GradeTwoLoop=0;//���Ҫѭ�����Ծ�����ɭ�����ҵ����������
	TreeList TreeBranch;
	TreeBranch.First=NULL;
	while( TreeBranch.First==NULL && tmpUnitSize>=MinUnitSize )
	{
		//wh���Ӹ���RegionIndex��ͷ�ڵ�������
		TreeBranch=TotalList[GradeTwoLoop].GetBranch(tmpUnitSize);//wh��ʵ����������[int((BranchSize-1)/2)��tmpUnitSize]���У�Ҳ����˵ʵ�ʲü����������ȿ��ܱ�MinUnitSizeС��
		GradeTwoLoop++;

		//����ֻ�һ�ܻ�û�ҵ�����ô��ģ���룬��ͷ��ʼ
		if(GradeTwoLoop==GradeTwoCount)
		{
			GradeTwoLoop=0;
			tmpUnitSize=tmpUnitSize/2;
		}
	}

	return TreeBranch;//����Ϊ�գ���ʱ�Ҳ������ʹ�ģ����
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
//��˹��������ģ��
void Transform(float* QIn1, float* QIn2, float* QIn3, float* QOut,int TimeInterval,long DataCount,Para * mPara)
{
	//Transform(Q1+Q2)+Q3=Qout
	float wavevelocity;
	//wetRadius:ˮ���뾶
	wavevelocity=mPara->WaveCoefficient*pow(mPara->WetRadius,2.0f/3)*sqrt(mPara->StreamSlope)/mPara->Manning;
	wavevelocity=4.0f;
	float k=mPara->StreamLength/wavevelocity;//k�൱�ں�ˮ���ںӶ��еĴ���ʱ��
	float c2=(0.5f*TimeInterval+k-k*mPara->x);//�൱��ˮ��ѧ����c0c1c2���ʽ�ķ�ĸ
	float c0=(0.5f*TimeInterval-k*mPara->x)/c2;
	float c1=(0.5f*TimeInterval+k*mPara->x)/c2;
	c2=1.0f-c0-c1;

	//cout<<c0<<c1<<c2<<endl;
	float tmp;//��QIn1[i]+QIn2[i]
	float ltmp;//��QIn1[i-1]+QIn2[i-1]
	float lnew;//����ʱ����ֵ������QIn3

	//��һ��ʱ���Ҫ���⴦��
	tmp=QIn1[0]+QIn2[0];
	QOut[0]=QIn3[0]+tmp;
	ltmp=tmp;//��һʱ�νڵ��������
	lnew=tmp;//��һʱ�νڵ��������

	//�����Ǿ���Muskingum�㷨
	//c0*��ʱ�ξ�ֵ+c1*��ʱ�ξ�ֵ+c2*��ʱ����ֵ
	for(long i=1;i<DataCount;i++)
	{
		tmp=QIn1[i]+QIn2[i];
		//cout<<QIn1[i]<<"\t"<<QIn2[i]<<"\t"<<tmp<<"\t";
		lnew=c0*tmp+c1*ltmp+c2*lnew;//O2 = c0*I2+c1*I1+c2*O2;
		//cout<<lnew<<"\n";
		QOut[i]=QIn3[i]+lnew;//�������ڲ��������εĻ�����������Ϊ����ֱ�ӵ������ˡ�
		ltmp=tmp;
		//if(i==1) cout<<QIn1[i]<<"\t"<<QIn2[i]<<"\t"<<QIn3[i]<<"\t"<<QOut[i]<<endl;
	}
	//�����Ǿ���Muskingum�㷨
}


//������Сʱ
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


//�õ�һ�����ж���d��
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

//ͨ������YearStart,MonthStart,DayStart,HourStart������(TimeStart),��������ղ��洢��*pYear,*pMonth,*pDay��
void HourToDate(float TimeStart,short YearStart,short MonthStart,short DayStart,float HourStart,short* pYear,short* pMonth,short* pDay,float* pHour)
{
	float Hours=TimeStart/3600;
	//YearStart,MonthStart,DayStart:��׼�꣬��׼�£���׼��
	int DayCount=int((Hours+HourStart-1)/24)+1;  //ת��СʱΪd

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


////׷�Ϸ�������Խ����Է�����
//template<class T>
//void TDMA(int N,T *a,T *b,T *c,T *f,T *x)
//{
//	//N:���������,a:���¶Խ���N-1,b:�Խ���N,c:���϶Խ���N-1,f:�������Ҷ���N,x:������N
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
////������
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

