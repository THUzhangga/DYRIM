#include "HighSlopeRunoff.h"
#include <cmath>
#include "functions.h"
#include <algorithm>

long HighSlopeRunoff::NumofHours=1; long HighSlopeRunoff::HourStart=438288; float HighSlopeRunoff::E=0.0f;      
float HighSlopeRunoff::P=0.0f;      bool HighSlopeRunoff::debug=true;  
Para* HighSlopeRunoff::myPara;      BSCode HighSlopeRunoff::mBSCode; WaterInRockSnow HighSlopeRunoff::Rock;

//����pair�ڶ���Ԫ�شӴ�С����
bool compare_1(pair<int,int> i1,pair<int,int> i2)
{
	return (i1.second>i2.second)?true:false;
}

HighSlopeRunoff::HighSlopeRunoff(void)
:theta1(PI/4),theta2(PI/4)
{
}

//���кӶμ�����Ϻ��ͷţ���Initialize���Ӧ
HighSlopeRunoff::~HighSlopeRunoff(void)
{
	if(debug){ myFile.close();}
}

//�Ӽ�����̴��룬�����кӶεļ���ȫ�ֲ��䣬�͹��캯��������ͬ
void HighSlopeRunoff::Initiallize(ADODB::_ConnectionPtr pCnn0,CString RainType0,long NumofHours0,long HourStart0,long StatusTime0,float BasinArea0,int MSTEP0,int Steps0)
{
	/**************************************����hydrousepara���ϵͳ����**************************************/
	pCnn=pCnn0;
	Steps=Steps0;
	
	//��������������
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
	
	/****************************************����HSPELEVATION��Ĳ���***************************************/
	CString cSQL;
	ADODB::_RecordsetPtr pRst;
	pRst.CreateInstance(__uuidof(ADODB::Recordset));
	pRst->CursorLocation = ADODB::adUseClient;
	
	//�߳�С��0ʱ��Ϊ�����ڸ����ͣ��̴߳Ӹߵ�������
	cSQL.Format("select * from hspelevation where elevation>=0 order by elevation desc");

	try
	{
		int Landtype,Elevation,i=0;
		pRst->Open(cSQL.GetString(),(ADODB::_Connection*)pCnn,ADODB::adOpenStatic,ADODB::adLockOptimistic,ADODB::adCmdText);
		while(!pRst->EndOfFile)
		{
			i++;
			Landtype=pRst->Fields->Item["landtype"]->Value;//0:ѩ,1:��,2:��,3:��,4:��

			if(Landtype>4 || Landtype<0)
			{
				cout<<"wrong landtype value in table hspelevation."<<endl;
				return;
			}

			Elevation=pRst->Fields->Item["elevation"]->Value;//���е�λ��m
			
			//��֤�߳��ϸ�Ӵ�С�������������
			if(i==1)
			{ 
				lv.push_back(make_pair(Landtype,Elevation));
				FVMunit::dc[Landtype]=pRst->Fields->Item["dc"]->Value;//ֲ��������������(0,1)

				FVMunit::fc[Landtype]=pRst->Fields->Item["fc"]->Value;//���е�λ��m/d
				FVMunit::fc[Landtype]=FVMunit::fc[Landtype]/24/3600;//ת��Ϊm/s

				FVMunit::thetai[Landtype]=pRst->Fields->Item["thetai"]->Value;//��ʼ������ˮ��(0,1)
			}
			else if(lv[lv.size()-1].second!=Elevation)
			{ 
				lv.push_back(make_pair(Landtype,Elevation));
				FVMunit::dc[Landtype]=pRst->Fields->Item["dc"]->Value;//ֲ��������������(0,1)

				FVMunit::fc[Landtype]=pRst->Fields->Item["fc"]->Value;//���е�λ��m/d
				FVMunit::fc[Landtype]=FVMunit::fc[Landtype]/24/3600;//ת��Ϊm/s

				FVMunit::thetai[Landtype]=pRst->Fields->Item["thetai"]->Value;//��ʼ������ˮ��(0,1)
			}

			pRst->MoveNext();
		}
		pRst->Close();

		//�����Ǳ�֤��߸̵߳�һ����ѩ����ʹ���Ͳ���ѩ������ѩ�����ͽ�������ֹ�������
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
	/*****************************************����HSPELEVATION�����****************************************/
	/*****************************************����HSPUSEPARA��Ĳ���****************************************/
	_variant_t tmp;
	CString temp;
	cSQL.Format("select * from hspusepara");
	try
	{
		pRst->Open(cSQL.GetString(),(ADODB::_Connection*)pCnn,ADODB::adOpenStatic,ADODB::adLockOptimistic,ADODB::adCmdText);
		if(!pRst->EndOfFile)
		{
			Min.theta1=pRst->Fields->Item["thetal"]->Value;//���е�λ����
			Min.theta1=Min.theta1/180*PI;//ת��Ϊ����
			Min.theta2=pRst->Fields->Item["thetar"]->Value;//���е�λ����
			Min.theta2=Min.theta2/180*PI;//ת��Ϊ����

			Min.B0=pRst->Fields->Item["width"]->Value;//���е�λ��m
			Min.Q0=pRst->Fields->Item["discharge"]->Value;//���е�λ��m3/s

			Min.Emax=pRst->Fields->Item["Emax"]->Value;//���е�λ��mm
			Min.Emax/=1000;//ת��Ϊm
			Min.Emin=pRst->Fields->Item["Emin"]->Value;//���е�λ��mm
			Min.Emin/=1000;//ת��Ϊm

			Min.sDeep=pRst->Fields->Item["erosionmax"]->Value;//���е�λ��m
			
			//BSTR��ת��Ϊstring�ͣ�û�鵽��ôֱ��ת����ת����CString����ת��string
			tmp = pRst->Fields->Item["elevationtype"]->Value;
			temp.Format((LPCTSTR)(_bstr_t)tmp.bstrVal);
			//CStringת����string
			Min.hType=temp.GetBuffer();
			temp.ReleaseBuffer();

			tmp=pRst->Fields->Item["slopeshape"]->Value;//"par" or "rec" or "tri"
			temp.Format((LPCTSTR)(_bstr_t)tmp.bstrVal);
			Min.sShape=temp.GetBuffer();
			temp.ReleaseBuffer();

			Min.Kr=pRst->Fields->Item["KRock"]->Value;//���е�λ��m/d
			Min.Kr=Min.Kr/24/3600;//ת��Ϊm/s

			Min.mu=pRst->Fields->Item["MuRock"]->Value;

			Min.dt1=pRst->Fields->Item["dt1"]->Value;//���е�λ��min
			Min.dt1*=60;//ת��Ϊs
			Min.dt2=pRst->Fields->Item["dt2"]->Value;//���е�λ��min
			Min.dt2*=60;//ת��Ϊs

			Min.dx=pRst->Fields->Item["dx"]->Value;//���е�λ��m
			Min.dz=pRst->Fields->Item["dz"]->Value;//���е�λ��m

			Min.sRatio=pRst->Fields->Item["sRatio"]->Value;//��Բ�͸ˮ�����
			Min.wRatio=pRst->Fields->Item["wRatio"]->Value;//��������ˮ����
			Min.rRatio=pRst->Fields->Item["rRatio"]->Value;//��ʯ��϶ˮ����

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
	
	//����ʱ�䲽��
	if(Min.dt1>3600  || Min.dt1<=0) { Min.dt1=3600; }
	if(Min.dt2>3600  || Min.dt2<=0) { Min.dt2=3600;}
	if(Min.dt2<Min.dt1) { Min.dt2=Min.dt1; }//��֤��������ʱ�䲽���������沿��
	FVMunit::dt1=Min.dt1;  
	FVMunit::dt2=Min.dt2;

	if(Min.Emax<0) { Min.Emax=0.1;}
	if(Min.Emin<0) { Min.Emax=0.05;}
	FVMunit::Emax=Min.Emax;
	FVMunit::Emin=Min.Emin;
	/*****************************************����HSPUSEPARA��Ĳ���****************************************/

	if(debug)
	{
		//trunc:�ǽ��ļ�������Ϊ0,app����׷�ӷ�ʽ��д�ļ�
		myFile.open("Out.txt",ios_base::out | ios_base::trunc);
	}
 
}


//ÿ����һ���Ӷξͳ�ʼ��һ��
void HighSlopeRunoff::InitializeOneRiver(BSCode mBSCode0,Para* myPara0,float* Qu10,float* Qu20,float* Qd10,float* Qd20,float* Quout0,float* Qdout0,float* FlowB0,float* FlowH0,float* FlowV0)
{
	xL=axL=fL=afL=aL=bL=cL=dL=NULL;
	xR=axR=fR=afR=aR=bR=cR=dR=NULL;
	xS=axS=fS=afS=aS=bS=cS=dS=NULL;

	Quout=Quout0;//��ǰ�Ӷ�"����ˮ"��������   
	Qdout=Qdout0;//��ǰ�Ӷ�"����ˮ"��������
	Qu1=Qu10;//����֧��1����ˮ       
	Qu2=Qu20;//����֧��2����ˮ
	Qd1=Qd10;//����֧��1����ˮ       
	Qd2=Qd20;//����֧��2����ˮ
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

	/*****************************************����HSPPARAMETER��Ĳ���****************************************/
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
			
			FVMunit::usD=pRst->Fields->Item["usD"]->Value;//���е�λ��cm2/min
			FVMunit::usD=FVMunit::usD*1e-4/60;//ת��Ϊm2/s

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
	/*****************************************����HSPPARAMETER��Ĳ���****************************************/

	//��ù����ļ��β���
	this->RiverChannel();
	
	//������ҡ�Դ�����������ɢ��Ԫ
	this->MeshRegion();

}

//ÿ�������һ���Ӷξ��ͷ�һ��
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



//����Ԫ����Ĳ�������
void HighSlopeRunoff::Calc(BSCode mBSCode0,Para* myPara0,float* Qu10,float* Qu20,float* Qd10,float* Qd20,float* Quout0,float* Qdout0,float* FlowB0,float* FlowH0,float* FlowV0)
{
	//��ڵ㲻�ò������㣬ֱ�ӽ�����������ӷ���
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
	
	//Ϊ����д�������������ͣ�ʹ�ø�������ʹ��ԭ������Ч��һ��
	float &TimeStart=FVMunit::TimeStart;//s
	TimeStart=0;

	//��¼ǰһʱ�̵�TimeStart
	float TimeStartBefore=0;
	
	//�����ڵĳ���ʱ�䲽��
	float &dt1=FVMunit::dt1;//s
	float &dt2=FVMunit::dt2;//s
	
	//����ˮ�ڷǽ������˶��ı�ʱ�䲽��s
	float vdt=0;

	//һ��ʱ�䲽���ڵĽ�����m
	float StepRain=0.0;

	//�������ڽ���֮���ʱ��s
	double TimeNoRain=0.0;
	
	//����������ˮ�����ʱ�䲽��s,����Ĭ��1��
	const float NoRainStep=1*24*3600;

	float YearS=FVMunit::StartYear,MonthS=FVMunit::StartMonth,DayS=FVMunit::StartDay,HourS=FVMunit::StartHour;
	float YearE,MonthE,DayE,HourE;
	const float RegionS=max(1e-2,myPara->AreaL)+max(1e-2,myPara->AreaR)+max(1e-2,myPara->AreaS);//Ԫ����ͶӰ���

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

		//�����ڵ�ˮ���˶�
		if(StepRain<=1e-5 && TimeNoRain>1e-4)
		{
			while(TimeNoRain>0)
			{
				float iTime=TimeStart;
				float Tmp=0;

				//�����1��ʾʱ�䲽��Ϊ1�죬������Ҫ���Ըı�
				if(TimeNoRain<=NoRainStep) 
				{ 
					vdt=TimeNoRain;
					TimeNoRain=-1;//����������0��0�ıȽϣ�������ܴ����⣬������ѭ����������︳ֵ-1
				}
				else
				{
					vdt=NoRainStep;
					TimeNoRain-=vdt;

					//����ʣ�µĻ�������0.5��,�������ֱ�Ӽӵ��ϸ�ʱ�䲽����
					if(TimeNoRain<0.5*vdt) 
					{ 
						vdt+=TimeNoRain;
						TimeNoRain=-1;
					}
				}

				//�Ҳ������ڸ������������⣬�����ڿ��ܻ�õ���С����������һ���жϣ�ֻ�д���10���Ӳ���
				if(vdt>600)
				{
					//��������ˮ����ʯ��϶ˮ�˶�
					if(myPara->AreaL>0.01 && fvL.size()>0){ this->SaturatedSoil("L",vdt);}
					if(myPara->AreaR>0.01 && fvR.size()>0){ this->SaturatedSoil("R",vdt);}
					if(myPara->AreaS>0.01 && fvS.size()>0){ this->SaturatedSoil("S",vdt);}

					//��������ˮ�˶�
					this->BellowChannel(vdt);

					//���������ˮ��dt1��ʱ�䲽���˶�
					while(Tmp<vdt)
					{
						//��������ֲ��ˮ�˶�
						if(myPara->AreaL>0.01 && fvL.size()>0){ this->SlopeRunoff("L",0,dt1);}
						if(myPara->AreaR>0.01 && fvR.size()>0){ this->SlopeRunoff("R",0,dt1);}
						if(myPara->AreaS>0.01 && fvS.size()>0){ this->SlopeRunoff("S",0,dt1);}

						//������ˮ�ݽ�
						Ichannel.InChannel();

						TimeStart+=dt1;
						Tmp+=dt1;
					}
					TimeStart=iTime;
					Tmp=0;

					//�Ǳ�������ˮ�˶�
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

			//��������ˮ����ʯ��϶ˮ�˶�
			if(myPara->AreaL>0.01 && fvL.size()>0){ this->SaturatedSoil("L",dt2);}
			if(myPara->AreaR>0.01 && fvR.size()>0){ this->SaturatedSoil("R",dt2);}
			if(myPara->AreaS>0.01 && fvS.size()>0){ this->SaturatedSoil("S",dt2);}

			//��������ˮ�˶�
			this->BellowChannel(dt2);

			//���������ˮ��dt1��ʱ�䲽���˶�
			while(Tmp<dt2)
			{
				//��������ֲ��ˮ�˶�
				if(myPara->AreaL>0.01 && fvL.size()>0){ this->SlopeRunoff("L",StepRain/dt2*dt1,dt1);}
				if(myPara->AreaR>0.01 && fvR.size()>0){ this->SlopeRunoff("R",StepRain/dt2*dt1,dt1);}
				if(myPara->AreaS>0.01 && fvS.size()>0){ this->SlopeRunoff("S",StepRain/dt2*dt1,dt1);}

				//������ˮ�ݽ�
				Ichannel.InChannel();

				TimeStart+=dt1;
				Tmp+=dt1;
			}
			TimeStart=iTime;
			Tmp=0;

			//�Ǳ�������ˮ�˶�
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

		}//end else(����ʱ)

		//��Ϊʱ�䶯����������ֻҪ����������֮���ʱ������һ��StatusTime���ڣ�����¼E��P
		if(debug && TimeStart>TimeStartBefore+StatusTime*3600)
		{
			TimeStartBefore=TimeStart;

			YearE=FVMunit::Year;
			MonthE=FVMunit::Month;
			DayE=FVMunit::Day;  
			HourE=int(FVMunit::Hour+0.5);

			myFile<<YearS<<"."<<MonthS<<"."<<DayS<<" "<<HourS<<"h-"<<YearE<<"."<<MonthE<<"."<<DayE<<" "<<HourE<<"h,Ԫ����ƽ������:"<<E/RegionS*1000<<"(mm),����"<<P/RegionS*1000<<"(mm)"<<endl;

			YearS=YearE;
			MonthS=MonthE;
			DayS=DayE;
			HourS=HourE;

			E=P=0;
		}
	}//end while(�ܼ���ʱ��ѭ��)

	this->FinalizeOneRiver();	
}


//����ȫ�������ƽ̲���������Ԫ����Ĺ������β���
void HighSlopeRunoff::RiverChannel(void)
{
	//��ʼֵ
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
			myFile<<"�Ӷ����ζ�����½�theta1Ҫ��0�㵽90��֮�䡣"<<endl;
			myFile<<"theta1����ʼ��Ϊ"<<45<<"��"<<endl;
		}
	}
	if(theta2<0 || theta2>PI/2 || theta2==NULL)
	{
		theta2 = PI/4;

		if(debug)
		{
			myFile<<"�Ӷ����ζ�����½�theta2Ҫ��0�㵽90��֮�䡣"<<endl;
			myFile<<"theta2����ʼ��Ϊ"<<45<<"��"<<endl;
		}
	}
	if(S<=0){ return;}
	
	//��Ϊ�ӿ���֮��=ƽ̲����֮��=��ˮ���֮��
	if(Min.B0<0 ||Min.B0==NULL || Min.B0>5000)
	{
		Min.B0 = 100;//m
	}
	this->gh = Min.B0*sqrt(S/this->BasinArea);
	if(gh<0.2) { gh=0.2;}//��С�ӿ��20cm

	if(Min.Q0<0 ||Min.Q0==NULL || Min.Q0>200000)
	{
		Min.Q0 = 5000;//m3/s
	}
	//sqrt(0)��õ�δ֪�Ľ����ʵ�����ѵõ����飬������Сȡ5%oo
	if(myPara->StreamSlope<1e-6) { myPara->StreamSlope=5e-4;}
	const float Q1 = Min.Q0*S/this->BasinArea*myPara->RiverManning/sqrt(myPara->StreamSlope);
	const float c1=1/tan(theta1)+1/tan(theta2);
	const float c2 = 1/sin(theta1)+1/sin(theta2);
	float fx=1,dfx;
	int t=0;//��������
	while(abs(fx)>1e-2)
	{
		t++;
		if(t>100) { break;}
		fx=pow(0.5*gm*(2*gh-gm*c1),5/3)*pow(gm*(c2-c1)+gh,-2/3)-Q1;
		dfx=5/3*(gh-c1*gm)*pow(gh*gm-0.5*c1*gm*gm,2/3)*pow((c2-c1)*gm+gh,-2/3)-2/3*(c2-c1)*pow(gh*gm-0.5*c1*gm*gm,5/3)*pow((c2-c1)*gm+gh,-5/3);
		gm=gm-fx/dfx;
	}
	//�����ζ���,gh���ֵ
	if(gm>gh/c1)
	{
		gm=gh/c1;
	}
	//������С0.1��ˮ��
	if(gm<0.1)
	{
		gm=max(gh/c1,0.1);//��С0.1����
	}
	ef = gh-gm*c1;

	if(debug)
	{
		myFile<<"RegionIndex:"<<mBSCode.RegionIndex<<",BsValue:"<<mBSCode.Value<<",BsLength:"<<mBSCode.Length<<endl;
		myFile<<"�����ߴ�:"<<"�����:"<<gh<<"m,"<<"������:"<<gm<<"m,"<<"�ӵ׿�:"<<ef<<"m"<<endl;
	}
}


//����Ԫ�����������֮����û�м�һ���ж���������(L,R,S)����ڲ�������Ϊģ��ͨ͸��(�����ʴ�뾶��)
//��Ҫ��ͳһ����ϵ���������
void HighSlopeRunoff::MeshRegion(void)
{
	////////////////////////////////////////////////////////////////////////////////////////
	///////////////////////////////����������״��������ͶӰ��///////////////////////////////
	////////////////////////////////////////////////////////////////////////////////////////
	float Bst;//Դ����������εױ߳�
	::transform(Min.sShape.begin(),Min.sShape.end(),Min.sShape.begin(),::tolower);
	::transform(Min.hType.begin(),Min.hType.end(),Min.hType.begin(),::tolower);

	if(myPara->StreamLength>0)
	{
		//�����Ƽ���������״����ʵ�ʱȽ�����������������ʵ���ձ�ƫС�������ձ�ƫ��
		//lengthL���������ĵ��ӵ���ˮƽ���룬�ڱ�֤����ͶӰ������������£�lengthLֻ�ܱ�����
		//ע�⣺���¾����������LengthL��LengthR��LengthS���Ѿ���Ϊȫ���泤ˮƽ��ͶӰ�����������Ĵ����ˡ�
		
		//�����߼���
		if(Min.sShape=="par")
		{
			myPara->LengthL=3*myPara->AreaL/2/myPara->StreamLength;
			myPara->LengthR=3*myPara->AreaR/2/myPara->StreamLength;
		}

		//�����μ���
		else if(Min.sShape=="tri")
		{
			myPara->LengthL=2*myPara->AreaL/myPara->StreamLength;
			myPara->LengthR=2*myPara->AreaR/myPara->StreamLength;
		}

		//���μ���
		else if(Min.sShape=="rec")
		{
			myPara->LengthL=myPara->AreaL/myPara->StreamLength;
			myPara->LengthR=myPara->AreaR/myPara->StreamLength;
		}
		else//���������ߴ���
		{
			myPara->LengthL=3*myPara->AreaL/2/myPara->StreamLength;
			myPara->LengthR=3*myPara->AreaR/2/myPara->StreamLength;
		}
	}
	//Դ���水�յ����������������������������Ϊ�����δ���Ļ�����Ȼ������������
	if(myPara->LengthS>0)
	{
		myPara->LengthS=myPara->LengthS*3/2;
		Bst=2*myPara->AreaS/myPara->LengthS;
	}
	//---------------------------------------END------------------------------------------//

	////////////////////////////////////////////////////////////////////////////////////////
	///////////////////////////////���������ٽ�߳��йز���///////////////////////////////
	////////////////////////////////////////////////////////////////////////////////////////
	//��ʼ��Ϊ�½Ŵ��ġ���ԡ��߳�
	int hSlopeDownLR=0.0;
	int hSlopeDownS=0.0;

	//��ʼ��Ϊ�¶����ġ���ԡ��̣߳�֮�󱣳ֲ��䣻�����ᵽ�ġ���ԡ�������GH��ΪX�ᡣ
	//��������if�Ǳ�֤�µ������Ը̲߳���0����Ϊint�ͣ��ģ������0����ͱ��һ������
	//����һ����С1.5�׸ߵ��¸�
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
		//ת��Ϊ�����ԡ��߳�
		hSlopeDownLR=0.5*(myPara->UElevation+myPara->DElevation);
		hSlopeDownS=myPara->UElevation;
	}

	//�ٽ�̴߳Ӹߵ�������
	//::stable_sort(lvL.begin(),lvL.end(),compare_1);

	//�ٽ�̸߳��Ƶ������桢�������Դ���棬����������������
	std::vector<pair<int,int>> lvL(lv.begin(),lv.end());
	std::vector<pair<int,int>> lvR(lv.begin(),lv.end());
	std::vector<pair<int,int>> lvS(lv.begin(),lv.end());
	std::vector<pair<int,int>>::iterator lpL=lvL.begin(),lpR=lvR.begin(),lpS=lvS.begin();

	int typeL=4,typeR=4,typeS=4;

	//���ٽ�߳�ת�����ˡ���ԡ��߳�
	for(lpL;lpL!=lvL.end();lpL++) 
	{ 
		lpL->second-=hSlopeDownLR;

		//��С��0�ĸ߳�ɾ���������Ը̡߳�ʱ���ܻ����С��0�������
		//��Ϊ�Ӵ�С���У�����һ������С��0������϶�ҲС��0
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

	/*******************************������������ֹ����Ը߳�*********************************/
	//��Ϊ�̴߳Ӹߵ������У���ʶɾ�����ڼ���(ָǰ����Щ���������㻹�ߵĸ߳���)
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

	//ɾ�����������㻹Ҫ�ߵ��ٽ�߳�
	if(eraseFlag>0)
	{
		//����ٽ�߳��鶼�����¶��̣߳�����������Ԫ�أ�����typeL
		if(eraseFlag==lvL.size())
		{
			lvL.push_back(make_pair(typeL,hSlopeUpL));
		}
		lvL.erase(lvL.begin(),lvL.begin()+eraseFlag);
	}

	//2009.9.11�����ȫ���涼��ѩ����ôeraseFlag����֮ǰ��lvL���Ѿ����ˣ��������ñ�֤lvL�����¶��
	if(lvL.size()==0)
	{
		lvL.push_back(make_pair(typeL,hSlopeUpL));
	}
	
	//���´�������½Ŵ��̵߳�
	for(lpL=lvL.begin();lpL!=lvL.end();lpL++)
	{
		//�����ifһ��Ҫ��
		if(lvL.size()==1)
		{
			lvL.push_back(make_pair(typeL,0));
			break;
		}
		
		//���Ԫ�ظ߳�Ϊ0���
		//wh: ��ס�������һ��Ԫ��ʹ��end()-1��������end(),���򱨴�end()ָ�����һ��Ԫ�ص���һ��Ԫ��
		if((lvL.end()-1)->second<=0)
		{
			(lvL.end()-1)->second=0;
			break;
		}
		
		//ĩԪ�ظ̴߳���0���
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
	/*******************************������������ֹ��߳����*********************************/

	/*******************************������������ֹ����Ը߳�*********************************/
	eraseFlag=0;
	for(lpR=lvR.begin();lpR!=lvR.end();lpR++)
	{
		//�������涥��߳�
		if(lpR->second>=hSlopeUpR){ eraseFlag++;}
		else
		{
			lvR.insert(lpR,make_pair(lpR->first,hSlopeUpR));
			break;
		}
	}

	//ɾ�����������㻹Ҫ�ߵ��ٽ�߳�
	if(eraseFlag>0)
	{
		if(eraseFlag==lvR.size())
		{
			lvR.push_back(make_pair(typeR,hSlopeUpR));
		}
		lvR.erase(lvR.begin(),lvR.begin()+eraseFlag);
	}

	//��ǰ���������˵��
	if(lvR.size()==0)
	{
		lvR.push_back(make_pair(typeR,hSlopeUpR));
	}

	for(lpR=lvR.begin();lpR!=lvR.end();lpR++)
	{
		//�����ifһ��Ҫ��
		if(lvR.size()==1)
		{
			lvR.push_back(make_pair(typeR,0));
			break;
		}

		//���Ԫ�ظ߳�Ϊ0���
		if((lvR.end()-1)->second<=0)
		{
			(lvR.end()-1)->second=0;
			break;
		}

		//ĩԪ�ظ̴߳���0���
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
	/*******************************������������ֹ��߳����*********************************/

	/*******************************����Դ������ֹ����Ը߳�*********************************/
	if(myPara->LengthS>0)
	{
		eraseFlag=0;
		for(lpS=lvS.begin();lpS!=lvS.end();lpS++)
		{
			//�������涥��߳�
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

		//��ǰ���������˵��
		if(lvS.size()==0)
		{
			lvS.push_back(make_pair(typeS,hSlopeUpS));
		}

		for(lpS=lvS.begin();lpS!=lvS.end();lpS++)
		{
			//�����ifһ��Ҫ��
			if(lvS.size()==1)
			{
				lvS.push_back(make_pair(typeS,0));
				break;
			}

			//���Ԫ�ظ߳�Ϊ0���
			if((lvS.end()-1)->second<=0)
			{
				(lvS.end()-1)->second=0;
				break;
			}

			//ĩԪ�ظ̴߳���0���
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
	/*******************************����Դ������ֹ��߳����*********************************/

	if(debug)
	{
		myFile<<"HSPELEVATION���ٽ�߳���Ŀ:"<<lv.size()<<endl;
		for(lpL=lvL.begin();lpL!=lvL.end();lpL++)
		{
			if(abs(lvL.begin()->second-hSlopeUpL)>0.1)
			{
				cout<<"warning:L��һ���ٽ�̲߳������涥�㡣"<<endl;
				return;
			}
			if((lvL.end()-1)->second!=0)
			{
				cout<<"warning:L���һ���ٽ�̲߳����½�0�̡߳�";
				return;
			}
		}

		for(lpR=lvR.begin();lpR!=lvR.end();lpR++)
		{
			if(abs(lvR.begin()->second-hSlopeUpR)>0.1)
			{
				cout<<"warning:R��һ���ٽ�̲߳������涥�㡣"<<endl;
				return;
			}
			if((lvR.end()-1)->second!=0)
			{
				cout<<"warning:R���һ���ٽ�̲߳����½�0�̡߳�";
				return;
			}
		}

		if(myPara->LengthS>0)
		{
			for(lpS=lvS.begin();lpS!=lvS.end();lpS++)
			{
				if(abs(lvS.begin()->second-hSlopeUpS)>0.1)
				{
					cout<<"warning:S��һ���ٽ�̲߳������涥�㡣"<<endl;
					return;
				}
				if((lvS.end()-1)->second!=0)
				{
					cout<<"warning:S���һ���ٽ�̲߳����½�0�̡߳�";
					return;
				}

			}//end for

		}//end if

	}//end if(debug)

	//---------------------------------------END------------------------------------------//


	////////////////////////////////////////////////////////////////////////////////////////
	///////////////////////////////����������͸ˮ���йز���///////////////////////////////
	////////////////////////////////////////////////////////////////////////////////////////
	
	//sitaAT��ֱ��ʵ�ʵ���б��[0,PI]�����µı���������"picture"�еı�ʶһ��
	
	//������
	float A[2],T[2],O[2],sitaAT;

	//������
	float D[2],T_1[2],Q[2],sitaDT_1;

	//Դ����
	float AS[2],TS[2],OS[2],sitaATS;
	
	//����
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
	
	//A���D���ǡ����ꡱ��ѩ�������أ��ٽ�̱߳�����ߣ������Ǽ�������ѩ�߽硣
	A[0]=max((O[1]-(lv[0].second-hSlopeDownLR))/myPara->SlopeL,0);
	A[1]=min(lv[0].second-hSlopeDownLR,O[1]);

	D[0]=Q[0]-max((Q[1]-(lv[0].second-hSlopeDownLR))/myPara->SlopeR,0);
	D[1]=min(lv[0].second-hSlopeDownLR,Q[1]);

	if(Min.sRatio<=0 || Min.sRatio>1 || Min.sRatio==NULL)
	{
		//Min.sRatio������
		Min.sRatio=min((G[0]-A[0])/G[0],(D[0]-H[0])/(Q[0]-H[0]));
	}
	else
	{
		Min.sRatio=min(Min.sRatio,min((G[0]-A[0])/G[0],(D[0]-H[0])/(Q[0]-H[0])));
	}

	T[0]=myPara->LengthL*(1-Min.sRatio);
	T[1]=0;

	//�õ�sitaAT������(��Сֵ)
	if(T[0]<=A[0]+1e-4)
	{ 
		sitaAT=PI/2; 
		T[0]=A[0];
	}

	//�õ�sitaAT������ֵ
	else
	{
		sitaAT=PI+atan2(T[1]-A[1],T[0]-A[0]); 
	}
	
	//�õ�sitaAT������
	if(tan(theta1)>myPara->SlopeL)
	{
		float sitaATU=PI+atan2(E[1]-A[1],E[0]-A[0]);
		sitaAT=min(sitaAT,sitaATU);

		if(abs(sitaAT-sitaATU)<1e-4)
		{
			T[0]=-E[1]*(E[0]-A[0])/(E[1]-A[1])+E[0];
		}
	}
	
	//ȷ�������治͸ˮ����б�Ƕ�
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
    //�õ�sitaDT_1������
	if(tan(theta2)>myPara->SlopeR)
	{
		float sitaDT_1U=atan2(D[1]-F[1],D[0]-F[0]);
		sitaDT_1=max(sitaDT_1,sitaDT_1U);

		if(abs(sitaDT_1-sitaDT_1U)<1e-4)
		{
			T_1[0]=-F[1]*(F[0]-D[0])/(F[1]-D[1])+F[0];
		}
	}

	//EF��������10cm������
	float sDeep=max(gm+0.1,Min.sDeep);
	
	//Min->sDeep����ȫ���������ʴ���,ͬ������������ϵ��ȫ�������,��Ϊ֧����ʴ���С��Խ��ƽԭ��Խ��
	//0.35���س̺����ϵ����Ĺ�ϵ(0.3,0.4)֮�䣬���ﲻһ�����ʣ�������ôȡ
	//sDeep*=pow(float(myPara->DrainingArea/this->BasinArea),0.35f);

	//ȷ��Դ���治͸ˮ����б�Ƕ�
	//����Դ���湵��KL��ƽ��,��Ϊû��Բ��,��AT��GK�ཻ,���ճ�ʼ��sRatio,����AKȷ��T
	if(myPara->LengthS>0)
	{
		KS[0]=myPara->LengthS;
		KS[1]=-(gm+sDeep);

		AS[0]=max((OS[1]-(lv[0].second-hSlopeDownS))/myPara->SlopeS,0);
		AS[1]=min(lv[0].second-hSlopeDownS,OS[1]);

		TS[0]=myPara->LengthS*(1-Min.sRatio);
		TS[1]=0;

		//�õ�ֱ��AK��X�ύ�������
		//�����if��ȫ���潵ѩ�����
		if(KS[0]<AS[0]+1e-4)
		{
			TS[0]=KS[0];
			sitaATS=PI-atan(myPara->SlopeS);
		}
		else
		{
			//AK��x�ύ��ĺ����꣬��ʱ������KS[1]=-(gm+sDeep)�����ƣ���ȷ��sRatio
			float XS;
			XS=KS[0]-KS[1]*(KS[0]-AS[0])/(KS[1]-AS[1]);

			//ȷ��Դ���治͸ˮ�����б��
			//���������TS[0]>A[0]����ΪA���ǿ��Ժܿ����½ŵģ�����ȫ�����ѩ��ҪС�ģ���ʱT��A�����
			//ϣ���ﵽһ��Ч����ֻҪ�����ϲ�ȫ��ѩ���͸������ɸ����㵥Ԫ����˵ñ�֤sitaATSΪ�۽�
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
	///////////////////////////////��������Բ�������йز���///////////////////////////////
	////////////////////////////////////////////////////////////////////////////////////////
	//��ʴ��Բ������
	float Center[2];
	
	//�е�����
	float B[2],C[2]={0,0};
	
	//����������Բ�͸ˮ�潻������
	float DD[2];
	
	//a:�����治͸ˮ����x�Ḻ��н�(�϶�Ϊ���);
	//b:�����治͸ˮ����x������н�(�϶�Ϊ���);
	//K:�����ʴ���(��X���������𣬿϶�Ϊ��);
	float a,b,K,R=0;

	a=PI-sitaAT;
	b=sitaDT_1;
	K=gm+sDeep;

	/********************************Ϊ�˵õ�K������********************************/
	//�������������߽���һ������
	if(abs(a-PI/2)<1e-4 && abs(b-PI/2)<1e-4)
	{
		//��ʾ����Զ
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
	//-DD[1]��K�����ֵ
	if(K>=-DD[1])
	{
		K=gm+(-DD[1]-gm)*(1-1e-6);
	}
	/********************************K������������********************************/


	/*********************���Բ�ĺ��е����꣬�����Ӱ뾶�����ж�********************/
	//E,F��Բ��C�ľ���ƽ��
	//float EC=1,FC=1;

	//�жϵ�����
	/*if(mBSCode.Length==61 && mBSCode.RegionIndex==301 && mBSCode.Value==33)
	{
		int mm=0;
	}*/

	//�������Բ�����ꡢ�е�����ȣ���������Բ���뾶�������жϣ������С��Բ������GEFH�ཻ
	K-=0.05;
	while(1)
	{
		if(K>-DD[1]-1e-3) 
		{
			//if(debug){ cout<<"EC2:"<<EC2<<",FC2:"<<FC2<<",GC2:"<<GC2<<",HC2:"<<HC2<<",�����ʴ���"<<-DD[1]<<"�ף�������"<<endl;}
			K=gm+(-DD[1]-gm)*(1-1e-6);
			
			//Բ�κͲ�͸ˮ�㽻���غ�
			Center[0]=B[0]=C[0]=DD[0];
			Center[1]=B[1]=C[1]=DD[1];

			break; 
		}

		K+=0.05;//ÿ������5cm
		
		//������͸ˮ�Ƕ�Ϊ90��
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

		//�����治͸ˮ��Ϊ90��,�������90��
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

		//�����治͸ˮ��Ϊ90��,�������90��
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

		//һ�������
		else
		{
			//�������εĶ�������Բ��O(x,R-K),T(Tx,Ty),T_1(T_1x,T_1y),���е�B(x-R*sin(a),R-K-Rcos(a)),�е�C(x+Rsin(b),R-K-Rcos(b))
			//�ٽ�B��C�����������ֱ�߷���������⣬���ɵõ��뾶R��Բ������ȣ�ע�������a��bָ�ĵ�����ǡ�
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

		//��ʱEF�·���ֱ�ߣ�BC���Բ���ο϶�������EF�ཻ����Ϊһ���ཻEF�·��Ͳ����ܶ���ֱ��
		if(E[0]>=C[0] || F[0]<=B[0])
		{
			break;
		}
		
		//��ʱE���·���Բ��,F������ֱ�ߣ���֤E������������Բ������Ӧ��������
		if(E[0]>=B[0] && E[0]<=C[0] && F[0]>=C[0])
		{
			if(E[1]>=Center[1]-sqrt(R*R-(E[0]-Center[0])*(E[0]-Center[0])))
			{
				break;
			}
		}

		//��ʱF���·���Բ��,E������ֱ�ߣ���֤F������������Բ������Ӧ��������
		if(F[0]>=B[0] && F[0]<=C[0] && E[0]<=B[0])
		{
			if(F[1]>=Center[1]-sqrt(R*R-(F[0]-Center[0])*(F[0]-Center[0])))
			{
				break;
			}
		}

		//E,F�·�����Բ��
		if(F[0]>=B[0] && F[0]<=C[0] && E[0]>=B[0] && E[0]<=C[0])
		{
			if(F[1]>=Center[1]-sqrt(R*R-(F[0]-Center[0])*(F[0]-Center[0])) && E[1]>=Center[1]-sqrt(R*R-(E[0]-Center[0])*(E[0]-Center[0])))
			{
				break;
			}
		}
	}

	//����ȷ��ͼ��K���L�������
	KK[0]=G[0];
	LL[0]=H[0];

	//KK��������
	//�������治͸ˮ��ֱ����
	if(KK[0]>=C[0])
	{
		KK[1]=C[1]+(D[1]-C[1])/(D[0]-C[0])*(KK[0]-C[0]);
	}
	//�������治͸ˮ��ֱ����
	else if(KK[0]<=B[0])
	{
		KK[1]=B[1]+(A[1]-B[1])/(A[0]-B[0])*(KK[0]-B[0]);
	}
	//��Բ����
	else
	{
		KK[1]=Center[1]-sqrt(R*R-pow((KK[0]-Center[0]),2));
	}

	//LL��������
	//�������治͸ˮ��ֱ����
	if(LL[0]>=C[0])
	{
		LL[1]=C[1]+(D[1]-C[1])/(D[0]-C[0])*(LL[0]-C[0]);
	}
	//�������治͸ˮ��ֱ����
	else if(LL[0]<=B[0])
	{
		LL[1]=B[1]+(A[1]-B[1])/(A[0]-B[0])*(LL[0]-B[0]);
	}
	//��Բ����
	else
	{
		LL[1]=Center[1]-sqrt(R*R-pow((LL[0]-Center[0]),2));
	}

	//KK��LL�����������0����������ϲ�Ӧ�ó���
	if(KK[1]>0 && debug) 
	{
		myFile<<"KK������:"<<KK[0]<<","<<KK[1]<<endl;
		KK[1]=0;
	}
	if(LL[1]>0 && debug) 
	{
		myFile<<"LL������:"<<LL[0]<<","<<LL[1]<<endl;
		LL[1]=0;
	}

	//�������K���L��֮���ʵ�ʾ��루ֱ�߶κ�Բ���Σ�
	//K,L��������������治͸ˮ����
	if(KK[0]>=C[0] || LL[0]<=B[0])
	{
		KL=sqrt(pow(KK[0]-LL[0],2)+pow(KK[1]-LL[1],2));
	}

	//K,L����Բ������
	else if(KK[0]>=B[0] && LL[0]<=C[0])
	{
		KL=sqrt(pow(KK[0]-LL[0],2)+pow(KK[1]-LL[1],2));
		KL=2*asin(KL/2/R)*R;
	}

	//Բ���ζ���K��L֮�䣬ͬʱ�������������ֱ�߶�
	else if(KK[0]<=B[0] && LL[0]>=C[0])
	{
		KL=sqrt(pow(B[0]-C[0],2)+pow(B[1]-C[1],2));
		KL=2*asin(KL/2/R)*R;
		KL=KL+sqrt(pow(B[0]-KK[0],2)+pow(B[1]-KK[1],2))+sqrt(pow(LL[0]-C[0],2)+pow(LL[1]-C[1],2));
	}

	//K,B֮��Բ����C��L֮��ֱ��
	else if(KK[0]>=B[0] && KK[0]<=C[0] && LL[0]>=C[0])
	{
		KL=sqrt(pow(B[0]-KK[0],2)+pow(B[1]-KK[1],2));
		KL=2*asin(KL/2/R)*R;
		KL=KL+sqrt(pow(LL[0]-C[0],2)+pow(LL[1]-C[1],2));
	}

	//C��L֮��Բ����K��B֮��ֱ��
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

	/********************************Բ�ĺ��е�����������********************************/

	/**********************************��������ˮ��ʼ����**********************************/
	//������ȡƽ��������ǰKS[1]Ϊ��͵�������
	KS[1]=(KK[1]+LL[1]+KS[1])/3;

	if(Min.wRatio<=0) { Min.wRatio=1e-5;}
	if(Min.wRatio>=1) { Min.wRatio=0.95;}
	Bchannel.H=max(KK[1],LL[1])*(1-Min.wRatio);
	
	//c++����������ƺ�����ʵ�ִ�ֵ��ֻ�ܴ�ַ�����Ծ����´���ʽ�ˡ�
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
	
	//�õ���������ˮ�ܴ洢�����ˮ��m3
	Bchannel.Wmax=Bchannel.HtoW(0);
	
	if(debug)
	{
		//myFile<<"����ˮ�����׼��GH�·�"<<-Bchannel.H<<"��"<<endl;
		if(Bchannel.H>E[1])
		{
			myFile<<"����ˮ����Ӵ�EF�Ϸ���"<<Bchannel.H-E[1]<<"��"<<endl;
		}
		else
		{
			myFile<<"����ˮ����Ӵ�EF�·���"<<E[1]-Bchannel.H<<"��"<<endl;
		}
	}
	/**********************************��������ˮ��ʼ����********************************/

    /**********************************��ʯ��϶ˮ��ʼ����********************************/
	if(Min.rRatio>=1) { Min.rRatio=0.99;}
	if(Min.mu<0 || Min.mu>1) { Min.mu=0.02;}
	if(Min.Kr<0) { Min.Kr=1;}
	
	Rock.H=min(O[1],Q[1])*Min.rRatio;
	Rock.Initiallize(A[1],Min.Kr,Min.mu,myPara->StreamLength*(G[0]+H[0])/2,myPara->StreamLength*(Q[0]-(G[0]+H[0])/2),Bst*myPara->LengthS,&fvL,&fvR,&fvS);

	if(myPara->LengthS>0)
	{
		Rock.H=min(min(O[1],Q[1]),OS[1])*Min.rRatio;
	}
	/**********************************��ʯ��϶ˮ��ʼ����********************************/

	/**********************************��������ˮ��ʼ����********************************/
	
	Ichannel.Initiallize(this->myPara,MSTEP,Steps,gh,gm,ef,theta1,theta2,Qu1,Qu2,Quout,&fvL,&fvR,&fvS,FlowB,FlowH,FlowV,Min.SaveFlowPattern,&myFile);
	
	/**********************************��������ˮ��ʼ����********************************/

	////////////////////////////////////////////////////////////////////////////////////////
	///////////////////////////////�������ɼ����������ɢ��Ԫ///////////////////////////////
	////////////////////////////////////////////////////////////////////////////////////////
	if(Min.dx<0 || Min.dx==NULL)
	{
		Min.dx=1;
	}

	//��dx����Сֵ����һ������
	float tmp1;
	if(myPara->LengthS>0)
	{
		tmp1=min(myPara->LengthS,min(myPara->LengthL,myPara->LengthR));
	}
	else
	{ 
		tmp1=min(myPara->LengthL,myPara->LengthR);
	}

	//��֤ÿ�������������淽����������������
	if(tmp1/Min.dx<3)
	{
		Min.dx=tmp1*0.33-1e-4;
	}

	float sinL,sinR,sinS;
	float cosL,cosR,cosS;
	float beta1L,beta1R,beta1S;
	
	//FVM�ײ���б�Ƕ�
	float beta2L,beta2R,beta2S;

	//�����ٽ�̼߳���������
	float len;

	//�����ٽ�̼߳��������
	int num;

	//�����ٽ�̼߳�Ĳ���
	float dx1=Min.dx,dx2;

	//ÿ��FVM���ĵ������
	float x,y1,y2;

	//ÿ��FVM���Ĵ������泤��ƽ�кӵ�����
	float Width;

	//ÿ��FVM���Ĵ���Ӧ����ʯ��϶��Ԫ���m3
	float VH;

	//��������ˮ�ĳ�ʼˮλ
	float hm;

	//W:ͼ����ʯ����ˮ�Ľ����
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

	/********************************��B��C�����ٽ�߳���********************************/
	//����������ת�۵�
	int hB;
	//������������ԣ��е������A��֮���Ϸ��������Ļ�B��Ҫ��Ϊ�߳������ͷ�ڵ㣬���ǲ��Եģ���˼�����B[0]>A[0]������
	if(B[0]<G[0] && B[0]>A[0])
	{
		//B������AG�϶�Ӧ��������
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

	//����������ת�۵�
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
	/********************************��B��C����߳������********************************/

	if(debug)
	{
		float tempS=myPara->AreaL+myPara->AreaR+myPara->AreaS;
		if(tempS>0)
		{
			myFile<<"Ԫ��������뾶(m):"<<sqrt(tempS/PI)<<endl;
		}
		if(myPara->LengthS>0)
		{
			myFile<<"���ҡ�Դ�����(m):"<<O[1]<<","<<Q[1]<<","<<OS[1]<<endl;
			myFile<<"���ҡ�Դ�¶�:"<<beta1L/PI*180<<"��,"<<beta1R/PI*180<<"��,"<<beta1S/PI*180<<"��"<<endl;
			myFile<<"���ҡ�Դ��͸ˮ���¶�:"<<(PI-sitaAT)/PI*180<<"��,"<<(sitaDT_1)/PI*180<<"��,"<<(PI-sitaATS)/PI*180<<"��"<<endl;
		}
		else
		{
			myFile<<"���������(m):"<<O[1]<<","<<Q[1]<<endl;
			myFile<<"�����¶�:"<<beta1L/PI*180<<"��,"<<beta1R/PI*180<<"��"<<endl;
			myFile<<"���Ҳ�͸ˮ���¶�:"<<(PI-sitaAT)/PI*180<<"��,"<<(sitaDT_1)/PI*180<<"��"<<endl;
		}

		myFile<<"Բ������(X,Y):"<<Center[0]<<","<<Center[1]<<endl;
		myFile<<"�е�B��C������(X,Y):"<<"("<<B[0]<<","<<B[1]<<"); ("<<C[0]<<","<<C[1]<<")"<<endl;
		
		if(B[0]<G[0] && B[0]>A[0])
		{
			myFile<<"�е�B��Ӧ��������Ը߳�(m):"<<hB<<endl;
		}
		
		if(C[0]>H[0] && C[0]<D[0])
		{
			myFile<<"�е�C��Ӧ��������Ը߳�(m):"<<hC<<endl;
		}
	}

	/********************************�������������޵�Ԫ��********************************/
	//ahupLR,ahupS�е�a��absolute����˼
	const float ahupLR=0.5*(myPara->UElevation+myPara->DElevation);
	const float ahupS=0.5*(myPara->UElevation);
	bool flag_rock=false;
	std::vector<pair<int,int>>::iterator pStart;

	x=A[0];y1=A[1];

	//�жϵ�����
	/*if(mBSCode.Length==61 && mBSCode.RegionIndex==301 && mBSCode.Value==33)
	{
		int mm=0;
	}*/

	//lvL�ĵ�һ��϶����¶��㣬�ڶ���Ҫ���ǲ���ѩ����Ϊ�����ѩ�Ļ�����ô�ڶ����������޵�Ԫ����ʼ��
	//�п��ܾ��������ѩ��˵��ȫ���潵ѩ����ʱ���������浥Ԫ
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
		//���ȫ���潵ѩ��lvLӦ�þ��¶����½������ôpStartҲӦ����lvL.end()-1
		if(lpL==lvL.end()-1)
		{
			break;
		}

		len=(lpL->second-(lpL+1)->second)/sinL;

		//��Ϊ����B��C�ٽ�̵߳��B��C��λ�ò�ȷ���Կ��ܵ���len�ӽ���0����dx1��dx2Ҳ�ᱻ�����Ϊ0����������ֵ����ġ�
		if(len<0.5)
		{
			continue;
		}
		
		//���ٱ�֤��һ������
		num=len/Min.dx+1;
		
		//��֤�������������ڵ���3,���lvS��size���ڵ���4,�������϶����ڵ���3,�������⴦��
		if(lvL.size()==2) { num=max(num,3); }
		if(lvL.size()==3 && lpL==lvL.begin()+1 && fvL.size()+num<=2){ num=2; }

		//�µĿռ䲽��
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
				
				//���ഹֱֱ��б��Ϊ������,beta2L��Χ(-PI/2,PI/2)
				beta2L=-atan2(x-Center[0],Center[1]-y2);
				
				//����֮����ȡ����ֵ��Ϊ��������ˮ�˶�������Ҫ��ˮͷ�����͸���ã�������û������������
				//Ҳ����˵�������Ÿ����������͸ϵ����ͬ���Ƕ�������hdown������������Բ���������
				//if(debug && beta2L<0)
				//{ 
					//myFile<<"������"<<lpL->first<<"����,��"<<1+fvL.size()<<"��Ԫ"<<"Ϊ����:"<<-beta2L/PI*180<<"��,x,y1,y2:("<<x<<","<<y1<<","<<y2<<")"<<endl; 
				//}
			}
			//������Ԫ�ײ�λ�������治͸ˮ����
			else
			{
				y2=tan(sitaDT_1)*(x-D[0])+D[1];
				
				//һ���Ƿ��£�����Ϊ��ֵ
				beta2L=-sitaDT_1;
			}

			//һ����˵y2�������y1������ʵ���з���y2����y1ʱһ��A[1]��Ϊ1���ҹ����Ǹ߳������ͱ�ʾ�������������󼸸���Ԫy2���Ա�y1��
			if(y2>y1)
			{
				int temp=y1;
				y1=y2;
				y2=temp;
			}

			dx2=dx1*cosL/cos(beta2L);

			//���������m
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
			else//������
			{
				Width=myPara->StreamLength*sqrt(1-2*myPara->StreamLength*(G[0]-x)/3/myPara->AreaL);
				VH=(3*myPara->AreaL/2/myPara->StreamLength-G[0]+x)*Width-myPara->AreaL*pow(Width,3)/4/pow(myPara->StreamLength,3);
				VH=VH*dx2*cos(beta2L);

				if(debug && (Width-myPara->StreamLength>1e-3 || Width<=0)) 
				{
					myFile<<"warning:������"<<lpL->first<<"����,��"<<1+fvL.size()<<"����Ԫ������:"<<Width<<",longer than river "<<myPara->StreamLength<<endl;
					Width=myPara->StreamLength;
				}
			}

			//Ϊ�����ʼhm
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

		//������Ǹ�ѭ������������˰���������������˻�ȥ
		x-=0.5*dx1*cosL;
		y1+=0.5*dx1*sinL;

	}//end for
	/******************************���������޵�Ԫ���������******************************/

	/********************************�������������޵�Ԫ��********************************/
	x=D[0];y1=D[1];
	flag_rock=false;

	//ȷ�����޵�Ԫ����ʼ�̵߳�
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
		
		//��Ϊ����B��C�ٽ�̵߳��B��C��λ�ò�ȷ���Կ��ܵ���len�ӽ���0����dx1��dx2Ҳ�ᱻ�����Ϊ0����������ֵ����ġ�
		if(len<0.5)
		{
			continue;
		}

		num=len/Min.dx+1;

		//��֤�������������ڵ���3,���lvS��size���ڵ���4,�������϶����ڵ���3,�������⴦��
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
					//myFile<<"������"<<lpR->first<<"����,��"<<1+fvR.size()<<"��Ԫ"<<"Ϊ����:"<<-beta2R/PI*180<<"��,x,y1,y2:("<<x<<","<<y1<<","<<y2<<")"<<endl; 
				//}
			}
			else
			{
				y2=tan(sitaAT)*(x-A[0])+A[1];

				//һ���Ƿ��£�����-PI
				beta2R=sitaAT-PI;
			}

			//������y2�϶�ҪС��y1
			if(y2>y1)
			{
				int temp=y1;
				y1=y2;
				y2=temp;
			}

			dx2=dx1*cosR/cos(beta2R);

			//���������m
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
			else//������
			{
				Width=myPara->StreamLength*sqrt(1-2*myPara->StreamLength*(x-H[0])/3/myPara->AreaR);
				VH=(3*myPara->AreaR/2/myPara->StreamLength-x+H[0])*Width-myPara->AreaR*pow(Width,3)/4/pow(myPara->StreamLength,3);
				VH=VH*dx2*cos(beta2R);

				if(debug && (Width-myPara->StreamLength>1e-3 || Width<=0)) 
				{
					myFile<<"warning:������"<<lpR->first<<"����,��"<<1+fvR.size()<<"����Ԫ���:"<<Width<<",longer than river "<<myPara->StreamLength<<endl;
					Width=myPara->StreamLength;
				}
			}

			//Ϊ�����ʼhm
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

	/******************************���������޵�Ԫ���������******************************/

	/********************************����Դ�������޵�Ԫ��********************************/
	if(myPara->LengthS>0)
	{
		x=AS[0];y1=AS[1];
		flag_rock=false;

		//ȷ�����޵�Ԫ��ʼ�̵߳�
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
			//��Ϊ����B��C�ٽ�̵߳��B��C��λ�ò�ȷ���Կ��ܵ���len�ӽ���0����dx1��dx2Ҳ�ᱻ�����Ϊ0����������ֵ����ġ�
			if(len<0.5)
			{
				continue;
			}
			num=len/Min.dx+1;

			//��֤�������������ڵ���3,���lvS��size���ڵ���4,�������϶����ڵ���3,�������⴦��
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

				//���㲻ͬ�̵߳�������
				Width=Bst*(myPara->LengthS-x)/myPara->LengthS;

				VH=0.5*(Bst+Width)*x*dx2*cos(beta2S);//���ν���

				//Ϊ�����ʼhm
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
			myFile<<"���ҡ�Դ�������޵�Ԫ��Ŀ(��):"<<fvL.size()<<","<<fvR.size()<<","<<fvS.size()<<endl<<endl;
		}
		else
		{
			myFile<<"�����������޵�Ԫ��Ŀ(��):"<<fvL.size()<<","<<fvR.size()<<endl<<endl;
		}
	}
	/******************************Դ�������޵�Ԫ���������******************************/
	
	/******************************��������������ʱ�洢����******************************/
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
	/******************************��������������ʱ�������******************************/
	
}


//һ��ʱ�䲽���ڵ�������ĵر���������,dt:����������ʱ�䲽��;p:dtʱ���ڵĽ���;
void HighSlopeRunoff::SlopeRunoff(string sType,float p,float dt)
{
	::transform(sType.begin(),sType.end(),sType.begin(),tolower);//string���͵Ĵ�Сдת������

	std::vector<FVMunit> *fv;
	float ks;
	double **a,**b,**c,**x,**f,**d,**ax,**af;//bx:x����������,ax�൱��x(i+1)
	
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
	
	//��������ʱ��֤Nһ�����ڵ���3
	//if(N<=1) { return;}

	const float maxss=0.002;//m
	const float maxqs=0.002;//m2/s
	
	//�ӵ�һ����Ԫ��ʼ����������������
	int StartUnit=1;
	
	//�����浥Ԫˮ�����1mm�ط���ʼ����
	//��ʱ����Ǽ�ʹ�н��꣬�����������������ˮ��ҲС��1mm,���������������㡣
	bool flag1=false,flag2=true;
	for(int i=0;i<N;i++)
	{
		//��͸��������������Դ������㣬�����������ˮ��
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

	//����������Ŀռ䲽��ת��Ϊ��ָ�ʽ�Ŀռ䲽��
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
	
	//preissmannƫ��ϵ��
	const float cita=0.7;

	//��һʱ�̱��ڵ����һ�ڵ��qsֵ
	float aa,bb;

	//ţ�ٵ���������޼�����������
	float error=0.01,maxN=0;

	//DisO:�²�ֵ|f(X(k))|,DisN:���ڶ�²�ֵ��ѡȡ��"�½�ֵ"|f(X(k+1))|��
	float DisO=1,DisN=1;

	//���Խ��е��¶Խ��߸�ֵΪ0���㲻��ֵΪ0�������������ֵ�ˡ�
	ZeroFill(*a,N-StartUnit+1);

	//ţ�ٵ�����ȫ0��ʼ��
	//ZeroFill(*x,N-StartUnit+1);
	for(int i=0;i<=N-StartUnit;i++)
	{
		(*x)[i]=(*fv)[i+StartUnit-1].qs;
	}
	
	//�ϱ߽�����
	(*fv)[StartUnit-1].qs=1e-8;
	(*fv)[StartUnit-1].ss=pow(double((*fv)[StartUnit-1].qs),double(1/alfa))*pow(float(ks),float(-1/alfa));//m

	//�½�ţ�ٵ���
	while(DisN>error)
	{
		maxN++;
		if(maxN==30)
		{
			if(debug)
			{
				myFile<<"��������"<<sType<<",���DisN:"<<DisN<<",TimeStart:"<<FVMunit::TimeStart<<","<<"������������30�β�����"<<endl<<endl;
			}
			break;
		}

		for(int i=StartUnit-1;i<=N-2;i++)
		{
			//��ǰʱ�̵�j��j+1�ڵ�����浥������m2/s
			aa=(*fv)[i].qs; 
			bb=(*fv)[i+1].qs;

			(*c)[i-StartUnit+1] = m*(*d)[i-StartUnit+1]*((*x)[i-StartUnit+2]+(*x)[i-StartUnit+1]-aa-bb)/4/dt * cita*(1/alfa-1) * pow(double(cita*((*x)[i-StartUnit+2]+(*x)[i-StartUnit+1])+(1-cita)*(aa+bb)),double(1/alfa-2))
				                + m*(*d)[i-StartUnit+1]/4/dt*pow(double(cita*((*x)[i-StartUnit+2]+(*x)[i-StartUnit+1])+(1-cita)*(aa+bb)),double(1/alfa-1)) + cita*((*fv)[i+1].Width+(*fv)[i].Width)/2/(*fv)[i].Width;

			(*b)[i-StartUnit+1] =(*c)[i-StartUnit+1]-2*cita;

			(*f)[i-StartUnit+1] = m*(*d)[i-StartUnit+1]/4/dt*pow(double(cita*((*x)[i-StartUnit+2]+(*x)[i-StartUnit+1])+(1-cita)*(aa+bb)),double(1/alfa-1))*((*x)[i-StartUnit+2]+(*x)[i-StartUnit+1]-aa-bb) + cita*((*x)[i-StartUnit+2]-(*x)[i-StartUnit+1])
				                + (1-cita)*(bb-aa) + ((*fv)[i+1].Width-(*fv)[i].Width)/2/(*fv)[i].Width*(cita*((*x)[i-StartUnit+2]+(*x)[i-StartUnit+1])+(1-cita)*(aa+bb))-(*fv)[i].ss/dt*(*d)[i-StartUnit+1];
		}

		//�±߽������������仯��Ϊ0
		aa=(*fv)[N-1].qs;

		(*b)[N-StartUnit] = m*(*d)[N-StartUnit] * ((*x)[N-StartUnit]-aa)/dt * cita*(1/alfa-1) * pow(double(cita*2*(*x)[N-StartUnit]+(1-cita)*2*aa),double(1/alfa-2)) + m*(*d)[N-StartUnit]/2/dt*pow(double(cita*2*(*x)[N-StartUnit]+(1-cita)*2*aa),double(1/alfa-1));
		(*c)[N-StartUnit] = 0;
		(*f)[N-StartUnit] = m*(*d)[N-StartUnit]/2/dt*pow(double(cita*2*(*x)[N-StartUnit]+(1-cita)*2*aa),double(1/alfa-1))*((*x)[N-StartUnit]-aa)-(*fv)[N-1].ss/dt*(*d)[N-StartUnit];

		//ȡ���������
		DisO=VecDistance8(N-StartUnit+1,*f);
		if(DisO<error)
		{
			break;
		}
		
		//����Ϊ��ȷ�����ʵ�sʹ�õ�������|f(X(k))|�½�
		for(int t=0;t<20;t++)
		{
			//��Ϊ���ŵ����⣬���Է�Ϊif��else
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

			//׷�Ϸ������Է����飬���ڴ���ɢ��ʽ�¶Խ���Ԫ�ض�Ϊ0����ʵ�Ƕ��Խǣ���Ϊֻ�õ������νڵ㣬����õ�X������ax���С�
			::TDMA(N-StartUnit+1,*a,*b,*c,*f,*ax);

			//�õ�X(k+1),�Դ洢��ax������
			for(int i=0;i<N-StartUnit+1;i++) 
			{ 
				//cout<<"(*ax):"<<(*ax)[i]<<endl;
				
				//ǿ��ax����0�����������Ҫǿ�ҷ�ɢ�ģ�ax���ܻ�ܴ�ܴ󣬸�����ţ�ٵ��������й���
				//���ﲻ�ܶ�ax�����е��κ�ֵ��ǿ�Ƹ�ֵ�Ĵ��룬�����ϱ߽�����ax[0]=0�������𲻿�֪���
				//(*ax)[i]=max(1e-8,(*ax)[i]+(*x)[i]);

				(*ax)[i]=(*ax)[i]+(*x)[i];
			}

			for(int i=StartUnit-1;i<=N-2;i++)
			{
				//��ǰʱ�̵�j��j+1�ڵ�����浥������m2/s
				aa=(*fv)[i].qs; 
				bb=(*fv)[i+1].qs;
				(*af)[i-StartUnit+1] = m*(*d)[i-StartUnit+1]/4/dt*pow(double(cita*((*ax)[i-StartUnit+2]+(*ax)[i-StartUnit+1])+(1-cita)*(aa+bb)),double(1/alfa-1))*((*ax)[i-StartUnit+2]+(*ax)[i-StartUnit+1]-aa-bb) + cita*((*ax)[i-StartUnit+2]-(*ax)[i-StartUnit+1])
					                 + (1-cita)*(bb-aa) + ((*fv)[i+1].Width-(*fv)[i].Width)/2/(*fv)[i].Width*(cita*((*ax)[i-StartUnit+2]+(*ax)[i-StartUnit+1])+(1-cita)*(aa+bb))-(*fv)[i].ss/dt*(*d)[i-StartUnit+1];

				//cout<<"(*af):"<<(*af)[i-StartUnit+1]<<",(*d):"<<(*d)[i-StartUnit+1]<<",(*ax):"<<(*ax)[i-StartUnit+1]<<",(*fv).qs:"<<aa<<",(*fv).ss:"<<(*fv)[i].ss<<endl;
			}
			
			//�±߽�����
			aa=(*fv)[N-1].qs;

			//pow(a,b),aҪ����0������ܿ��ܳ�������������
			(*af)[N-StartUnit] = m*(*d)[N-StartUnit]/2/dt*pow(double(cita*2*(*ax)[N-StartUnit]+(1-cita)*2*aa),double(1/alfa-1))*((*ax)[N-StartUnit]-aa)-(*fv)[N-1].ss/dt*(*d)[N-StartUnit];
			
			//cout<<"(*af):"<<(*af)[N-StartUnit]<<",(*d):"<<(*d)[N-StartUnit]<<",(*ax):"<<(*ax)[N-StartUnit]<<",(*fv).qs:"<<aa<<",(*fv).ss:"<<(*fv)[N-1].ss<<endl;

			//ȡ���������
			DisN=VecDistance8(N-StartUnit+1,*af);

			//��ʹDisN����Dis0,ֻҪDis0С��error,��ȻҲû����.���������һ��.
			if(DisN<DisO || t==49)
			{
				for(int i=0;i<N-StartUnit+1;i++) 
				{ 
					(*x)[i]=(*ax)[i];
				}

				//if(debug) 
				//{ 
					//cout<<"��������"<<sType<<",ţ�ٵ�������:"<<maxN<<",�½�����Ҵ���:"<<t<<",��ǰ�����ܴ���:"<<maxN*t<<",�������������½���."<<endl;
				//}
				break;
			}
		}

	}//end while(DisN>error)

	//�����±��澭���������̵ĵ�������qs��ˮ��ss

	
	for(int i=StartUnit-1;i<N;i++)
	{
		//��Сֵ�ٶ�Ϊ1e-6
		(*fv)[i].qs=max(1e-6,(*x)[i-StartUnit+1]);//m2/s

		if(!_finite((*fv)[i].qs))
		{
			myFile<<"warning:��������INF��."<<endl;
			myFile<<"RIndex:"<<mBSCode.RegionIndex<<",Value:"<<mBSCode.Value<<",Length:"<<mBSCode.Length<<",sType:"<<sType<<",��"<<i<<"����Ԫ"<<endl;
			(*fv)[i].qs=1e-5;
		}

		//�������10�͵��Ǽ�����Ľ����ɢ��
		if((*fv)[i].qs>10)
		{ 
			myFile<<"warning:��������>10(m2/s)"<<endl;
			(*fv)[i].qs=1e-5; 
		}

		/*if((*fv)[i].qs>1e2 || (*fv)[i].qs<-1e-2)
		{
			int xx=0;
			break;
		}*/

		(*fv)[i].ss=pow(double((*fv)[i].qs),double(1/alfa))*pow(float(ks),float(-1/alfa));//m

		//cout<<"��Ԫ��:"<<i<<","<<(*fv)[i].qs<<"(m2/s),"<<(*fv)[i].ss<<"(m)"<<endl;
	}

}


//��������ˮ�˶�(����ʯ��϶ˮ����)
void HighSlopeRunoff::SaturatedSoil(string sType,float dt)
{
	::transform(sType.begin(),sType.end(),sType.begin(),::tolower); //string���͵Ĵ�Сдת������
	
	std::vector<FVMunit> *fv;
	double **a,**b,**c,**x,**f;//bx:x����������,ax�൱��x(i+1)
	
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
	
	//��������ʱ��֤Nһ�����ڵ���3
	//if(N<=2){ return; }

    /***********************Ԥ�ⲽ����ȷ����ˮ�ȣ����õ���FVM��Ԫ��ʽ��hmֵ***************************/
	
	//T1:T(i+1/2,n),T2:T(i-1/2,n)
	float T1,T2,TK;

	//�ռ䲽��
	float xl,xr,xm;
	
	//avg:��̬�䶯����ƽ����ˮ��
	float p=0,Z=0,sum=0,num=0;

	//Ԥ�ⲽ�����ã�����Ǳ������������·����ͺ�ͨ��Ԥ�ⲽ��ת���ɱ�������ˮ���ͱ����˷Ǳ�����������ʯˮ��ֱ�ӽ�������
	//���û��Ԥ�ⲽ����֪���Ǳ���������ʱ���ͣ�У����ֻ�ܶ����е�Ԫ��⡣
	for(int i=1;i<=N-2;i++)
	{
		//i��i+1��Ԫ��---BKcos(theta)(H-Hpd)---ȡƽ��ֵ
		T1=0.5*((*fv)[i].Width*(*fv)[i].ksa*cos((*fv)[i].beta2)*((*fv)[i].hm-(*fv)[i].hdown) + (*fv)[i+1].Width*(*fv)[i+1].ksa*cos((*fv)[i+1].beta2)*((*fv)[i+1].hm-(*fv)[i+1].hdown));

		//i��i-1��Ԫ��---BKcos(theta)(H-Hpd)---ȡƽ��ֵ
		T2=0.5*((*fv)[i].Width*(*fv)[i].ksa*cos((*fv)[i].beta2)*((*fv)[i].hm-(*fv)[i].hdown) + (*fv)[i-1].Width*(*fv)[i-1].ksa*cos((*fv)[i-1].beta2)*((*fv)[i-1].hm-(*fv)[i-1].hdown));

		xl=0.5*((*fv)[i].dx2+(*fv)[i-1].dx2);
		xr=0.5*((*fv)[i].dx2+(*fv)[i+1].dx2);
		xm=(*fv)[i].dx2;

		//����ˮ������ʯˮ�����
		if((*fv)[i].hdown>Rock.H)
		{
			TK=Rock.Kr*cos((*fv)[i].beta2);
		}
		//��ʯˮ��������ˮ�����
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
				myFile<<"warning:(index,value,length):("<<mBSCode.RegionIndex<<","<<mBSCode.Value<<","<<mBSCode.Length<<"):"<<"Ԥ�ⲽ��������ˮp is INF"<<endl;
			}
			(*x)[i]=(*fv)[i].hup-1e-5;
			p=((*x)[i]-(*fv)[i].hm)*(*fv)[i].mu/dt;
		}

		//��ˮ�ȵ�ȷ��--ˮλ�������
		if(p<=0) 
		{ 
			(*fv)[i].mu = FVMunit::thetas-FVMunit::thetaf;
			(*x)[i]=max((*fv)[i].hdown,p*dt/(*fv)[i].mu+(*fv)[i].hm);
		}
		//��ˮ�ȵ�ȷ��--ˮλ�������
		else
		{
			for(int t=(*fv)[i].ri.size()-1;t>=0;t--)
			{
				//�൱��mu=s-0��ȫ����ʧ��ת��Ϊ���������������������������������ڳ�ʼ��ˮ�����⣬��������ȡ1
				if((*fv)[i].ri.empty())
				{
					//��������൱�ڱ�������ˮ���Ϸ�ȫ�ǿ�������
					(*fv)[i].mu=1;
					(*x)[i]=min((*fv)[i].hup,max((*fv)[i].hdown,p*dt/(*fv)[i].mu+(*fv)[i].hm));
					break;
				}

				//��ˮ��:���ͺ�ˮ��-�䶯����ƽ����ˮ��
				sum+=(*fv)[i].ri[t].theta;
				Z+=(*fv)[i].ri[t].dz/1000;//mm->m
				num+=1;

				(*fv)[i].mu=FVMunit::thetas-sum/num;

				//��ֹ��ˮ��Ϊ0����Ϊ��ʱ��Ϊ��ĸ��
				if(abs((*fv)[i].mu)<1e-5)
				{
					(*fv)[i].mu=1e-5;
				}

				if(!_finite((*fv)[i].mu))
				{
					if(debug)
					{
						myFile<<"warning:(index,value,length):("<<mBSCode.RegionIndex<<","<<mBSCode.Value<<","<<mBSCode.Length<<"):"<<"��������ˮ��ˮ��mu is INF"<<endl;
					}
					(*fv)[i].mu=1;
				}

				//p*dt/(*fv)[i].mu:Ԥ�ⲽ��H(i,k+1)-H(i,k)
				//��������ˮ����������Ӧ�ķǱ�������������֮����һ�������ڣ�˵���ҵ��˺��ʵ�mu
				if(abs(p*dt/(*fv)[i].mu-Z) <= (*fv)[i].ri[t].dz/1000)//mm->m
				{
					//��������൱�ڱ�������ˮ���Ϸ�ȫ������
					(*x)[i]=min((*fv)[i].hup,max((*fv)[i].hdown,p*dt/(*fv)[i].mu+(*fv)[i].hm));
					break;
				}

				//��������û��break��˵����������ˮ������棬�ع�������
				if(t==0)
				{
					//��������൱�ڱ�������ˮ�˶�һ�������󣬼������������ֳ�¶�ر����Ը�ˮ�ȱ������⴦��
					//��Ϊ���ֻ������������ȡ����ʱ�ĸ�ˮ�Ⱥ�С���������ع��������á�

					//ratio:����ˮ��ˮ�����������ռ��Ȩ��
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
				myFile<<"warning:(index,value,length):("<<mBSCode.RegionIndex<<","<<mBSCode.Value<<","<<mBSCode.Length<<"):"<<"Ԥ�ⲽ��������ˮhm is INF"<<endl;
				myFile<<"p:"<<p<<"\tmu:"<<(*fv)[i].mu<<endl;
			}
			(*x)[i]=(*fv)[i].hup-1e-5;
		}

		//���¶���ƽ����͸ϵ��K(m/s)
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

	//�����ϱ߽�ˮ��
	(*x)[0]=(*fv)[0].hm=(*fv)[0].hdown;

	//�����ϱ߽��Ksa
	(*fv)[0].ksa=FVMunit::fc[(*fv)[0].landtype]*exp(-FVMunit::usf*((*fv)[0].hup-(*x)[0]));

	//�����ϱ߽��mu
	(*fv)[0].mu=1;

	//�����±߽�ˮ��
	(*x)[N-1]=(*fv)[N-1].hm=max(Bchannel.H,(*fv)[N-1].hdown);

	//�����±߽��Ksa
	if((*x)[N-1]-(*fv)[N-1].hdown>1e-3)
	{
		(*fv)[N-1].ksa=FVMunit::fc[(*fv)[N-1].landtype]*(exp(-FVMunit::usf*abs((*fv)[N-1].hup-(*x)[N-1])-exp(-FVMunit::usf*((*fv)[N-1].hup-(*fv)[N-1].hdown))))/FVMunit::usf/((*x)[N-1]-(*fv)[N-1].hdown);
	}
	else
	{
		(*fv)[N-1].ksa=FVMunit::fc[(*fv)[N-1].landtype]*exp(-FVMunit::usf*((*fv)[N-1].hup-(*x)[N-1]));
	}

	//�����±߽��mu
	(*fv)[N-1].mu=(*fv)[N-2].mu;

	/****************************************Ԥ�ⲽ��ɣ�У������ʼ******************************************/
	//Ԥ�ⲽ�����Ը�ʽ��У���������������������ʽ
	
	//ȷ���ӵڼ�����Ԫ��ʼ���㣬֮�ϲ��ֲ����㱥������ˮ�˶�
	int StartUnit=0;
	for(int i=0;i<N;i++)
	{
		//Ϊ�˲���StartUnit�����һ����Ԫ����Ϊ�����õ���i+1
		if(i==N-1)
		{
			return;//��������ˮ̫�٣�ֱ�ӷ��ء�
		}

		//�õ��hm����2mm����һ���Ժ���㶼����2mm�����ܻ�С��0
		if(((*x)[i]-(*fv)[i].hdown)>2e-3)
		{
			StartUnit=i+1;
			break;
		}
		else
		{
		     //����0.1mm��,����0.1mmСˮ��
			(*fv)[i].hm=max((*x)[i],(*fv)[i].hdown+1e-4);
		}
	}

	//RockW:��ʯ��϶ˮ�ı仯ˮ��m3
	float S,RockW=0;
	float TE,TW,TP,TB;

	/*************************************************�ϱ߽縳ֵ*************************************************/
	TP=(*fv)[StartUnit-1].Width*(*fv)[StartUnit-1].ksa*cos((*fv)[StartUnit-1].beta2)*((*fv)[StartUnit-1].hm-(*fv)[StartUnit-1].hdown);
	TE=(*fv)[StartUnit].Width*(*fv)[StartUnit].ksa*cos((*fv)[StartUnit].beta2)*((*fv)[StartUnit].hm-(*fv)[StartUnit].hdown);

	(*b)[0]=(*fv)[StartUnit-1].mu*(*fv)[StartUnit-1].Width*(*fv)[StartUnit-1].dx2*((*fv)[StartUnit-1].dx2+(*fv)[StartUnit].dx2)/dt+TP+TE;
	(*c)[0]=-(TE+TP);
	(*f)[0]=(*fv)[StartUnit-1].mu*(*fv)[StartUnit-1].Width*(*fv)[StartUnit-1].dx2*((*fv)[StartUnit-1].dx2+(*fv)[StartUnit].dx2)/dt*(*fv)[StartUnit-1].hm;

	S=(*fv)[StartUnit-1].dx2*cos((*fv)[StartUnit-1].beta2)*(*fv)[StartUnit-1].Width;

	//���2��������ʯˮ�����ǲ����ָ�ˮ�����
	if((*fv)[StartUnit-1].hdown>Rock.H && (*x)[StartUnit-1]>=2e-3)
	{
		(*f)[0]-=(*fv)[StartUnit-1].Width*Rock.Kr*((*fv)[StartUnit-1].dx2+(*fv)[StartUnit].dx2)*(*fv)[StartUnit-1].dx2*cos((*fv)[StartUnit-1].beta2);
		RockW += Rock.Kr*dt*S;//m3
	}
	/*************************************************�ϱ߽縳ֵ*************************************************/

	/************************************************�м�ڵ㸳ֵ************************************************/
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

		//Ϊ���⸺ˮ����������Ժ����Ĵ���ʽҪ��֤��һʱ��ûˮ�ĵ�Ԫ�����е�Ԫǰ������������������м�ĳ��ûˮ�����඼��ˮ��������ô���أ�
		//���2��������ʯˮ�����ǲ����ָ�ˮ�����
		if((*fv)[i].hdown>Rock.H && (*x)[i]>=2e-3)
		{
			(*f)[i-StartUnit+1]-=0.5*((*fv)[i].dx2+(*fv)[i+1].dx2)*((*fv)[i].dx2+(*fv)[i-1].dx2)*(*fv)[i].dx2*(*fv)[i].Width*Rock.Kr*cos((*fv)[i].beta2);
			RockW += Rock.Kr*cos((*fv)[i].beta2)*dt*S;//m3
		}
		
		//���3����ʯˮ��������ˮ���
		if((*fv)[i].hdown<=Rock.H)
		{
			(*a)[i-StartUnit]-=TK;
			(*b)[i-StartUnit+1]+=TK;
			RockW -= (*fv)[i].ksa*abs((*fv)[i].hm-(*fv)[i-1].hm)/(*fv)[i].dx2*dt*S;
			//RockW -= (*fv)[i].ksa*sin((*fv)[i].beta2)*dt*S;
		}
	}
	/************************************************�м�ڵ㸳ֵ************************************************/

	/*************************************************�±߽縳ֵ*************************************************/
	TP=(*fv)[N-1].Width*(*fv)[N-1].ksa*cos((*fv)[N-1].beta2)*((*fv)[N-1].hm-(*fv)[N-1].hdown);
	TW=(*fv)[N-2].Width*(*fv)[N-2].ksa*cos((*fv)[N-2].beta2)*((*fv)[N-2].hm-(*fv)[N-2].hdown);
	TB=TP;//ע������Ĵ���ʽ

	TK=(*fv)[N-1].Width*(*fv)[N-1].ksa*((*fv)[N-1].dx2+(*fv)[N-2].dx2)*(*fv)[N-1].dx2;

	(*a)[N-StartUnit-1]=-(TW+TP)*(*fv)[N-1].dx2;
	(*b)[N-StartUnit]=(*fv)[N-1].mu*pow((*fv)[N-1].Width,2)*((*fv)[N-1].dx2+(*fv)[N-1].dx2)/dt+2*TB*((*fv)[N-1].Width+(*fv)[N-2].Width)+(TW+TP)*(*fv)[N-1].Width;
	(*f)[N-StartUnit]=(*fv)[N-1].mu*pow((*fv)[N-1].Width,2)*((*fv)[N-1].dx2+(*fv)[N-1].dx2)/dt*(*fv)[N-1].hm+2*TB*max((*fv)[N-1].hdown,max((*fv)[N-1].hdown,Bchannel.H))*((*fv)[N-1].Width+(*fv)[N-2].Width);

	//���2��������ʯˮ�����ǲ����ָ�ˮ�����
	if((*fv)[N-1].hdown>Rock.H && (*x)[N-1]>=1e-3)
	{
		(*f)[N-StartUnit]-=((*fv)[N-1].dx2+(*fv)[N-2].dx2)*(*fv)[N-1].dx2*(*fv)[N-1].dx2*(*fv)[N-1].Width*Rock.Kr*cos((*fv)[N-1].beta2);
	}

	//���3����ʯˮ��������ˮ���
	if((*fv)[N-1].hdown<=Rock.H)
	{
		(*a)[N-StartUnit-1]-=TK;
		(*b)[N-StartUnit]+=TK;
	}
	/*************************************************�±߽縳ֵ*************************************************/
	
	::TDMA(N-StartUnit+1,*a,*b,*c,*f,*x);
	/**************************************************У�������****************************************************/

	//������ʯˮ��ˮλ
	Rock.WtoH(RockW);

	//���±�������ˮˮ�ƽ����͸ϵ��ksa(m/s)����ˮ�ȾͲ������ˣ��Ƚ��鷳����Ϊ��ˮ����Ҫ������������ʱ�̵�H��������Ȼ����Ԥ�ⲽ��ֵ��
	for(int i=StartUnit-1;i<N;i++)
	{
		(*fv)[i].hm=(*x)[i-StartUnit+1];

		if(!_finite((*fv)[i].hm))
		{
			if(debug)
			{
				myFile<<"warning:(index,value,length):("<<mBSCode.RegionIndex<<","<<mBSCode.Value<<","<<mBSCode.Length<<"):"<<"У������������ˮhm is INF"<<endl;
				myFile<<"TimeStart:"<<FVMunit::TimeStart<<",sType:"<<sType<<",��Ԫ:"<<i<<",hdown:"<<(*fv)[i].hdown<<",hup:"<<(*fv)[i].hup<<",hm:"<<(*fv)[i].hm<<endl;
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

		//�лع�������
		if((*fv)[i].hm>(*fv)[i].hup)
		{
			(*fv)[i].hv=((*fv)[i].hm-(*fv)[i].hup)/dt;//m/s
			(*fv)[i].hm=(*fv)[i].hup-1e-4;
			
			//����TimeEnd��ֵ��������TimeStart��TimeEnd֮���лع�������
			(*fv)[i].TimeEnd=FVMunit::TimeStart+dt;

			if(debug)
			{
				int sLength;
				if(sType=="l") sLength=HighSlopeRunoff::myPara->LengthL/cos((*fv)[i].beta1);
				if(sType=="r") sLength=HighSlopeRunoff::myPara->LengthR/cos((*fv)[i].beta1);
				if(sType=="s") sLength=HighSlopeRunoff::myPara->LengthS/cos((*fv)[i].beta1);
				myFile<<"sType:"<<sType<<",���泤:"<<sLength<<"(m),���¶�б�����:"<<(*fv)[i].Dis<<"(m),����ˮ����:"<<(*fv)[i].hup-(*fv)[i].hm<<"(m)"<<endl;
				myFile<<"Year:"<<FVMunit::Year<<",Month:"<<FVMunit::Month<<",Day:"<<FVMunit::Day<<",Hour:"<<FVMunit::Hour<<"-"<<FVMunit::Hour+dt/3600<<",�����ع���:"<<(*fv)[i].hv*dt<<"(m),��������:"<<(*fv)[i].hv*1000<<"(mm/s)"<<endl;
			}
		}

		if((*fv)[i].hm<(*fv)[i].hdown)
		{
			(*fv)[i].hm=(*fv)[i].hdown+1e-4;
		}
		
		//���¶�����͸ϵ��ksa
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

		//cout<<sType<<"��Ԫ:"<<i<<",hdown:"<<(*fv)[i].hdown<<",hup:"<<(*fv)[i].hup<<",hm:"<<(*fv)[i].hm<<",ksa:"<<(*fv)[i].ksa<<endl<<endl;

		//���·Ǳ�������
		(*fv)[i].UpdateRichardsUnit();
	}
}


//��������ˮ����
void HighSlopeRunoff::BellowChannel(float dt)
{
	//ǰ��˼·����ʯˮ(����ˮ)t+1ʱ��״̬������ˮ(��ʯˮ)tʱ�̾��������ڴ���������ˮ�˶���ʹ������ˮt+1ʱ�̵�ֵ
	float BchannelW=0;
	
	//wu:dtʱ�������β�����ˮ��m3;wd:dtʱ����������Ԫ�����ˮ��m3;wL,wR:�������油����ˮ��m3,wm:����������m3
	float wL=0,wR=0,wu=0,wd=0,wm=0,wRock=0;
	int N1=min(int(FVMunit::TimeStart/this->MSTEP),Steps);
	int N2=min(int((FVMunit::TimeStart+dt)/this->MSTEP)+1,Steps);

	/**************��������Ĳ�����**************/
	float ksaL,ksaR,ksaS;
	ksaL=ksaR=ksaS=FVMunit::fc[0];//��ʱ�����϶���ѩ������ȡfc[0]

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

	/************����֧���ӵ��Ĳ�����************/
	if(myPara->AreaS>1e-2 && fvS.size()>1)
	{
		//Դ���油��
		wu=cos(fvS[fvS.size()-1].beta2)*ksaS*(fvS[fvS.size()-2].hm-fvS[fvS.size()-1].hm)/(0.5*(fvS[fvS.size()-2].dx2+fvS[fvS.size()-1].dx2))*dt*gh*(fvS[fvS.size()-1].hm-(LL[1]+KK[1])/2);
	}
	else if(myPara->AreaS>1e-2 && fvS.size()<=1)
	{
		//��ѩ������������ڲ�����
		wu=0;
	}
	else
	{
		//���ε���ˮ��������
		for(int i=N1;i<=N2;i++)
		{
			wu+=(Qd1[i]+Qd2[i])*MSTEP;
		}
	}

	/***************�ӵ����ڶ���������************/
	double Kchannel=0.5*(ksaL+ksaR);//ע��,�����������������Ԫ��͸ϵ����ƽ��ֵ��Ϊ�ӵ���͸ϵ��

	float sinT=(myPara->StreamSlope/sqrt(1+myPara->StreamSlope*myPara->StreamSlope));//ˮͷ�Ƚ�Ϊ�ӵ��¶�����sinx=tanx/sqrt(1+tanx^2);
	
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
		Qdout[i]=wd/dt;//���ڵ��¾�������

		if(Qdout[i]>1000)
		{
			int xxx=0;
		}
	}

	//cout<<"N1:"<<N1<<",N2:"<<N2<<",wd:"<<wd<<",dt(h):"<<dt/3600<<",wd/dt:"<<wd/dt<<",gm:"<<gm<<",BH:"<<Bchannel.H<<",BW:"<<Bchannel.W<<endl;

	//����ʯˮ����,SDΪ��ʯˮ�͹�������ˮ�Ľ������
	const float SD=KL*myPara->StreamLength;
	
	if(Bchannel.H<Rock.H)//��ʯˮ�������ˮ
	{
		wRock=Kchannel*sinT*dt*SD;
	}
	else//����ˮ������ʯˮ
	{
		wRock=-Rock.Kr*dt*SD;
	}

	Rock.WtoH(-wRock);//��ʯˮˮλ����

	/**************������ˮ֮��Ľ�����***********/
	const float GE=gm/sin(theta1),FH=gm/sin(theta2);
	float ratio=1.0;
	
	//����ˮλ���ڵ���ˮλ,����õ���wm�϶�������
	if(Ichannel.H>=Bchannel.H)
	{
		//����ˮ����EF��
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

			//ע������ûˮ�Ͳ��ܼ�����������
			wm=min(Ichannel.H+gm,Kchannel*dt)*myPara->StreamLength* (ratio*(GE*cos(theta1)+FH*cos(theta2)) + (Bchannel.F[0]-Bchannel.E[0]));
		}
		//����ˮ����EF��
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
	//����ˮλ��������ˮλ,����õ���wm�϶��Ǹ���
	else
	{
		//�μ�����ȫ������������
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

	//����ˮ����
	//Ichannel.TimeEnd=FVMunit::TimeStart+dt;
	Ichannel.q=-wm/dt/myPara->StreamLength;//m2/s

	//�õ����빵�����µ�ˮ���������ɸ���
	BchannelW=wL+wR+wu-wd+wm+wRock;

	if(!_finite(BchannelW))
	{
		if(debug)
		{
			myFile<<"warning:(index,value,length):("<<mBSCode.RegionIndex<<","<<mBSCode.Value<<","<<mBSCode.Length<<"):"<<"BchannelW is INF"<<endl;
		}
		BchannelW=0;
	}
	
	//�õ�������������²��ֺ�������������ˮ��Wout�������¹�������ˮˮλ
	float Wout=Bchannel.WtoH(BchannelW);

	//��������ˮ�õ��Ĳ���Ͳ���ȫ��"������"���ǿ�����һ���ˡ�
	if(Wout>1-4)
	{
		Ichannel.q+=(Wout/dt/myPara->StreamLength);//m2/s
	}

}


