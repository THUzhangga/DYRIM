
#include "stdafx.h"
#include ".\MParameters.h"
#include <fstream>
#include <string>
#include "datastruct.h"

using namespace std;

MParameters::MParameters(void)
{
}

MParameters::~MParameters(void)
{
}

//将Para结构体中的参数以及该河段前一天的土壤含水量信息都读入parameter类中
bool MParameters::GetParameters(struct Para * pPara,char Position,BSCode mBSCode)
{
	bool isOK;

	if(GetWaterInSoil(mBSCode,*pPara))
	{
		if(Position=='S')
		{
			RouteLength=pPara->LengthS;
			SlopeJ=pPara->SlopeS;
			Area=pPara->AreaS;
			SoilW=WSU;//潜水层初始水量
			MidSoilW=WSM;
			DSW=WSD; //深层土的初始水量
			LeafW=WSL; //叶面积水深，m

			isOK= 1;
		}
		else if(Position=='L')
		{
			RouteLength=pPara->LengthL;
			SlopeJ=pPara->SlopeL;
			Area=pPara->AreaL;
			SoilW=WLU;//潜水层初始水量
			MidSoilW=WLM;
			DSW=WLD; //深层土的初始水量
			LeafW=WLL; //叶面积水深，m
			
			isOK= 1;
		}
		else if(Position=='R')
		{
			RouteLength=pPara->LengthR;
			SlopeJ=pPara->SlopeR;
			Area=pPara->AreaR;
			SoilW=WRU;//潜水层初始水量
			MidSoilW=WRM;
			DSW=WRD; //深层土的初始水量
			LeafW=WRL; //叶面积水深，m
			
			isOK= 1;
		}
		else
		{
			isOK= 0;
		}

		SediD=pPara->D50;

		//每小时可下渗的水量 m/h
		PKV0=pPara->PKV0;
		PKV1=pPara->PKV1;
		PKV2=pPara->PKV2;//addbyiwish20060220
		
		//潜水层土壤水平渗透系数, m/second
		//输入水平渗透率单位与下渗率统一为m/hr
		PKH1=pPara->PKH1/3600;
		PKH2=pPara->PKH2/3600;

		USita1=pPara->Sita1;
		USita2=pPara->Sita2;
		MSita1=pPara->MSita1;
		MSita2=pPara->MSita2;
		DSita1=pPara->DSita1;
		DSita2=pPara->DSita2;

		UDepth=pPara->UDepth;
		DSDepth=pPara->DDepth;

		Roughnessn=pPara->Manning;//0.12f;//在riversegs表中
		
		//20051219 薛海 增加产沙k
		ErosionK=pPara->ErosionK;
		ErosionBeta=pPara->ErosionBeta;//parameter表中

		//David 坡面产沙模型
	    ErosionK1=pPara->ErosionK1;
	    ErosionK2=pPara->ErosionK2;
	    ErosionBeta1=pPara->ErosionBeta1;
	    ErosionBeta2=pPara->ErosionBeta2;

		//addbyiwish 20060222
		//au=0.5;
		//bu=3.5;
		//am=1;
		//bm=4.21;
		//ad=1.5;
		//bd=4.21;

		//20070118,xiaofc,党进谦的公式算出来的是kPa,为转成水头m,需要/Rou/g,所以给所有的a除了一个10
		//au=6.7377/10;
		//bu=2.5283;
		//am=0.1973;
		//bm=2.325;
		//ad=0.1;
		//bd=4.21;

		//20141229
		au=0.674;
		bu=2.528;
		am=0.674;
		bm=2.325;
		ad=0.674;
		bd=2.325;

		//20111208,qjh,shy
		//au=6.7377/10;
		//bu=2.5283;
		//am=0.1973;
		//bm=2.325;
		//ad=0.1;
		//bd=4.21;

		return isOK;
		}
	else 
		return false;

}

// 把状态存入文件
bool MParameters::SaveStatusFile(void)
{
	
	string pfilename("status.txt");
	const char* cfilename=pfilename.c_str();
	ofstream fps(cfilename,ios_base::out);
	
	if( !fps )
	{
		//MessageBox(NULL,"Open file status.txt error.","ERROR",MB_OK);
		cout<<"Open file status.txt error."<<endl;
		return false;	
	}
	
	fps<<"SoilW\tDeepSoilW"<<endl;

	fps<<SoilW<<"\t"<<DSW;
	
	fps.close();

	return true;

}

//将每条河段的初始土壤体积含水量写入status表中，并同时存入内存中的相应变量。
bool MParameters::GetWaterInSoil(BSCode mBSCode,Para myPara)
{
	//读土层初始含水量
	CComVariant tempCom(VT_EMPTY);
	ADODB::_RecordsetPtr pStatRst;
	CString SQL;

	SQL.Format(_T("Select * from Status where sccd='%s' and RegionIndex=%I64u and BSValue=%I64u and BSLength=%d and Houroffset=%d"),sccd,mBSCode.RegionIndex,mBSCode.Value,mBSCode.Length,long(lHourStart/24)*24);
	_bstr_t bSQL=SQL.GetString();
	try
	{
		pStatRst.CreateInstance(__uuidof(ADODB::Recordset));
		pStatRst->Open(bSQL,(ADODB::_Connection*)pTabFileCnn,ADODB:: adOpenKeyset,ADODB::adLockOptimistic,ADODB::adCmdText);
	}
	catch(_com_error e)
	{
		cout<<"Open Status table error: "<<SQL<<endl;
		e.ErrorMessage();
		e.Description();
		//给出连接错误警告
	}

	if(pStatRst->EndOfFile)//没数就把默认值写进去
	{
		//altered by wh,土壤含水量初值改为由数据库读取
		//20070607,xiaofc,土壤含水量初值改为由文件读取
		//含水量数值为 Vwater/Vtotal
		//TCHAR path[MAX_PATH];
		//::GetModuleFileName(NULL,path,MAX_PATH);
		//CString Str(path);
		//int i=Str.ReverseFind('\\');
		//CString m_FilePath=Str.Left(i+1);

		//CString m_strFilePath=m_FilePath+"FilePath.ini";
		//TCHAR temp[100];

		//float UpInitWaterContent;
		//float MidInitWaterContent;
		//float DownInitWaterContent;

		//::GetPrivateProfileString(_T("SOIL INIT"),_T("UpWaterContent"),_T("0"),temp,sizeof(temp),m_strFilePath);
		//UpInitWaterContent=_wtof(temp);
		if(UpInitWaterContent<1e-5) UpInitWaterContent=1e-5;
		//ZeroMemory(temp,sizeof(temp));

		//::GetPrivateProfileString(_T("SOIL INIT"),_T("MidWaterContent"),_T("0"),temp,sizeof(temp),m_strFilePath);
		//MidInitWaterContent=_wtof(temp);
		if(MidInitWaterContent<1e-5) MidInitWaterContent=1e-5;
		//ZeroMemory(temp,sizeof(temp));

		//::GetPrivateProfileString(_T("SOIL INIT"),_T("DownWaterContent"),_T("0"),temp,sizeof(temp),m_strFilePath);
		//DownInitWaterContent=_wtof(temp);
		if(DownInitWaterContent<1e-5) DownInitWaterContent=1e-5;
		//ZeroMemory(temp,sizeof(temp));
		
		pStatRst->AddNew();  
		//ChannelIndex, HourOffset

		//wh,2008.3.25
		vtmp = this->sccd;
		pStatRst->Fields->Item["sccd"]->Value = _variant_t(vtmp);

		tempCom.ChangeType(VT_DECIMAL);
		tempCom.ullVal=mBSCode.RegionIndex;
		pStatRst->Fields->Item["RegionIndex"]->Value=tempCom;
		tempCom.ullVal=mBSCode.Value;
		pStatRst->Fields->Item["BSValue"]->Value=tempCom;
		pStatRst->Fields->Item["BSLength"]->Value=mBSCode.Length;
		pStatRst->Fields->Item["HourOffset"]->Value=long(lHourStart/24)*24;//从整天开始

		//以下值为2004年末算小花间时调出来的默认初值，对上游区来说偏大
		//20070117，xiaofc,通过对蒸发量的统计对比，对于黄土高原，初始条件给一定的自由水是多余的，且由于中层土的体积较大，自由水很难耗尽，从而对结果有不可预计的影响。
		//20070118,xiaofc,采用红皮书中1967年4月1日的实测含水量，转化为所占体积比，作为初始含水量
		pStatRst->Fields->Item["WLL"]->Value=0.0f;/*叶面积水深*/
		pStatRst->Fields->Item["WLU"]->Value=myPara.AreaL*myPara.UDepth*UpInitWaterContent;//0.159;//myPara.Sita1;//+myPara.AreaL*myPara.UDepth*myPara.Sita2/15.0f;
		
		if(myPara.SlopeL<1.1e-5)//20110927,xiaofc,坡面坡度太小，中层土体积过小，计算失败
			pStatRst->Fields->Item["WLM"]->Value=myPara.AreaL*myPara.UDepth*MidInitWaterContent;
		else
			pStatRst->Fields->Item["WLM"]->Value=myPara.AreaL*myPara.LengthL*myPara.SlopeL*MidInitWaterContent; //0.179;//myPara.MSita1;//+myPara.AreaL*myPara.LengthL*myPara.SlopeL*myPara.MSita2/15.0f;
		
		
		pStatRst->Fields->Item["WLD"]->Value=myPara.AreaL*myPara.DDepth*DownInitWaterContent; //(myPara.DSita1+myPara.DSita2);
		pStatRst->Fields->Item["WRL"]->Value=0.0f;
		pStatRst->Fields->Item["WRU"]->Value=myPara.AreaR*myPara.UDepth*UpInitWaterContent;//0.159;//myPara.Sita1;//+myPara.AreaR*myPara.UDepth*myPara.Sita2/15.0f;
		
		if(myPara.SlopeR<1.1e-5)//20110927,xiaofc,坡面坡度太小，中层土体积过小，计算失败
			pStatRst->Fields->Item["WRM"]->Value=myPara.AreaR*myPara.UDepth*MidInitWaterContent;
		else
			pStatRst->Fields->Item["WRM"]->Value=myPara.AreaR*myPara.LengthR*myPara.SlopeR*MidInitWaterContent;//0.179;//myPara.MSita1;//+myPara.AreaR*myPara.LengthR*myPara.SlopeR*myPara.MSita2/15.0f;
		
		
		pStatRst->Fields->Item["WRD"]->Value=myPara.AreaR*myPara.DDepth*DownInitWaterContent;//(myPara.DSita1+myPara.DSita2);
		pStatRst->Fields->Item["WSL"]->Value=0.0f;
		pStatRst->Fields->Item["WSU"]->Value=myPara.AreaS*myPara.UDepth*UpInitWaterContent;//0.159;//myPara.Sita1;//+myPara.AreaS*myPara.UDepth*myPara.Sita2/15.0f;
		
		
		if(myPara.SlopeS<1.1e-5)//20110927,xiaofc,坡面坡度太小，中层土体积过小，计算失败
			pStatRst->Fields->Item["WSM"]->Value=myPara.AreaS*myPara.UDepth*MidInitWaterContent;
		else
			pStatRst->Fields->Item["WSM"]->Value=myPara.AreaS*myPara.LengthS*myPara.SlopeS*MidInitWaterContent; //0.179;//myPara.MSita1;//+myPara.AreaS*myPara.LengthS*myPara.SlopeS*myPara.MSita2/15.0f;
		
		
		
		pStatRst->Fields->Item["WSD"]->Value=myPara.AreaS*myPara.DDepth*DownInitWaterContent; //(myPara.DSita1+myPara.DSita2);

		if(myPara.AreaL<-1e-6)
		{
			pStatRst->Fields->Item["WLL"]->Value=-1.0f;
			pStatRst->Fields->Item["WLU"]->Value=-1.0f;
			pStatRst->Fields->Item["WLM"]->Value=-1.0f;
			pStatRst->Fields->Item["WLD"]->Value=-1.0f;
		}

		if(myPara.AreaR<-1e-6)
		{
			pStatRst->Fields->Item["WRL"]->Value=-1.0f;
			pStatRst->Fields->Item["WRU"]->Value=-1.0f;
			pStatRst->Fields->Item["WRM"]->Value=-1.0f;
			pStatRst->Fields->Item["WRD"]->Value=-1.0f;
		}

		if(myPara.AreaS<-1e-6)
		{
			pStatRst->Fields->Item["WSL"]->Value=-1.0f;
			pStatRst->Fields->Item["WSU"]->Value=-1.0f;
			pStatRst->Fields->Item["WSM"]->Value=-1.0f;
			pStatRst->Fields->Item["WSD"]->Value=-1.0f;
		}

		pStatRst->Update();
	}

	//读出
	WLL=pStatRst->Fields->Item["WLL"]->Value;
	WLU=pStatRst->Fields->Item["WLU"]->Value;
	WLM=pStatRst->Fields->Item["WLM"]->Value;
	WLD=pStatRst->Fields->Item["WLD"]->Value;
	WRL=pStatRst->Fields->Item["WRL"]->Value;
	WRU=pStatRst->Fields->Item["WRU"]->Value;
	WRM=pStatRst->Fields->Item["WRM"]->Value;
	WRD=pStatRst->Fields->Item["WRD"]->Value;
	WSL=pStatRst->Fields->Item["WSL"]->Value;
	WSU=pStatRst->Fields->Item["WSU"]->Value;
	WSM=pStatRst->Fields->Item["WSM"]->Value;
	WSD=pStatRst->Fields->Item["WSD"]->Value;

	pStatRst->Close();
	return true;
}


void MParameters::ReadLE(BSCode mBSCode)
{
	//读LAI EPI

	CString SQL;
	//SQL.Format(_T("Select * from ( select * from tvarparameter where RegionIndex=%I64u and BSValue=%I64u and BSLength=%d and year between %d and %d minus select * from tvarparameter where RegionIndex=%I64u and BSValue=%I64u and BSLength=%d and ( (year=%d and mon<%d) or (year=%d and mon>%d) ) ) order by year,mon"),mBSCode.RegionIndex,mBSCode.Value,mBSCode.Length,YearStart,YearEnd,mBSCode.RegionIndex,mBSCode.Value,mBSCode.Length,YearStart,MonthStart,YearEnd,MonthEnd);

    //altered by weihong
	//wh,将month改为了mon，原来就是mon
	//SQL.Format(_T("Select * from ( select * from tvarparameter where\
	//	RegionIndex=%I64u and BSValue=%I64u and BSLength=%d and year between %d and %d \
	//	minus \
	//	select * from tvarparameter where RegionIndex=%I64u and BSValue=%I64u and BSLength=%d and ( (year=%d and mon<%d) or (year=%d and mon>%d) ) ) \
	//	order by year,mon"),\
	//	mBSCode.RegionIndex,mBSCode.Value,mBSCode.Length,YearStart,YearEnd,\
	//	mBSCode.RegionIndex,mBSCode.Value,mBSCode.Length,YearStart,MonthStart,YearEnd,MonthEnd);

	SQL.Format(_T("Select * from tvarparameter where RegionIndex=%I64u and BSValue=%I64u and BSLength=%d order by mon"),mBSCode.RegionIndex,mBSCode.Value,mBSCode.Length);

	//cout<<SQL<<endl;
//	ofstream a2333;
//	a2333.open("23333.txt",ios::app);
//		wchar_t * chTest = new wchar_t[SQL.GetLength()+1]; //动态申请空间
//       chTest = SQL.GetBuffer(0); //获取CString内容的地址
// 
//       for(int i = 0; i<SQL.GetLength();i++){
//    a2333<<(char)*(chTest+i);
//          }
//SQL.ReleaseBuffer();    //调用了GetBuffer后一定要调用ReleaseBuffer释放，否则会出现内存泄露
//a2333<<endl<<flush;
//a2333.close();
	_bstr_t bSQL=SQL.GetString();

	try
	{
		pStatRst.CreateInstance(__uuidof(ADODB::Recordset));
		pStatRst->Open(bSQL,(ADODB::_Connection*)pTabFileCnn,ADODB:: adOpenKeyset,ADODB::adLockOptimistic,ADODB::adCmdText);
	}
	catch(_com_error e)//打不开就建个新的再打开
	{
		//给出连接错误警告
		cout<<"Open TVarParameter Table Error: "<<SQL<<endl;
		e.Description();
		e.ErrorMessage();
	}
	
	//如果没有数据取的默认值
	if(pStatRst->EndOfFile)
	{
		LAI=1.0f;
		WaterEVA=2000.0f;//年蒸发能力，mm/年。
		cout<<"111111111111111111111111111111111111111"<<endl;
	}
	else//wh？只取第一条记录？,yes，没错，精确到月了，而且yearstart是变化的。
	{
		try
		{
			LAI=pStatRst->Fields->Item["LAI"]->Value;
			WaterEVA=pStatRst->Fields->Item["WaterEVA"]->Value;//wh:如果watereva为空，则该句会报错的。
		}
		catch(...)
		{
			cout<<"Error while reading table STATUS, maybe NULL value encountered."<<endl;
		}
	}
	
	//wh：再看看getrsdata中的watereva是不是年蒸发能力，即每个月的数据都是一样的。
	EPI=WaterEVA/24.0f/365.0f/1000.0f;//每小时的蒸发能力，mm变为m

	//wh：注意以上的处理方式：虽然从tvarparameter表中选取了很多条记录，每条代表一个月，但其实只读了一条记录的信息，默认每个月存的是年蒸发能力，觉得蛮招笑的。
	//wh：以后可以改进蒸发能力的分摊方式，不是简单的平均分摊得到EPI，跟季节有关系。

	//20070122,xiaofc,因为要多次对连向tvarparameter的记录集操作，因此不能将其关掉
	//pStatRst->Close();

}
//20070122,xiaofc
void MParameters::ReadLE(BSCode mBSCode,short Year,short Month)
{
	if(pStatRst->EndOfFile)
		return;

	short RstCurYear;
	short RstCurMonth;
	RstCurYear=pStatRst->Fields->Item["Year"]->Value;
	RstCurMonth=pStatRst->Fields->Item["Mon"]->Value;
	
	//cout<<Year<<"\t"<<Month<<endl;
	//cout<<RstCurYear<<"\t"<<RstCurMonth<<endl;

	//if(Year>RstCurYear || (Year==RstCurYear && Month>RstCurMonth))
	//{
	//	pStatRst->MoveNext();
	//	if(pStatRst->EndOfFile)
	//		return;
	//	LAI=pStatRst->Fields->Item["LAI"]->Value;
	//	WaterEVA=pStatRst->Fields->Item["WaterEVA"]->Value;
	//	//cout<<"WaterEVA("<<Year<<","<<Month<<")="<<WaterEVA<<endl;
	//	EPI=WaterEVA/24.0f/365.0f/1000.0f;//每小时的蒸发能力，mm变为m
	//}
	if (Month!=RstCurMonth)
	{
		pStatRst->MoveNext();
		if(pStatRst->EndOfFile)
			pStatRst->MoveFirst();
		LAI=pStatRst->Fields->Item["LAI"]->Value;
		WaterEVA=pStatRst->Fields->Item["WaterEVA"]->Value;
		EPI=WaterEVA/24.0f/365.0f/1000.0f;//每小时的蒸发能力，mm变为m
	}

	//20070122,xiaofc,因为要多次对连向tvarparameter的记录集操作，因此在最后一次操作时再关掉记录集
	if(Year==YearEnd && Month==MonthEnd)
		pStatRst->Close();
	
}