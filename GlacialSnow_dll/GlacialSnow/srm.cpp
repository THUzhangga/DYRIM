// SRM.cpp : CSRM 的实现

#include "stdafx.h"
#include "SRM.h"
#include <fstream>
#include <iomanip>//下面的setw要用到，而且得有std空间
using namespace std;


// CSRM

//被wateryield.dll中的WaterBasin类中的Initialize函数调用
//该函数在并行计算的始终只被执行一次，也应该如此。
STDMETHODIMP CSRM::OpenOracle(/*_Connection** pCnn*/BSTR user, BSTR password, BSTR sid)//必须_Connection**，一颗星不好使
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	 //TODO: 在此添加实现代码
	//::CoInitialize(NULL);
	CString User(user),Password(password),Sid(sid);
	//ADODB::_ConnectionPtr pCnn;	
	CString CSSQL;
	_bstr_t bSQL;
	
	CSSQL.Format(_T("Provider=OraOLEDB.Oracle.1;Persist Security Info=False;User ID=%s;Data Source=%s;Extended Properties=''"),User,Sid);
	bSQL=CSSQL.GetString();
	try
	{
		minput.pConn.CreateInstance(__uuidof(ADODB::Connection));//创建对象
		minput.pConn->Open(bSQL,User.GetString(),Password.GetString(),0);
		//minput.pConn = pCnn;
	}
	catch(_com_error e)
	{
		//给出数据库连接失败的警告.
		cout<<"Failture Of Opening Oracle In GlacialSnow.dll."<<endl;
		e.ErrorMessage();
		e.Description();
		return S_FALSE;
	}

	//minput.pConn = *pCnn;//注意此方式

    //为了得到计算的起止时间，特地在hydrousepara表中增加了若干项。
	ADODB::_RecordsetPtr pRst;
	CSSQL.Format(_T("select startyear,startmonth,startday,endyear,endmonth,endday from hydrousepara"));
	bSQL = CSSQL.GetString();
	try
	{
		pRst.CreateInstance(__uuidof(ADODB::Recordset));
		pRst->Open(bSQL,(ADODB::_Connection*)minput.pConn,ADODB::adOpenKeyset,ADODB::adLockOptimistic,ADODB::adCmdText);
		if(!pRst->EndOfFile)
		{
			minput.YearStart = pRst->Fields->Item["startyear"]->Value;
			minput.YearEnd = pRst->Fields->Item["endyear"]->Value;
			minput.MonthStart = pRst->Fields->Item["startmonth"]->Value;
			minput.MonthEnd = pRst->Fields->Item["endmonth"]->Value;
			minput.DayStart = pRst->Fields->Item["startday"]->Value;
			minput.DayEnd = pRst->Fields->Item["endday"]->Value;
		}
		pRst->Close();
	}
	
	catch(_com_error &er)
	{
		cout << "error in GlacialSnow.dll:Read from hydrousepara"<<endl; 
		cout<<er.ErrorMessage() << endl;
		cout << er.Source() << endl;
		cout<<er.Description()<<endl;
		return S_FALSE;
	}

	minput.initialzie();//wh

	return S_OK;
}

//对于每条河段该函数只执行一次。
STDMETHODIMP CSRM::SnowInitialize(FLOAT myParaX, FLOAT myParaY, ULONGLONG RegionIndex, LONG Length, ULONGLONG Value, FLOAT A, FLOAT Havg)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	// TODO: 在此添加实现代码
	minput.jx = myParaX;
	minput.wy = myParaY;
	minput.RegionIndex = RegionIndex;
	minput.Length = Length;
	minput.A = A;
	minput.Havg = Havg;

	if(Value<0.5)
	{
		minput.Value = 0;//如果Value是0时，发现minput可能是一个非常大的数，所以为了保险，特殊处理下。
	}
	else
	{
		minput.Value = Value;
	}

	minput.GetDataByTS();
	minput.ReadDayValue();
	minput.ReadHourValue();
	
	/*ofstream out_Prunoff("d:\\c++\\Prunoff.txt",ios::app);	
	out_Prunoff<<setw(10)<<"RegionIndex"<<setw(10)<<"BSLength"<<setw(10)<<"BSValue"<<setw(10)<<"hour"<<setw(15)<<"HourRain_1"<<setw(10)<<"Thour"\
			<<setw(10)<<"SCA"<<setw(10)<<"A"<<setw(15)<<"W"<<setw(15)<<"HourRain"<<endl;*/

	return S_OK;
}

//每条河段的每个时间步长内执行一次
STDMETHODIMP CSRM::SnowCalc(LONG Houroffset, FLOAT* HourRain)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	// TODO: 在此添加实现代码
	minput.ToWaterYield(Houroffset,HourRain);//该函数对HourRain直接改变，因此不需要返回什么值。
	return S_OK;
}

//全局运行一次，释放堆空间。
STDMETHODIMP CSRM::SRMFinalize(void)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	// TODO: 在此添加实现代码
	minput.finalize();

	return S_OK;
}

//每算完一条河段执行一次
STDMETHODIMP CSRM::ReleaseHeap(void)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	// TODO: 在此添加实现代码
	minput.Deleteall();
	return S_OK;
}
