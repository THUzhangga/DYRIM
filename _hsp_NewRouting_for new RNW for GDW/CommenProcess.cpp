#include "CommenProcess.h"


CommenProcess::CommenProcess(void)
:tempCom(VT_EMPTY)
{
}

CommenProcess::~CommenProcess(void)
{
}

//MPI�����Ĳ��ֳ�ʼ��
void CommenProcess::ProcessInitialize(int worldsize,int Rank,CString user,CString password,CString sid)
{

	WorldSize = worldsize - 2;//wh,�����㡱�ڵ�ĸ���,have minused rank0 and rank1

	MPI_Get_processor_name(processor_name,&namelen);
	rank = Rank;//��ǰ���̺�

	TransferProcessRank = this->WorldSize + 1;//��ת���̵Ľ��̺ţ�master���̺�slave���̶���Ҫ֪����2008.3.24
	
	//Oracle�����ַ���
	SParameter.CSUser = user;
	SParameter.CSPassword = password;
	SParameter.CSDatasource = sid;

	cout<<"{"<<processor_name<<","<<rank<<"}"<<endl;
}

//�ⲿ��Ӧ�÷ŵ�ģ�͵Ĵ�����
//wh��20080323,�����ݿ��ж�ȡ����
void CommenProcess::ReadHydroUsePara(void)
{

	//��Oracle���ݿ�
	_bstr_t bSQL;
	CString CSSQL;
	CString cSQL;
	CSSQL.Format("Provider=OraOLEDB.Oracle.1;Persist Security Info=False;User ID=%s;Data Source=%s;Extended Properties=''",SParameter.CSUser,SParameter.CSDatasource);
	//CSSQL.Format("Provider=MSDAORA;User ID=xhj;Data Source=dwxhj");  
	bSQL=CSSQL.GetString();
	try
	{

		pCnn.CreateInstance(__uuidof(ADODB::Connection));//��������

		pCnn->CursorLocation=ADODB::adUseClient;//20110113,xiaofc, Oracle OLE DB X64λ BUG: 6623430, ֻ��ʹ���û����α�;http://forums.oracle.com/forums/thread.jspa?threadID=488292

		pCnn->Open(bSQL,SParameter.CSUser.GetString(),SParameter.CSPassword.GetString(),0);

	}
	
	catch(_com_error e)
	{
		cout<<"Failture Of Opening Oracle On Rank."<<rank<<endl;//�������ݿ�����ʧ�ܵľ���.

		//wh�ڿ���̨��CATCH_ERROR��ʾ����������䣬��Ϊ����MFC�������Ը�Ϊ�±ߵ�cout�����
		cout<<e.Error()<<endl;
		cout<<e.ErrorMessage()<<endl;
	    cout<<(LPCSTR)e.Source()<<endl;        
		cout<<(LPCSTR)e.Description()<<endl;  

		MPI_Finalize();
		exit(0);
	}
	cout<<1<<endl;
	//����hydrousepara��,�����ж�ȡ����
	ADODB::_RecordsetPtr pRstParam;
	pRstParam.CreateInstance(__uuidof(ADODB::Recordset));
	pRstParam->CursorLocation = ADODB::adUseClient;

	_variant_t tmp;
	cSQL.Format("select * from HYDROUSEPARA");

	try
	{
		pRstParam->Open(cSQL.GetString(),(ADODB::_Connection*)pCnn,ADODB::adOpenStatic,ADODB::adLockOptimistic,ADODB::adCmdText);
	}
	catch(_com_error e)
	{
		cout<<"Failture Of Opening Table HydroUsePara."<<rank<<endl;

		cout<<e.Error()<<endl;
		cout<<e.ErrorMessage()<<endl;
	    cout<<(LPCSTR)e.Source()<<endl;        
		cout<<(LPCSTR)e.Description()<<endl;  

		MPI_Finalize();
		exit(-1);
	}
	cout<<2<<endl;
	try
	{
		SParameter.RegionSystem = pRstParam->Fields->Item["RegionSystem"]->Value;//20130904, shy, Regionindex��λ���ɱ䣬100 or 1000
		SParameter.CompRegion = pRstParam->Fields->Item["calcregion"]->Value;//20140514, shy

		//����һ��c++��int��ȡvarchar2��������᲻�����
		/*ofstream myFile333;
	    myFile333.open("example333.txt",ios::trunc);
        if (myFile333.is_open()) {
            myFile333 <<"CompRegion="<<SParameter.CompRegion<<" .\n";
           }
		myFile333.close();*/
		SParameter.CompValue = pRstParam->Fields->Item["calcoutvalue"]->Value;//20140514, shy
		SParameter.CompLength = pRstParam->Fields->Item["calcoutlength"]->Value;//20140514, shy


		//ֻ�����ؽ��̶�
		if(rank==0)
		{
			SParameter.TaskUnitSize = pRstParam->Fields->Item["maxbranches"]->Value;
			SParameter.MinTaskUnitSize = pRstParam->Fields->Item["minbranches"]->Value;

			tmp = pRstParam->Fields->Item["calcregion"]->Value;//wh��Ϊ�������"��RegionIndex"������������23����ʵ�ʼ�������Ϊ23,2301��230100��2302�ȵȡ�
			if(tmp.vt != VT_NULL)
				SParameter.CompRegions.Format((LPCTSTR)(_bstr_t)tmp.bstrVal);

			tmp = pRstParam->Fields->Item["sccd"]->Value;
			if(tmp.vt != VT_NULL)
				SParameter.sccd.Format((LPCTSTR)(_bstr_t)tmp.bstrVal);//wh���������

		}
		
		//ֻ����ת���̶�
		else if(rank == TransferProcessRank)
		{
			SParameter.HourStart = pRstParam->Fields->Item["starthouroffset"]->Value;
			SParameter.NumofHours = (long)pRstParam->Fields->Item["endhouroffset"]->Value - SParameter.HourStart;
			SParameter.PrepareHours = pRstParam->Fields->Item["preparehours"]->Value;
			SParameter.MSTEP = pRstParam->Fields->Item["timestep"]->Value;//wh,ʱ�䲽������

			/**/Steps = SParameter.NumofHours * 60 / SParameter.MSTEP;
			/**/lTimeInterval = SParameter.MSTEP * 60;//һ�������ж����롣

		}

		//������̶�
		else
		{
			tmp = pRstParam->Fields->Item["sccd"]->Value;
			if(tmp.vt != VT_NULL)
				SParameter.sccd.Format((LPCTSTR)(_bstr_t)tmp.bstrVal);//wh���������

			SParameter.HourStart = pRstParam->Fields->Item["starthouroffset"]->Value;
			SParameter.NumofHours = (long)pRstParam->Fields->Item["endhouroffset"]->Value - SParameter.HourStart;
			SParameter.PrepareHours = pRstParam->Fields->Item["preparehours"]->Value;
			SParameter.StatusTime = pRstParam->Fields->Item["statustime"]->Value;//wh,20080406,�����洢ÿ���೤ʱ����status��д��һ������

			SParameter.MSTEP = pRstParam->Fields->Item["timestep"]->Value;//wh,ʱ�䲽������
			SParameter.bSaveAllDischarge = pRstParam->Fields->Item["savealldischarge"]->Value;//wh,�Ƿ���discharge���б������кӶεļ�����
			SParameter.bCalRsvUp = pRstParam->Fields->Item["calcrsvup"]->Value;
			tmp = pRstParam->Fields->Item["raintype"]->Value;
			if(tmp.vt != VT_NULL)
				SParameter.sRainType.Format((LPCTSTR)(_bstr_t)tmp.bstrVal);

			/**/Steps = SParameter.NumofHours * 60 / SParameter.MSTEP;
			/**/lTimeInterval = SParameter.MSTEP * 60;//һ�������ж����롣

			
			//////////////////////////////////////////////////�����Ǿ���ģ�͵Ĳ���////////////////////////////////////////////////////////
			//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
			
			/*ltj�ƺ�ˮɳ���ģ�ͺ�AVPM��������ʴģ��*/
			SParameter.bSaveGravityEvents = pRstParam->Fields->Item["savegravityerosion"]->Value;;
			SParameter.bSaveFlowPattern = pRstParam->Fields->Item["saveflowpattern"]->Value;
			
			SParameter.iCalcSediTrans = pRstParam->Fields->Item["calcseditrans"]->Value;

			SParameter.fAlphaErosion = pRstParam->Fields->Item["flushcoef"]->Value;
			SParameter.fAlphaDeposition = pRstParam->Fields->Item["depositioncoef"]->Value;
			SParameter.iCalGravityErosion = pRstParam->Fields->Item["calcgravityerosion"]->Value;
			SParameter.GravityErosioinP2 = pRstParam->Fields->Item["gravityerosionratex"]->Value; // WHAT??? by weihong
			SParameter.SoilErosionEquation = pRstParam->Fields->Item["soilerosionequation"]->Value;//wh,20080803

			tmp = pRstParam->Fields->Item["seditranscapf"]->Value;
			if(tmp.vt != VT_NULL)
				SParameter.SediTransCapF.Format((LPCTSTR)(_bstr_t)tmp.bstrVal);

			tmp = pRstParam->Fields->Item["EFormula"]->Value;// coefficients needed to construct "vapotranspiration"
			if(tmp.vt != VT_NULL)
				SParameter.emethod.Format((LPCTSTR)(_bstr_t)tmp.bstrVal);

			SParameter.thetab = pRstParam->Fields->Item["thetab"]->Value;
			SParameter.thetaw = pRstParam->Fields->Item["thetaw"]->Value;
			SParameter.N = pRstParam->Fields->Item["N"]->Value;
			SParameter.E0_a = pRstParam->Fields->Item["E0_a"]->Value;

			SParameter.UpInitWaterContent = pRstParam->Fields->Item["UpWaterContent"]->Value;
			SParameter.MidInitWaterContent = pRstParam->Fields->Item["MidWaterContent"]->Value;
			SParameter.DownInitWaterContent = pRstParam->Fields->Item["DownWaterContent"]->Value;

			pRstParam->Close();
			
			/*********************************************/
			
			/***************2008.11.5,WH,�°���ˮ��ģ��****************/
			cSQL.Format("select * from XAJUSEPARA where sccd=%s",SParameter.sccd);
			pRstParam->Open(cSQL.GetString(),(ADODB::_Connection*)pCnn,ADODB::adOpenStatic,ADODB::adLockOptimistic,ADODB::adCmdText);

			XAJ.WUM = pRstParam->Fields->Item["WUM"]->Value;
			XAJ.WLM = pRstParam->Fields->Item["WLM"]->Value;
			XAJ.WDM = pRstParam->Fields->Item["WDM"]->Value;
			XAJ.C = pRstParam->Fields->Item["C"]->Value;
			XAJ.B = pRstParam->Fields->Item["B"]->Value;
			XAJ.IMP = pRstParam->Fields->Item["IMP"]->Value;
			XAJ.SM = pRstParam->Fields->Item["SM"]->Value;
			XAJ.EX = pRstParam->Fields->Item["EX"]->Value;
			XAJ.KG = pRstParam->Fields->Item["KG"]->Value;
			XAJ.KSS = pRstParam->Fields->Item["KSS"]->Value;
			XAJ.KKG = pRstParam->Fields->Item["KKG"]->Value;
			XAJ.KKSS = pRstParam->Fields->Item["KKSS"]->Value;

			XAJ.WU0 = pRstParam->Fields->Item["WU0"]->Value;
			XAJ.WL0 = pRstParam->Fields->Item["WL0"]->Value;
			XAJ.WD0 = pRstParam->Fields->Item["WD0"]->Value;
			XAJ.S0 = pRstParam->Fields->Item["S0"]->Value;
			XAJ.QRS0 = pRstParam->Fields->Item["QRS0"]->Value;
			XAJ.QRSS0 = pRstParam->Fields->Item["QRSS0"]->Value;
			XAJ.QRG0 = pRstParam->Fields->Item["QRG0"]->Value;
			/**********************************************************/
			//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
			//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

		}//end if(rank>1)
		cout<<3<<endl;
		pRstParam->Close();//��Ϊ���ؽ���ҲҪ�رո����ӣ����Էŵ�����

	}//end try

	catch(_com_error e)
	{
		cout<<"Failture Of Reading From Table HydroUsePara."<<rank<<endl;

		cout<<e.Error()<<endl;
		cout<<e.ErrorMessage()<<endl;
	    cout<<(LPCSTR)e.Source()<<endl;        
		cout<<(LPCSTR)e.Description()<<endl;  

		MPI_Finalize();
		exit(0);
	}
}

//��RegionConnection�������ݶ��뵽���˽�б����У��ú�������������̶�ִ�С�
void CommenProcess::ReadRegionConnection(void)
{
	//������������Ϣд���ڴ�
	//ֻ��Ҫ�ϼ���RegionGrade,Regionindex����һ����BSValue,BSlength
	//�������BSCode�ṹ��
	_bstr_t bSQL;
	bSQL = "select * from regionconnection";
	try
	{
		pRst.CreateInstance(__uuidof(ADODB::Recordset));
		pRst->Open(bSQL,(ADODB::_Connection*)pCnn,ADODB::adOpenStatic,ADODB::adLockOptimistic,ADODB::adCmdText);

		RCCount = pRst->RecordCount;
		RegionConnection = new BSCode[RCCount];

		for(GradeTwoLoop=0;GradeTwoLoop<RCCount;GradeTwoLoop++)
		{
			RegionConnection[GradeTwoLoop].Length=pRst->Fields->Item["ToLength"]->Value;

			tempCom=pRst->Fields->Item["ToValue"]->Value;
			RegionConnection[GradeTwoLoop].Value=tempCom.ullVal;

			RegionConnection[GradeTwoLoop].RegionGrade=pRst->Fields->Item["RegionGrade"]->Value;

			tempCom=pRst->Fields->Item["RegionIndex"]->Value;
			RegionConnection[GradeTwoLoop].RegionIndex=tempCom.ullVal;

			pRst->MoveNext();
		}
		pRst->Close();
	}
	catch(_com_error e)
	{
		cout<<"Failture Of Reading From Table RegionConnection."<<rank<<endl;

		cout<<e.Error()<<endl;
		cout<<e.ErrorMessage()<<endl;
	    cout<<(LPCSTR)e.Source()<<endl;        
		cout<<(LPCSTR)e.Description()<<endl;  

		MPI_Finalize();
		exit(0);
	}
}


//��FilePath���������ļ��ж�ȡ���������˽�б����У��ú�������������̶�ִ�С�
//void CommenProcess::ReadFilePath(void)
//{
//	//�õ�FilePath.ini���ļ�·��
//	char path[MAX_PATH];
//	::GetModuleFileName(NULL,path,MAX_PATH);
//	CString m_strFilePath(path);
//	m_strFilePath = m_strFilePath.Left( m_strFilePath.ReverseFind('\\') + 1 );
//	m_strFilePath = m_strFilePath + "FilePath.ini";
//
//	cout<<m_strFilePath<<endl;
//
//	//�õ�������Access���ݿ�·��
//	char temp[100];
//	::GetPrivateProfileString("RAINFALL","PathName","Error RAINFALL PathName",temp,sizeof(temp),m_strFilePath);
//	SParameter.m_RainFile.Format("%s",temp);
//	::ZeroMemory(temp,sizeof(temp));
//
//	//�õ�ģ�ͼ����ʱ����Ϣ
//	SParameter.HourStart = ::GetPrivateProfileInt("TIME","HourOffset",-1,m_strFilePath);//����1950���Сʱ��
//	SParameter.NumofHours = ::GetPrivateProfileInt("TIME","NumOfHours",-1,m_strFilePath);//�����������Сʱ��
//	SParameter.PrepareHours = ::GetPrivateProfileInt("TIME","PrepareHours",-1,m_strFilePath);
//
//	//2008.0217
//	Steps = SParameter.NumofHours * 60 / MSTEP;
//	lTimeInterval = MSTEP * 60;//һ�������ж����롣
//	
//	//�õ�Oracle���ݿ��½�ַ���
//	::GetPrivateProfileString("SYSTEM","OraSID","error OraSID",temp,sizeof(temp),m_strFilePath);
//	SParameter.CSDatasource.Format("%s",temp);
//	::ZeroMemory(temp,sizeof(temp));
//	::GetPrivateProfileString("SYSTEM","OraUser","error OraUser",temp,sizeof(temp),m_strFilePath);
//	SParameter.CSUser.Format("%s",temp);
//	::ZeroMemory(temp,sizeof(temp));
//	::GetPrivateProfileString("SYSTEM","OraPsw","error OraPsw",temp,sizeof(temp),m_strFilePath);
//	SParameter.CSPassword.Format("%s",temp);
//	::ZeroMemory(temp,sizeof(temp));
//
//	//Mpi���ؽڵ�ü����������Ӷ���Ŀ�����½�
//	SParameter.TaskUnitSize=::GetPrivateProfileInt("SYSTEM","MaxBranches",-1,m_strFilePath);
//	SParameter.MinTaskUnitSize=::GetPrivateProfileInt("SYSTEM","MinBranches",-1,m_strFilePath);
//
//	//20060906,����������������
//	::GetPrivateProfileString("SYSTEM","CalcRegion","",temp,sizeof(temp),m_strFilePath);
//	SParameter.CompRegions.Format("%s",temp);
//
//	//20060327,������,�Ƿ񱣴�������ʴ�¼�
//	SParameter.bSaveGravityEvents=::GetPrivateProfileInt("SYSTEM","SaveGravityErosion",0,m_strFilePath);
//
//	//20070622,xiaofc, whether save the values of flow pattern to table discharge.(B,H,v)
//    SParameter.bSaveFlowPattern=::GetPrivateProfileInt("SYSTEM","SaveFlowPattern",1,m_strFilePath);
//
//	//20080216,wh,whether save discharge of all segments
//	SParameter.bSaveAllDischarge = ::GetPrivateProfileInt("SYSTEM","SaveAllDischarge",0,m_strFilePath);
//
//	//The model for flow routing.
//	::GetPrivateProfileString("MODEL","RoutingMethod","error RoutingMethod",temp,sizeof(temp),m_strFilePath);
//	SParameter.sRoutingMethod.Format("%s",temp);
//	ZeroMemory(temp,sizeof(temp));
//	
//	//�Ƿ����ˮ�����ϵĲ���
//	SParameter.bCalRsvUp=::GetPrivateProfileInt("MODEL","CalcRsvUP",0,m_strFilePath);
//
//	//20070608,xiaofc,�Ƿ������ɳ���ͣ��ݼ�Ϊ�Ƿ񱣴�
//	SParameter.iCalcSediTrans=::GetPrivateProfileInt("MODEL","CalcSediTrans",0,m_strFilePath);
//
//	//20060317,������,�ָ�����ϵ��
//	::GetPrivateProfileString("MODEL","FlushCoef","-1",temp,sizeof(temp),m_strFilePath);
//	SParameter.fAlphaErosion=atof(temp);
//	ZeroMemory(temp,sizeof(temp));
//	::GetPrivateProfileString("MODEL","DepositionCoef","-1",temp,sizeof(temp),m_strFilePath);
//	SParameter.fAlphaDeposition=atof(temp);
//	ZeroMemory(temp,sizeof(temp));
//	
//	//20060316,������,�����Ƿ����������ʴ�Ĳ���
//	SParameter.iCalGravityErosion=::GetPrivateProfileInt("MODEL","CalcGravityErosion",-1,m_strFilePath);
//	
//	//20060324,������,����������ʴ������Χ�ĸ���
//	::GetPrivateProfileString("MODEL","GravityErosionRateX","-1",temp,sizeof(temp),m_strFilePath);
//	SParameter.GravityErosioinP2=atof(temp);
//	ZeroMemory(temp,sizeof(temp));
//
//	//���������ͣ�ʱ�λ����ս�����
//	::GetPrivateProfileString("MODEL","RainType","-1",temp,sizeof(temp),m_strFilePath);
//	SParameter.sRainType.Format("%s",temp);
//	ZeroMemory(temp,sizeof(temp));
//
//    cout<<"end of read filepath"<<endl;
//}
