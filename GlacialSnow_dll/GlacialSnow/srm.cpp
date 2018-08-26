// SRM.cpp : CSRM ��ʵ��

#include "stdafx.h"
#include "SRM.h"
#include <fstream>
#include <iomanip>//�����setwҪ�õ������ҵ���std�ռ�
using namespace std;


// CSRM

//��wateryield.dll�е�WaterBasin���е�Initialize��������
//�ú����ڲ��м����ʼ��ֻ��ִ��һ�Σ�ҲӦ����ˡ�
STDMETHODIMP CSRM::OpenOracle(/*_Connection** pCnn*/BSTR user, BSTR password, BSTR sid)//����_Connection**��һ���ǲ���ʹ
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	 //TODO: �ڴ����ʵ�ִ���
	//::CoInitialize(NULL);
	CString User(user),Password(password),Sid(sid);
	//ADODB::_ConnectionPtr pCnn;	
	CString CSSQL;
	_bstr_t bSQL;
	
	CSSQL.Format(_T("Provider=OraOLEDB.Oracle.1;Persist Security Info=False;User ID=%s;Data Source=%s;Extended Properties=''"),User,Sid);
	bSQL=CSSQL.GetString();
	try
	{
		minput.pConn.CreateInstance(__uuidof(ADODB::Connection));//��������
		minput.pConn->Open(bSQL,User.GetString(),Password.GetString(),0);
		//minput.pConn = pCnn;
	}
	catch(_com_error e)
	{
		//�������ݿ�����ʧ�ܵľ���.
		cout<<"Failture Of Opening Oracle In GlacialSnow.dll."<<endl;
		e.ErrorMessage();
		e.Description();
		return S_FALSE;
	}

	//minput.pConn = *pCnn;//ע��˷�ʽ

    //Ϊ�˵õ��������ֹʱ�䣬�ص���hydrousepara���������������
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

//����ÿ���Ӷθú���ִֻ��һ�Ρ�
STDMETHODIMP CSRM::SnowInitialize(FLOAT myParaX, FLOAT myParaY, ULONGLONG RegionIndex, LONG Length, ULONGLONG Value, FLOAT A, FLOAT Havg)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	// TODO: �ڴ����ʵ�ִ���
	minput.jx = myParaX;
	minput.wy = myParaY;
	minput.RegionIndex = RegionIndex;
	minput.Length = Length;
	minput.A = A;
	minput.Havg = Havg;

	if(Value<0.5)
	{
		minput.Value = 0;//���Value��0ʱ������minput������һ���ǳ������������Ϊ�˱��գ����⴦���¡�
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

//ÿ���Ӷε�ÿ��ʱ�䲽����ִ��һ��
STDMETHODIMP CSRM::SnowCalc(LONG Houroffset, FLOAT* HourRain)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	// TODO: �ڴ����ʵ�ִ���
	minput.ToWaterYield(Houroffset,HourRain);//�ú�����HourRainֱ�Ӹı䣬��˲���Ҫ����ʲôֵ��
	return S_OK;
}

//ȫ������һ�Σ��ͷŶѿռ䡣
STDMETHODIMP CSRM::SRMFinalize(void)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	// TODO: �ڴ����ʵ�ִ���
	minput.finalize();

	return S_OK;
}

//ÿ����һ���Ӷ�ִ��һ��
STDMETHODIMP CSRM::ReleaseHeap(void)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	// TODO: �ڴ����ʵ�ִ���
	minput.Deleteall();
	return S_OK;
}
