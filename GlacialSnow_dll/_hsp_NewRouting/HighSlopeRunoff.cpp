#include "HighSlopeRunoff.h"
#include <cmath>
#include "functions.h"
#include <algorithm>

long HighSlopeRunoff::NumofHours=1; long HighSlopeRunoff::HourStart=438288; float HighSlopeRunoff::E=0.0f;      
float HighSlopeRunoff::P=0.0f;      bool HighSlopeRunoff::debug=true;  
Para* HighSlopeRunoff::myPara;      BSCode HighSlopeRunoff::mBSCode; WaterInRockSnow HighSlopeRunoff::Rock;

//按照pair第二个元素从大到小排列
bool compare_1(pair<int,int> i1,pair<int,int> i2)
{
	return (i1.second>i2.second)?true:false;
}

HighSlopeRunoff::HighSlopeRunoff(void)
:theta1(PI/4),theta2(PI/4)
{
}

//所有河段计算完毕后释放，与Initialize相对应
HighSlopeRunoff::~HighSlopeRunoff(void)
{
	if(debug){ myFile.close();}
}

//从计算进程传入，对所有河段的计算全局不变，和构造函数作用相同
void HighSlopeRunoff::Initiallize(ADODB::_ConnectionPtr pCnn0,CString RainType0,long NumofHours0,long HourStart0,long StatusTime0,float BasinArea0,int MSTEP0,int Steps0)
{
	/**************************************来自hydrousepara表的系统参数**************************************/
	pCnn=pCnn0;
	Steps=Steps0;
	
	//计算结果采样周期
	MSTEP=MSTEP0; 
	MSTEP*=60;//min->s
	
	BasinArea=BasinArea0; 
	RainType=RainType0;   
	HourStart=HourStart0;
	NumofHours=NumofHours0;
	StatusTime=StatusTime0;
	
	//mSaveStatus.pCnn = this->pCnn;
	PMin.pConnection = this->pCnn;	
	PMinCBG.pCon = this->pCnn;
	
	/****************************************来自HSPELEVATION表的参数***************************************/
	CString cSQL;
	ADODB::_RecordsetPtr pRst;
	pRst.CreateInstance(__uuidof(ADODB::Recordset));
	pRst->CursorLocation = ADODB::adUseClient;
	
	//高程小于0时认为不存在该类型，高程从高到低排列
	cSQL.Format("select * from hspelevation where elevation>=0 order by elevation desc");

	try
	{
		int Landtype,Elevation,i=0;
		pRst->Open(cSQL.GetString(),(ADODB::_Connection*)pCnn,ADODB::adOpenStatic,ADODB::adLockOptimistic,ADODB::adCmdText);
		while(!pRst->EndOfFile)
		{
			i++;
			Landtype=pRst->Fields->Item["landtype"]->Value;//0:雪,1:林,2:灌,3:草,4:耕

			if(Landtype>4 || Landtype<0)
			{
				cout<<"wrong landtype value in table hspelevation."<<endl;
				return;
			}

			Elevation=pRst->Fields->Item["elevation"]->Value;//表中单位：m
			
			//保证高程严格从大到小，不存在相等项
			if(i==1)
			{ 
				lv.push_back(make_pair(Landtype,Elevation));
				FVMunit::dc[Landtype]=pRst->Fields->Item["dc"]->Value;//植被覆盖率无量纲(0,1)

				FVMunit::fc[Landtype]=pRst->Fields->Item["fc"]->Value;//表中单位：m/d
				FVMunit::fc[Landtype]=FVMunit::fc[Landtype]/24/3600;//转化为m/s

				FVMunit::thetai[Landtype]=pRst->Fields->Item["thetai"]->Value;//初始土壤含水量(0,1)
			}
			else if(lv[lv.size()-1].second!=Elevation)
			{ 
				lv.push_back(make_pair(Landtype,Elevation));
				FVMunit::dc[Landtype]=pRst->Fields->Item["dc"]->Value;//植被覆盖率无量纲(0,1)

				FVMunit::fc[Landtype]=pRst->Fields->Item["fc"]->Value;//表中单位：m/d
				FVMunit::fc[Landtype]=FVMunit::fc[Landtype]/24/3600;//转化为m/s

				FVMunit::thetai[Landtype]=pRst->Fields->Item["thetai"]->Value;//初始土壤含水量(0,1)
			}

			pRst->MoveNext();
		}
		pRst->Close();

		//以下是保证最高高程的一定是雪，即使类型不是雪，和是雪的类型交换，防止程序出错
		if(lv[0].first!=0)
		{
			for(int i=0;i<lv.size();i++)
			{
				if(lv[i].first==0)
				{
					lv[i].first=lv[0].first;
					break;
				}
			}
			lv[0].first=0;
		}
	}
	catch(_com_error e)
	{
		cout<<"Failture Of Opening Table HSPELEVATION."<<endl;
		cout<<e.Error()<<endl;
		cout<<e.ErrorMessage()<<endl;
	    cout<<(LPCSTR)e.Source()<<endl;        
		cout<<(LPCSTR)e.Description()<<endl;  
		exit(-1);
	}
	/*****************************************来自HSPELEVATION表参数****************************************/
	/*****************************************来自HSPUSEPARA表的参数****************************************/
	_variant_t tmp;
	CString temp;
	cSQL.Format("select * from hspusepara");
	try
	{
		pRst->Open(cSQL.GetString(),(ADODB::_Connection*)pCnn,ADODB::adOpenStatic,ADODB::adLockOptimistic,ADODB::adCmdText);
		if(!pRst->EndOfFile)
		{
			Min.theta1=pRst->Fields->Item["thetal"]->Value;//表中单位：度
			Min.theta1=Min.theta1/180*PI;//转化为弧度
			Min.theta2=pRst->Fields->Item["thetar"]->Value;//表中单位：度
			Min.theta2=Min.theta2/180*PI;//转化为弧度

			Min.B0=pRst->Fields->Item["width"]->Value;//表中单位：m
			Min.Q0=pRst->Fields->Item["discharge"]->Value;//表中单位：m3/s

			Min.Emax=pRst->Fields->Item["Emax"]->Value;//表中单位：mm
			Min.Emax/=1000;//转化为m
			Min.Emin=pRst->Fields->Item["Emin"]->Value;//表中单位：mm
			Min.Emin/=1000;//转化为m

			Min.sDeep=pRst->Fields->Item["erosionmax"]->Value;//表中单位：m
			
			//BSTR型转化为string型，没查到怎么直接转，先转化成CString，再转成string
			tmp = pRst->Fields->Item["elevationtype"]->Value;
			temp.Format((LPCTSTR)(_bstr_t)tmp.bstrVal);
			//CString转化成string
			Min.hType=temp.GetBuffer();
			temp.ReleaseBuffer();

			tmp=pRst->Fields->Item["slopeshape"]->Value;//"par" or "rec" or "tri"
			temp.Format((LPCTSTR)(_bstr_t)tmp.bstrVal);
			Min.sShape=temp.GetBuffer();
			temp.ReleaseBuffer();

			Min.Kr=pRst->Fields->Item["KRock"]->Value;//表中单位：m/d
			Min.Kr=Min.Kr/24/3600;//转化为m/s

			Min.mu=pRst->Fields->Item["MuRock"]->Value;

			Min.dt1=pRst->Fields->Item["dt1"]->Value;//表中单位：min
			Min.dt1*=60;//转化为s
			Min.dt2=pRst->Fields->Item["dt2"]->Value;//表中单位：min
			Min.dt2*=60;//转化为s

			Min.dx=pRst->Fields->Item["dx"]->Value;//表中单位：m
			Min.dz=pRst->Fields->Item["dz"]->Value;//表中单位：m

			Min.sRatio=pRst->Fields->Item["sRatio"]->Value;//相对不透水层比例
			Min.wRatio=pRst->Fields->Item["wRatio"]->Value;//沟道地下水比例
			Min.rRatio=pRst->Fields->Item["rRatio"]->Value;//岩石裂隙水比例

			Min.SaveFlowPattern=pRst->Fields->Item["SaveFlowPattern"]->Value;//0 or 1
			debug=pRst->Fields->Item["debug"]->Value;//0 or 1
		}
		pRst->Close();
	}
	catch(_com_error e)
	{
		cout<<"Failture Of Opening Table HSPUSEPARA."<<endl;
		cout<<e.Error()<<endl;
		cout<<e.ErrorMessage()<<endl;
	    cout<<(LPCSTR)e.Source()<<endl;        
		cout<<(LPCSTR)e.Description()<<endl;  
		exit(-1);
	}

	if(Min.dz<=0) { Min.dz=1;}
	FVMunit::dz=Min.dz;
	
	//处理时间步长
	if(Min.dt1>3600  || Min.dt1<=0) { Min.dt1=3600; }
	if(Min.dt2>3600  || Min.dt2<=0) { Min.dt2=3600;}
	if(Min.dt2<Min.dt1) { Min.dt2=Min.dt1; }//保证土壤计算时间步长大于坡面部分
	FVMunit::dt1=Min.dt1;  
	FVMunit::dt2=Min.dt2;

	if(Min.Emax<0) { Min.Emax=0.1;}
	if(Min.Emin<0) { Min.Emax=0.05;}
	FVMunit::Emax=Min.Emax;
	FVMunit::Emin=Min.Emin;
	/*****************************************来自HSPUSEPARA表的参数****************************************/

	if(debug)
	{
		//trunc:是将文件长度置为0,app是以追加方式来写文件
		myFile.open("Out.txt",ios_base::out | ios_base::trunc);
	}
 
}


//每计算一条河段就初始化一次
void HighSlopeRunoff::InitializeOneRiver(BSCode mBSCode0,Para* myPara0,float* Qu10,float* Qu20,float* Qd10,float* Qd20,float* Quout0,float* Qdout0,float* FlowB0,float* FlowH0,float* FlowV0)
{
	xL=axL=fL=afL=aL=bL=cL=dL=NULL;
	xR=axR=fR=afR=aR=bR=cR=dR=NULL;
	xS=axS=fS=afS=aS=bS=cS=dS=NULL;

	Quout=Quout0;//当前河段"明渠水"出流过程   
	Qdout=Qdout0;//当前河段"地下水"出流过程
	Qu1=Qu10;//上游支流1明渠水       
	Qu2=Qu20;//上游支流2明渠水
	Qd1=Qd10;//上游支流1地下水       
	Qd2=Qd20;//上游支流2地下水
	FlowB=FlowB0;
	FlowH=FlowH0;
	FlowV=FlowV0;
	myPara=myPara0; 
	mBSCode=mBSCode0;
	
	if(RainType.MakeLower()=="dayaverage")
	{
		PMin.Initialize(myPara->X,myPara->Y,HourStart,NumofHours); 
		FVMunit::StartYear = FVMunit::Year = PMin.m_YearStart;
		FVMunit::StartMonth = FVMunit::Month = PMin.m_MonthStart;
		FVMunit::StartDay = FVMunit::Day = PMin.m_DayStart;
		FVMunit::StartHour = FVMunit::Hour = PMin.m_HourStart;
	}
	else if(RainType.MakeLower()=="timepoint")
	{
		PMinCBG.Initialize(myPara->X,myPara->Y,HourStart,NumofHours);
		FVMunit::StartYear = FVMunit::Year = PMinCBG.m_YearStart;
		FVMunit::StartMonth = FVMunit::Month = PMinCBG.m_MonthStart;
		FVMunit::StartDay = FVMunit::Day = PMinCBG.m_DayStart;
		FVMunit::StartHour = FVMunit::Hour = PMinCBG.m_HourStart;
	}
	if(debug)
	{
		FVMunit::myFile=&myFile;
		//myFile<<"StartYear,StartMonth,StartDay:"<<FVMunit::Year<<","<<FVMunit::Month<<","<<FVMunit::Day<<endl;
		myFile<<"**********************************************************************************************************************"<<endl;
	}

	/*****************************************来自HSPPARAMETER表的参数****************************************/
	CString cSQL;
	ADODB::_RecordsetPtr pRst;
	pRst.CreateInstance(__uuidof(ADODB::Recordset));
	pRst->CursorLocation = ADODB::adUseClient;
	cSQL.Format("select * from hspparameter where landuse=%d and soiltype=%d",myPara->LandUse,myPara->SoilType);
	try
	{
		pRst->Open(cSQL.GetString(),(ADODB::_Connection*)pCnn,ADODB::adOpenStatic,ADODB::adLockOptimistic,ADODB::adCmdText);
		if(!pRst->EndOfFile)
		{
			FVMunit::thetas=pRst->Fields->Item["thetas"]->Value;
			FVMunit::thetaf=pRst->Fields->Item["thetaf"]->Value;
			FVMunit::thetaw=pRst->Fields->Item["thetaw"]->Value;
			FVMunit::thetab=pRst->Fields->Item["thetab"]->Value;
			
			FVMunit::usD=pRst->Fields->Item["usD"]->Value;//表中单位：cm2/min
			FVMunit::usD=FVMunit::usD*1e-4/60;//转化为m2/s

			FVMunit::usL=pRst->Fields->Item["usL"]->Value;
			FVMunit::usf=pRst->Fields->Item["usF"]->Value;
		}
		pRst->Close();
	}
	catch(_com_error e)
	{
		cout<<"Failture Of Opening Table HSPPARAMETER."<<endl;
		cout<<e.Error()<<endl;
		cout<<e.ErrorMessage()<<endl;
	    cout<<(LPCSTR)e.Source()<<endl;        
		cout<<(LPCSTR)e.Description()<<endl;  
		exit(-1);
	}
	/*****************************************来自HSPPARAMETER表的参数****************************************/

	//获得沟道的几何参数
	this->RiverChannel();
	
	//获得左、右、源坡面的网格离散单元
	this->MeshRegion();

}

//每计算完成一条河段就释放一次
void HighSlopeRunoff::FinalizeOneRiver(void)
{
	if(!fvL.empty()) { fvL.clear();} 
	if(!fvR.empty()) { fvR.clear();} 
	if(!fvS.empty()) { fvS.clear();}
	if(NULL!=axL){ delete[] axL;} if(NULL!=axR){ delete[] axR;} if(NULL!=axS){ delete[] axS;}
	if(NULL!=afL){ delete[] afL;} if(NULL!=afR){ delete[] afR;} if(NULL!=afS){ delete[] afS;}
	if(NULL!=aL) { delete[] aL;}  if(NULL!=bL) { delete[] bL;}  if(NULL!=cL) { delete[] cL;}
	if(NULL!=xL) { delete[] xL;}  if(NULL!=fL) { delete[] fL;}  if(NULL!=aR) { delete[] aR;}
	if(NULL!=bR) { delete[] bR;}  if(NULL!=cR) { delete[] cR;}  if(NULL!=xR) { delete[] xR;}
	if(NULL!=fR) { delete[] fR;}  if(NULL!=aS) { delete[] aS;}  if(NULL!=bS) { delete[] bS;} 
	if(NULL!=cS) { delete[] cS;}  if(NULL!=xS) { delete[] xS;}  if(NULL!=fS) { delete[] fS;}
	if(NULL!=dL) { delete[] dL;}  if(NULL!=dR) { delete[] dR;}  if(NULL!=dS) { delete[] dS;}
}



//整个元流域的产流计算
void HighSlopeRunoff::Calc(BSCode mBSCode0,Para* myPara0,float* Qu10,float* Qu20,float* Qd10,float* Qd20,float* Quout0,float* Qdout0,float* FlowB0,float* FlowH0,float* FlowV0)
{
	//虚节点不用产流计算，直接将入流过程相加返回
	if(myPara0->AreaL+myPara0->AreaR+myPara0->AreaS<1e-2)
	{
		for(int i=0;i<Steps+1;i++)
		{
			Quout0[i]=Qu10[i]+Qu20[i];
			Qdout0[i]=Qd10[i]+Qd20[i];
		}
		return;
	}

	this->InitializeOneRiver(mBSCode0,myPara0,Qu10,Qu20,Qd10,Qd20,Quout0,Qdout0,FlowB0,FlowH0,FlowV0);
	
	//为了书写方面用引用类型，使用该类型与使用原变量名效果一样
	float &TimeStart=FVMunit::TimeStart;//s
	TimeStart=0;

	//记录前一时刻的TimeStart
	float TimeStartBefore=0;
	
	//降雨期的常规时间步长
	float &dt1=FVMunit::dt1;//s
	float &dt2=FVMunit::dt2;//s
	
	//土壤水在非降雨期运动的变时间步长s
	float vdt=0;

	//一个时间步长内的降雨量m
	float StepRain=0.0;

	//两次相邻降雨之间的时间s
	double TimeNoRain=0.0;
	
	//无雨期土壤水计算的时间步长s,这里默认1天
	const float NoRainStep=1*24*3600;

	float YearS=FVMunit::StartYear,MonthS=FVMunit::StartMonth,DayS=FVMunit::StartDay,HourS=FVMunit::StartHour;
	float YearE,MonthE,DayE,HourE;
	const float RegionS=max(1e-2,myPara->AreaL)+max(1e-2,myPara->AreaR)+max(1e-2,myPara->AreaS);//元流域投影面积

	/*if(mBSCode.RegionIndex==301 && mBSCode.Length==84 && mBSCode.Value==2051)
	{
		int xx=0;
	}*/
	
	while(TimeStart<=NumofHours*3600)
	{
		if(RainType.MakeLower()=="dayaverage")
		{
			StepRain=PMin.P(double(HourStart+TimeStart/3600),FVMunit::Hour,&TimeNoRain,dt2);
		}
		else if(RainType.MakeLower()=="timepoint")
		{
			StepRain=PMinCBG.P(double(HourStart+TimeStart/3600),&TimeNoRain,dt2);
		}

		//无雨期的水分运动
		if(StepRain<=1e-5 && TimeNoRain>1e-4)
		{
			while(TimeNoRain>0)
			{
				float iTime=TimeStart;
				float Tmp=0;

				//下面的1表示时间步长为1天，根据需要可以改变
				if(TimeNoRain<=NoRainStep) 
				{ 
					vdt=TimeNoRain;
					TimeNoRain=-1;//浮点数不做0与0的比较，否则出很大问题，陷入死循环，因此这里赋值-1
				}
				else
				{
					vdt=NoRainStep;
					TimeNoRain-=vdt;

					//看看剩下的还够不够0.5天,如果不够直接加到上个时间步长里
					if(TimeNoRain<0.5*vdt) 
					{ 
						vdt+=TimeNoRain;
						TimeNoRain=-1;
					}
				}

				//我猜想由于浮点数计算问题，无雨期可能会得到很小，因此这里加一个判断，只有大于10分钟才算
				if(vdt>600)
				{
					//饱和土壤水和岩石裂隙水运动
					if(myPara->AreaL>0.01 && fvL.size()>0){ this->SaturatedSoil("L",vdt);}
					if(myPara->AreaR>0.01 && fvR.size()>0){ this->SaturatedSoil("R",vdt);}
					if(myPara->AreaS>0.01 && fvS.size()>0){ this->SaturatedSoil("S",vdt);}

					//沟道地下水运动
					this->BellowChannel(vdt);

					//坡面和明渠水以dt1短时间步长运动
					while(Tmp<vdt)
					{
						//坡面流及植被水运动
						if(myPara->AreaL>0.01 && fvL.size()>0){ this->SlopeRunoff("L",0,dt1);}
						if(myPara->AreaR>0.01 && fvR.size()>0){ this->SlopeRunoff("R",0,dt1);}
						if(myPara->AreaS>0.01 && fvS.size()>0){ this->SlopeRunoff("S",0,dt1);}

						//明渠洪水演进
						Ichannel.InChannel();

						TimeStart+=dt1;
						Tmp+=dt1;
					}
					TimeStart=iTime;
					Tmp=0;

					//非饱和土壤水运动
					if(myPara->AreaL>0.01 && fvL.size()>0)
					{
						for(int i=0;i<fvL.size();i++)
						{
							fvL[i].UnsaturatedSoil(vdt);
						}	
					}

					if(myPara->AreaR>0.01 && fvR.size()>0)
					{
						for(int i=0;i<fvR.size();i++)
						{ 
							fvR[i].UnsaturatedSoil(vdt);
						}	
					}

					if(myPara->AreaS>0.01 && fvS.size()>0)
					{
						for(int i=0;i<fvS.size();i++)
						{ 
							fvS[i].UnsaturatedSoil(vdt);
						}
					}
				}

				TimeStart+=vdt;

			}//end while
		}//end if

		else if(StepRain>1e-5)
		{
			float iTime=TimeStart;
			float Tmp=0;

			//饱和土壤水和岩石裂隙水运动
			if(myPara->AreaL>0.01 && fvL.size()>0){ this->SaturatedSoil("L",dt2);}
			if(myPara->AreaR>0.01 && fvR.size()>0){ this->SaturatedSoil("R",dt2);}
			if(myPara->AreaS>0.01 && fvS.size()>0){ this->SaturatedSoil("S",dt2);}

			//沟道地下水运动
			this->BellowChannel(dt2);

			//坡面和明渠水以dt1短时间步长运动
			while(Tmp<dt2)
			{
				//坡面流及植被水运动
				if(myPara->AreaL>0.01 && fvL.size()>0){ this->SlopeRunoff("L",StepRain/dt2*dt1,dt1);}
				if(myPara->AreaR>0.01 && fvR.size()>0){ this->SlopeRunoff("R",StepRain/dt2*dt1,dt1);}
				if(myPara->AreaS>0.01 && fvS.size()>0){ this->SlopeRunoff("S",StepRain/dt2*dt1,dt1);}

				//明渠洪水演进
				Ichannel.InChannel();

				TimeStart+=dt1;
				Tmp+=dt1;
			}
			TimeStart=iTime;
			Tmp=0;

			//非饱和土壤水运动
			if(myPara->AreaL>0.01 && fvL.size()>0)
			{
				for(int i=0;i<fvL.size();i++)
				{ 
					fvL[i].UnsaturatedSoil(dt2);
				}	
			}

			if(myPara->AreaR>0.01 && fvR.size()>0)
			{
				for(int i=0;i<fvR.size();i++)
				{ 
					fvR[i].UnsaturatedSoil(dt2);
				}	
			}

			if(myPara->AreaS>0.01 && fvS.size()>0)
			{
				for(int i=0;i<fvS.size();i++)
				{ 
					fvS[i].UnsaturatedSoil(dt2);
				}
			}

			TimeStart+=dt2;

		}//end else(有雨时)

		//因为时间动步长，所以只要相邻两步长之间的时间差大于一个StatusTime周期，即记录E和P
		if(debug && TimeStart>TimeStartBefore+StatusTime*3600)
		{
			TimeStartBefore=TimeStart;

			YearE=FVMunit::Year;
			MonthE=FVMunit::Month;
			DayE=FVMunit::Day;  
			HourE=int(FVMunit::Hour+0.5);

			myFile<<YearS<<"."<<MonthS<<"."<<DayS<<" "<<HourS<<"h-"<<YearE<<"."<<MonthE<<"."<<DayE<<" "<<HourE<<"h,元流域平均蒸发:"<<E/RegionS*1000<<"(mm),降雨"<<P/RegionS*1000<<"(mm)"<<endl;

			YearS=YearE;
			MonthS=MonthE;
			DayS=DayE;
			HourS=HourE;

			E=P=0;
		}
	}//end while(总计算时间循环)

	this->FinalizeOneRiver();	
}


//根据全流域出口平滩流量推算各元流域的沟道几何参数
void HighSlopeRunoff::RiverChannel(void)
{
	//初始值
	gm=2.5; 
	gh=5; 
	ef=0; 

	theta1 = this->Min.theta1;
    theta2 = this->Min.theta2;

	float S = myPara->DrainingArea;

	if(theta1<0 || theta1>PI/2 || theta1==NULL)
	{
		theta1 =PI/4;

		if(debug)
		{
			myFile<<"河段梯形断面边坡角theta1要在0°到90°之间。"<<endl;
			myFile<<"theta1被初始化为"<<45<<"°"<<endl;
		}
	}
	if(theta2<0 || theta2>PI/2 || theta2==NULL)
	{
		theta2 = PI/4;

		if(debug)
		{
			myFile<<"河段梯形断面边坡角theta2要在0°到90°之间。"<<endl;
			myFile<<"theta2被初始化为"<<45<<"°"<<endl;
		}
	}
	if(S<=0){ return;}
	
	//认为河宽开方之比=平滩流量之比=集水面积之比
	if(Min.B0<0 ||Min.B0==NULL || Min.B0>5000)
	{
		Min.B0 = 100;//m
	}
	this->gh = Min.B0*sqrt(S/this->BasinArea);
	if(gh<0.2) { gh=0.2;}//最小河宽给20cm

	if(Min.Q0<0 ||Min.Q0==NULL || Min.Q0>200000)
	{
		Min.Q0 = 5000;//m3/s
	}
	//sqrt(0)会得到未知的结果，实践中已得到检验，所以最小取5%oo
	if(myPara->StreamSlope<1e-6) { myPara->StreamSlope=5e-4;}
	const float Q1 = Min.Q0*S/this->BasinArea*myPara->RiverManning/sqrt(myPara->StreamSlope);
	const float c1=1/tan(theta1)+1/tan(theta2);
	const float c2 = 1/sin(theta1)+1/sin(theta2);
	float fx=1,dfx;
	int t=0;//迭代次数
	while(abs(fx)>1e-2)
	{
		t++;
		if(t>100) { break;}
		fx=pow(0.5*gm*(2*gh-gm*c1),5/3)*pow(gm*(c2-c1)+gh,-2/3)-Q1;
		dfx=5/3*(gh-c1*gm)*pow(gh*gm-0.5*c1*gm*gm,2/3)*pow((c2-c1)*gm+gh,-2/3)-2/3*(c2-c1)*pow(gh*gm-0.5*c1*gm*gm,5/3)*pow((c2-c1)*gm+gh,-5/3);
		gm=gm-fx/dfx;
	}
	//三角形断面,gh最大值
	if(gm>gh/c1)
	{
		gm=gh/c1;
	}
	//假设最小0.1米水深
	if(gm<0.1)
	{
		gm=max(gh/c1,0.1);//最小0.1米深
	}
	ef = gh-gm*c1;

	if(debug)
	{
		myFile<<"RegionIndex:"<<mBSCode.RegionIndex<<",BsValue:"<<mBSCode.Value<<",BsLength:"<<mBSCode.Length<<endl;
		myFile<<"沟道尺寸:"<<"河面宽:"<<gh<<"m,"<<"沟道深:"<<gm<<"m,"<<"河底宽:"<<ef<<"m"<<endl;
	}
}


//生成元流域计算网格，之所以没有加一个判断坡面类型(L,R,S)的入口参数，因为模型通透性(最大侵蚀半径等)
//需要在统一坐标系下整体计算
void HighSlopeRunoff::MeshRegion(void)
{
	////////////////////////////////////////////////////////////////////////////////////////
	///////////////////////////////根据坡面形状更新破面投影长///////////////////////////////
	////////////////////////////////////////////////////////////////////////////////////////
	float Bst;//源坡面的三角形底边长
	::transform(Min.sShape.begin(),Min.sShape.end(),Min.sShape.begin(),::tolower);
	::transform(Min.hType.begin(),Min.hType.end(),Min.hType.begin(),::tolower);

	if(myPara->StreamLength>0)
	{
		//建议推荐抛物线形状，与实际比较相符，三角形面积比实际普遍偏小，矩形普遍偏大
		//lengthL是坡面形心到河道的水平距离，在保证坡面投影面积不变的情况下，lengthL只能被调整
		//注意：以下经过调整后的LengthL、LengthR、LengthS，已经成为全破面长水平向投影，而不是质心处的了。
		
		//抛物线假设
		if(Min.sShape=="par")
		{
			myPara->LengthL=3*myPara->AreaL/2/myPara->StreamLength;
			myPara->LengthR=3*myPara->AreaR/2/myPara->StreamLength;
		}

		//三角形假设
		else if(Min.sShape=="tri")
		{
			myPara->LengthL=2*myPara->AreaL/myPara->StreamLength;
			myPara->LengthR=2*myPara->AreaR/myPara->StreamLength;
		}

		//矩形假设
		else if(Min.sShape=="rec")
		{
			myPara->LengthL=myPara->AreaL/myPara->StreamLength;
			myPara->LengthR=myPara->AreaR/myPara->StreamLength;
		}
		else//按照抛物线处理
		{
			myPara->LengthL=3*myPara->AreaL/2/myPara->StreamLength;
			myPara->LengthR=3*myPara->AreaR/2/myPara->StreamLength;
		}
	}
	//源坡面按照倒立三角形来处理，左右坡面如果作为三角形处理的话，当然是正立三角形
	if(myPara->LengthS>0)
	{
		myPara->LengthS=myPara->LengthS*3/2;
		Bst=2*myPara->AreaS/myPara->LengthS;
	}
	//---------------------------------------END------------------------------------------//

	////////////////////////////////////////////////////////////////////////////////////////
	///////////////////////////////下面求解跟临界高程有关部分///////////////////////////////
	////////////////////////////////////////////////////////////////////////////////////////
	//初始化为坡脚处的“相对”高程
	int hSlopeDownLR=0.0;
	int hSlopeDownS=0.0;

	//初始化为坡顶处的“相对”高程，之后保持不变；以下提到的“相对”均是以GH线为X轴。
	//以下三个if是保证坡的最高相对高程不是0（因为int型）的，如果是0坡面就变成一条线了
	//给了一个最小1.5米高的坡高
	if(myPara->LengthL>0 && int(myPara->LengthL*myPara->SlopeL)<1)
	{
		myPara->SlopeL=1.5/myPara->LengthL;
	}

	if(myPara->LengthR>0 && int(myPara->LengthR*myPara->SlopeR)<1)
	{
		myPara->SlopeR=1.5/myPara->LengthR;
	}

	if(myPara->LengthS>0 && int(myPara->LengthS*myPara->SlopeS)<1)
	{
		myPara->SlopeS=1.5/myPara->LengthS;
	}

	const int hSlopeUpL=myPara->LengthL*myPara->SlopeL;
    const int hSlopeUpR=myPara->LengthR*myPara->SlopeR;
	const int hSlopeUpS=myPara->LengthS*myPara->SlopeS;

	if(Min.hType!="relative" && Min.hType!="absolute")
	{
		Min.hType="absolute";
	}
	if(Min.hType=="absolute")
	{
		//转化为“绝对”高程
		hSlopeDownLR=0.5*(myPara->UElevation+myPara->DElevation);
		hSlopeDownS=myPara->UElevation;
	}

	//临界高程从高到低排序
	//::stable_sort(lvL.begin(),lvL.end(),compare_1);

	//临界高程复制到左坡面、右坡面和源坡面，并定义向量迭代器
	std::vector<pair<int,int>> lvL(lv.begin(),lv.end());
	std::vector<pair<int,int>> lvR(lv.begin(),lv.end());
	std::vector<pair<int,int>> lvS(lv.begin(),lv.end());
	std::vector<pair<int,int>>::iterator lpL=lvL.begin(),lpR=lvR.begin(),lpS=lvS.begin();

	int typeL=4,typeR=4,typeS=4;

	//把临界高程转化成了“相对”高程
	for(lpL;lpL!=lvL.end();lpL++) 
	{ 
		lpL->second-=hSlopeDownLR;

		//将小于0的高程删除，“绝对高程”时可能会出现小于0这种情况
		//因为从大到小排列，所以一旦出现小于0，后面肯定也小于0
		if(lpL->second<0)
		{
			typeL=lpL->first;
			lvL.erase(lpL,lvL.end());
			break;
		}
	}

	for(lpR;lpR!=lvR.end();lpR++) 
	{
		lpR->second-=hSlopeDownLR;
		if(lpR->second<0)
		{
			typeR=lpR->first;
			lvR.erase(lpR,lvR.end());
			break;
		}
	}

	if(myPara->LengthS>0)
	{	
		for(lpS;lpS!=lvS.end();lpS++)
		{ 
			lpS->second-=hSlopeDownS;
			if(lpS->second<0)
			{
				typeS=lpS->first;
				lvS.erase(lpS,lvS.end());
				break;
			}
		}
	}

	/*******************************增加左坡面起止点相对高程*********************************/
	//因为高程从高到低排列，标识删除到第几项(指前面那些比最高坡面点还高的高程项)
	int eraseFlag=0;
	for(lpL=lvL.begin();lpL!=lvL.end();lpL++)
	{
		if(lpL->second>=hSlopeUpL){ eraseFlag++;}
		else
		{
			lvL.insert(lpL,make_pair(lpL->first,hSlopeUpL));
			break;
		}
	}

	//删除比最高坡面点还要高的临界高程
	if(eraseFlag>0)
	{
		//如果临界高程组都大于坡顶高程，则在最后添加元素，类型typeL
		if(eraseFlag==lvL.size())
		{
			lvL.push_back(make_pair(typeL,hSlopeUpL));
		}
		lvL.erase(lvL.begin(),lvL.begin()+eraseFlag);
	}

	//2009.9.11，如果全坡面都降雪，那么eraseFlag定义之前，lvL就已经空了，因此这里得保证lvL包含坡顶项。
	if(lvL.size()==0)
	{
		lvL.push_back(make_pair(typeL,hSlopeUpL));
	}
	
	//以下处理添加坡脚处高程点
	for(lpL=lvL.begin();lpL!=lvL.end();lpL++)
	{
		//下面的if一定要加
		if(lvL.size()==1)
		{
			lvL.push_back(make_pair(typeL,0));
			break;
		}
		
		//最后元素高程为0情况
		//wh: 记住访问最后一个元素使用end()-1，而不是end(),否则报错，end()指向最后一个元素的下一个元素
		if((lvL.end()-1)->second<=0)
		{
			(lvL.end()-1)->second=0;
			break;
		}
		
		//末元素高程大于0情况
		if((lvL.end()-1)->second>0 && (lvL.end()-1)->first!=typeL)
		{
			lvL.push_back(make_pair(typeL,0));
			break;
		}

		if((lvL.end()-1)->second>0 && (lvL.end()-1)->first==typeL)
		{
			(lvL.end()-1)->second=0;
			break;
		}
	}
	/*******************************增加左坡面起止点高程完毕*********************************/

	/*******************************增加右坡面起止点相对高程*********************************/
	eraseFlag=0;
	for(lpR=lvR.begin();lpR!=lvR.end();lpR++)
	{
		//插入坡面顶点高程
		if(lpR->second>=hSlopeUpR){ eraseFlag++;}
		else
		{
			lvR.insert(lpR,make_pair(lpR->first,hSlopeUpR));
			break;
		}
	}

	//删除比最高坡面点还要高的临界高程
	if(eraseFlag>0)
	{
		if(eraseFlag==lvR.size())
		{
			lvR.push_back(make_pair(typeR,hSlopeUpR));
		}
		lvR.erase(lvR.begin(),lvR.begin()+eraseFlag);
	}

	//见前面左坡面的说明
	if(lvR.size()==0)
	{
		lvR.push_back(make_pair(typeR,hSlopeUpR));
	}

	for(lpR=lvR.begin();lpR!=lvR.end();lpR++)
	{
		//下面的if一定要加
		if(lvR.size()==1)
		{
			lvR.push_back(make_pair(typeR,0));
			break;
		}

		//最后元素高程为0情况
		if((lvR.end()-1)->second<=0)
		{
			(lvR.end()-1)->second=0;
			break;
		}

		//末元素高程大于0情况
		if((lvR.end()-1)->second>0 && (lvR.end()-1)->first!=typeR)
		{
			lvR.push_back(make_pair(typeR,0));
			break;
		}

		if((lvR.end()-1)->second>0 && (lvR.end()-1)->first==typeR)
		{
			(lvR.end()-1)->second=0;
			break;
		}
	}
	/*******************************增加右坡面起止点高程完毕*********************************/

	/*******************************增加源坡面起止点相对高程*********************************/
	if(myPara->LengthS>0)
	{
		eraseFlag=0;
		for(lpS=lvS.begin();lpS!=lvS.end();lpS++)
		{
			//插入坡面顶点高程
			if(lpS->second>=hSlopeUpS) { eraseFlag++;}
			else
			{
				lvS.insert(lpS,make_pair(lpS->first,hSlopeUpS));
				break;	
			}
		}

		if(eraseFlag>0)
		{
			if(eraseFlag==lvS.size())
			{
				lvS.push_back(make_pair(typeS,hSlopeUpS));
			}
			lvS.erase(lvS.begin(),lvS.begin()+eraseFlag);
		}

		//见前面左坡面的说明
		if(lvS.size()==0)
		{
			lvS.push_back(make_pair(typeS,hSlopeUpS));
		}

		for(lpS=lvS.begin();lpS!=lvS.end();lpS++)
		{
			//下面的if一定要加
			if(lvS.size()==1)
			{
				lvS.push_back(make_pair(typeS,0));
				break;
			}

			//最后元素高程为0情况
			if((lvS.end()-1)->second<=0)
			{
				(lvS.end()-1)->second=0;
				break;
			}

			//末元素高程大于0情况
			if((lvS.end()-1)->second>0 && (lvS.end()-1)->first!=typeS)
			{
				lvS.push_back(make_pair(typeS,0));
				break;
			}

			if((lvS.end()-1)->second>0 && (lvS.end()-1)->first==typeS)
			{
				(lvS.end()-1)->second=0;
				break;
			}
		}
	}
	/*******************************增加源坡面起止点高程完毕*********************************/

	if(debug)
	{
		myFile<<"HSPELEVATION表临界高程数目:"<<lv.size()<<endl;
		for(lpL=lvL.begin();lpL!=lvL.end();lpL++)
		{
			if(abs(lvL.begin()->second-hSlopeUpL)>0.1)
			{
				cout<<"warning:L第一个临界高程不是坡面顶点。"<<endl;
				return;
			}
			if((lvL.end()-1)->second!=0)
			{
				cout<<"warning:L最后一个临界高程不是坡脚0高程。";
				return;
			}
		}

		for(lpR=lvR.begin();lpR!=lvR.end();lpR++)
		{
			if(abs(lvR.begin()->second-hSlopeUpR)>0.1)
			{
				cout<<"warning:R第一个临界高程不是坡面顶点。"<<endl;
				return;
			}
			if((lvR.end()-1)->second!=0)
			{
				cout<<"warning:R最后一个临界高程不是坡脚0高程。";
				return;
			}
		}

		if(myPara->LengthS>0)
		{
			for(lpS=lvS.begin();lpS!=lvS.end();lpS++)
			{
				if(abs(lvS.begin()->second-hSlopeUpS)>0.1)
				{
					cout<<"warning:S第一个临界高程不是坡面顶点。"<<endl;
					return;
				}
				if((lvS.end()-1)->second!=0)
				{
					cout<<"warning:S最后一个临界高程不是坡脚0高程。";
					return;
				}

			}//end for

		}//end if

	}//end if(debug)

	//---------------------------------------END------------------------------------------//


	////////////////////////////////////////////////////////////////////////////////////////
	///////////////////////////////下面求解跟不透水层有关部分///////////////////////////////
	////////////////////////////////////////////////////////////////////////////////////////
	
	//sitaAT是直线实际的倾斜角[0,PI]，以下的变量命名和"picture"中的标识一致
	
	//左坡面
	float A[2],T[2],O[2],sitaAT;

	//右坡面
	float D[2],T_1[2],Q[2],sitaDT_1;

	//源坡面
	float AS[2],TS[2],OS[2],sitaATS;
	
	//沟道
	float G[2],H[2];
	float E[2],F[2];
	
	O[0]=0;
	O[1]=hSlopeUpL;
	Q[0]=myPara->LengthL+gh+myPara->LengthR;
	Q[1]=hSlopeUpR;
	if(myPara->LengthS>0)
	{
		OS[0]=0;
		OS[1]=hSlopeUpS;
	}

	G[0]=myPara->LengthL;
	G[1]=0;
	H[0]=myPara->LengthL+gh;
	H[1]=0;
	
	E[0]=G[0]+gm/tan(theta1);
	E[1]=-gm;
	F[0]=E[0]+ef;
	F[1]=-gm;
	
	//A点和D点是“终年”积雪区的下沿（临界高程必须最高），不是季节性融雪边界。
	A[0]=max((O[1]-(lv[0].second-hSlopeDownLR))/myPara->SlopeL,0);
	A[1]=min(lv[0].second-hSlopeDownLR,O[1]);

	D[0]=Q[0]-max((Q[1]-(lv[0].second-hSlopeDownLR))/myPara->SlopeR,0);
	D[1]=min(lv[0].second-hSlopeDownLR,Q[1]);

	if(Min.sRatio<=0 || Min.sRatio>1 || Min.sRatio==NULL)
	{
		//Min.sRatio的上限
		Min.sRatio=min((G[0]-A[0])/G[0],(D[0]-H[0])/(Q[0]-H[0]));
	}
	else
	{
		Min.sRatio=min(Min.sRatio,min((G[0]-A[0])/G[0],(D[0]-H[0])/(Q[0]-H[0])));
	}

	T[0]=myPara->LengthL*(1-Min.sRatio);
	T[1]=0;

	//得到sitaAT的下限(最小值)
	if(T[0]<=A[0]+1e-4)
	{ 
		sitaAT=PI/2; 
		T[0]=A[0];
	}

	//得到sitaAT的正常值
	else
	{
		sitaAT=PI+atan2(T[1]-A[1],T[0]-A[0]); 
	}
	
	//得到sitaAT的上限
	if(tan(theta1)>myPara->SlopeL)
	{
		float sitaATU=PI+atan2(E[1]-A[1],E[0]-A[0]);
		sitaAT=min(sitaAT,sitaATU);

		if(abs(sitaAT-sitaATU)<1e-4)
		{
			T[0]=-E[1]*(E[0]-A[0])/(E[1]-A[1])+E[0];
		}
	}
	
	//确定右坡面不透水层倾斜角度
	T_1[0]=H[0]+myPara->LengthR*Min.sRatio;
	T_1[1]=0;
	if(T_1[0]>=D[0]-1e-4) 
	{ 
		sitaDT_1=PI/2;
		T_1[0]=D[0];
	}
	else 
	{ 
		sitaDT_1=atan2(D[1]-T_1[1],D[0]-T_1[0]);
	}
    //得到sitaDT_1的下限
	if(tan(theta2)>myPara->SlopeR)
	{
		float sitaDT_1U=atan2(D[1]-F[1],D[0]-F[0]);
		sitaDT_1=max(sitaDT_1,sitaDT_1U);

		if(abs(sitaDT_1-sitaDT_1U)<1e-4)
		{
			T_1[0]=-F[1]*(F[0]-D[0])/(F[1]-D[1])+F[0];
		}
	}

	//EF下至少有10cm土壤厚
	float sDeep=max(gm+0.1,Min.sDeep);
	
	//Min->sDeep：是全流域最大侵蚀深度,同样根据流量关系在全流域分配,认为支流侵蚀深度小，越到平原区越厚
	//0.35是沿程河相关系槽深的关系(0.3,0.4)之间，这里不一定合适，但先这么取
	//sDeep*=pow(float(myPara->DrainingArea/this->BasinArea),0.35f);

	//确定源坡面不透水层倾斜角度
	//对于源坡面沟底KL是平的,认为没有圆形,若AT与GK相交,则按照初始的sRatio,否则按AK确定T
	if(myPara->LengthS>0)
	{
		KS[0]=myPara->LengthS;
		KS[1]=-(gm+sDeep);

		AS[0]=max((OS[1]-(lv[0].second-hSlopeDownS))/myPara->SlopeS,0);
		AS[1]=min(lv[0].second-hSlopeDownS,OS[1]);

		TS[0]=myPara->LengthS*(1-Min.sRatio);
		TS[1]=0;

		//得到直线AK与X轴交点横坐标
		//下面的if是全破面降雪的情况
		if(KS[0]<AS[0]+1e-4)
		{
			TS[0]=KS[0];
			sitaATS=PI-atan(myPara->SlopeS);
		}
		else
		{
			//AK与x轴交点的横坐标，此时先满足KS[1]=-(gm+sDeep)的限制，再确定sRatio
			float XS;
			XS=KS[0]-KS[1]*(KS[0]-AS[0])/(KS[1]-AS[1]);

			//确定源坡面不透水层的倾斜角
			//这里加上了TS[0]>A[0]，因为A点是可以很靠近坡脚的，几乎全坡面积雪，要小心，此时T在A的左侧
			//希望达到一种效果，只要坡面上不全是雪，就给他若干个计算单元，因此得保证sitaATS为钝角
			if(XS>TS[0] && TS[0]>A[0])
			{
				sitaATS=PI+atan2(TS[1]-AS[1],TS[0]-AS[0]); 
			}
			else
			{
				TS[0]=XS;
				sitaATS=PI+atan2(KS[1]-AS[1],KS[0]-AS[0]);
			}
		}
	}
	//---------------------------------------END------------------------------------------//

	////////////////////////////////////////////////////////////////////////////////////////
	///////////////////////////////下面求解跟圆形区域有关部分///////////////////////////////
	////////////////////////////////////////////////////////////////////////////////////////
	//侵蚀区圆心坐标
	float Center[2];
	
	//切点坐标
	float B[2],C[2]={0,0};
	
	//左右坡面相对不透水面交点坐标
	float DD[2];
	
	//a:左坡面不透水层与x轴负向夹角(肯定为锐角);
	//b:右坡面不透水层与x轴正向夹角(肯定为锐角);
	//K:最大侵蚀深度(从X轴往下算起，肯定为正);
	float a,b,K,R=0;

	a=PI-sitaAT;
	b=sitaDT_1;
	K=gm+sDeep;

	/********************************为了得到K的上限********************************/
	//以下是两条切线交于一点的情况
	if(abs(a-PI/2)<1e-4 && abs(b-PI/2)<1e-4)
	{
		//表示无穷远
		DD[0]=(T[0]+T_1[0])/2;
		DD[1]=-1e6;
	}
	else if(abs(a-PI/2)<1e-4 && abs(b-PI/2)>=1e-4)
	{
		DD[0]=T[0];
		DD[1]=-(T_1[0]-T[0])*tan(b);
	}
	else if(abs(b-PI/2)<1e-4 && abs(a-PI/2)>=1e-4)
	{
		DD[0]=T_1[0];
		DD[1]=-(T_1[0]-T[0])*tan(a);
	}
	else
	{
		DD[1]=-(T_1[0]-T[0])*tan(a)*tan(b)/(tan(a)+tan(b));
		DD[0]=T[0]+(-DD[1]/tan(a));
	}
	//-DD[1]是K的最大值
	if(K>=-DD[1])
	{
		K=gm+(-DD[1]-gm)*(1-1e-6);
	}
	/********************************K的上限求解完毕********************************/


	/*********************求解圆心和切点坐标，并增加半径下限判断********************/
	//E,F到圆心C的距离平方
	//float EC=1,FC=1;

	//中断调试用
	/*if(mBSCode.Length==61 && mBSCode.RegionIndex==301 && mBSCode.Value==33)
	{
		int mm=0;
	}*/

	//下面求解圆心坐标、切点坐标等，并增加了圆弧半径的下限判断，如果过小，圆弧会与GEFH相交
	K-=0.05;
	while(1)
	{
		if(K>-DD[1]-1e-3) 
		{
			//if(debug){ cout<<"EC2:"<<EC2<<",FC2:"<<FC2<<",GC2:"<<GC2<<",HC2:"<<HC2<<",最大侵蚀深度"<<-DD[1]<<"米，不收敛"<<endl;}
			K=gm+(-DD[1]-gm)*(1-1e-6);
			
			//圆形和不透水层交点重合
			Center[0]=B[0]=C[0]=DD[0];
			Center[1]=B[1]=C[1]=DD[1];

			break; 
		}

		K+=0.05;//每次增加5cm
		
		//两个不透水角都为90°
		if(abs(a-PI/2)<1e-4 && abs(b-PI/2)<1e-4)
		{
			R=(T_1[0]-T[0])/2;
			Center[0]=T[0]+R;
			Center[1]=R-K;
			B[0]=T[0];
			C[0]=T_1[0];
			B[1]=Center[1];
			C[1]=Center[1];
		}

		//左坡面不透水角为90°,右坡面非90°
		else if(abs(a-PI/2)<1e-4 && abs(b-PI/2)>=1e-4)
		{
			R=(K-tan(b)*(T_1[0]-T[0]))/(1-cos(b)-tan(b)*(1+sin(b)));
			Center[0]=T[0]+R;
			Center[1]=R-K;
			B[0]=T[0];
			B[1]=Center[1];
			C[0]=Center[0]+R*sin(b);
			C[1]=Center[1]-R*cos(b);
		}

		//右坡面不透水角为90°,左坡面非90°
		else if(abs(b-PI/2)<1e-4 && abs(a-PI/2)>=1e-4)
		{
			R=(K+tan(a)*(T[0]-T_1[0]))/(1-cos(a)-tan(a)*(1+sin(a)));
			Center[0]=T_1[0]-R;
			Center[1]=R-K;
			B[0]=Center[0]-R*sin(a);
			B[1]=Center[1]-R*cos(a);
			C[0]=T_1[0];
			C[1]=Center[1];
		}

		//一般情况下
		else
		{
			//解析几何的东西，令圆心O(x,R-K),T(Tx,Ty),T_1(T_1x,T_1y),则切点B(x-R*sin(a),R-K-Rcos(a)),切点C(x+Rsin(b),R-K-Rcos(b))
			//再将B和C坐标代入两条直线方程联立求解，即可得到半径R和圆心坐标等，注意这里的a和b指的的是锐角。
			R=( K*(tan(b)+tan(a)) + tan(a)*tan(b)*(T[0]-T_1[0]) )/( tan(b)*(1-cos(a)-sin(a)*tan(a))+tan(a)*(1-cos(b)-tan(b)*sin(b)) );
			Center[0]=(K+tan(a)*T[0]-R*(1-cos(a)-sin(a)*tan(a)))/tan(a);
			Center[1]=R-K;
			B[0]=Center[0]-R*sin(a);
			B[1]=R-K-R*cos(a);
			C[0]=Center[0]+R*sin(b);
			C[1]=R-K-R*cos(b);
		}

		//EC=sqrt( (E[0]-Center[0])*(E[0]-Center[0])+(E[1]-Center[1])*(E[1]-Center[1]) );
		//FC=sqrt( (F[0]-Center[0])*(F[0]-Center[0])+(F[1]-Center[1])*(F[1]-Center[1]) );
		//GC2=(G[0]-Center[0])*(G[0]-Center[0])+(G[1]-Center[1])*(G[1]-Center[1]);
		//HC2=(H[0]-Center[0])*(H[0]-Center[0])+(H[1]-Center[1])*(H[1]-Center[1]);

		//此时EF下方是直线，BC间的圆弧段肯定不会与EF相交，因为一旦相交EF下方就不可能都是直线
		if(E[0]>=C[0] || F[0]<=B[0])
		{
			break;
		}
		
		//此时E点下方是圆弧,F点下是直线，保证E点的纵坐标大于圆弧上相应的纵坐标
		if(E[0]>=B[0] && E[0]<=C[0] && F[0]>=C[0])
		{
			if(E[1]>=Center[1]-sqrt(R*R-(E[0]-Center[0])*(E[0]-Center[0])))
			{
				break;
			}
		}

		//此时F点下方是圆弧,E点下是直线，保证F点的纵坐标大于圆弧上相应的纵坐标
		if(F[0]>=B[0] && F[0]<=C[0] && E[0]<=B[0])
		{
			if(F[1]>=Center[1]-sqrt(R*R-(F[0]-Center[0])*(F[0]-Center[0])))
			{
				break;
			}
		}

		//E,F下方都是圆弧
		if(F[0]>=B[0] && F[0]<=C[0] && E[0]>=B[0] && E[0]<=C[0])
		{
			if(F[1]>=Center[1]-sqrt(R*R-(F[0]-Center[0])*(F[0]-Center[0])) && E[1]>=Center[1]-sqrt(R*R-(E[0]-Center[0])*(E[0]-Center[0])))
			{
				break;
			}
		}
	}

	//下面确定图中K点和L点的坐标
	KK[0]=G[0];
	LL[0]=H[0];

	//KK点纵坐标
	//在右坡面不透水层直线上
	if(KK[0]>=C[0])
	{
		KK[1]=C[1]+(D[1]-C[1])/(D[0]-C[0])*(KK[0]-C[0]);
	}
	//在左坡面不透水层直线上
	else if(KK[0]<=B[0])
	{
		KK[1]=B[1]+(A[1]-B[1])/(A[0]-B[0])*(KK[0]-B[0]);
	}
	//在圆弧上
	else
	{
		KK[1]=Center[1]-sqrt(R*R-pow((KK[0]-Center[0]),2));
	}

	//LL点纵坐标
	//在右坡面不透水层直线上
	if(LL[0]>=C[0])
	{
		LL[1]=C[1]+(D[1]-C[1])/(D[0]-C[0])*(LL[0]-C[0]);
	}
	//在左坡面不透水层直线上
	else if(LL[0]<=B[0])
	{
		LL[1]=B[1]+(A[1]-B[1])/(A[0]-B[0])*(LL[0]-B[0]);
	}
	//在圆弧上
	else
	{
		LL[1]=Center[1]-sqrt(R*R-pow((LL[0]-Center[0]),2));
	}

	//KK和LL点纵坐标大于0的情况理论上不应该出现
	if(KK[1]>0 && debug) 
	{
		myFile<<"KK点坐标:"<<KK[0]<<","<<KK[1]<<endl;
		KK[1]=0;
	}
	if(LL[1]>0 && debug) 
	{
		myFile<<"LL点坐标:"<<LL[0]<<","<<LL[1]<<endl;
		LL[1]=0;
	}

	//下面计算K点和L点之间的实际距离（直线段和圆弧段）
	//K,L都在左或者右坡面不透水层上
	if(KK[0]>=C[0] || LL[0]<=B[0])
	{
		KL=sqrt(pow(KK[0]-LL[0],2)+pow(KK[1]-LL[1],2));
	}

	//K,L都在圆弧段上
	else if(KK[0]>=B[0] && LL[0]<=C[0])
	{
		KL=sqrt(pow(KK[0]-LL[0],2)+pow(KK[1]-LL[1],2));
		KL=2*asin(KL/2/R)*R;
	}

	//圆弧段都在K，L之间，同时还包括了两侧的直线段
	else if(KK[0]<=B[0] && LL[0]>=C[0])
	{
		KL=sqrt(pow(B[0]-C[0],2)+pow(B[1]-C[1],2));
		KL=2*asin(KL/2/R)*R;
		KL=KL+sqrt(pow(B[0]-KK[0],2)+pow(B[1]-KK[1],2))+sqrt(pow(LL[0]-C[0],2)+pow(LL[1]-C[1],2));
	}

	//K,B之间圆弧，C，L之间直线
	else if(KK[0]>=B[0] && KK[0]<=C[0] && LL[0]>=C[0])
	{
		KL=sqrt(pow(B[0]-KK[0],2)+pow(B[1]-KK[1],2));
		KL=2*asin(KL/2/R)*R;
		KL=KL+sqrt(pow(LL[0]-C[0],2)+pow(LL[1]-C[1],2));
	}

	//C，L之间圆弧，K，B之间直线
	else if(LL[0]>=B[0] && LL[0]<=C[0] && KK[0]<=B[0])
	{
		KL=sqrt(pow(C[0]-LL[0],2)+pow(C[1]-LL[1],2));
		KL=2*asin(KL/2/R)*R;
		KL=KL+sqrt(pow(KK[0]-B[0],2)+pow(KK[1]-B[1],2));
	}

	else
	{
		myFile<<"warning:impossible KL distance."<<endl;
	}

	/********************************圆心和切点坐标计算完毕********************************/

	/**********************************沟道地下水初始条件**********************************/
	//三个点取平均，计算前KS[1]为最低点纵坐标
	KS[1]=(KK[1]+LL[1]+KS[1])/3;

	if(Min.wRatio<=0) { Min.wRatio=1e-5;}
	if(Min.wRatio>=1) { Min.wRatio=0.95;}
	Bchannel.H=max(KK[1],LL[1])*(1-Min.wRatio);
	
	//c++的数组参数似乎不能实现传值，只能传址，所以就如下处理方式了。
	Bchannel.A[0]=A[0];  Bchannel.A[1]=A[1];
	Bchannel.B[0]=B[0];  Bchannel.B[1]=B[1];
	Bchannel.C[0]=C[0];  Bchannel.C[1]=C[1];
	Bchannel.D[0]=D[0];  Bchannel.D[1]=D[1];
	Bchannel.E[0]=E[0];  Bchannel.E[1]=E[1];
	Bchannel.F[0]=F[0];  Bchannel.F[1]=F[1];
	Bchannel.G[0]=G[0];  Bchannel.G[1]=G[1];
	Bchannel.HH[0]=H[0]; Bchannel.HH[1]=H[1];
	Bchannel.K[0]=KK[0]; Bchannel.K[1]=KK[1];
	Bchannel.L[0]=LL[0]; Bchannel.L[1]=LL[1];
	Bchannel.Center[0]=Center[0]; 
	Bchannel.Center[1]=Center[1];
	Bchannel.R=R; 
	Bchannel.Length=myPara->StreamLength;
	
	//得到沟道地下水能存储的最大水量m3
	Bchannel.Wmax=Bchannel.HtoW(0);
	
	if(debug)
	{
		//myFile<<"地下水距离基准面GH下方"<<-Bchannel.H<<"米"<<endl;
		if(Bchannel.H>E[1])
		{
			myFile<<"地下水距离河床EF上方的"<<Bchannel.H-E[1]<<"米"<<endl;
		}
		else
		{
			myFile<<"地下水距离河床EF下方的"<<E[1]-Bchannel.H<<"米"<<endl;
		}
	}
	/**********************************沟道地下水初始条件********************************/

    /**********************************岩石裂隙水初始条件********************************/
	if(Min.rRatio>=1) { Min.rRatio=0.99;}
	if(Min.mu<0 || Min.mu>1) { Min.mu=0.02;}
	if(Min.Kr<0) { Min.Kr=1;}
	
	Rock.H=min(O[1],Q[1])*Min.rRatio;
	Rock.Initiallize(A[1],Min.Kr,Min.mu,myPara->StreamLength*(G[0]+H[0])/2,myPara->StreamLength*(Q[0]-(G[0]+H[0])/2),Bst*myPara->LengthS,&fvL,&fvR,&fvS);

	if(myPara->LengthS>0)
	{
		Rock.H=min(min(O[1],Q[1]),OS[1])*Min.rRatio;
	}
	/**********************************岩石裂隙水初始条件********************************/

	/**********************************沟道明渠水初始条件********************************/
	
	Ichannel.Initiallize(this->myPara,MSTEP,Steps,gh,gm,ef,theta1,theta2,Qu1,Qu2,Quout,&fvL,&fvR,&fvS,FlowB,FlowH,FlowV,Min.SaveFlowPattern,&myFile);
	
	/**********************************沟道明渠水初始条件********************************/

	////////////////////////////////////////////////////////////////////////////////////////
	///////////////////////////////下面生成计算各坡面离散单元///////////////////////////////
	////////////////////////////////////////////////////////////////////////////////////////
	if(Min.dx<0 || Min.dx==NULL)
	{
		Min.dx=1;
	}

	//对dx的最小值进行一下限制
	float tmp1;
	if(myPara->LengthS>0)
	{
		tmp1=min(myPara->LengthS,min(myPara->LengthL,myPara->LengthR));
	}
	else
	{ 
		tmp1=min(myPara->LengthL,myPara->LengthR);
	}

	//保证每个坡面沿着坡面方向都至少有三个网格
	if(tmp1/Min.dx<3)
	{
		Min.dx=tmp1*0.33-1e-4;
	}

	float sinL,sinR,sinS;
	float cosL,cosR,cosS;
	float beta1L,beta1R,beta1S;
	
	//FVM底部倾斜角度
	float beta2L,beta2R,beta2S;

	//相邻临界高程间的坡向距离
	float len;

	//相邻临界高程间的网格数
	int num;

	//相邻临界高程间的步长
	float dx1=Min.dx,dx2;

	//每个FVM中心点的坐标
	float x,y1,y2;

	//每个FVM中心处的坡面长（平行河道方向）
	float Width;

	//每个FVM中心处对应的岩石裂隙单元体积m3
	float VH;

	//饱和土壤水的初始水位
	float hm;

	//W:图中岩石土壤水的交汇点
	float W[2],M[2],N[2],MS[2];

	W[1]=Rock.H;
	M[0]=G[0];
	N[0]=H[0];
	MS[0]=KS[0];
	MS[1]=M[1]=N[1]=Bchannel.H;

	sinL=myPara->SlopeL/sqrt(1+myPara->SlopeL*myPara->SlopeL);
	cosL=sqrt(1-sinL*sinL);
	beta1L=asin(sinL);

	sinR=myPara->SlopeR/sqrt(1+myPara->SlopeR*myPara->SlopeR);
	cosR=sqrt(1-sinR*sinR);
	beta1R=asin(sinR);

	if(myPara->LengthS>0)
	{
		sinS=myPara->SlopeS/sqrt(1+myPara->SlopeS*myPara->SlopeS);
		cosS=sqrt(1-sinS*sinS);
		beta1S=asin(sinS);
	}

	/********************************把B和C加入临界高程组********************************/
	//处理左坡面转折点
	int hB;
	//对于左坡面而言，切点可能在A点之左上方，这样的话B就要成为高程链表的头节点，这是不对的，因此加入了B[0]>A[0]的限制
	if(B[0]<G[0] && B[0]>A[0])
	{
		//B在坡面AG上对应的纵坐标
		hB=-myPara->SlopeL*(B[0]-G[0]);
		for(lpL=lvL.begin();lpL!=lvL.end();lpL++)
		{
			if(lpL->second<hB)
			{
				lvL.insert(lpL,make_pair(lpL->first,hB));
				break;
			}
		}
	}

	//处理右坡面转折点
	int hC;
	if(C[0]>H[0] && C[0]<D[0])
	{
		hC=myPara->SlopeR*(C[0]-H[0]);
		for(lpR=lvR.begin();lpR!=lvR.end();lpR++)
		{
			if(lpR->second<hC)
			{
				lvR.insert(lpR,make_pair(lpR->first,hC));
				break;
			}
		}
	}
	/********************************把B和C加入高程组完毕********************************/

	if(debug)
	{
		float tempS=myPara->AreaL+myPara->AreaR+myPara->AreaS;
		if(tempS>0)
		{
			myFile<<"元流域面积半径(m):"<<sqrt(tempS/PI)<<endl;
		}
		if(myPara->LengthS>0)
		{
			myFile<<"左、右、源坡面高(m):"<<O[1]<<","<<Q[1]<<","<<OS[1]<<endl;
			myFile<<"左、右、源坡度:"<<beta1L/PI*180<<"°,"<<beta1R/PI*180<<"°,"<<beta1S/PI*180<<"°"<<endl;
			myFile<<"左、右、源不透水层坡度:"<<(PI-sitaAT)/PI*180<<"°,"<<(sitaDT_1)/PI*180<<"°,"<<(PI-sitaATS)/PI*180<<"°"<<endl;
		}
		else
		{
			myFile<<"左、右坡面高(m):"<<O[1]<<","<<Q[1]<<endl;
			myFile<<"左、右坡度:"<<beta1L/PI*180<<"°,"<<beta1R/PI*180<<"°"<<endl;
			myFile<<"左、右不透水层坡度:"<<(PI-sitaAT)/PI*180<<"°,"<<(sitaDT_1)/PI*180<<"°"<<endl;
		}

		myFile<<"圆心坐标(X,Y):"<<Center[0]<<","<<Center[1]<<endl;
		myFile<<"切点B和C的坐标(X,Y):"<<"("<<B[0]<<","<<B[1]<<"); ("<<C[0]<<","<<C[1]<<")"<<endl;
		
		if(B[0]<G[0] && B[0]>A[0])
		{
			myFile<<"切点B对应坡面点的相对高程(m):"<<hB<<endl;
		}
		
		if(C[0]>H[0] && C[0]<D[0])
		{
			myFile<<"切点C对应坡面点的相对高程(m):"<<hC<<endl;
		}
	}

	/********************************生成左坡面有限单元组********************************/
	//ahupLR,ahupS中的a是absolute的意思
	const float ahupLR=0.5*(myPara->UElevation+myPara->DElevation);
	const float ahupS=0.5*(myPara->UElevation);
	bool flag_rock=false;
	std::vector<pair<int,int>>::iterator pStart;

	x=A[0];y1=A[1];

	//中断调试用
	/*if(mBSCode.Length==61 && mBSCode.RegionIndex==301 && mBSCode.Value==33)
	{
		int mm=0;
	}*/

	//lvL的第一项肯定是坡顶点，第二项要看是不是雪，因为如果是雪的话，那么第二个点是有限单元的起始点
	//有可能就两项，都是雪，说明全坡面降雪，此时不生成坡面单元
	if((lvL.begin()+1)->first==0)
	{
		pStart=lvL.begin()+1;
	}
	else
	{
		pStart=lvL.begin();
	}

	for(lpL=pStart;lpL!=lvL.end();lpL++)
	{
		//如果全坡面降雪，lvL应该就坡顶和坡脚两项，那么pStart也应该是lvL.end()-1
		if(lpL==lvL.end()-1)
		{
			break;
		}

		len=(lpL->second-(lpL+1)->second)/sinL;

		//因为加入B、C临界高程点后，B和C的位置不确定性可能导致len接近与0，则dx1和dx2也会被计算出为0，这会因此数值错误的。
		if(len<0.5)
		{
			continue;
		}
		
		//至少保证有一个网格
		num=len/Min.dx+1;
		
		//保证坡面网格数大于等于3,如果lvS的size大于等于4,网格数肯定大于等于3,不用特殊处理
		if(lvL.size()==2) { num=max(num,3); }
		if(lvL.size()==3 && lpL==lvL.begin()+1 && fvL.size()+num<=2){ num=2; }

		//新的空间步长
		dx1=len/num;

		x+=0.5*dx1*cosL;
	    y1-=0.5*dx1*sinL;
		
		for(int i=0;i<num;i++)
		{
			if(x<=B[0])
			{
				y2=tan(sitaAT)*(x-A[0])+A[1];
				beta2L=PI-sitaAT;
			}
			else if(x>B[0] && x<C[0])
			{
				y2=Center[1]-sqrt(R*R-(x-Center[0])*(x-Center[0]));
				
				//互相垂直直线斜率为负倒数,beta2L范围(-PI/2,PI/2)
				beta2L=-atan2(x-Center[0],Center[1]-y2);
				
				//这里之所以取绝对值因为饱和土壤水运动方向主要看水头差，是渗透作用，方程中没有重力作用项
				//也就是说土壤朝着各个方向的渗透系数相同，角度体现在hdown，反坡是由于圆弧段引起的
				//if(debug && beta2L<0)
				//{ 
					//myFile<<"左坡面"<<lpL->first<<"类型,第"<<1+fvL.size()<<"单元"<<"为反坡:"<<-beta2L/PI*180<<"°,x,y1,y2:("<<x<<","<<y1<<","<<y2<<")"<<endl; 
				//}
			}
			//土壤单元底部位于右坡面不透水层上
			else
			{
				y2=tan(sitaDT_1)*(x-D[0])+D[1];
				
				//一定是反坡，所以为负值
				beta2L=-sitaDT_1;
			}

			//一般来说y2不会大于y1，但是实际中发现y2大于y1时一般A[1]都为1，我估计是高程用整型表示引起的误差，以至最后几个单元y2稍稍比y1大
			if(y2>y1)
			{
				int temp=y1;
				y1=y2;
				y2=temp;
			}

			dx2=dx1*cosL/cos(beta2L);

			//计算坡面宽m
			if(Min.sShape=="rec")
			{ 
				Width=myPara->StreamLength;
				VH=x*Width*dx2*cos(beta2L);
			}
			else if(Min.sShape=="tri")
			{
				Width=myPara->StreamLength*x/myPara->LengthL;
				VH=0.5*x*Width*dx2*cos(beta2L);
			}
			else//抛物线
			{
				Width=myPara->StreamLength*sqrt(1-2*myPara->StreamLength*(G[0]-x)/3/myPara->AreaL);
				VH=(3*myPara->AreaL/2/myPara->StreamLength-G[0]+x)*Width-myPara->AreaL*pow(Width,3)/4/pow(myPara->StreamLength,3);
				VH=VH*dx2*cos(beta2L);

				if(debug && (Width-myPara->StreamLength>1e-3 || Width<=0)) 
				{
					myFile<<"warning:左坡面"<<lpL->first<<"类型,第"<<1+fvL.size()<<"个单元坡面宽度:"<<Width<<",longer than river "<<myPara->StreamLength<<endl;
					Width=myPara->StreamLength;
				}
			}

			//为计算初始hm
			if(y2>W[1]) 
			{
				//W[0]=x;
				hm=y2+1e-4;
			}
			else
			{
				if(flag_rock==false)
				{
					flag_rock=true;
					W[0]=x;
				}
				hm=M[1]+(M[1]-W[1])/(M[0]-W[0])*(x-M[0]);
			}

			if(!_finite(y2) || !_finite(y1))
			{
				myFile<<"warning:slopeL:y1 or y2 is INF"<<endl;
			}
			
			fvL.push_back(FVMunit(x,y1,y2,hm,y1+ahupLR,dx1,dx2,beta1L,beta2L,Width,(lpL+1)->first,"L",x/cosL,VH));

			x+=dx1*cosL;
			y1-=dx1*sinL;

		}//end for

		//上面的那个循环结束后多走了半个网格，下面两行退回去
		x-=0.5*dx1*cosL;
		y1+=0.5*dx1*sinL;

	}//end for
	/******************************左坡面有限单元组生成完毕******************************/

	/********************************生成右坡面有限单元组********************************/
	x=D[0];y1=D[1];
	flag_rock=false;

	//确定有限单元的起始高程点
	if((lvR.begin()+1)->first==0)
	{
		pStart=lvR.begin()+1;
	}
	else
	{
		pStart=lvR.begin();
	}

	for(lpR=pStart;lpR!=lvR.end();lpR++)
	{
		if(lpR==lvR.end()-1)
		{
			break;
		}

		len=(lpR->second-(lpR+1)->second)/sinR;
		
		//因为加入B、C临界高程点后，B和C的位置不确定性可能导致len接近与0，则dx1和dx2也会被计算出为0，这会因此数值错误的。
		if(len<0.5)
		{
			continue;
		}

		num=len/Min.dx+1;

		//保证坡面网格数大于等于3,如果lvS的size大于等于4,网格数肯定大于等于3,不用特殊处理
		if(lvR.size()==2) { num=max(num,3); }
		if(lvR.size()==3 && lpR==lvR.begin()+1 && fvR.size()+num<=2){ num=2; }

		dx1=len/num;

		x-=0.5*dx1*cosR;
		y1-=0.5*dx1*sinR;
		
		for(int i=0;i<num;i++)
		{
			if(x>=C[0])
			{
				y2=tan(sitaDT_1)*(x-D[0])+D[1];
				beta2R=sitaDT_1;
			}
			else if(x<C[0] && x>B[0])
			{
				y2=Center[1]-sqrt(R*R-(x-Center[0])*(x-Center[0]));
				beta2R=atan2(x-Center[0],Center[1]-y2);

				//if(debug && beta2R<0)
				//{ 
					//myFile<<"右坡面"<<lpR->first<<"类型,第"<<1+fvR.size()<<"单元"<<"为反坡:"<<-beta2R/PI*180<<"°,x,y1,y2:("<<x<<","<<y1<<","<<y2<<")"<<endl; 
				//}
			}
			else
			{
				y2=tan(sitaAT)*(x-A[0])+A[1];

				//一定是反坡，所以-PI
				beta2R=sitaAT-PI;
			}

			//理论上y2肯定要小于y1
			if(y2>y1)
			{
				int temp=y1;
				y1=y2;
				y2=temp;
			}

			dx2=dx1*cosR/cos(beta2R);

			//计算坡面宽m
			if(Min.sShape=="rec")
			{ 
				Width=myPara->StreamLength;
				VH=(Q[0]-x)*Width*dx2*cos(beta2R);
			}
			else if(Min.sShape=="tri")
			{
				Width=myPara->StreamLength*x/myPara->LengthR;
				VH=0.5*(Q[0]-x)*Width*dx2*cos(beta2R);
			}
			else//抛物线
			{
				Width=myPara->StreamLength*sqrt(1-2*myPara->StreamLength*(x-H[0])/3/myPara->AreaR);
				VH=(3*myPara->AreaR/2/myPara->StreamLength-x+H[0])*Width-myPara->AreaR*pow(Width,3)/4/pow(myPara->StreamLength,3);
				VH=VH*dx2*cos(beta2R);

				if(debug && (Width-myPara->StreamLength>1e-3 || Width<=0)) 
				{
					myFile<<"warning:右坡面"<<lpR->first<<"类型,第"<<1+fvR.size()<<"个单元宽度:"<<Width<<",longer than river "<<myPara->StreamLength<<endl;
					Width=myPara->StreamLength;
				}
			}

			//为计算初始hm
			if(y2>W[1]) 
			{
				//W[0]=x;
				hm=y2+1e-4;
			}
			else
			{
				if(flag_rock==false)
				{
					flag_rock=true;
					W[0]=x;
				}
				hm=N[1]+(N[1]-W[1])/(N[0]-W[0])*(x-N[0]);
			}

			if(!_finite(y2) || !_finite(y1))
			{
				myFile<<"warning:slopeR:y1 or y2 is INF"<<endl;
			}

			fvR.push_back(FVMunit(x,y1,y2,hm,y1+ahupLR,dx1,dx2,beta1R,beta2R,Width,(lpR+1)->first,"R",(x-H[0])/cosR,VH));

			x-=dx1*cosR;
			y1-=dx1*sinR;

		}//end for

		x+=0.5*dx1*cosR;
		y1+=0.5*dx1*sinR;

	}//end for

	/******************************右坡面有限单元组生成完毕******************************/

	/********************************生成源坡面有限单元组********************************/
	if(myPara->LengthS>0)
	{
		x=AS[0];y1=AS[1];
		flag_rock=false;

		//确定有限单元起始高程点
		if((lvS.begin()+1)->first==0)
		{
			pStart=lvS.begin()+1;
		}
		else
		{
			pStart=lvS.begin();
		}

		for(lpS=pStart;lpS!=lvS.end();lpS++)
		{
			if(lpS==lvS.end()-1)
			{
				break;
			}

			len=(lpS->second-(lpS+1)->second)/sinS;
			//因为加入B、C临界高程点后，B和C的位置不确定性可能导致len接近与0，则dx1和dx2也会被计算出为0，这会因此数值错误的。
			if(len<0.5)
			{
				continue;
			}
			num=len/Min.dx+1;

			//保证坡面网格数大于等于3,如果lvS的size大于等于4,网格数肯定大于等于3,不用特殊处理
			if(lvS.size()==2) { num=max(num,3); }
			if(lvS.size()==3 && lpS==lvS.begin()+1 && fvS.size()+num<=2){ num=2; }

			dx1=len/num;

			x+=0.5*dx1*cosS;
		    y1-=0.5*dx1*sinS;

			for(int i=0;i<num;i++)
			{
				y2=tan(sitaATS)*(x-TS[0]);
				beta2S=PI-sitaATS;
				dx2=dx1*cosS/cos(beta2S);

				//计算不同高程点坡面宽度
				Width=Bst*(myPara->LengthS-x)/myPara->LengthS;

				VH=0.5*(Bst+Width)*x*dx2*cos(beta2S);//梯形截面

				//为计算初始hm
				if(y2>W[1]) 
				{
					//W[0]=x;
					hm=y2+1e-4;
				}
				else
				{
					if(flag_rock==false)
					{
						flag_rock=true;
						W[0]=x;
					}
					hm=MS[1]+(MS[1]-W[1])/(MS[0]-W[0])*(x-MS[0]);
				}

				fvS.push_back(FVMunit(x,y1,y2,hm,y1+ahupS,dx1,dx2,beta1S,beta2S,Width,(lpS+1)->first,"S",x/cosS,VH));

				x+=dx1*cosS;
				y1-=dx1*sinS;

			}//end for

			x-=0.5*dx1*cosS;
		    y1+=0.5*dx1*sinS;

		}//end for

	}//end if

	if(debug)
	{
		if(myPara->LengthS>0)
		{
			myFile<<"左、右、源坡面有限单元数目(个):"<<fvL.size()<<","<<fvR.size()<<","<<fvS.size()<<endl<<endl;
		}
		else
		{
			myFile<<"左、右坡面有限单元数目(个):"<<fvL.size()<<","<<fvR.size()<<endl<<endl;
		}
	}
	/******************************源坡面有限单元组生成完毕******************************/
	
	/******************************生成坡面漫流临时存储变量******************************/
	 if(fvL.size()>0)
	 {
		 aL=new double[fvL.size()-1]; bL=new double[fvL.size()]; cL=new double[fvL.size()-1];  
		 fL=new double[fvL.size()];   xL=new double[fvL.size()]; dL=new double[fvL.size()];   
		 axL=new double[fvL.size()];  afL=new double[fvL.size()];    
	 }

	 if(fvR.size()>0)
	 {
		 aR=new double[fvR.size()-1]; bR=new double[fvR.size()]; cR=new double[fvR.size()-1];
		 fR=new double[fvR.size()];   xR=new double[fvR.size()]; dR=new double[fvR.size()];        
		 axR=new double[fvR.size()];  afR=new double[fvR.size()];
	 }

	if(myPara->LengthS>0 && fvS.size()>0)
	{
		aS=new double[fvS.size()-1]; cS=new double[fvS.size()-1]; xS=new double[fvS.size()];   bS=new double[fvS.size()]; 
		fS=new double[fvS.size()];   dS=new double[fvS.size()];  axS=new double[fvS.size()];  afS=new double[fvS.size()];
	}
	/******************************生成坡面漫流临时变量完毕******************************/
	
}


//一个时间步长内单个坡面的地表漫流计算,dt:坡面漫流的时间步长;p:dt时间内的降雨;
void HighSlopeRunoff::SlopeRunoff(string sType,float p,float dt)
{
	::transform(sType.begin(),sType.end(),sType.begin(),tolower);//string类型的大小写转换方法

	std::vector<FVMunit> *fv;
	float ks;
	double **a,**b,**c,**x,**f,**d,**ax,**af;//bx:x的增量序列,ax相当于x(i+1)
	
	if(sType=="l")
	{
		fv=&fvL;
	    ks=sqrt(myPara->SlopeL)/myPara->Manning;
		a=&aL;b=&bL;c=&cL;x=&xL;f=&fL;d=&dL;ax=&axL,af=&afL;
	}
	else if(sType=="r")
	{
		fv=&fvR;
		ks=sqrt(myPara->SlopeR)/myPara->Manning;
		a=&aR;b=&bR;c=&cR;x=&xR;f=&fR;d=&dR;ax=&axR,af=&axR;
	}
	else if(sType=="s")
	{
		fv=&fvS;
		ks=sqrt(myPara->SlopeS)/myPara->Manning;
		a=&aS;b=&bS;c=&cS;x=&xS;f=&fS;d=&dS;ax=&axS,af=&axS;
	}
	else
	{
		cout<<"wrong slope type"<<endl;
		return;
	}

	int N=(*fv).size();
	
	//网格生成时保证N一定大于等于3
	//if(N<=1) { return;}

	const float maxss=0.002;//m
	const float maxqs=0.002;//m2/s
	
	//从第一个单元开始进行坡面漫流计算
	int StartUnit=1;
	
	//从坡面单元水深大于1mm地方开始算起
	//此时情况是即使有降雨，但是落在坡面后的最大水深也小于1mm,不进行坡面流计算。
	bool flag1=false,flag2=true;
	for(int i=0;i<N;i++)
	{
		//渗透蒸发，坡面漫流源汇项计算，更新下坡面的水深
		(*fv)[i].SlopeSourceTerm(p,dt);
		if((*fv)[i].ss>maxss && (*fv)[i].qs>maxqs && flag2==true) 
		{ 
			StartUnit=i+1;
			flag2=false;
			flag1=true;
		}
	}
	
	if(flag1==false){ return;}

	/*if(FVMunit::TimeStart==259200)
	{
		int xxx=0;
	}*/

	//将有限体积的空间步长转化为差分格式的空间步长
	for(int i=0;i<N-StartUnit+1;i++)
	{
		if(i<=N-2)
		{
			(*d)[i]=((*fv)[i].dx1+(*fv)[i+1].dx1)/2;
		}
		else
		{
			(*d)[i]=(*fv)[i].dx1/2;
		}	
	}
	
	const float alfa=0.6;
	const float m=pow(ks,-1/alfa)/alfa;
	
	//preissmann偏心系数
	const float cita=0.7;

	//上一时刻本节点和下一节点的qs值
	float aa,bb;

	//牛顿迭代误差上限及最多迭代次数
	float error=0.01,maxN=0;

	//DisO:猜测值|f(X(k))|,DisN:从众多猜测值中选取的"下降值"|f(X(k+1))|。
	float DisO=1,DisN=1;

	//三对角中的下对角线赋值为0，你不赋值为0，程序就是任意值了。
	ZeroFill(*a,N-StartUnit+1);

	//牛顿迭代的全0初始解
	//ZeroFill(*x,N-StartUnit+1);
	for(int i=0;i<=N-StartUnit;i++)
	{
		(*x)[i]=(*fv)[i+StartUnit-1].qs;
	}
	
	//上边界条件
	(*fv)[StartUnit-1].qs=1e-8;
	(*fv)[StartUnit-1].ss=pow(double((*fv)[StartUnit-1].qs),double(1/alfa))*pow(float(ks),float(-1/alfa));//m

	//下降牛顿迭代
	while(DisN>error)
	{
		maxN++;
		if(maxN==30)
		{
			if(debug)
			{
				myFile<<"坡面类型"<<sType<<",误差DisN:"<<DisN<<",TimeStart:"<<FVMunit::TimeStart<<","<<"坡面漫流迭代30次不收敛"<<endl<<endl;
			}
			break;
		}

		for(int i=StartUnit-1;i<=N-2;i++)
		{
			//当前时刻的j和j+1节点的坡面单宽流量m2/s
			aa=(*fv)[i].qs; 
			bb=(*fv)[i+1].qs;

			(*c)[i-StartUnit+1] = m*(*d)[i-StartUnit+1]*((*x)[i-StartUnit+2]+(*x)[i-StartUnit+1]-aa-bb)/4/dt * cita*(1/alfa-1) * pow(double(cita*((*x)[i-StartUnit+2]+(*x)[i-StartUnit+1])+(1-cita)*(aa+bb)),double(1/alfa-2))
				                + m*(*d)[i-StartUnit+1]/4/dt*pow(double(cita*((*x)[i-StartUnit+2]+(*x)[i-StartUnit+1])+(1-cita)*(aa+bb)),double(1/alfa-1)) + cita*((*fv)[i+1].Width+(*fv)[i].Width)/2/(*fv)[i].Width;

			(*b)[i-StartUnit+1] =(*c)[i-StartUnit+1]-2*cita;

			(*f)[i-StartUnit+1] = m*(*d)[i-StartUnit+1]/4/dt*pow(double(cita*((*x)[i-StartUnit+2]+(*x)[i-StartUnit+1])+(1-cita)*(aa+bb)),double(1/alfa-1))*((*x)[i-StartUnit+2]+(*x)[i-StartUnit+1]-aa-bb) + cita*((*x)[i-StartUnit+2]-(*x)[i-StartUnit+1])
				                + (1-cita)*(bb-aa) + ((*fv)[i+1].Width-(*fv)[i].Width)/2/(*fv)[i].Width*(cita*((*x)[i-StartUnit+2]+(*x)[i-StartUnit+1])+(1-cita)*(aa+bb))-(*fv)[i].ss/dt*(*d)[i-StartUnit+1];
		}

		//下边界条件，流量变化率为0
		aa=(*fv)[N-1].qs;

		(*b)[N-StartUnit] = m*(*d)[N-StartUnit] * ((*x)[N-StartUnit]-aa)/dt * cita*(1/alfa-1) * pow(double(cita*2*(*x)[N-StartUnit]+(1-cita)*2*aa),double(1/alfa-2)) + m*(*d)[N-StartUnit]/2/dt*pow(double(cita*2*(*x)[N-StartUnit]+(1-cita)*2*aa),double(1/alfa-1));
		(*c)[N-StartUnit] = 0;
		(*f)[N-StartUnit] = m*(*d)[N-StartUnit]/2/dt*pow(double(cita*2*(*x)[N-StartUnit]+(1-cita)*2*aa),double(1/alfa-1))*((*x)[N-StartUnit]-aa)-(*fv)[N-1].ss/dt*(*d)[N-StartUnit];

		//取向量无穷范数
		DisO=VecDistance8(N-StartUnit+1,*f);
		if(DisO<error)
		{
			break;
		}
		
		//下面为了确定合适的s使得迭代距离|f(X(k))|下降
		for(int t=0;t<20;t++)
		{
			//因为负号的问题，所以分为if和else
			if(t==0) 
			{ 
				for(int i=0;i<=N-StartUnit;i++) 
				{ 
					(*f)[i]=-(*f)[i]*0.8;
				}
			}
			else     
			{ 
				for(int i=0;i<=N-StartUnit;i++) 
				{ 
					(*f)[i]/=2;
				}
			}

			//追赶法求线性方程组，对于此离散格式下对角线元素都为0，其实是二对角，因为只用到了下游节点，这里得到X的增量ax序列。
			::TDMA(N-StartUnit+1,*a,*b,*c,*f,*ax);

			//得到X(k+1),仍存储在ax向量中
			for(int i=0;i<N-StartUnit+1;i++) 
			{ 
				//cout<<"(*ax):"<<(*ax)[i]<<endl;
				
				//强制ax大于0发现最后结果是要强烈发散的，ax可能会很大很大，干扰了牛顿迭代的运行规律
				//这里不能对ax数组中的任何值有强制赋值的代码，包括上边界条件ax[0]=0，会引起不可知结果
				//(*ax)[i]=max(1e-8,(*ax)[i]+(*x)[i]);

				(*ax)[i]=(*ax)[i]+(*x)[i];
			}

			for(int i=StartUnit-1;i<=N-2;i++)
			{
				//当前时刻的j和j+1节点的坡面单宽流量m2/s
				aa=(*fv)[i].qs; 
				bb=(*fv)[i+1].qs;
				(*af)[i-StartUnit+1] = m*(*d)[i-StartUnit+1]/4/dt*pow(double(cita*((*ax)[i-StartUnit+2]+(*ax)[i-StartUnit+1])+(1-cita)*(aa+bb)),double(1/alfa-1))*((*ax)[i-StartUnit+2]+(*ax)[i-StartUnit+1]-aa-bb) + cita*((*ax)[i-StartUnit+2]-(*ax)[i-StartUnit+1])
					                 + (1-cita)*(bb-aa) + ((*fv)[i+1].Width-(*fv)[i].Width)/2/(*fv)[i].Width*(cita*((*ax)[i-StartUnit+2]+(*ax)[i-StartUnit+1])+(1-cita)*(aa+bb))-(*fv)[i].ss/dt*(*d)[i-StartUnit+1];

				//cout<<"(*af):"<<(*af)[i-StartUnit+1]<<",(*d):"<<(*d)[i-StartUnit+1]<<",(*ax):"<<(*ax)[i-StartUnit+1]<<",(*fv).qs:"<<aa<<",(*fv).ss:"<<(*fv)[i].ss<<endl;
			}
			
			//下边界条件
			aa=(*fv)[N-1].qs;

			//pow(a,b),a要大于0，否则很可能出现无意义的情况
			(*af)[N-StartUnit] = m*(*d)[N-StartUnit]/2/dt*pow(double(cita*2*(*ax)[N-StartUnit]+(1-cita)*2*aa),double(1/alfa-1))*((*ax)[N-StartUnit]-aa)-(*fv)[N-1].ss/dt*(*d)[N-StartUnit];
			
			//cout<<"(*af):"<<(*af)[N-StartUnit]<<",(*d):"<<(*d)[N-StartUnit]<<",(*ax):"<<(*ax)[N-StartUnit]<<",(*fv).qs:"<<aa<<",(*fv).ss:"<<(*fv)[N-1].ss<<endl;

			//取向量无穷范数
			DisN=VecDistance8(N-StartUnit+1,*af);

			//即使DisN大于Dis0,只要Dis0小于error,当然也没问题.解决迭代第一步.
			if(DisN<DisO || t==49)
			{
				for(int i=0;i<N-StartUnit+1;i++) 
				{ 
					(*x)[i]=(*ax)[i];
				}

				//if(debug) 
				//{ 
					//cout<<"坡面类型"<<sType<<",牛顿迭代次数:"<<maxN<<",下降解查找次数:"<<t<<",当前迭代总次数:"<<maxN*t<<",坡面漫流存在下降解."<<endl;
				//}
				break;
			}
		}

	}//end while(DisN>error)

	//更新坡表面经过汇流过程的单宽流量qs和水深ss

	
	for(int i=StartUnit-1;i<N;i++)
	{
		//最小值假定为1e-6
		(*fv)[i].qs=max(1e-6,(*x)[i-StartUnit+1]);//m2/s

		if(!_finite((*fv)[i].qs))
		{
			myFile<<"warning:坡面漫流INF解."<<endl;
			myFile<<"RIndex:"<<mBSCode.RegionIndex<<",Value:"<<mBSCode.Value<<",Length:"<<mBSCode.Length<<",sType:"<<sType<<",第"<<i<<"个单元"<<endl;
			(*fv)[i].qs=1e-5;
		}

		//如果大于10就当是计算出的结果发散了
		if((*fv)[i].qs>10)
		{ 
			myFile<<"warning:坡面漫流>10(m2/s)"<<endl;
			(*fv)[i].qs=1e-5; 
		}

		/*if((*fv)[i].qs>1e2 || (*fv)[i].qs<-1e-2)
		{
			int xx=0;
			break;
		}*/

		(*fv)[i].ss=pow(double((*fv)[i].qs),double(1/alfa))*pow(float(ks),float(-1/alfa));//m

		//cout<<"单元数:"<<i<<","<<(*fv)[i].qs<<"(m2/s),"<<(*fv)[i].ss<<"(m)"<<endl;
	}

}


//饱和土壤水运动(和岩石裂隙水交互)
void HighSlopeRunoff::SaturatedSoil(string sType,float dt)
{
	::transform(sType.begin(),sType.end(),sType.begin(),::tolower); //string类型的大小写转换方法
	
	std::vector<FVMunit> *fv;
	double **a,**b,**c,**x,**f;//bx:x的增量序列,ax相当于x(i+1)
	
	if(sType=="l")
	{ 
		fv=&fvL;a=&aL;b=&bL;c=&cL;x=&xL;f=&fL;
	}
	if(sType=="r")
	{ 
		fv=&fvR;a=&aR;b=&bR;c=&cR;x=&xR;f=&fR;
	}
	if(sType=="s")
	{ 
		fv=&fvS;a=&aS;b=&bS;c=&cS;x=&xS;f=&fS;
	}
	
	int N=(*fv).size();
	
	//网格生成时保证N一定大于等于3
	//if(N<=2){ return; }

    /***********************预测步可以确定给水度，并得到各FVM单元显式的hm值***************************/
	
	//T1:T(i+1/2,n),T2:T(i-1/2,n)
	float T1,T2,TK;

	//空间步长
	float xl,xr,xm;
	
	//avg:动态变动区的平均含水量
	float p=0,Z=0,sum=0,num=0;

	//预测步有作用，如果非饱和土壤的最下方饱和后，通过预测步会转化成饱和土壤水，就避免了非饱和土壤与岩石水的直接交互处理。
	//如果没有预测步，不知道非饱和土壤何时饱和，校正步只能对所有单元求解。
	for(int i=1;i<=N-2;i++)
	{
		//i和i+1单元的---BKcos(theta)(H-Hpd)---取平均值
		T1=0.5*((*fv)[i].Width*(*fv)[i].ksa*cos((*fv)[i].beta2)*((*fv)[i].hm-(*fv)[i].hdown) + (*fv)[i+1].Width*(*fv)[i+1].ksa*cos((*fv)[i+1].beta2)*((*fv)[i+1].hm-(*fv)[i+1].hdown));

		//i和i-1单元的---BKcos(theta)(H-Hpd)---取平均值
		T2=0.5*((*fv)[i].Width*(*fv)[i].ksa*cos((*fv)[i].beta2)*((*fv)[i].hm-(*fv)[i].hdown) + (*fv)[i-1].Width*(*fv)[i-1].ksa*cos((*fv)[i-1].beta2)*((*fv)[i-1].hm-(*fv)[i-1].hdown));

		xl=0.5*((*fv)[i].dx2+(*fv)[i-1].dx2);
		xr=0.5*((*fv)[i].dx2+(*fv)[i+1].dx2);
		xm=(*fv)[i].dx2;

		//土壤水补充岩石水的情况
		if((*fv)[i].hdown>Rock.H)
		{
			TK=Rock.Kr*cos((*fv)[i].beta2);
		}
		//岩石水补充土壤水的情况
		else
		{
			TK=(*fv)[i].ksa*((*fv)[i].hm-(*fv)[i-1].hm)/xm;
			//TK=(*fv)[i].ksa*sin((*fv)[i].beta2);
		}

		p=(T1*((*fv)[i+1].hm-(*fv)[i].hm)*xl - T2*((*fv)[i].hm-(*fv)[i-1].hm)*xr)/(*fv)[i].Width/xl/xm/xr-TK;

		if(!_finite(p))
		{
			if(debug)
			{
				myFile<<"warning:(index,value,length):("<<mBSCode.RegionIndex<<","<<mBSCode.Value<<","<<mBSCode.Length<<"):"<<"预测步饱和土壤水p is INF"<<endl;
			}
			(*x)[i]=(*fv)[i].hup-1e-5;
			p=((*x)[i]-(*fv)[i].hm)*(*fv)[i].mu/dt;
		}

		//给水度的确定--水位降低情况
		if(p<=0) 
		{ 
			(*fv)[i].mu = FVMunit::thetas-FVMunit::thetaf;
			(*x)[i]=max((*fv)[i].hdown,p*dt/(*fv)[i].mu+(*fv)[i].hm);
		}
		//给水度的确定--水位升高情况
		else
		{
			for(int t=(*fv)[i].ri.size()-1;t>=0;t--)
			{
				//相当于mu=s-0，全部损失，转化为坡面流，不过坡面流不是土壤，不存在初始含水量问题，所以这里取1
				if((*fv)[i].ri.empty())
				{
					//此种情况相当于饱和土壤水的上方全是空气介质
					(*fv)[i].mu=1;
					(*x)[i]=min((*fv)[i].hup,max((*fv)[i].hdown,p*dt/(*fv)[i].mu+(*fv)[i].hm));
					break;
				}

				//给水度:饱和含水量-变动区的平均含水量
				sum+=(*fv)[i].ri[t].theta;
				Z+=(*fv)[i].ri[t].dz/1000;//mm->m
				num+=1;

				(*fv)[i].mu=FVMunit::thetas-sum/num;

				//防止给水度为0（因为此时作为分母）
				if(abs((*fv)[i].mu)<1e-5)
				{
					(*fv)[i].mu=1e-5;
				}

				if(!_finite((*fv)[i].mu))
				{
					if(debug)
					{
						myFile<<"warning:(index,value,length):("<<mBSCode.RegionIndex<<","<<mBSCode.Value<<","<<mBSCode.Length<<"):"<<"饱和土壤水给水度mu is INF"<<endl;
					}
					(*fv)[i].mu=1;
				}

				//p*dt/(*fv)[i].mu:预测步的H(i,k+1)-H(i,k)
				//饱和土壤水升高量与相应的非饱和土壤减少量之差在一个网格内，说明找到了合适的mu
				if(abs(p*dt/(*fv)[i].mu-Z) <= (*fv)[i].ri[t].dz/1000)//mm->m
				{
					//此种情况相当于饱和土壤水的上方全是土壤
					(*x)[i]=min((*fv)[i].hup,max((*fv)[i].hdown,p*dt/(*fv)[i].mu+(*fv)[i].hm));
					break;
				}

				//如果上面的没有break，说明饱和土壤水溢出坡面，回归流产生
				if(t==0)
				{
					//此种情况相当于饱和土壤水运动一个步长后，既填满土壤，又初露地表，所以给水度必须特殊处理。
					//因为如果只按照土壤的来取（此时的给水度很小），则会夸大回归流的作用。

					//ratio:土壤水给水度与空气比所占的权重
					float ratio=Z/(p*dt/(*fv)[i].mu+(*fv)[i].hm);
					
					//(*fv)[i].mu=(*fv)[i].mu*ratio+1*(1-ratio);
					(*fv)[i].mu=max(1e-5,min(1,1-(1-(*fv)[i].mu)*ratio));

					(*x)[i]=min((*fv)[i].hup,max((*fv)[i].hdown,p*dt/(*fv)[i].mu+(*fv)[i].hm));

					break;
				}

			}//end for(int t...

		}//end else

		if(!_finite((*x)[i]))
		{
			if(debug)
			{
				myFile<<"warning:(index,value,length):("<<mBSCode.RegionIndex<<","<<mBSCode.Value<<","<<mBSCode.Length<<"):"<<"预测步饱和土壤水hm is INF"<<endl;
				myFile<<"p:"<<p<<"\tmu:"<<(*fv)[i].mu<<endl;
			}
			(*x)[i]=(*fv)[i].hup-1e-5;
		}

		//更新断面平均渗透系数K(m/s)
		if((*x)[i]-(*fv)[i].hdown>1e-3)
		{
			(*fv)[i].ksa=FVMunit::fc[(*fv)[i].landtype]*(exp(-FVMunit::usf*abs((*fv)[i].hup-(*x)[i])-exp(-FVMunit::usf*((*fv)[i].hup-(*fv)[i].hdown))))/FVMunit::usf/((*x)[i]-(*fv)[i].hdown);
		}
		else
		{
			(*fv)[i].ksa=FVMunit::fc[(*fv)[i].landtype]*exp(-FVMunit::usf*((*fv)[i].hup-(*x)[i]));
		}

		if(!_finite((*fv)[i].ksa))
		{
			float hup=(*fv)[i].hup;
			float xx=(*x)[i];
			float hdown=(*fv)[i].hdown;
		}

	}//end for(int i...

	//更形上边界水深
	(*x)[0]=(*fv)[0].hm=(*fv)[0].hdown;

	//更新上边界的Ksa
	(*fv)[0].ksa=FVMunit::fc[(*fv)[0].landtype]*exp(-FVMunit::usf*((*fv)[0].hup-(*x)[0]));

	//更新上边界的mu
	(*fv)[0].mu=1;

	//更新下边界水深
	(*x)[N-1]=(*fv)[N-1].hm=max(Bchannel.H,(*fv)[N-1].hdown);

	//更新下边界的Ksa
	if((*x)[N-1]-(*fv)[N-1].hdown>1e-3)
	{
		(*fv)[N-1].ksa=FVMunit::fc[(*fv)[N-1].landtype]*(exp(-FVMunit::usf*abs((*fv)[N-1].hup-(*x)[N-1])-exp(-FVMunit::usf*((*fv)[N-1].hup-(*fv)[N-1].hdown))))/FVMunit::usf/((*x)[N-1]-(*fv)[N-1].hdown);
	}
	else
	{
		(*fv)[N-1].ksa=FVMunit::fc[(*fv)[N-1].landtype]*exp(-FVMunit::usf*((*fv)[N-1].hup-(*x)[N-1]));
	}

	//更新下边界的mu
	(*fv)[N-1].mu=(*fv)[N-2].mu;

	/****************************************预测步完成，校正步开始******************************************/
	//预测步采用显格式，校正步采用有限体积法隐格式
	
	//确定从第几个单元开始计算，之上部分不计算饱和土壤水运动
	int StartUnit=0;
	for(int i=0;i<N;i++)
	{
		//为了不让StartUnit是最后一个单元，因为下面用到了i+1
		if(i==N-1)
		{
			return;//饱和土壤水太少，直接返回。
		}

		//该点的hm大于2mm，不一定以后各点都大于2mm，可能会小于0
		if(((*x)[i]-(*fv)[i].hdown)>2e-3)
		{
			StartUnit=i+1;
			break;
		}
		else
		{
		     //不足0.1mm的,给个0.1mm小水深
			(*fv)[i].hm=max((*x)[i],(*fv)[i].hdown+1e-4);
		}
	}

	//RockW:岩石裂隙水的变化水量m3
	float S,RockW=0;
	float TE,TW,TP,TB;

	/*************************************************上边界赋值*************************************************/
	TP=(*fv)[StartUnit-1].Width*(*fv)[StartUnit-1].ksa*cos((*fv)[StartUnit-1].beta2)*((*fv)[StartUnit-1].hm-(*fv)[StartUnit-1].hdown);
	TE=(*fv)[StartUnit].Width*(*fv)[StartUnit].ksa*cos((*fv)[StartUnit].beta2)*((*fv)[StartUnit].hm-(*fv)[StartUnit].hdown);

	(*b)[0]=(*fv)[StartUnit-1].mu*(*fv)[StartUnit-1].Width*(*fv)[StartUnit-1].dx2*((*fv)[StartUnit-1].dx2+(*fv)[StartUnit].dx2)/dt+TP+TE;
	(*c)[0]=-(TE+TP);
	(*f)[0]=(*fv)[StartUnit-1].mu*(*fv)[StartUnit-1].Width*(*fv)[StartUnit-1].dx2*((*fv)[StartUnit-1].dx2+(*fv)[StartUnit].dx2)/dt*(*fv)[StartUnit-1].hm;

	S=(*fv)[StartUnit-1].dx2*cos((*fv)[StartUnit-1].beta2)*(*fv)[StartUnit-1].Width;

	//情况2：补给岩石水，但是不出现负水深情况
	if((*fv)[StartUnit-1].hdown>Rock.H && (*x)[StartUnit-1]>=2e-3)
	{
		(*f)[0]-=(*fv)[StartUnit-1].Width*Rock.Kr*((*fv)[StartUnit-1].dx2+(*fv)[StartUnit].dx2)*(*fv)[StartUnit-1].dx2*cos((*fv)[StartUnit-1].beta2);
		RockW += Rock.Kr*dt*S;//m3
	}
	/*************************************************上边界赋值*************************************************/

	/************************************************中间节点赋值************************************************/
	for(int i=StartUnit;i<=N-2;i++)
	{
		TW=(*fv)[i-1].Width*(*fv)[i-1].ksa*cos((*fv)[i-1].beta2)*((*fv)[i-1].hm-(*fv)[i-1].hdown);
		TP=(*fv)[i].Width*(*fv)[i].ksa*cos((*fv)[i].beta2)*((*fv)[i].hm-(*fv)[i].hdown);
		TE=(*fv)[i+1].Width*(*fv)[i+1].ksa*cos((*fv)[i+1].beta2)*((*fv)[i+1].hm-(*fv)[i+1].hdown);

		TK=0.5*(*fv)[i].Width*(*fv)[i].ksa*((*fv)[i].dx2+(*fv)[i+1].dx2)*((*fv)[i].dx2+(*fv)[i-1].dx2);

		(*a)[i-StartUnit]=-(TW+TP)*((*fv)[i].dx2+(*fv)[i+1].dx2)/2;
		(*b)[i-StartUnit+1]=(*fv)[i].mu*(*fv)[i].Width*(*fv)[i].dx2*((*fv)[i].dx2+(*fv)[i+1].dx2)*((*fv)[i].dx2+(*fv)[i-1].dx2)/2/dt+(TE+TP)*((*fv)[i].dx2+(*fv)[i-1].dx2)/2+(TW+TP)*((*fv)[i].dx2+(*fv)[i+1].dx2)/2;
		(*c)[i-StartUnit+1]=-(TE+TP)*((*fv)[i].dx2+(*fv)[i-1].dx2)/2;
		(*f)[i-StartUnit+1]=(*fv)[i].mu*(*fv)[i].Width*(*fv)[i].dx2*((*fv)[i].dx2+(*fv)[i+1].dx2)*((*fv)[i].dx2+(*fv)[i-1].dx2)/2/dt*(*fv)[i].hm;

		S=(*fv)[i].dx2*(*fv)[i].Width;

		//为避免负水深，我这里先显后隐的处理方式要求保证下一时刻没水的单元在所有单元前面且连续，否则如果中间某点没水，两侧都有水，你能怎么样呢？
		//情况2：补给岩石水，但是不出现负水深情况
		if((*fv)[i].hdown>Rock.H && (*x)[i]>=2e-3)
		{
			(*f)[i-StartUnit+1]-=0.5*((*fv)[i].dx2+(*fv)[i+1].dx2)*((*fv)[i].dx2+(*fv)[i-1].dx2)*(*fv)[i].dx2*(*fv)[i].Width*Rock.Kr*cos((*fv)[i].beta2);
			RockW += Rock.Kr*cos((*fv)[i].beta2)*dt*S;//m3
		}
		
		//情况3：岩石水补给土壤水情况
		if((*fv)[i].hdown<=Rock.H)
		{
			(*a)[i-StartUnit]-=TK;
			(*b)[i-StartUnit+1]+=TK;
			RockW -= (*fv)[i].ksa*abs((*fv)[i].hm-(*fv)[i-1].hm)/(*fv)[i].dx2*dt*S;
			//RockW -= (*fv)[i].ksa*sin((*fv)[i].beta2)*dt*S;
		}
	}
	/************************************************中间节点赋值************************************************/

	/*************************************************下边界赋值*************************************************/
	TP=(*fv)[N-1].Width*(*fv)[N-1].ksa*cos((*fv)[N-1].beta2)*((*fv)[N-1].hm-(*fv)[N-1].hdown);
	TW=(*fv)[N-2].Width*(*fv)[N-2].ksa*cos((*fv)[N-2].beta2)*((*fv)[N-2].hm-(*fv)[N-2].hdown);
	TB=TP;//注意这里的处理方式

	TK=(*fv)[N-1].Width*(*fv)[N-1].ksa*((*fv)[N-1].dx2+(*fv)[N-2].dx2)*(*fv)[N-1].dx2;

	(*a)[N-StartUnit-1]=-(TW+TP)*(*fv)[N-1].dx2;
	(*b)[N-StartUnit]=(*fv)[N-1].mu*pow((*fv)[N-1].Width,2)*((*fv)[N-1].dx2+(*fv)[N-1].dx2)/dt+2*TB*((*fv)[N-1].Width+(*fv)[N-2].Width)+(TW+TP)*(*fv)[N-1].Width;
	(*f)[N-StartUnit]=(*fv)[N-1].mu*pow((*fv)[N-1].Width,2)*((*fv)[N-1].dx2+(*fv)[N-1].dx2)/dt*(*fv)[N-1].hm+2*TB*max((*fv)[N-1].hdown,max((*fv)[N-1].hdown,Bchannel.H))*((*fv)[N-1].Width+(*fv)[N-2].Width);

	//情况2：补给岩石水，但是不出现负水深情况
	if((*fv)[N-1].hdown>Rock.H && (*x)[N-1]>=1e-3)
	{
		(*f)[N-StartUnit]-=((*fv)[N-1].dx2+(*fv)[N-2].dx2)*(*fv)[N-1].dx2*(*fv)[N-1].dx2*(*fv)[N-1].Width*Rock.Kr*cos((*fv)[N-1].beta2);
	}

	//情况3：岩石水补给土壤水情况
	if((*fv)[N-1].hdown<=Rock.H)
	{
		(*a)[N-StartUnit-1]-=TK;
		(*b)[N-StartUnit]+=TK;
	}
	/*************************************************下边界赋值*************************************************/
	
	::TDMA(N-StartUnit+1,*a,*b,*c,*f,*x);
	/**************************************************校正步完成****************************************************/

	//更新岩石水的水位
	Rock.WtoH(RockW);

	//更新饱和土壤水水深及平均渗透系数ksa(m/s)，给水度就不更新了，比较麻烦，因为给水度需要保存相邻两个时刻的H，这里仍然采用预测步的值。
	for(int i=StartUnit-1;i<N;i++)
	{
		(*fv)[i].hm=(*x)[i-StartUnit+1];

		if(!_finite((*fv)[i].hm))
		{
			if(debug)
			{
				myFile<<"warning:(index,value,length):("<<mBSCode.RegionIndex<<","<<mBSCode.Value<<","<<mBSCode.Length<<"):"<<"校正步饱和土壤水hm is INF"<<endl;
				myFile<<"TimeStart:"<<FVMunit::TimeStart<<",sType:"<<sType<<",单元:"<<i<<",hdown:"<<(*fv)[i].hdown<<",hup:"<<(*fv)[i].hup<<",hm:"<<(*fv)[i].hm<<endl;
			}
			
			if(i==0)
			{
				(*fv)[i].hm=(*fv)[i].hdown+1e-6; 
			}
			else
			{
				(*fv)[i].hm=(*fv)[i-1].hm;
				(*fv)[i].hm=max((*fv)[i].hdown,min((*fv)[i].hm,(*fv)[i].hup));
			}
		}

		//有回归流产生
		if((*fv)[i].hm>(*fv)[i].hup)
		{
			(*fv)[i].hv=((*fv)[i].hm-(*fv)[i].hup)/dt;//m/s
			(*fv)[i].hm=(*fv)[i].hup-1e-4;
			
			//这句给TimeEnd赋值，表明在TimeStart与TimeEnd之间有回归流发生
			(*fv)[i].TimeEnd=FVMunit::TimeStart+dt;

			if(debug)
			{
				int sLength;
				if(sType=="l") sLength=HighSlopeRunoff::myPara->LengthL/cos((*fv)[i].beta1);
				if(sType=="r") sLength=HighSlopeRunoff::myPara->LengthR/cos((*fv)[i].beta1);
				if(sType=="s") sLength=HighSlopeRunoff::myPara->LengthS/cos((*fv)[i].beta1);
				myFile<<"sType:"<<sType<<",坡面长:"<<sLength<<"(m),距坡顶斜向距离:"<<(*fv)[i].Dis<<"(m),地下水埋深:"<<(*fv)[i].hup-(*fv)[i].hm<<"(m)"<<endl;
				myFile<<"Year:"<<FVMunit::Year<<",Month:"<<FVMunit::Month<<",Day:"<<FVMunit::Day<<",Hour:"<<FVMunit::Hour<<"-"<<FVMunit::Hour+dt/3600<<",产生回归流:"<<(*fv)[i].hv*dt<<"(m),析出速率:"<<(*fv)[i].hv*1000<<"(mm/s)"<<endl;
			}
		}

		if((*fv)[i].hm<(*fv)[i].hdown)
		{
			(*fv)[i].hm=(*fv)[i].hdown+1e-4;
		}
		
		//更新断面渗透系数ksa
		if((*fv)[i].hm-(*fv)[i].hdown>1e-4)
		{
			(*fv)[i].ksa=FVMunit::fc[(*fv)[i].landtype]*(exp(-FVMunit::usf*((*fv)[i].hup-(*fv)[i].hm)-exp(-FVMunit::usf*((*fv)[i].hup-(*fv)[i].hdown))))/FVMunit::usf/((*fv)[i].hm-(*fv)[i].hdown);
		}
		else
		{
			(*fv)[i].ksa=FVMunit::fc[(*fv)[i].landtype]*exp(-FVMunit::usf*((*fv)[i].hup-(*fv)[i].hm));
		}

		if(!_finite((*fv)[i].ksa))
		{
			if(debug)
			{
				float hup=(*fv)[i].hup,hm=(*fv)[i].hm,hdown=(*fv)[i].hdown;
				myFile<<"warning:ksa is INF:hup:"<<hup<<",hm:"<<hm<<",hdown:"<<hdown<<endl;
			}
			(*fv)[i].ksa=1e-6;
		}

		//cout<<sType<<"单元:"<<i<<",hdown:"<<(*fv)[i].hdown<<",hup:"<<(*fv)[i].hup<<",hm:"<<(*fv)[i].hm<<",ksa:"<<(*fv)[i].ksa<<endl<<endl;

		//更新非饱和网格
		(*fv)[i].UpdateRichardsUnit();
	}
}


//沟道地下水计算
void HighSlopeRunoff::BellowChannel(float dt)
{
	//前面思路：岩石水(土壤水)t+1时刻状态由土壤水(岩石水)t时刻决定，现在处理沟道地下水运动，使用土壤水t+1时刻的值
	float BchannelW=0;
	
	//wu:dt时间内上游补给的水量m3;wd:dt时间内流出本元流域的水量m3;wL,wR:左右坡面补给的水量m3,wm:与明渠补给m3
	float wL=0,wR=0,wu=0,wd=0,wm=0,wRock=0;
	int N1=min(int(FVMunit::TimeStart/this->MSTEP),Steps);
	int N2=min(int((FVMunit::TimeStart+dt)/this->MSTEP)+1,Steps);

	/**************左右坡面的补给量**************/
	float ksaL,ksaR,ksaS;
	ksaL=ksaR=ksaS=FVMunit::fc[0];//此时坡面上都是雪，所以取fc[0]

	if(fvL.size()>0)
	{
		ksaL=fvL[fvL.size()-1].ksa;
		wL=cos(fvL[fvL.size()-1].beta2)*ksaL*(fvL[fvL.size()-2].hm-fvL[fvL.size()-1].hm)/(0.5*(fvL[fvL.size()-2].dx2+fvL[fvL.size()-1].dx2))*dt*myPara->StreamLength*(fvL[fvL.size()-1].hm-KK[1]);
	}

	if(fvR.size()>0)
	{
		ksaR=fvR[fvR.size()-1].ksa;
		wR=cos(fvR[fvR.size()-1].beta2)*ksaR*(fvR[fvR.size()-2].hm-fvR[fvR.size()-1].hm)/(0.5*(fvR[fvR.size()-2].dx2+fvR[fvR.size()-1].dx2))*dt*myPara->StreamLength*(fvR[fvR.size()-1].hm-LL[1]);
	}

	if(fvS.size()>0)
	{
		ksaS=fvS[fvS.size()-1].ksa;
	}
	
	/*if(!_finite(wL) || !_finite(wR))
	{
		int x=0;
	}*/

	/************上游支流河道的补给量************/
	if(myPara->AreaS>1e-2 && fvS.size()>1)
	{
		//源坡面补给
		wu=cos(fvS[fvS.size()-1].beta2)*ksaS*(fvS[fvS.size()-2].hm-fvS[fvS.size()-1].hm)/(0.5*(fvS[fvS.size()-2].dx2+fvS[fvS.size()-1].dx2))*dt*gh*(fvS[fvS.size()-1].hm-(LL[1]+KK[1])/2);
	}
	else if(myPara->AreaS>1e-2 && fvS.size()<=1)
	{
		//融雪补给情况，现在不处理
		wu=0;
	}
	else
	{
		//上游地下水来流补给
		for(int i=N1;i<=N2;i++)
		{
			wu+=(Qd1[i]+Qd2[i])*MSTEP;
		}
	}

	/***************河道出口断面渗出量************/
	double Kchannel=0.5*(ksaL+ksaR);//注意,这里用两侧坡面最后单元渗透系数的平均值作为河道渗透系数

	float sinT=(myPara->StreamSlope/sqrt(1+myPara->StreamSlope*myPara->StreamSlope));//水头比降为河道坡度正弦sinx=tanx/sqrt(1+tanx^2);
	
	wd=Kchannel*sinT*dt*(Bchannel.HtoW(Bchannel.H)/myPara->StreamLength);
	wd=max(wd,0);

	if(!_finite(wd))
	{
		if(debug)
		{
			myFile<<"warning:(index,value,length):("<<mBSCode.RegionIndex<<","<<mBSCode.Value<<","<<mBSCode.Length<<"):"<<"wd is INF"<<endl;
			myFile<<"Kchannel:"<<Kchannel<<",Bchannel.H:"<<Bchannel.H<<",gm:"<<gm<<endl;
		}
		wd=0;
	}

	for(int i=N1;i<=N2;i++)
	{
		Qdout[i]=wd/dt;//出口地下径流过程

		if(Qdout[i]>1000)
		{
			int xxx=0;
		}
	}

	//cout<<"N1:"<<N1<<",N2:"<<N2<<",wd:"<<wd<<",dt(h):"<<dt/3600<<",wd/dt:"<<wd/dt<<",gm:"<<gm<<",BH:"<<Bchannel.H<<",BW:"<<Bchannel.W<<endl;

	//与岩石水交互,SD为岩石水和沟道地下水的交互面积
	const float SD=KL*myPara->StreamLength;
	
	if(Bchannel.H<Rock.H)//岩石水补充地下水
	{
		wRock=Kchannel*sinT*dt*SD;
	}
	else//地下水补充岩石水
	{
		wRock=-Rock.Kr*dt*SD;
	}

	Rock.WtoH(-wRock);//岩石水水位调整

	/**************与明渠水之间的交互量***********/
	const float GE=gm/sin(theta1),FH=gm/sin(theta2);
	float ratio=1.0;
	
	//明渠水位高于地下水位,这里得到的wm肯定是正的
	if(Ichannel.H>=Bchannel.H)
	{
		//地下水低于EF线
		if(Bchannel.H<=Bchannel.E[1])
		{
			if(Ichannel.H>=0)
			{ 
				ratio=1;
			}
			else
			{ 
				ratio=(gm-abs(Ichannel.H))/gm;
			} 

			//注意明渠没水就不能继续往下渗了
			wm=min(Ichannel.H+gm,Kchannel*dt)*myPara->StreamLength* (ratio*(GE*cos(theta1)+FH*cos(theta2)) + (Bchannel.F[0]-Bchannel.E[0]));
		}
		//地下水高于EF线
		else
		{
			if(Ichannel.H>=0) 
			{ 
				ratio=Bchannel.H/gm;
			}
			else
			{ 
				ratio=(Ichannel.H-Bchannel.H)/gm;
			} 
			
			wm=min(Ichannel.H-Bchannel.H,Kchannel*dt)*ratio*myPara->StreamLength*(GE*cos(theta1)+FH*cos(theta2));
		}
	}
	//地下水位高于明渠水位,这里得到的wm肯定是负的
	else
	{
		//参见“完全自流井”内容
		float xEL,xFR,QL,QR;
		xEL=Ichannel.H*(Bchannel.E[0]-Bchannel.G[0])/Bchannel.E[1]+Bchannel.G[0];
		xFR=Ichannel.H*(Bchannel.F[0]-Bchannel.HH[0])/Bchannel.F[1]+Bchannel.HH[0];
		QL=Kchannel*myPara->StreamLength/2/(xEL-Bchannel.G[0])*(pow(Bchannel.H-Bchannel.E[1],2)-pow(Ichannel.H-Bchannel.E[1],2));
		QR=Kchannel*myPara->StreamLength/2/(Bchannel.HH[0]-xFR)*(pow(Bchannel.H-Bchannel.F[1],2)-pow(Ichannel.H-Bchannel.F[1],2));
		wm=-(QL+QR)*dt;

		if(!_finite(wm))
		{
			if(debug)
			{
				myFile<<"warning:(index,value,length):("<<mBSCode.RegionIndex<<","<<mBSCode.Value<<","<<mBSCode.Length<<"):"<<"wm is INF"<<endl;
			}
			wm=0;
		}
	}

	//明渠水调整
	//Ichannel.TimeEnd=FVMunit::TimeStart+dt;
	Ichannel.q=-wm/dt/myPara->StreamLength;//m2/s

	//得到进入沟道地下的水量（可正可负）
	BchannelW=wL+wR+wu-wd+wm+wRock;

	if(!_finite(BchannelW))
	{
		if(debug)
		{
			myFile<<"warning:(index,value,length):("<<mBSCode.RegionIndex<<","<<mBSCode.Value<<","<<mBSCode.Length<<"):"<<"BchannelW is INF"<<endl;
		}
		BchannelW=0;
	}
	
	//得到填充满沟道地下部分后又流入明渠的水量Wout，并更新沟道地下水水位
	float Wout=Bchannel.WtoH(BchannelW);

	//这样明渠水得到的补充就不完全靠"自流井"的那可怜的一点了。
	if(Wout>1-4)
	{
		Ichannel.q+=(Wout/dt/myPara->StreamLength);//m2/s
	}

}


