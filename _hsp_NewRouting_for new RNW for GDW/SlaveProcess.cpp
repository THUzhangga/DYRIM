#include "SlaveProcess.h"
#include "functions.h"
#undef SEEK_SET
#undef SEEK_END
#undef SEEK_CUR
#include "C:\Program Files\MPICH2\include\mpi.h"
#include <time.h>
#include <assert.h>
#include <atlbase.h>

#include "HighSlopeRunoff.h"
//������������ɳģ��dll�������
#include "E:\models\DYRIM_raw\WaterYield_dll20130915 for GDW\WaterYield\WaterYield.h"
#include "E:\models\DYRIM_raw\WaterYield_dll20130915 for GDW\WaterYield\WaterYield_i.c"
#include "E:\models\DYRIM_raw\XAJ_dll\�°���ģ��\�°���ģ��\My.h"
#include "E:\models\DYRIM_raw\XAJ_dll\�°���ģ��\�°���ģ��\My_i.c"

CComQIPtr <IWaterBasin,&IID_IWaterBasin> spWaterBasin;//ltjˮɳ���ģ��
CComQIPtr <IMy,&IID_IMy> spXAJmodel;//��ˮԴ�°���ģ��
HighSlopeRunoff HSP;//�߱��²���ģ��

SlaveProcess::SlaveProcess(void)
{	
	MLTJYR = "NONE";
	MXAJ = "NONE";  
	MSRM = "NONE";  
	MHSP = "NONE";
}

SlaveProcess::~SlaveProcess(void)
{
}

//�������һЩ�����ĳ�ʼ������֯ˮ��ڵ�������Ϊ�ܶ����ֻ�д��ı���ȡ�����ֵ����˲��ܷŵ����캯����
//ÿ��������̶�����һ��
void SlaveProcess::SlaveProcessInitialize(void)
{
	///////////ģ�ͼ�����ת������ʼ��///////////
	ZeroSerial = new float[Steps+1];   ZeroFill(ZeroSerial,Steps+1);
	pQUpRegion = new float[Steps+1];   ZeroFill(pQUpRegion,Steps+1);
	pSUpRegion = new float[Steps+1];   ZeroFill(pSUpRegion,Steps+1);
	pDUpRegion = new float[Steps+1];   ZeroFill(pDUpRegion,Steps+1);


	//////////////���ݿ������ʼ��/////////////
	pRstData.CreateInstance(__uuidof(ADODB::Recordset));//ָ��Discharge��
	pRstQ.CreateInstance(__uuidof(ADODB::Recordset));//ָ��Discharge��
	pRstRsv.CreateInstance(__uuidof(ADODB::Recordset));
	pRstIndicator.CreateInstance(__uuidof(ADODB::Recordset));
	pRstGravityEvents.CreateInstance(__uuidof(ADODB::Recordset));


	//wh added,2008,����BasinModel����ģ�ͳ�ʼ��
	/********************************************************************************************************************/
	CString CSSQL;
	_bstr_t bSQL;

	CSSQL.Format("Select * from basinmodel where sccd=%s order by regionindex desc",SParameter.sccd);//�Ӵ�С����
	bSQL = CSSQL.GetString();
	int i=0;//��ͬregionindex�ĸ���
	int j=0;//�������洢ģ��
	try
	{
		pRstIndicator->Open(bSQL,(ADODB::_Connection*)pCnn,ADODB::adOpenStatic,ADODB::adLockOptimistic,ADODB::adCmdText);

		RowCount = pRstIndicator->RecordCount;
		ColumnCount = 4;//wh,�Ժ�ģ�����Ӻ��ֵҪ�ġ�

		while(!pRstIndicator->EndOfFile)
		{
			tempCom = pRstIndicator->Fields->Item["regionindex"]->Value;
			BasinModelIndex[i] = tempCom.ullVal;

			vtmp = pRstIndicator->Fields->Item["m_runoff"]->Value;
			if(vtmp.vt != VT_NULL)
				BasinModel[i][j].Format((LPCTSTR)(_bstr_t)vtmp.bstrVal);

			j++;

			vtmp = pRstIndicator->Fields->Item["m_confluence"]->Value;
			if(vtmp.vt != VT_NULL)
				BasinModel[i][j].Format((LPCTSTR)(_bstr_t)vtmp.bstrVal);

			j++;

			vtmp = pRstIndicator->Fields->Item["m_sedimentyield"]->Value;
			if(vtmp.vt != VT_NULL)
				BasinModel[i][j].Format((LPCTSTR)(_bstr_t)vtmp.bstrVal);

			j++;

			vtmp = pRstIndicator->Fields->Item["m_snow"]->Value;
			if(vtmp.vt != VT_NULL)
				BasinModel[i][j].Format((LPCTSTR)(_bstr_t)vtmp.bstrVal);

			j=0;

			pRstIndicator->MoveNext();
			i++;
		}
		pRstIndicator->Close();
	}
	catch(_com_error e)
	{
		cout<<"Failture Of Reading From Table BasinModel."<<rank<<endl;

		cout<<e.Error()<<endl;
		cout<<e.ErrorMessage()<<endl;
		cout<<(LPCSTR)e.Source()<<endl;        
		cout<<(LPCSTR)e.Description()<<endl;

		MPI_Finalize();
		exit(0);
	}
	/********************************************************************************************************************/
	//��ȡȫ����������������
	float BasinArea;
	CSSQL.Format("Select catchmentarea from riversegs where regionindex=%d and bsvalue=%d and bslength=%d",SParameter.CompRegion,SParameter.CompValue,SParameter.CompLength);//20140514,shy
	bSQL = CSSQL.GetString();
	try
	{
		pRstIndicator->Open(bSQL,(ADODB::_Connection*)pCnn,ADODB::adOpenStatic,ADODB::adLockOptimistic,ADODB::adCmdText);
		if(!pRstIndicator->EndOfFile)
		{
			BasinArea = pRstIndicator->Fields->Item["catchmentarea"]->Value;
		}
		pRstIndicator->Close();
	}
	catch(_com_error e)
	{
		cout<<"Failture Of Reading CatchmentArea."<<rank<<endl;//20140514,shy

		cout<<e.Error()<<endl;
		cout<<e.ErrorMessage()<<endl;
		cout<<(LPCSTR)e.Source()<<endl;        
		cout<<(LPCSTR)e.Description()<<endl;

		MPI_Finalize();
		exit(0);
	}
	/********************************************************************************************************************/


	///////////////ģ��ѡ���ʼ��//////////////

	//���¶�ӦBasinModel����ʾ��Ӧģ���Ƿ���ڣ�������������Ƿ����Ӹ�ģ�͵�dll,�������һ����ȫ���ӣ�ȱһ���޹ؽ�Ҫ��dll���������в���ȥ//
	//û��dll������ν
	bool FLAG_LTJYR=false; bool FLAG_XAJ=false; 
	bool FLAG_HSP=false;   bool FLAG_SRM=false;
	for(int k=0; k<RowCount; k++)
	{
		if( BasinModel[k][0].MakeLower()=="ltjyr" && FLAG_LTJYR==false )
		{
			MLTJYR = "ltjyr";
			FLAG_LTJYR = true;
		}
		if( BasinModel[k][0].MakeLower()=="xaj" && FLAG_XAJ==false )
		{
			MXAJ = "xaj";
			FLAG_XAJ = true;
		}
		if( BasinModel[k][0].MakeLower()=="hsp" && FLAG_HSP==false )
		{
			MHSP = "hsp";
			FLAG_HSP = true;
		}
		if( BasinModel[k][3].MakeLower()=="srm" && FLAG_SRM==false )
		{
			MSRM = "srm";
			FLAG_SRM = true;
		}
		if(	FLAG_LTJYR && FLAG_XAJ && FLAG_SRM && FLAG_HSP)
			break;
	}

	if(MLTJYR=="NONE" && MXAJ=="NONE" && MHSP=="NONE")
	{
		cout<<"��û��ѡ���κβ���ģ�ͣ�������ָ��."<<endl;
		return;
	}

	if(MLTJYR=="ltjyr")//˵�����ڣ�������
	{
		CString name=this->processor_name;
		hr = spWaterBasin.CoCreateInstance( CLSID_WaterBasin );
		//20101210,xiaofc, Assert is not good, err message should be given
		//assert(SUCCEEDED(hr));
		if(!SUCCEEDED(hr))
		{
			cout<<"'ltjyr'@"<<this->processor_name<<"��ʼ��ʧ�ܡ�\n����ģ��ģ����ش������鲢ע��WaterYield.dll��"<<endl;
			exit(88888);
		}
		hr = spWaterBasin->Initialize(name.AllocSysString(),rank,MSRM.AllocSysString(),SParameter.StatusTime,SParameter.CSUser.AllocSysString(),SParameter.CSPassword.AllocSysString(),SParameter.CSDatasource.AllocSysString(),this->Steps,SParameter.HourStart,SParameter.NumofHours,false,SParameter.sccd.AllocSysString(),SParameter.sRainType.AllocSysString(),SParameter.UpInitWaterContent,SParameter.MidInitWaterContent,SParameter.DownInitWaterContent,SParameter.emethod.AllocSysString(),SParameter.thetab,SParameter.thetaw,SParameter.N,SParameter.E0_a,SParameter.SoilErosionEquation.AllocSysString());
		assert(SUCCEEDED(hr));
	}

	if(MXAJ=="xaj")
	{
		hr = spXAJmodel.CoCreateInstance(CLSID_My);
		//20101210,xiaofc, Assert is not good, err message should be given
		//assert(SUCCEEDED(hr));
		if(!SUCCEEDED(hr))
		{
			cout<<"'xaj'@"<<this->processor_name<<"��ʼ��ʧ�ܡ�\n����ģ��ģ����ش������鲢ע���°���ģ��.dll��"<<endl;
			exit(88888);
		}
		hr = spXAJmodel->Initialize(MSRM.AllocSysString(),BasinArea,XAJ,SParameter.sccd.AllocSysString(),SParameter.CSUser.AllocSysString(),SParameter.CSPassword.AllocSysString(),SParameter.CSDatasource.AllocSysString(),SParameter.sRainType.AllocSysString(),SParameter.HourStart,SParameter.NumofHours,SParameter.MSTEP,this->Steps,SParameter.StatusTime);
		assert(SUCCEEDED(hr));
	}
	if(MHSP=="hsp")
	{
		HSP.Initiallize(pCnn,SParameter.sRainType,SParameter.NumofHours,SParameter.HourStart,SParameter.StatusTime,BasinArea,SParameter.MSTEP,this->Steps);
	}

	///////////////ˮ��ڵ��ʼ��//////////////
	if(SParameter.bCalRsvUp)//�ò��������Ϊ1��ʾ����Ҫ��ˮ�⣬���ǲ�����˼�������������С�
	{
		cout<<"Rerservoir Codes were not well prepared, please contact developer!"<<endl;//�����Ժ�Ҫ����ˮ�����ϻ�����ˮ����̣����д��reservoir����
	}

	//��definednodes���е�RS�ڵ���뵽RsvList������
	else//������ˮ�����ϵĻ�����ˮ�����
	{
		RsvList = NULL;
		RsvList = new List();

		CSSQL.Format("Select * from definednodes where nodetype='RS'");
		bSQL=CSSQL.GetString();
		try
		{
			_variant_t vtmp2;
			pRstIndicator->Open(bSQL,(ADODB::_Connection*)pCnn,ADODB::adOpenStatic,ADODB::adLockOptimistic,ADODB::adCmdText);
			while (!pRstIndicator->EndOfFile) 
			{
				vtmp = pRstIndicator->Fields->Item["BSValue"]->Value;//�������variant���͵ı���������Ҫ�ȴ���vtmp
				vtmp2 = pRstIndicator->Fields->Item["RegionIndex"]->Value;//�������variant���͵ı���������Ҫ�ȴ���vtmp
				RsvList->insert(pRstIndicator->Fields->Item["BSLength"]->Value,vtmp.ullVal,-1,vtmp2.ullVal);//ˮ��û��Sorder������дΪ-1
				pRstIndicator->MoveNext();
			}
			pRstIndicator->Close();
		}

		catch(_com_error e)
		{
			cout<<"Failture Of Making Up RS List."<<rank<<endl;

			cout<<e.Error()<<endl;
			cout<<e.ErrorMessage()<<endl;
			cout<<(LPCSTR)e.Source()<<endl;        
			cout<<(LPCSTR)e.Description()<<endl;

			MPI_Finalize();
			exit(0);
		}
	}
}


//wh,2008���õ��Ӷ�����Parameter���ĸ�RegionIndex�Ĳ���
//�������ȵľ�������ȵģ�û����ȵľ������������һ���ĳ�����
//�������Ϊ�Ӷε�RegionIndex
int SlaveProcess::GetBasinModelRegionIndex(unsigned long long regionindex)
{
	assert(RowCount>0);
	while(1>0)
	{
		for(int i=0; i<RowCount; i++)
		{
			if(regionindex == BasinModelIndex[i])
				return i;
			if(regionindex>BasinModelIndex[i])//��Ϊ��С�����ŵ�
				break;
		}
		regionindex /= SParameter.RegionSystem;//20130904, shy
		//if(regionindex==1)
		if(regionindex<SParameter.CompRegion+1)//shy, regionindexС��CompRegion+1�����ѵ�����,20140514
			break;
	}//end while
	return 0;
}


//�������ѭ���������ؽ��̷��͵���Ϣ��������������غ���ת����ͨ��
void SlaveProcess::RecvMasterProcess(void)
{
	Para mPara;//��ת����
	BSCode mBSCode;
	int TaskLoop;

	//pFlowB��ÿ���������ֻ��Ҫ��һ�����飬����Ҫÿ���յ�һ�������newһ��
	bool BHV_flag=true;

	while(1)
	{
		//////////////////////////////////////////////////////////////////////////
		/////////////////////////�����ؽڵ���պӶ�����///////////////////////////
		//////////////////////////////////////////////////////////////////////////
		CommTime=MPI_Wtime();//ͨѶʱ��
		cout<<"@"<<processor_name<<","<<rank<<",is ready to receive."<<endl;//isǰ��ġ���������ɾ��������Ҫ�ģ��Ǻǡ�
		MPI_Recv(&TaskCount,1,MPI_INT,0,10,MPI_COMM_WORLD,&Status);

		if( TaskCount==0 )
		{
			cout<<"@"<<processor_name<<","<<rank<<",is ready to exit."<<endl;
			break;
		}	
		else
		{
			mTreeNode = new TreeNode[TaskCount];//һ��TreeNode����һ���Ӷ�
			MPI_Recv(mTreeNode,TaskCount*sizeof(TreeNode),MPI_BYTE,0,10,MPI_COMM_WORLD,&Status);
		}
		CommTime=MPI_Wtime()-CommTime;
		cout<<"#"<<processor_name<<","<<rank<<",RCV,"<<TaskCount<<":"<<endl;//��������ĺӶ���
		cout<<"#"<<processor_name<<","<<rank<<",TCM,"<<CommTime<<":"<<endl<<endl;//���������ʱ��


		//////////////////////////////////////////////////////////////////////////
		/////////////////////2008.3.23��������Ӷ�ģ���жϽӿ�////////////////////
		//////////////////////////////////////////////////////////////////////////
		CalTime=MPI_Wtime();//����ʱ�俪ʼ��ʱ

		ModelSelect = GetBasinModelRegionIndex(mTreeNode[TaskCount-1].mBSCode.RegionIndex);//�õ����������������ڵڼ���

		//����ģ���ж�
		if(((BasinModel[ModelSelect][1].MakeLower()=="avpm") || (BasinModel[ModelSelect][0].MakeLower()=="hsp" && HSP.Min.SaveFlowPattern==true)) && BHV_flag)
		{
			BHV_flag=false;

			//��ǰ������̼���ʼ��ֻnewһ��
			pFlowB  = new float[Steps+1];  ZeroFill(pFlowB,Steps+1);
			pFlowH  = new float[Steps+1];  ZeroFill(pFlowH,Steps+1);
			pFlow_v = new float[Steps+1];  ZeroFill(pFlow_v,Steps+1);				
		}

		//////////////////////////////////////////////////////////////////////////
		//////////////�ڵ�Ԫ����ѭ��ǰ����ת���ǵ�ָ��������ٿռ�////////////////
		//////////////////////////////////////////////////////////////////////////
		if(BasinModel[ModelSelect][0].MakeLower()=="hsp")
		{
			//wh:ÿ�õ�һ�������Ҫnewһ�Σ���Ϊÿ����������Ӷ����Ƕ�̬��ȷ���ġ�
			pDout = new float* [TaskCount]; 
			pQout = new float* [TaskCount];
		}
		else
		{
			pQin = new float* [TaskCount];  pQout= new float* [TaskCount];
			pSin = new float* [TaskCount];  pSout= new float* [TaskCount];
			pWLM = new float* [TaskCount];  pWRM = new float* [TaskCount];
		}

		CalTime=MPI_Wtime()-CalTime;//����ʱ�����
		cout<<"#"<<processor_name<<","<<rank<<",TCL,"<<CalTime<<":"<<endl;

		//////////////////////////////////////////////////////////////////////////
		//////////////////////////��ÿһ������ĺӶν��м���//////////////////////
		//////////////////////////////////////////////////////////////////////////
		for(TaskLoop=0; TaskLoop<TaskCount; TaskLoop++)
		{
			CalTime=MPI_Wtime();//����ʱ�俪ʼ��ʱ

			mBSCode=mTreeNode[TaskLoop].mBSCode;
			mPara=mTreeNode[TaskLoop].mPara;

			if(BasinModel[ModelSelect][0].MakeLower()=="hsp")
			{
				pDout[TaskLoop]= new float[Steps+1]; pQout[TaskLoop]= new float[Steps+1];
				::ZeroFill(pDout[TaskLoop],Steps+1); ::ZeroFill(pQout[TaskLoop],Steps+1);
			}
			else
			{
				pQin[TaskLoop] = new float[Steps+1];   pSin[TaskLoop] = new float[Steps+1];
				pQout[TaskLoop]= new float[Steps+1];   pSout[TaskLoop]= new float[Steps+1];
				pWLM[TaskLoop] = new float[Steps+1];   pWRM[TaskLoop] = new float[Steps+1];

				//wh:��Ҫ��ʼ��������new������ÿ��ֵ���ɿأ����°���ģ�ͼ���������û����ɳ��pSout�����ֵ�����Խ�磬����д�����ݿ⡣
				::ZeroFill(pQin[TaskLoop] ,Steps+1);   ::ZeroFill(pSin[TaskLoop] ,Steps+1);
				::ZeroFill(pQout[TaskLoop],Steps+1);   ::ZeroFill(pSout[TaskLoop],Steps+1);
				::ZeroFill(pWLM[TaskLoop] ,Steps+1);   ::ZeroFill(pWRM[TaskLoop] ,Steps+1);
			}

			Parent=TaskLoop;
			Boy=-1;//ȡQBoy QGirl,��-1,�����ں���֪����û�ҵ�
			Girl=-1;

			CalTime=MPI_Wtime()-CalTime;//����ʱ�����
			cout<<"#"<<processor_name<<","<<rank<<",TCL,"<<CalTime<<":Initialization"<<endl;

			//�����ˮ��ڵ�
			if(RsvList->find(mBSCode.Length,mBSCode.Value,mBSCode.RegionIndex))
			{
				CalTime=MPI_Wtime();//����ʱ�俪ʼ��ʱ
				ReservoirSegment(mBSCode,TaskLoop);
				CalTime=MPI_Wtime()-CalTime;//����ʱ�����
				cout<<"#"<<processor_name<<","<<rank<<",TCL,"<<CalTime<<":Reservoir"<<endl;
			}
			else
			{
				//20090918,xiaofc,runoffcalc��������ݿ�϶࣬Ӧ���ڲ�������ʱ
				//����WaterYield��Դ���뵥���޸�
				RunoffCalc(mBSCode,mPara,TaskLoop);//������ɳ����//wh,�Ѿ�������ģ��

				CalTime=MPI_Wtime();//����ʱ�俪ʼ��ʱ

				//2008.2.17,wh,������ת���̣�������Ҫ�������кӶ�������Ϣʱ��ͨ����Ϣ�����������ڵ����������
				if(mTreeNode[TaskLoop].StralherOrder>1)
				{
					for(int j=0;j<TaskLoop;j++)//find which is boy and girl	
					{			
						if((mTreeNode[j].mBSCode.Length==mBSCode.Length+1) && (mTreeNode[j].mBSCode.Value==mBSCode.Value<<1)) { Boy=j;}
						else if((mTreeNode[j].mBSCode.Length==mBSCode.Length+1) && (mTreeNode[j].mBSCode.Value==(mBSCode.Value<<1)+1)) { Girl=j;}		
					}
					if(Boy==-1 && Girl==-1)//��Ҫȥ������
						this->NoChild(mBSCode);

					if((Boy==-1 && Girl!=-1) || (Boy!=-1 && Girl==-1))
						this->OneChild(mBSCode,Boy,Girl);
				}

				ConfluenceCalc(TaskLoop,mBSCode,mPara);//�������㣬�ھ�ȷ��ɢ�����м�����������ʴ���ӿ�ˮ����ٵȡ�

				CalTime=MPI_Wtime()-CalTime;//����ʱ�����
				cout<<"#"<<processor_name<<","<<rank<<",TCL,"<<CalTime<<":Confluence"<<endl;
			}//end else


			/***********************************************************�������ݿ⼰��ת���̲���*******************************************************/
			DBUpdateTime=MPI_Wtime();//д�����ݿ�ʱ�俪ʼ��ʱ

			//wh,�����ѡ��avpm������������ʴҲ����������㣬����Ҳ����Ȼ���ñ���
			if(BasinModel[ModelSelect][1].MakeLower()=="avpm" && SParameter.bSaveGravityEvents)//����������ʴ�¼�
				SaveGravityEvents(TaskLoop);

			WriteToDischarge(TaskLoop);//�������Ͳ�ɳ���д��discharge��

			DBUpdateTime=MPI_Wtime()-DBUpdateTime;//���ݿ�д��ʱ�����
			cout<<"#"<<processor_name<<","<<rank<<",TDB,"<<DBUpdateTime<<":"<<endl;
			/******************************************************************************************************************************************/

			//����һ���Ӷ�,��ͬģ���ںӶμ䴫�ݵı�����һ����ͬ�����յ���ȻҲ�Ͳ�ͬ
			if(BasinModel[ModelSelect][0].MakeLower()=="hsp")
			{
				if(Boy!=-1)
				{
					delete[] pQout[Boy];  pQout[Boy]=NULL;
					delete[] pDout[Boy];  pDout[Boy]=NULL;	
				}
				if(Girl!=-1)
				{
					delete[] pQout[Girl]; pQout[Girl]=NULL;
					delete[] pDout[Girl]; pDout[Girl]=NULL;	
				}
			}
			else
			{
				delete[] pQin[Parent];  pQin[Parent]=NULL;
				delete[] pSin[Parent];  pSin[Parent]=NULL;
				delete[] pWLM[Parent];  pWLM[Parent]=NULL;
				delete[] pWRM[Parent];  pWRM[Parent]=NULL;
				if(Boy!=-1)
				{
					delete[] pQout[Boy];  pQout[Boy]=NULL;
					delete[] pSout[Boy];  pSout[Boy]=NULL;	
				}
				if(Girl!=-1)
				{
					delete[] pQout[Girl]; pQout[Girl]=NULL;
					delete[] pSout[Girl]; pSout[Girl]=NULL;	
				}
			}

			cout<<"#"<<processor_name<<","<<rank<<",CAL,1:"<<mBSCode.RegionIndex<<","<<mBSCode.Value<<","<<mBSCode.Length<<endl;

			lCpuUsage=m_CpuUsage.GetCpuUsageNT();
			iCpuUsage=(int)lCpuUsage;
			cout<<"#"<<processor_name<<","<<rank<<",CPU,"<<iCpuUsage<<":"<<endl<<endl;//20060327,������,����CPU������//renewed by xia	

		}//end for(TaskLoop...)

		//2008.2.17,wh,����ת���̷�����Ϣ
		SendTime = MPI_Wtime();
	    PackSend(TaskCount-1);//wh����ת���̷���"��"�ڵ����������
	    MPI_Send(&(mTreeNode[TaskCount-1].mBSCode),sizeof(BSCode),MPI_BYTE,0,10,MPI_COMM_WORLD);//�����ؽ��̷�����Ϣ

		SendTime = MPI_Wtime() - SendTime;
		cout<<"#"<<processor_name<<","<<rank<<",TCM,"<<SendTime<<":"<<endl;//���������ؽ��̺���ת���̷�����Ϣ��ʱ��

		//����������
		//�ڵ�Ԫ����ѭ�������ת���ǵ�ָ���ջؿռ�
		if(BasinModel[ModelSelect][0].MakeLower()=="hsp")
		{
			delete[] pQout[TaskCount-1]; pQout[TaskCount-1]=NULL;//wh
			delete[] pDout[TaskCount-1]; pDout[TaskCount-1]=NULL;//wh
			delete[] pQout; delete[] pDout;
		}
		else
		{
			delete[] pQout[TaskCount-1]; pQout[TaskCount-1]=NULL;//wh
			delete[] pSout[TaskCount-1]; pSout[TaskCount-1]=NULL;//wh
			delete[] pQin; delete[] pQout;
			delete[] pSin; delete[] pSout;
			delete[] pWLM; delete[] pWRM;
		}
		delete [] mTreeNode;

		cout<<"#"<<processor_name<<","<<rank<<",FNS,"<<TaskCount<<":"<<endl<<endl;

	}//end while(1)
	Finalize();	
}


//ˮ��ڵ�Ĵ�����ʱˮ��ڵ�ļ������Ѿ���Ϊ����ˣ�����ֻ��ִ�в�ֵ�Ͳ�ѯ����
void SlaveProcess::ReservoirSegment(BSCode mBSCode,int TaskLoop)
{
	if(BasinModel[ModelSelect][0].MakeLower()!="hsp")
	{
		ZeroFill(pQin[TaskLoop],Steps+1);
		ZeroFill(pSin[TaskLoop],Steps+1);
	}
	cout<<"This is a RSV node!"<<endl;
	float preODischarge,nowODischarge;
	float preS,nowS;
	//float aaaaa,bbbbb;
	double preHourOffset,nowHourOffset;
	double calHourOffset;
	CString cSQL;
	_bstr_t bSQL;
	CComVariant tempVar;
	//����һ��ˮ�����ģ��
	//ofstream myFileReservoir;
	//    myFileReservoir.open("myFileReservoir.txt",ios::trunc);
	//	myFileReservoir.precision(18);

	try
	{
		//��reservior����ȡֵ��houroffset��С��������
		cSQL.Format("Select * from reservoir where  regionindex=%I64u and BSValue=%I64u and BSLength=%d and houroffset>=%d and houroffset<%d order by houroffset,minuteoffset",\
			mBSCode.RegionIndex, mBSCode.Value,mBSCode.Length,SParameter.HourStart,SParameter.HourStart+SParameter.NumofHours);
		bSQL = cSQL.GetString();
		pRstRsv->Open(bSQL,(ADODB::_Connection*)pCnn,ADODB::adOpenStatic,ADODB::adLockOptimistic,ADODB::adCmdText);
		if(pRstRsv->EndOfFile)
		{ 
			cout<<"No reservoir data for "<<mBSCode.RegionIndex<<"----"<<mBSCode.Length<<"----"<<mBSCode.Value<<endl;
			cSQL.Format("Select * from reservoir where  regionindex=%I64u and BSValue=%I64u and BSLength=%d order by houroffset,minuteoffset",\
				mBSCode.RegionIndex, mBSCode.Value,mBSCode.Length);
			bSQL = cSQL.GetString();
			pRstRsv->Close();
			pRstRsv->Open(bSQL,(ADODB::_Connection*)pCnn,ADODB::adOpenStatic,ADODB::adLockOptimistic,ADODB::adCmdText);
		}

		pRstRsv->MoveFirst();

		nowHourOffset=pRstRsv->Fields->Item["HourOffset"]->Value;
		tempVar=pRstRsv->Fields->Item["MinuteOffset"]->Value;
		tempVar.ChangeType(VT_R8);
		nowHourOffset+=tempVar.dblVal/60.0;

		preHourOffset=nowHourOffset-24;

		preODischarge=pRstRsv->Fields->Item["QOutput"]->Value;
		nowODischarge=preODischarge;

		tempVar=pRstRsv->Fields->Item["SOutput"]->Value;

		//preS=(tempVar.vt==VT_NULL ? 0:tempVar.fltVal);

		//david,20150520
		preS=(tempVar.vt==VT_NULL ? 0:pRstRsv->Fields->Item["SOutput"]->Value);

		nowS=preS;

		pRstRsv->MoveNext();//��һ�¸���ֵ����

		for(long j=0;j<Steps;j++) 
		{
			calHourOffset = SParameter.HourStart+double(j*lTimeInterval)/3600.0;
			while(calHourOffset>nowHourOffset)//ȡ��һ����ֵ����ʹ��¼�����µ���ֵ����
			{
				if(pRstRsv->EndOfFile)//��¼����û��������
				{
					nowHourOffset=ceil(calHourOffset);
					preODischarge=nowODischarge;
					preS=nowS;
					break;
				}
				preHourOffset=nowHourOffset;
				preODischarge=nowODischarge;
				preS=nowS;
				nowHourOffset=pRstRsv->Fields->Item["Houroffset"]->Value;
				tempVar=pRstRsv->Fields->Item["MinuteOffset"]->Value;
				tempVar.ChangeType(VT_R8);
				nowHourOffset+=tempVar.dblVal/60.0;

				nowODischarge=pRstRsv->Fields->Item["QOutput"]->Value;

				tempVar=pRstRsv->Fields->Item["SOutput"]->Value;
			    //aaaaa=pRstRsv->Fields->Item["SOutput"]->Value;
		        //bbbbb=tempVar.fltVal;

				//nowS=(tempVar.vt==VT_NULL ? 0:tempVar.fltVal);
				//david,20150520
				nowS=(tempVar.vt==VT_NULL ? 0:pRstRsv->Fields->Item["SOutput"]->Value);

				pRstRsv->MoveNext();
			}//end while
			//��������һ�£���ô˵�Ļ����ڼ��㿪ʼ��һ��ʱ�����ˮ����й�Ļ�������йǰ������ʽ�������pQout����predischarge��
			pQout[TaskLoop][j]=(preODischarge*(nowHourOffset-calHourOffset)+nowODischarge*(calHourOffset-preHourOffset))/(nowHourOffset-preHourOffset);

			if(BasinModel[ModelSelect][0].MakeLower()!="hsp")
			{
				pSout[TaskLoop][j]=(preS*(nowHourOffset-calHourOffset)+nowS*(calHourOffset-preHourOffset))/(nowHourOffset-preHourOffset);
			}

			//if (myFileReservoir.is_open()) 
			//{
			//	myFileReservoir <<mBSCode.RegionIndex<<" "<<mBSCode.Value<<" "<<mBSCode.Length<<" "<<preODischarge<<" "<<nowODischarge<<" "<<preS<<" "<<nowS<<" "<<aaaaa<<" "<<bbbbb<<" "<<preHourOffset<<" "<<nowHourOffset<<" "<<calHourOffset<<" "<<pQout[TaskLoop][j]<<" "<<pSout[TaskLoop][j]<<"\n";
			//}

			//cout<<pQout[TaskLoop][j]<<endl;
		}//end for

		pRstRsv->Close();
		//myFileReservoir.close();
	}//end try

	catch (_com_error e) 
	{
		cout<<e.Error()<<endl;
		cout<<e.ErrorMessage()<<endl;
		cout<<(LPCSTR)e.Source()<<endl;        
		cout<<(LPCSTR)e.Description()<<endl;

		cout<<"Error Occured On Rank "<<rank<<" While Calc Reservoir."<<endl;
		cout<<"Task "<<TaskLoop<<" of "<<TaskCount<<"."<<endl;
		cout<<"RegionIndex="<<mBSCode.RegionIndex<<"\tBSValue="<<mBSCode.Value<<"\tBSLength="<<mBSCode.Length<<endl;
		cout<<"SQL: "<<cSQL<<endl;

		exit(0);
	}

}


//wh,������ģ���Ϊdll�������������ģ��ѡ��ӿ�
void SlaveProcess::RunoffCalc(BSCode mBSCode,Para mPara,int TaskLoop)
{
	//hspģ�Ͳ��������в�������
	if(BasinModel[ModelSelect][0].MakeLower() == "hsp")
		return;

	if(BasinModel[ModelSelect][0].MakeLower() == "ltjyr")
	{
		try
		{
			CString SnowModelType = BasinModel[ModelSelect][3].MakeLower();
			//����Ӧ����wateryield�����������ɳ����
			hr = spWaterBasin->calc(SnowModelType.AllocSysString(),mBSCode,mPara,pQin[TaskLoop],pSin[TaskLoop],pWLM[TaskLoop],pWRM[TaskLoop]);
			assert( SUCCEEDED( hr ) );
			//cout<<"RegionIndex="<<mBSCode.RegionIndex<<"\tBSValue="<<mBSCode.Value<<"\tBSLength="<<mBSCode.Length<<endl;
		}
		catch (_com_error e) 
		{
			cout<<"Error While Calculating WaterYield on Rank "<<rank<<endl;
			cout<<"RegionIndex="<<mBSCode.RegionIndex<<"\tBSValue="<<mBSCode.Value<<"\tBSLength="<<mBSCode.Length<<endl;
			exit(0);
		}
	}

	if(BasinModel[ModelSelect][0].MakeLower() == "xaj")
	{
		try
		{
			CString SnowModelType = BasinModel[ModelSelect][3].MakeLower();
			hr = spXAJmodel->Calc(SnowModelType.AllocSysString(),mBSCode,mPara,pQin[TaskLoop],pWLM[TaskLoop],pWRM[TaskLoop]);
			assert( SUCCEEDED( hr ) );
			cout<<"RegionIndex="<<mBSCode.RegionIndex<<"\tBSValue="<<mBSCode.Value<<"\tBSLength="<<mBSCode.Length<<endl;
		}
		catch (_com_error e) 
		{
			cout<<"Error While Calculating XAJ on Rank "<<rank<<endl;
			cout<<"RegionIndex="<<mBSCode.RegionIndex<<"\tBSValue="<<mBSCode.Value<<"\tBSLength="<<mBSCode.Length<<endl;
			exit(0);
		}
	}
}

//ģ�ͻ�������,������Դ����һ������˹������һ����avpm��
void SlaveProcess::ConfluenceCalc(int TaskLoop,BSCode mBSCode,Para mPara)
{
	if(BasinModel[ModelSelect][0].MakeLower() == "hsp")
	{
		//wh:��ǰ����Ӷε��������Ӷε�"����Qu"��"����Qd"ˮ��������
		float* pQu1; float* pQu2;
		float* pQd1; float* pQd2;
		
		if(Girl==-1 && Boy==-1)
		{
			pQu1=ZeroSerial;  pQu2=pQUpRegion;
			pQd1=ZeroSerial;  pQd2=pDUpRegion;
		}
		else if(Girl==-1 && Boy!=-1)
		{
			pQu1=pQUpRegion;  pQu2=pQout[Boy];
			pQd1=pDUpRegion;  pQd2=pDout[Boy];
		}
		else if(Girl!=-1 && Boy==-1)
		{
			pQu1=pQout[Girl]; pQu2=pQUpRegion;
			pQd1=pDout[Girl]; pQd2=pDUpRegion;	
		}
		else if(Girl!=-1 && Boy!=-1)
		{
			pQu1=pQout[Girl]; pQu2=pQout[Boy];
			pQd1=pDout[Girl]; pQd2=pDout[Boy];	
		}

		//����ĵ�ģ�ͼ���
		HSP.Calc(mBSCode,&mPara,pQu1,pQu2,pQd1,pQd2,pQout[Parent],pDout[Parent],pFlowB,pFlowH,pFlow_v);

		return;
	}

	//��������ǰ��ģ�ͣ�Ϊ�˲���죬���浥��дhsp�йصġ�
	if(mTreeNode[TaskLoop].StralherOrder == 1)//���������
	{
		ZeroFill(pQout[Parent],Steps+1);//Parent=TaskLoop;
		ZeroFill(pSout[Parent],Steps+1);
		SwapQp(&pQout[Parent],&pQin[Parent]);
		SwapQp(&pSout[Parent],&pSin[Parent]);

		//�����Ϊʲô�������߽�����浥Ԫ�ĺӶ�Qinput��Sinput��0���������������ɳ����
		//��������ΪQout����Ҫ��Ϊ�Ӷγ��ڹ���Ϊ���κӶ��ṩ�߽������ء�
	}
	else //�����
	{
		//��������������ֵ��pQout1����girl��pQout2����boy
		float* pQout1; float* pQout2;
		float* pSout1; float* pSout2;

		if(mBSCode.RegionIndex==1122012001 && mBSCode.Value==0 && mBSCode.Length==940)
			int xxx=0;

		//david,�������������һ���Ӷ�avpm calc()���������������Ӷ����ʱgirl�Ӷ�ˮɳΪ0��ԭ�򣬻���˳��������ϸ����
		//ofstream myFile4;
		//if (mBSCode.Length==67 && mBSCode.Value==65536)
		//{
		//	 myFile4.open("example3.txt",ios::trunc);
		//}
		if(Girl==-1 && Boy==-1)
		{
			pQout1=ZeroSerial;  pQout2=pQUpRegion;
			pSout1=ZeroSerial;  pSout2=pSUpRegion;
		}
		else if(Girl==-1 && Boy!=-1)
		{
			pQout1=pQUpRegion;  pQout2=pQout[Boy];
			pSout1=pSUpRegion;  pSout2=pSout[Boy];
		}
		else if(Girl!=-1 && Boy==-1)
		{
			pQout1=pQout[Girl]; pQout2=pQUpRegion;
			pSout1=pSout[Girl]; pSout2=pSUpRegion;	
		}
		else if(Girl!=-1 && Boy!=-1)
		{
			pQout1=pQout[Girl]; pQout2=pQout[Boy];
			pSout1=pSout[Girl]; pSout2=pSout[Boy];	
		}
		//if (myFile4.is_open()) 
		//{
		//	myFile4<<"Boy="<<Boy<<" Girl="<<Girl<<"\n";
		//}
		//if (myFile4.is_open())
	 //   {
		//    myFile4.close();
	 //   }
		if(BasinModel[ModelSelect][1].MakeLower() == "muskingum")//��˹����������//wh
		{
			Transform(pQout1,pQout2,pQin[Parent],pQout[Parent],lTimeInterval,Steps,&mPara);
		}
		else if(BasinModel[ModelSelect][1].MakeLower() == "avpm")//��ȷ��ɢ����//wh
		{
			try
			{
				AVPM mAVPM(SParameter.SediTransCapF,SParameter.MSTEP,SParameter.sccd);//wh

				mAVPM.iTSteps=Steps;
				mAVPM.isDebug=false;

				//20060316,������,ͳһ���Ƿ����������ʴ�Ĵ���
				//if(SParameter.iCalGravityErosion==0)      
				mAVPM.bCalcGravity=false;	

				/*else*/ if(SParameter.iCalGravityErosion==1) 
					if(mPara.DrainingArea<100000000)	//20080306,xiaofc,100km2���ϵĻ�ˮ����²�����������ʴ
						mAVPM.bCalcGravity=true;

					else if(SParameter.iCalGravityErosion==2)
					{
						if(mBSCode.RegionIndex==0 && mBSCode.Value==0)//20051212 ������,AVPM���ӿ����Ƿ����������ʴ�Ĳ���,���Һ������ͷ���������Ÿ���������������ʴ
							mAVPM.bCalcGravity=false;
						else
							mAVPM.bCalcGravity=true;
					}

					mAVPM.mBSCode = mBSCode;//wh

					mAVPM.initialize(mPara);
					mAVPM.alphaE = SParameter.fAlphaErosion;//20060317,������,�ָ�����ϵ��
					mAVPM.alphaD = SParameter.fAlphaDeposition;
					mAVPM.P2 = SParameter.GravityErosioinP2;//20060324,������,����������ʴ������Χ�ĸ���

					//20060327,������,Ϊ�洢������ʴ��Ϣ��
					if(SParameter.bSaveGravityEvents){ mAVPM.pGravityEvents=&GravityEvents; }	
					else{ mAVPM.pGravityEvents=NULL; }

					mAVPM.delta_t=lTimeInterval;
					mAVPM.WL=pWLM[Parent];    mAVPM.WR=pWRM[Parent];
					mAVPM.Qout=pQout[Parent]; mAVPM.Sout=pSout[Parent];
					mAVPM.Qin1=pQout1;        mAVPM.Qin2=pQout2;         mAVPM.Qin3=pQin[Parent];
					mAVPM.SinL=pSout1;        mAVPM.SinR=pSout2;
					mAVPM.SinMe=pSin[Parent];

					//20080303,xiaofc,Ϊ����׳̬����
					mAVPM.HourStart=SParameter.HourStart;
					mAVPM.pCnn=pCnn;

					mAVPM.SaveFlowPattern = SParameter.bSaveFlowPattern;//20070622,xiaofc, flow pattern
					if(SParameter.bSaveFlowPattern)
					{
						mAVPM.FlowB=pFlowB;
						mAVPM.FlowH=pFlowH;
						mAVPM.Flow_v=pFlow_v;
					}
					//mAVPM.Calc_choose();
					mAVPM.Calc_tjm();
					//mAVPM.Calc();
					//20080303,xiaofc,��״̬
					if(BasinModel[ModelSelect][0].MakeLower() == "ltjyr") { mAVPM.SaveStatus(); }

					mAVPM.finalize();
			}
			catch(...) 
			{
				cout<<"Error While Calculating AVPM on Rank "<<rank<<endl;
			}
		}
		else
		{
			cout<<"No such Routing Method Defined: "<<BasinModel[ModelSelect][1]<<"!"<<endl;
		}
	}//end of �����
}


//wh,����ת���̷��͸��ڵ��������ɳ�������Լ�RegionIndex��BSValue��BSLength��������
void SlaveProcess::PackSend(int k)
{
	int len[5];
	MPI_Aint disp[5];
	MPI_Datatype type[5],newtype;

	int position=0;//�����ʼ��λ��
	int size=2*(Steps+1)*sizeof(float)+2*sizeof(unsigned long long)+sizeof(long);
	char* buff = new char[size];

	//���������Ͱ������ݵĸ���
	len[0] = len[1] = len[2] = 1;
	len[3] = len[4] = Steps+1;

	MPI_Address(&mTreeNode[k].mBSCode.RegionIndex,disp);//mTreeNode[k].mBSCode.RegionIndex�����MPI_BOTTOM��ƫ�ƣ��洢��disp+0��
	MPI_Address(&mTreeNode[k].mBSCode.Value,disp+1);
	MPI_Address(&mTreeNode[k].mBSCode.Length,disp+2);
	MPI_Address(pQout[k],disp+3);

	//�ⲻ�ǽ�����ⳤ��֮�ƣ�����֮���ǻ�Ҫ����һ��ģ�������б���������ñ���ת����
	if(BasinModel[ModelSelect][0].MakeLower()=="hsp")
	{
		MPI_Address(pDout[k],disp+4);
	}
	else
	{
		MPI_Address(pSout[k],disp+4);
	}

	type[0] = type[1] = MPI_UNSIGNED_LONG_LONG;
	type[2] = MPI_LONG;
	type[3] = type[4] = MPI_FLOAT;

	MPI_Type_struct(5,len,disp,type,&newtype);//�����µ���������
	MPI_Type_commit(&newtype);//�������ύ

	MPI_Pack(MPI_BOTTOM,1,newtype,buff,size,&position,MPI_COMM_WORLD);//���ݴ��
	MPI_Send(buff,position,MPI_PACKED,TransferProcessRank,10,MPI_COMM_WORLD);//���ʹ�����ݵ���ת���̡�
	delete[] buff;
}


//whΪ���ҵ��Ӷ��ӽڵ�ĳ���������Ϊ������������Ӷε��ӽڵ�϶�������������������ֵ���С���ĸ��ڵ㣬
//NoChild�������������Ϊ����������ķ������Ϊ��ͬRegionIndex�����ķ���,����NoChild������������һ�����ӡ�
void SlaveProcess::NoChild(BSCode mBSCode)
{
	int i=0;
	for(i=0; i<RCCount; i++)
	{
		if(mBSCode.RegionIndex != unsigned long long(RegionConnection[i].RegionIndex/SParameter.RegionSystem))//20130904, shy
			continue;
		if(mBSCode.Length != RegionConnection[i].Length)
			continue;
		if(mBSCode.Value != RegionConnection[i].Value)
			continue;
		break;
	}

	if(i == RCCount)//�������ӶΣ�������һ���ӽڵ���Ϊ����������ķ���
	{
		int len[4];
		MPI_Aint disp[4];
		MPI_Datatype type[4],newtype;
		int position=0;//�����ʼ��λ��
		int size = 3 * sizeof(unsigned long long) + sizeof(long);
		char* buff = new char[size];

		unsigned long long BoyBsvalue = mBSCode.Value<<1;
		unsigned long long GirlBsvalue = ((mBSCode.Value<<1)+(unsigned long long)1);
		mBSCode.Length++;//wh,2008.3.26����,���͵����ӽڵ��BSlength�����ǵ�ǰ�ڵ��

		len[0] = len[1] = len[2] = len[3] = 1;
		MPI_Address(&mBSCode.RegionIndex,disp);//mBSCode.RegionIndex�����MPI_BOTTOM��ƫ�ƣ��洢��disp+0��
		MPI_Address(&BoyBsvalue,disp+1);
		MPI_Address(&GirlBsvalue,disp+2);
		MPI_Address(&mBSCode.Length,disp+3);

		type[0] = type[1] = type[2] = MPI_UNSIGNED_LONG_LONG;
		type[3] = MPI_LONG;

		MPI_Type_struct(4,len,disp,type,&newtype);//�����µ���������
		MPI_Type_commit(&newtype);//�������ύ

		MPI_Pack(MPI_BOTTOM,1,newtype,buff,size,&position,MPI_COMM_WORLD);//���ݴ��
		MPI_Send(buff,position,MPI_PACKED,TransferProcessRank,11,MPI_COMM_WORLD);//������������ı�ǩ�����������͵���ת���̣���������Ҫ����ˡ�
		delete[] buff;

		MPI_Recv(pQUpRegion,Steps+1,MPI_FLOAT,TransferProcessRank,11,MPI_COMM_WORLD,&Status);//����ת���̽�����������

		//wh
		if(BasinModel[ModelSelect][0].MakeLower()=="hsp")
		{
			MPI_Recv(pDUpRegion,Steps+1,MPI_FLOAT,TransferProcessRank,11,MPI_COMM_WORLD,&Status);//����ת���̽��յ���ˮ������
		}
		else
		{
			MPI_Recv(pSUpRegion,Steps+1,MPI_FLOAT,TransferProcessRank,11,MPI_COMM_WORLD,&Status);//����ת���̽���ɳ������
		}
	}

	else//�����ӶΣ�ȥ��һ��region���ң�ֻ��һ���ӽڵ�
	{
		MPI_Send(&RegionConnection[i].RegionIndex,1,MPI_UNSIGNED_LONG_LONG,TransferProcessRank,12,MPI_COMM_WORLD);//bsvalueһ����0�����Բ��÷���
		
		//shy, 20130831,region���ڵ�bslength���ٴ�1��ʼ���̳���һ��region�ļ������ӣ�����Ҫȡ���ݵ�ʱ��Ҫָ��bslength
		long UpRegionBSLength;
		UpRegionBSLength=RegionConnection[i].Length+1;
		//david,Ȼ���ɺ���bslength���Ǵ�1��ʼ
		UpRegionBSLength=1;
		MPI_Send(&UpRegionBSLength,1,MPI_LONG,TransferProcessRank,12,MPI_COMM_WORLD);
		
		MPI_Recv(pQUpRegion,Steps+1,MPI_FLOAT,TransferProcessRank,12,MPI_COMM_WORLD,&Status);

		//wh
		if(BasinModel[ModelSelect][0].MakeLower()=="hsp")
		{
			MPI_Recv(pDUpRegion,Steps+1,MPI_FLOAT,TransferProcessRank,12,MPI_COMM_WORLD,&Status);
		}
		else
		{
			MPI_Recv(pSUpRegion,Steps+1,MPI_FLOAT,TransferProcessRank,12,MPI_COMM_WORLD,&Status);
		}
	}

}

//whֻ�����ڴ����ҵ���ǰ�ڵ��һ���ӽڵ㣬������Ϊ�ڵ������һ�����������ӣ������ӽڵ�����ת�����п����У�Ҳ����û�С�
void SlaveProcess::OneChild(BSCode mBSCode,int Boy,int Girl)
{
	ofstream myFile888;
	myFile888.open("OneChild.txt",ios::app);
	assert((Boy==-1 && Girl!=-1) || (Boy!=-1 && Girl==-1));

	int len[3];
	MPI_Aint disp[3];
	MPI_Datatype type[3],newtype;
	int position=0;//�����ʼ��λ��
	int size = 2 * sizeof(unsigned long long) + sizeof(long);
	char* buff = new char[size];

	unsigned long long bsvalue;

	if(mBSCode.RegionIndex==1 && mBSCode.Value==0 && mBSCode.Length==218)
		int xxx=0;

	//�ж��Ƿ����Ӷ�,20131126,shy
	int i=0;
	for(i=0; i<RCCount; i++)
	{
		if(mBSCode.RegionIndex != unsigned long long(RegionConnection[i].RegionIndex/SParameter.RegionSystem))//20130904, shy
			continue;
		if(mBSCode.Length != RegionConnection[i].Length)
			continue;
		if(mBSCode.Value != RegionConnection[i].Value)
			continue;
		break;
	}
	
	if(i == RCCount)//�������ӶΣ�������һ���ӽڵ���Ϊ����������ķ���
	{
		myFile888<<"i == RCCount\n"<<flush;
		if(Boy == -1)
		{
			bsvalue = mBSCode.Value<<1;
		}
		else
		{
			bsvalue =  ((mBSCode.Value<<1)+(unsigned long long)1);
		}
		mBSCode.Length++;//wh,2008.3.26����,���͵����ӽڵ��BSlength�����ǵ�ǰ�ڵ��
		
		len[0] = len[1] = len[2] = 1;
		MPI_Address(&mBSCode.RegionIndex,disp);//mBSCode.RegionIndex�����MPI_BOTTOM��ƫ�ƣ��洢��disp+0��
		MPI_Address(&bsvalue,disp+1);
		MPI_Address(&mBSCode.Length,disp+2);

		type[0] = type[1] = MPI_UNSIGNED_LONG_LONG;
		type[2] = MPI_LONG;

		MPI_Type_struct(3,len,disp,type,&newtype);//�����µ���������
		MPI_Type_commit(&newtype);//�������ύ

		MPI_Pack(MPI_BOTTOM,1,newtype,buff,size,&position,MPI_COMM_WORLD);//���ݴ��
		MPI_Send(buff,position,MPI_PACKED,TransferProcessRank,13,MPI_COMM_WORLD);//������������ı�ǩ�����������͵���ת���̣���������Ҫ����ˡ�
		delete buff;

		MPI_Recv(pQUpRegion,Steps+1,MPI_FLOAT,TransferProcessRank,13,MPI_COMM_WORLD,&Status);//����ת���̽�����������

		//wh
		if(BasinModel[ModelSelect][0].MakeLower()=="hsp")
		{
			MPI_Recv(pDUpRegion,Steps+1,MPI_FLOAT,TransferProcessRank,13,MPI_COMM_WORLD,&Status);//����ת���̽���ɳ������
		}
		else
		{
			MPI_Recv(pSUpRegion,Steps+1,MPI_FLOAT,TransferProcessRank,13,MPI_COMM_WORLD,&Status);//����ת���̽���ɳ������
		}
	}
	else
    {
		myFile888<<"i != RCCount\n"<<flush;
		//shy, 20130831,region���ڵ�bslength���ٴ�1��ʼ���̳���һ��region�ļ������ӣ�����Ҫȡ���ݵ�ʱ��Ҫָ��bslength
		long UpRegionBSLength;
		UpRegionBSLength=RegionConnection[i].Length+1;
		//david,Ȼ���ɺ���bslength���Ǵ�1��ʼ
		UpRegionBSLength=1;

		len[0] = len[1] = len[2] = 1;
		MPI_Address(&RegionConnection[i].RegionIndex,disp);//mBSCode.RegionIndex�����MPI_BOTTOM��ƫ�ƣ��洢��disp+0��
		MPI_Address(&RegionConnection[i].Value,disp+1);
		MPI_Address(&UpRegionBSLength,disp+2);

		type[0] = type[1] = MPI_UNSIGNED_LONG_LONG;
		type[2] = MPI_LONG;

		MPI_Type_struct(3,len,disp,type,&newtype);//�����µ���������
		MPI_Type_commit(&newtype);//�������ύ

		MPI_Pack(MPI_BOTTOM,1,newtype,buff,size,&position,MPI_COMM_WORLD);//���ݴ��
		MPI_Send(buff,position,MPI_PACKED,TransferProcessRank,13,MPI_COMM_WORLD);//������������ı�ǩ�����������͵���ת���̣���������Ҫ����ˡ�
		delete buff;

		MPI_Recv(pQUpRegion,Steps+1,MPI_FLOAT,TransferProcessRank,13,MPI_COMM_WORLD,&Status);

		if(BasinModel[ModelSelect][0].MakeLower()=="hsp")
		{
			MPI_Recv(pDUpRegion,Steps+1,MPI_FLOAT,TransferProcessRank,13,MPI_COMM_WORLD,&Status);
		}
		else
		{
			MPI_Recv(pSUpRegion,Steps+1,MPI_FLOAT,TransferProcessRank,13,MPI_COMM_WORLD,&Status);
		}
	}
	myFile888.close();
}


//�����ݿ���д��������ʴ�¼���ǰ��û�漰��д���ݿ⣬����ſ�ʼ��
void SlaveProcess::SaveGravityEvents(int TaskLoop)
{
	//20060327,������,д������ʴ�¼������ݿ�
	//ADODB::_RecordsetPtr pRstGravityEvents;
	//pRstGravityEvents.CreateInstance(__uuidof(ADODB::Recordset));
	unsigned long iVecLoop;
	CString cSQL;
	_bstr_t SQL;
	try
	{
		//��ɾ��ͬʱ�ڵľ����ݣ���Ϊ��������ǲ����ظ���
		pCnn->BeginTrans();
		cSQL.Format("delete from gravityevents where sccd=%s and houroffset>=%d and houroffset<%d and regionindex=%I64d and bsvalue=%I64d and bslength=%d",SParameter.sccd,SParameter.HourStart,SParameter.HourStart+SParameter.NumofHours,mTreeNode[TaskLoop].mBSCode.RegionIndex,mTreeNode[TaskLoop].mBSCode.Value,mTreeNode[TaskLoop].mBSCode.Length);
		SQL=cSQL.GetString();
		pCnn->Execute(SQL,NULL,ADODB::adCmdText);
		pCnn->CommitTrans();

		pCnn->BeginTrans();
		pRstGravityEvents->Open("GravityEvents",(ADODB::_Connection*)pCnn,ADODB::adOpenStatic,ADODB::adLockOptimistic,ADODB::adCmdTable);

		CString LRTemp;
//		cout<<"VecSize:"<<GravityEvents.size()<<endl;
		tempCom.ChangeType(VT_DECIMAL);
		//cout<<1<<endl;
		for(iVecLoop=0;iVecLoop<GravityEvents.size();iVecLoop++)
		{
			//cout<<2<<endl;
			pRstGravityEvents->AddNew();
			//cout<<3<<endl;
			//wh,added,sccd,2008.3.24
			vtmp = SParameter.sccd;
			//cout<<4<<endl;
			pRstGravityEvents->Fields->Item["sccd"]->Value = _variant_t(vtmp);
			//cout<<5<<endl;

			tempCom.ullVal=mTreeNode[TaskLoop].mBSCode.RegionIndex;
			//cout<<6<<endl;
			pRstGravityEvents->Fields->Item["Regionindex"]->Value=tempCom;
			//cout<<7<<endl;
			tempCom.ullVal=mTreeNode[TaskLoop].mBSCode.Value;
			//cout<<8<<endl;
			pRstGravityEvents->Fields->Item["BSValue"]->Value=tempCom;
			//cout<<9<<endl;
			pRstGravityEvents->Fields->Item["BSLength"]->Value=mTreeNode[TaskLoop].mBSCode.Length;
			//cout<<10<<endl;
			pRstGravityEvents->Fields->Item["Houroffset"]->Value=SParameter.HourStart+long(GravityEvents[iVecLoop].TimeStep*SParameter.MSTEP/60);
			//cout<<11<<endl;
			pRstGravityEvents->Fields->Item["Minuteoffset"]->Value=GravityEvents[iVecLoop].TimeStep*SParameter.MSTEP%60;
			//cout<<12<<endl;
			LRTemp.Format("%c",GravityEvents[iVecLoop].LR);
			//cout<<13<<endl;
			pRstGravityEvents->Fields->Item["LR"]->Value=LRTemp.GetString();
			//cout<<14<<endl;
			pRstGravityEvents->Fields->Item["FD"]->Value=GravityEvents[iVecLoop].FD;
		    pRstGravityEvents->Fields->Item["Amount"]->Value=GravityEvents[iVecLoop].Amount;
			//cout<<18<<endl;
			//cout<<15<<endl;
			pRstGravityEvents->Fields->Item["FR"]->Value=GravityEvents[iVecLoop].FR;
			//cout<<16<<endl;
			pRstGravityEvents->Fields->Item["Probability"]->Value=GravityEvents[iVecLoop].Probability;
			//cout<<17<<endl;

			
			if(iVecLoop%(40+rank)==0)
			{
				pRstGravityEvents->UpdateBatch(ADODB::adAffectAll);
			}
			//cout<<19<<endl;
			if(iVecLoop%(1000+rank*10)==0)
			{
				pRstGravityEvents->UpdateBatch(ADODB::adAffectAll);
				pCnn->CommitTrans();
				pCnn->BeginTrans();
				pRstGravityEvents->Requery(ADODB::adCmdTable);
			}
			//cout<<20<<endl;
		}//end of for
		//cout<<21<<endl;
		pRstGravityEvents->UpdateBatch(ADODB::adAffectAll);
		//cout<<22<<endl;
		pRstGravityEvents->Close();
		//cout<<23<<endl;
		pCnn->CommitTrans();
		//cout<<24<<endl;
	}
	catch(_com_error e)
	{
		//cout<<25<<endl;
		cout<<e.Error()<<endl;
		cout<<e.ErrorMessage()<<endl;
		cout<<(LPCSTR)e.Source()<<endl;        
		cout<<(LPCSTR)e.Description()<<endl;

		cout<<"Error Occured On Rank "<<rank<<" While Writing GravityEvents to DB."<<endl;
		cout<<"Task "<<TaskLoop<<" of "<<TaskCount<<"."<<endl;
		cout<<"sccd="<<SParameter.sccd<<endl;//wh
		cout<<"RegionIndex="<<mTreeNode[TaskLoop].mBSCode.RegionIndex<<"\tBSValue="<<mTreeNode[TaskLoop].mBSCode.Value<<"\tBSLength="<<mTreeNode[TaskLoop].mBSCode.Length<<endl;
		cout<<"HourOffset="<<SParameter.HourStart+long(GravityEvents[iVecLoop].TimeStep*SParameter.MSTEP/60)<<"\tMinuteOffset="<<GravityEvents[iVecLoop].TimeStep*SParameter.MSTEP%60<<"\tiVecLoop="<<iVecLoop;
		cout<<"\tFD="<<GravityEvents[iVecLoop].FD<<"\tFR="<<GravityEvents[iVecLoop].FR<<"\tProbability="<<GravityEvents[iVecLoop].Probability<<"\tAmount="<<GravityEvents[iVecLoop].Amount<<endl;
		exit(0);
	}
	GravityEvents.clear();//д�����ݿ����֮��,�����ε�������ʴ��¼
}


//wh2009����ʵ�ú�����Ӧ�÷������Ҳ��ģ�������ģ�Ӧ����ģ�͵Ĵ������棬��Ϊ����������ģ�͵���������
//д��discharge������һ��������������һ�µģ���Ϊ���ǳ��ڵ�ˮɳ�������С�
//��������ɳ���д�����ݿ�
void SlaveProcess::WriteToDischarge(int TaskLoop)
{
	//ADODB::_RecordsetPtr pRstIndicator;//ָ��DefinedNodes��
	//pRstIndicator.CreateInstance(__uuidof(ADODB::Recordset));
	long j=0;
	CString cSQL;
	_bstr_t SQL;
	try
	{
		cSQL.Format("Select * from DefinedNodes where RegionIndex=%I64u and BSValue=%I64u and BSLength=%d and nodetype in ('CS','GS','RS')",mTreeNode[TaskLoop].mBSCode.RegionIndex,mTreeNode[TaskLoop].mBSCode.Value,mTreeNode[TaskLoop].mBSCode.Length);
		SQL=cSQL.GetString();
		pRstIndicator->Open(SQL,(ADODB::_Connection*)pCnn,ADODB::adOpenStatic,ADODB::adLockOptimistic,ADODB::adCmdText);

		//wh,�ж��Ƿ���Ҫд����,�������˼��ֻд�յ��������ĸ��ڵ��Լ���definednodes�е�CS��GS�㡣
		//if(pRstIndicator->EndOfFile && TaskLoop!=TaskCount-1)
		//2008.1.17,wh,ֻ��definednodesд�����߽�������ȫ��д����ʱҲ�Ͳ���ʲôdefinednodes�ˣ������ͨ���ڴ洫�ݸ��ڵ���Ϣ���Ǹ��ڵ��û��Ҫ�����ݿ�����д�ˡ�
		if(pRstIndicator->EndOfFile && SParameter.bSaveAllDischarge==0)
			pRstIndicator->Close();
		else//д��
		{
			pRstIndicator->Close();

			//20051209,xiaofc,Ϊ�������޺ӵ���ʼ���������preparehours�ڵ�������Ч���������,�޸���д���ʱ������HourStart��ΪHourStart+PrepareHours
			//20051209����,�������Ӷ���ǰû�������ô�����Ǵ������ҲҪд��ȥ����Ϊ���εĻ�Ҫ�����������
			CComVariant comSQL;//�ϳ�˵�����÷�
			long lDataCount;

			if(BasinModel[ModelSelect][0].MakeLower()=="hsp")
			{
				cSQL.Format("select count(*) as mycount from hspdischarge where BSValue=%I64u and BSLength=%d and houroffset>=%d and houroffset<%d and regionindex=%I64u",mTreeNode[TaskLoop].mBSCode.Value,mTreeNode[TaskLoop].mBSCode.Length,SParameter.HourStart,SParameter.HourStart+SParameter.PrepareHours,mTreeNode[TaskLoop].mBSCode.RegionIndex);
			}
			else
			{
				cSQL.Format("select count(*) as mycount from discharge where sccd=%s and BSValue=%I64u and BSLength=%d and houroffset>=%d and houroffset<%d and regionindex=%I64u",SParameter.sccd,mTreeNode[TaskLoop].mBSCode.Value,mTreeNode[TaskLoop].mBSCode.Length,SParameter.HourStart,SParameter.HourStart+SParameter.PrepareHours,mTreeNode[TaskLoop].mBSCode.RegionIndex);//wh,add sccd
			}

			comSQL=cSQL;
			pRstData->Open(comSQL.bstrVal,(ADODB::_Connection*)pCnn,ADODB::adOpenForwardOnly,ADODB::adLockOptimistic,ADODB::adCmdText);
			lDataCount=pRstData->Fields->Item["mycount"]->Value;
			pRstData->Close();

			//wh,ǰpreparehoursģ�Ϳ�ת�����Ǽ�¼�Ѿ�д�뵽�����ݿ�,�����κӶ�preparehours�ڼ�¼������ʱ�����preparehours��ʼȡ����
			long lNoStorageHours;//���ʱ��
			if(lDataCount<SParameter.PrepareHours*60/SParameter.MSTEP){ lNoStorageHours=0;}
			else{ lNoStorageHours=SParameter.PrepareHours;}

			//20051219 ������ �Ľ�Ϊд����ʱ��update��delete+addnew,�Լ�С���ݿ⸺��
			if(BasinModel[ModelSelect][0].MakeLower()=="hsp")
			{
				cSQL.Format("select count(*) as mycount from hspdischarge where BSValue=%I64u and BSLength=%d and houroffset>=%d and houroffset<%d and regionindex=%I64u",mTreeNode[TaskLoop].mBSCode.Value,mTreeNode[TaskLoop].mBSCode.Length,SParameter.HourStart,SParameter.HourStart+SParameter.PrepareHours,mTreeNode[TaskLoop].mBSCode.RegionIndex);
			}
			else
			{
				cSQL.Format("select count(*) as mycount from discharge where sccd=%s and regionindex=%I64u and bsvalue=%I64u and bslength=%d and houroffset>=%d and houroffset<%d",SParameter.sccd,mTreeNode[TaskLoop].mBSCode.RegionIndex,mTreeNode[TaskLoop].mBSCode.Value,mTreeNode[TaskLoop].mBSCode.Length,SParameter.HourStart+lNoStorageHours,SParameter.HourStart+SParameter.NumofHours);//wh,add sccd
			}
			comSQL=cSQL;
			pRstData->Open(comSQL.bstrVal,(ADODB::_Connection*)pCnn,ADODB::adOpenForwardOnly,ADODB::adLockOptimistic,ADODB::adCmdText);
			lDataCount=pRstData->Fields->Item["mycount"]->Value;
			pRstData->Close();

			bool bIsRewrite;
			bIsRewrite=(lDataCount==(SParameter.NumofHours-lNoStorageHours)*60/SParameter.MSTEP);//֤��discharge���Ѿ��д�
			pCnn->BeginTrans();//��ʼ���񣬱�������commit

			if(bIsRewrite)//��д
			{

				if(BasinModel[ModelSelect][0].MakeLower()=="hsp")
				{
					cSQL.Format("select * from hspdischarge where regionindex=%I64u and bsvalue=%I64u and bslength=%d and houroffset>=%d and houroffset<%d order by Houroffset,minuteoffset",mTreeNode[TaskLoop].mBSCode.RegionIndex,mTreeNode[TaskLoop].mBSCode.Value,mTreeNode[TaskLoop].mBSCode.Length,SParameter.HourStart+lNoStorageHours,SParameter.HourStart+SParameter.NumofHours);
				}
				else
				{
					cSQL.Format("select * from discharge where sccd=%s and regionindex=%I64u and bsvalue=%I64u and bslength=%d and houroffset>=%d and houroffset<%d order by Houroffset,minuteoffset",SParameter.sccd,mTreeNode[TaskLoop].mBSCode.RegionIndex,mTreeNode[TaskLoop].mBSCode.Value,mTreeNode[TaskLoop].mBSCode.Length,SParameter.HourStart+lNoStorageHours,SParameter.HourStart+SParameter.NumofHours);
				}
				comSQL=cSQL;

				pRstData->Open(comSQL.bstrVal,pCnn.GetInterfacePtr(),ADODB::adOpenForwardOnly,ADODB::adLockBatchOptimistic,ADODB::adCmdText);


				if(BasinModel[ModelSelect][0].MakeLower()=="ltjyr")
				{
					for(j=lNoStorageHours*60/SParameter.MSTEP;j<Steps;j++)
					{
						//CComVariant tempVar;
						//tempVar=pRstData->Fields->Item["sccd"]->Value;
						//tempVar=pRstData->Fields->Item["regionindex"]->Value;		
						//tempVar=pRstData->Fields->Item["bsvalue"]->Value;
						//tempVar=pRstData->Fields->Item["bslength"]->Value;
						//tempVar=pRstData->Fields->Item["houroffset"]->Value;
						//cout<<j<<"\t"<<tempVar.lVal;
						//tempVar=pRstData->Fields->Item["minuteoffset"]->Value;
						//cout<<"\t"<<tempVar.lVal<<endl;
						
						//�������������˶��ٲ�,ʱ��Բ��Ե���
						//cout<<"===============lNoStorageHours="<<lNoStorageHours<<"Steps="<<Steps<<"========"<<endl;
						pRstData->Fields->Item["QOutput"]->Value=pQout[Parent][j];

						if(SParameter.iCalcSediTrans){ pRstData->Fields->Item["SOutput"]->Value=pSout[Parent][j]; }//20070608,xiaofc,�����Ƿ񱣴���ɳ���	
						else{ pRstData->Fields->Item["SOutput"]->Value=0.0f;}

						pRstData->Fields->Item["QInput"]->Value=pQin[Parent][j];

						pRstData->Fields->Item["SInput"]->Value=pSin[Parent][j];
						pRstData->Fields->Item["trapsout"]->Value=trapSout[Parent][j];
						pRstData->Fields->Item["trapqout"]->Value=trapQout[Parent][j];
						//catch(...)
						//{
						//	cout<<"============errror==========="<<endl;
						//}

						//wh,����ģ���ж�,Ҳ����˵���bSaveFlowPatternΪ1����û��ѡ��avpm����������
						//��Ϊ����BHV�ĳ���д����AVPM��������Ǻ���ġ�
						if(BasinModel[ModelSelect][1].MakeLower()=="avpm" && SParameter.bSaveFlowPattern)//20070622, xiaofc, save flow pattern
						{
							pRstData->Fields->Item["B"]->Value=pFlowB[j];
							pRstData->Fields->Item["H"]->Value=pFlowH[j];
							pRstData->Fields->Item["v"]->Value=pFlow_v[j];
						}

						//20110112, xiaofc, ���������ݿ���ת��������, ���¸��Ӳ���ȡ��
						if(j%(1000+rank*10)==0)
						{
							pRstData->UpdateBatch(ADODB::adAffectAll);
							pCnn->CommitTrans();
							pCnn->BeginTrans();
							//pRstData->Move(j-lNoStorageHours*60/SParameter.MSTEP);20110113,����3�в��ı䵱ǰλ�ã�move������ƶ���Ӧ��������ɴ��󣬻�������
						}
						else if(j%(50+rank)==0)
						{ 
							pRstData->UpdateBatch(ADODB::adAffectAll); 
						}

						pRstData->MoveNext();

					}//end for step
				}//end if ltjyr

				if(BasinModel[ModelSelect][0].MakeLower()=="hsp")
				{
					if(HSP.Min.SaveFlowPattern)
					{
						for(j=lNoStorageHours*60/SParameter.MSTEP;j<Steps;j++)
						{
							pRstData->Fields->Item["qchannel"]->Value=pQout[Parent][j];
							pRstData->Fields->Item["qsoil"]->Value=pDout[Parent][j];
							pRstData->Fields->Item["qoutput"]->Value=pQout[Parent][j]+pDout[Parent][j];
							pRstData->Fields->Item["B"]->Value=pFlowB[j];
							pRstData->Fields->Item["H"]->Value=pFlowH[j];
							pRstData->Fields->Item["v"]->Value=pFlow_v[j];

							if(j%(50+rank)==0){ pRstData->UpdateBatch(ADODB::adAffectAll); }
							if(j%(1000+rank*10)==0)
							{
								pRstData->UpdateBatch(ADODB::adAffectAll);
								pCnn->CommitTrans();
								pCnn->BeginTrans();
								pRstData->Requery(ADODB::adCmdText);
								pRstData->Move(j-lNoStorageHours*60/SParameter.MSTEP);
							}
							pRstData->MoveNext();
						}//end for
					}
					else
					{
						for(j=lNoStorageHours*60/SParameter.MSTEP;j<Steps;j++)
						{
							pRstData->Fields->Item["qchannel"]->Value=pQout[Parent][j];
							pRstData->Fields->Item["qsoil"]->Value=pDout[Parent][j];
							pRstData->Fields->Item["qoutput"]->Value=pQout[Parent][j]+pDout[Parent][j];

							if(j%(50+rank)==0){ pRstData->UpdateBatch(ADODB::adAffectAll); }
							if(j%(1000+rank*10)==0)
							{
								pRstData->UpdateBatch(ADODB::adAffectAll);
								pCnn->CommitTrans();
								pCnn->BeginTrans();
								pRstData->Requery(ADODB::adCmdText);
								pRstData->Move(j-lNoStorageHours*60/SParameter.MSTEP);
							}
							pRstData->MoveNext();
						}//end for step
					}//end else
				}//end if

				pRstData->UpdateBatch(ADODB::adAffectAll);
				pRstData->Close();
			}
			else//����
			{
				//ɾ���ɼ�¼			
				if(BasinModel[ModelSelect][0].MakeLower()=="hsp")
				{
					cSQL.Format("delete from hspdischarge where BSValue=%I64u and BSLength=%d and houroffset>=%d and houroffset<%d and regionindex=%I64u",\
						mTreeNode[TaskLoop].mBSCode.Value,mTreeNode[TaskLoop].mBSCode.Length,SParameter.HourStart+lNoStorageHours,SParameter.HourStart+SParameter.NumofHours,mTreeNode[TaskLoop].mBSCode.RegionIndex);
				}
				else
				{
					cSQL.Format("delete from discharge where sccd=%s and BSValue=%I64u and BSLength=%d and houroffset>=%d and houroffset<%d and regionindex=%I64u",\
						SParameter.sccd,mTreeNode[TaskLoop].mBSCode.Value,mTreeNode[TaskLoop].mBSCode.Length,SParameter.HourStart+lNoStorageHours,SParameter.HourStart+SParameter.NumofHours,mTreeNode[TaskLoop].mBSCode.RegionIndex);//wh,add sccd
				}

				SQL=cSQL.GetString();
				pCnn->Execute(SQL,NULL,ADODB::adCmdText);

				if(BasinModel[ModelSelect][0].MakeLower()=="hsp")
				{
					cSQL.Format("select * from hspdischarge where BSValue=%I64u and BSLength=%d and houroffset>=%d and houroffset<%d and regionindex=%I64u",\
						mTreeNode[TaskLoop].mBSCode.Value,mTreeNode[TaskLoop].mBSCode.Length,SParameter.HourStart+lNoStorageHours,SParameter.HourStart+SParameter.NumofHours,mTreeNode[TaskLoop].mBSCode.RegionIndex);
				}
				else
				{
					cSQL.Format("select * from discharge where sccd=%s and BSValue=%I64u and BSLength=%d and houroffset>=%d and houroffset<%d and regionindex=%I64u",\
						SParameter.sccd,mTreeNode[TaskLoop].mBSCode.Value,mTreeNode[TaskLoop].mBSCode.Length,SParameter.HourStart+lNoStorageHours,SParameter.HourStart+SParameter.NumofHours,mTreeNode[TaskLoop].mBSCode.RegionIndex);//wh,add sccd
				}

				SQL=cSQL.GetString();
				pRstData->Open(SQL,(ADODB::_Connection*)pCnn,ADODB::adOpenForwardOnly,ADODB::adLockOptimistic,ADODB::adCmdText);
				tempCom.ChangeType(VT_DECIMAL);

				if(BasinModel[ModelSelect][0].MakeLower() == "ltjyr")
				{
					for(j=lNoStorageHours*60/SParameter.MSTEP;j<Steps;j++)
					{
						pRstData->AddNew();

						tempCom.ullVal=mTreeNode[TaskLoop].mBSCode.RegionIndex;
						pRstData->Fields->Item["Regionindex"]->Value=tempCom;
						tempCom.ullVal=mTreeNode[TaskLoop].mBSCode.Value;
						pRstData->Fields->Item["BSValue"]->Value=tempCom;
						pRstData->Fields->Item["BSLength"]->Value=mTreeNode[TaskLoop].mBSCode.Length;
						pRstData->Fields->Item["Houroffset"]->Value=SParameter.HourStart+long(j*SParameter.MSTEP/60);
						pRstData->Fields->Item["MinuteOffset"]->Value=j*SParameter.MSTEP%60;

						//20070608,xiaofc,�����Ƿ񱣴���ɳ���
						//2008.3.23,wh,����ģ���жϻ���
						//wh,added,sccd,2008.3.24
						vtmp = SParameter.sccd;
						pRstData->Fields->Item["sccd"]->Value = _variant_t(vtmp);

						pRstData->Fields->Item["QOutput"]->Value=pQout[Parent][j];
						pRstData->Fields->Item["trapqout"]->Value=trapQout[Parent][j];

						if(SParameter.iCalcSediTrans) {pRstData->Fields->Item["SOutput"]->Value=pSout[Parent][j];pRstData->Fields->Item["trapsout"]->Value=trapSout[Parent][j];}
						else { pRstData->Fields->Item["SOutput"]->Value=0.0f;pRstData->Fields->Item["trapsout"]->Value=0.0f;}

						pRstData->Fields->Item["QInput"]->Value=pQin[Parent][j];
						pRstData->Fields->Item["SInput"]->Value=pSin[Parent][j];	

						//2008.3.23,����ģ���жϻ���
						if(BasinModel[ModelSelect][1].MakeLower()=="avpm" && SParameter.bSaveFlowPattern)//20070622, xiaofc, save flow pattern
						{
							pRstData->Fields->Item["B"]->Value=pFlowB[j];
							pRstData->Fields->Item["H"]->Value=pFlowH[j];
							pRstData->Fields->Item["v"]->Value=pFlow_v[j];
						}

						if(j%(40+rank)==0){ pRstData->UpdateBatch(ADODB::adAffectAll); }
						if(j%(1000+rank*10)==0)
						{
							pRstData->UpdateBatch(ADODB::adAffectAll);
							pCnn->CommitTrans();
							pCnn->BeginTrans();
							pRstData->Requery(ADODB::adCmdText);
						}
					}//end for step
				}//end if ltjyr

				if(BasinModel[ModelSelect][0].MakeLower()=="hsp")
				{
					//��if�ó�����Ϊ�˼ӿ��ٶȣ�����ѭ���������߼��жϵĴ���
					if(HSP.Min.SaveFlowPattern)
					{
						for(j=lNoStorageHours*60/SParameter.MSTEP;j<Steps;j++)
						{
							pRstData->AddNew();
							tempCom.ullVal=mTreeNode[TaskLoop].mBSCode.RegionIndex;
							pRstData->Fields->Item["Regionindex"]->Value=tempCom;
							tempCom.ullVal=mTreeNode[TaskLoop].mBSCode.Value;
							pRstData->Fields->Item["BSValue"]->Value=tempCom;
							pRstData->Fields->Item["BSLength"]->Value=mTreeNode[TaskLoop].mBSCode.Length;
							pRstData->Fields->Item["Houroffset"]->Value=SParameter.HourStart+long(j*SParameter.MSTEP/60);
							pRstData->Fields->Item["MinuteOffset"]->Value=j*SParameter.MSTEP%60;

							pRstData->Fields->Item["qchannel"]->Value=pQout[Parent][j];
							pRstData->Fields->Item["qsoil"]->Value=pDout[Parent][j];
							pRstData->Fields->Item["qoutput"]->Value=pQout[Parent][j]+pDout[Parent][j];
							pRstData->Fields->Item["B"]->Value=pFlowB[j];
							pRstData->Fields->Item["H"]->Value=pFlowH[j];
							pRstData->Fields->Item["v"]->Value=pFlow_v[j];

							if(j%(40+rank)==0){ pRstData->UpdateBatch(ADODB::adAffectAll); }
							if(j%(1000+rank*10)==0)
							{
								pRstData->UpdateBatch(ADODB::adAffectAll);
								pCnn->CommitTrans();
								pCnn->BeginTrans();
								pRstData->Requery(ADODB::adCmdText);
							}
						}//end for step
					}
					else
					{
						for(j=lNoStorageHours*60/SParameter.MSTEP;j<Steps;j++)
						{
							pRstData->AddNew();
							tempCom.ullVal=mTreeNode[TaskLoop].mBSCode.RegionIndex;
							pRstData->Fields->Item["Regionindex"]->Value=tempCom;
							tempCom.ullVal=mTreeNode[TaskLoop].mBSCode.Value;
							pRstData->Fields->Item["BSValue"]->Value=tempCom;
							pRstData->Fields->Item["BSLength"]->Value=mTreeNode[TaskLoop].mBSCode.Length;
							pRstData->Fields->Item["Houroffset"]->Value=SParameter.HourStart+long(j*SParameter.MSTEP/60);
							pRstData->Fields->Item["MinuteOffset"]->Value=j*SParameter.MSTEP%60;

							pRstData->Fields->Item["qchannel"]->Value=pQout[Parent][j];
							pRstData->Fields->Item["qsoil"]->Value=pDout[Parent][j];
							pRstData->Fields->Item["qoutput"]->Value=pQout[Parent][j]+pDout[Parent][j];

							if(j%(40+rank)==0){ pRstData->UpdateBatch(ADODB::adAffectAll); }
							if(j%(1000+rank*10)==0)
							{
								pRstData->UpdateBatch(ADODB::adAffectAll);
								pCnn->CommitTrans();
								pCnn->BeginTrans();
								pRstData->Requery(ADODB::adCmdText);
							}
						}//end for step
					}

				}//end if hsp

				pRstData->UpdateBatch(ADODB::adAffectAll);
				pRstData->Close();
			}//end else����
			pCnn->CommitTrans();//��������
		}//end else
	}//end try
	catch (_com_error e) 
	{
		cout<<e.Error()<<endl;
		cout<<e.ErrorMessage()<<endl;
		cout<<(LPCSTR)e.Source()<<endl;        
		cout<<(LPCSTR)e.Description()<<endl;

		cout<<"Error Occured On Rank "<<rank<<" While Writing to DB."<<endl;
		cout<<"Task "<<TaskLoop+1<<" of "<<TaskCount<<"."<<endl;
		cout<<"sccd="<<SParameter.sccd<<endl;//wh
		cout<<"RegionIndex="<<mTreeNode[TaskLoop].mBSCode.RegionIndex<<"\tBSValue="<<mTreeNode[TaskLoop].mBSCode.Value<<"\tBSLength="<<mTreeNode[TaskLoop].mBSCode.Length<<endl;
		cout<<"HourOffset="<<SParameter.HourStart+long(j*SParameter.MSTEP/60)<<"\tMinuteOffset="<<j*SParameter.MSTEP%60<<endl;

		if(BasinModel[ModelSelect][0].MakeLower() == "ltjyr")
		{ 
			cout<<"Qout="<<pQout[Parent][j]<<"\tDout="<<pDout[Parent][j]<<endl;
			if(HSP.Min.SaveFlowPattern)
				cout<<"B="<<pFlowB[j]<<"\tH="<<pFlowH[j]<<"\tv="<<pFlow_v[j]<<endl;
		}
		else
		{ 
			cout<<"Qout="<<pQout[Parent][j]<<"\tSout="<<pSout[Parent][j]<<"\tQin="<<pQin[Parent][j]<<"\tSin="<<pSin[Parent][j]<<endl;
			if(SParameter.bSaveFlowPattern)//20070622,xiaofc
				cout<<"B="<<pFlowB[j]<<"\tH="<<pFlowH[j]<<"\tv="<<pFlow_v[j]<<endl;
		}
		cout<<"SQL: "<<cSQL<<endl;
		exit(0);
	}//end catch
}


//�ӽڵ������ɺ����β����//������ͷţ����ݿ�Ĺرգ��ѿռ��ͷ�
void SlaveProcess::Finalize(void)
{
	pCnn->Close();

	delete[] ZeroSerial; ZeroSerial = NULL;   
	delete[] pQUpRegion; pQUpRegion = NULL;
	delete[] pSUpRegion; pSUpRegion = NULL;
	delete[] pDUpRegion; pDUpRegion = NULL;

	//20070622, xiaofc, Clear memory for flow pattern
	if(BasinModel[ModelSelect][1].MakeLower()=="avpm" || (BasinModel[ModelSelect][0].MakeLower()=="hsp" && HSP.Min.SaveFlowPattern==true))
	{
		if(NULL!=pFlowB) { delete[] pFlowB;  pFlowB = NULL; }
		if(NULL!=pFlowH) { delete[] pFlowH;  pFlowH = NULL; }
		if(NULL!=pFlow_v){ delete[] pFlow_v; pFlow_v = NULL;}
	}

	if(MLTJYR=="ltjyr")
	{
		spWaterBasin->Finalize(this->MSRM.AllocSysString());//�ͷ�SRM��dll��Ӧ�����ڲ���dll�������Ժ���ͷţ�������ѩ���޷��㡣
		spWaterBasin.Release();	//���⣬����ͷ�
	}
	if(MXAJ=="xaj")
	{
		spXAJmodel->Finalize(this->MSRM.AllocSysString());
		spXAJmodel.Release();
	}
}


void SlaveProcess::RecvMasterProcess_reservoir(void)
{
	Para mPara;//��ת����
	BSCode mBSCode;
	int TaskLoop;

	//pFlowB��ÿ���������ֻ��Ҫ��һ�����飬����Ҫÿ���յ�һ�������newһ��
	bool BHV_flag=true;

	while(1)
	{
		//////////////////////////////////////////////////////////////////////////
		/////////////////////////�����ؽڵ���պӶ�����///////////////////////////
		//////////////////////////////////////////////////////////////////////////
		CommTime=MPI_Wtime();//ͨѶʱ��
		cout<<"@"<<processor_name<<","<<rank<<",is ready to receive."<<endl;//isǰ��ġ���������ɾ��������Ҫ�ģ��Ǻǡ�
		MPI_Recv(&TaskCount,1,MPI_INT,0,10,MPI_COMM_WORLD,&Status);

		if( TaskCount==0 )
		{
			cout<<"@"<<processor_name<<","<<rank<<",is ready to exit."<<endl;
			break;
		}	
		else
		{
			mTreeNode = new TreeNode[TaskCount];//һ��TreeNode����һ���Ӷ�
			MPI_Recv(mTreeNode,TaskCount*sizeof(TreeNode),MPI_BYTE,0,10,MPI_COMM_WORLD,&Status);
		}
		CommTime=MPI_Wtime()-CommTime;
		cout<<"#"<<processor_name<<","<<rank<<",RCV,"<<TaskCount<<":"<<endl;//��������ĺӶ���
		cout<<"#"<<processor_name<<","<<rank<<",TCM,"<<CommTime<<":"<<endl<<endl;//���������ʱ��


		//////////////////////////////////////////////////////////////////////////
		/////////////////////2008.3.23��������Ӷ�ģ���жϽӿ�////////////////////
		//////////////////////////////////////////////////////////////////////////
		CalTime=MPI_Wtime();//����ʱ�俪ʼ��ʱ

		ModelSelect = GetBasinModelRegionIndex(mTreeNode[TaskCount-1].mBSCode.RegionIndex);//�õ����������������ڵڼ���

		//����ģ���ж�
		if(((BasinModel[ModelSelect][1].MakeLower()=="avpm") || (BasinModel[ModelSelect][0].MakeLower()=="hsp" && HSP.Min.SaveFlowPattern==true)) && BHV_flag)
		{
			BHV_flag=false;

			//��ǰ������̼���ʼ��ֻnewһ��
			pFlowB  = new float[Steps+1];  ZeroFill(pFlowB,Steps+1);
			pFlowH  = new float[Steps+1];  ZeroFill(pFlowH,Steps+1);
			pFlow_v = new float[Steps+1];  ZeroFill(pFlow_v,Steps+1);				
		}

		//////////////////////////////////////////////////////////////////////////
		//////////////�ڵ�Ԫ����ѭ��ǰ����ת���ǵ�ָ��������ٿռ�////////////////
		//////////////////////////////////////////////////////////////////////////
		if(BasinModel[ModelSelect][0].MakeLower()=="hsp")
		{
			//wh:ÿ�õ�һ�������Ҫnewһ�Σ���Ϊÿ����������Ӷ����Ƕ�̬��ȷ���ġ�
			pDout = new float* [TaskCount]; 
			pQout = new float* [TaskCount];
		}
		else
		{
			pQin = new float* [TaskCount];  pQout= new float* [TaskCount];
			pSin = new float* [TaskCount];  pSout= new float* [TaskCount];
			pWLM = new float* [TaskCount];  pWRM = new float* [TaskCount];
			trapQout = new float* [TaskCount];  trapSout= new float* [TaskCount];
		}


		CalTime=MPI_Wtime()-CalTime;//����ʱ�����
		cout<<"#"<<processor_name<<","<<rank<<",TCL,"<<CalTime<<":"<<endl;

		//////////////////////////////////////////////////////////////////////////
		//////////////////////////��ÿһ������ĺӶν��м���//////////////////////
		//////////////////////////////////////////////////////////////////////////
		for(TaskLoop=0; TaskLoop<TaskCount; TaskLoop++)
		{
			CalTime=MPI_Wtime();//����ʱ�俪ʼ��ʱ

			mBSCode=mTreeNode[TaskLoop].mBSCode;
			mPara=mTreeNode[TaskLoop].mPara;

			if(BasinModel[ModelSelect][0].MakeLower()=="hsp")
			{
				pDout[TaskLoop]= new float[Steps+1]; pQout[TaskLoop]= new float[Steps+1];
				::ZeroFill(pDout[TaskLoop],Steps+1); ::ZeroFill(pQout[TaskLoop],Steps+1);
			}
			else
			{
				pQin[TaskLoop] = new float[Steps+1];   pSin[TaskLoop] = new float[Steps+1];
				pQout[TaskLoop]= new float[Steps+1];   pSout[TaskLoop]= new float[Steps+1];
				pWLM[TaskLoop] = new float[Steps+1];   pWRM[TaskLoop] = new float[Steps+1];
				trapQout[TaskLoop]= new float[Steps+1];  trapSout[TaskLoop]= new float[Steps+1];;

				//wh:��Ҫ��ʼ��������new������ÿ��ֵ���ɿأ����°���ģ�ͼ���������û����ɳ��pSout�����ֵ�����Խ�磬����д�����ݿ⡣
				::ZeroFill(pQin[TaskLoop] ,Steps+1);   ::ZeroFill(pSin[TaskLoop] ,Steps+1);
				::ZeroFill(pQout[TaskLoop],Steps+1);   ::ZeroFill(pSout[TaskLoop],Steps+1);
				::ZeroFill(pWLM[TaskLoop] ,Steps+1);   ::ZeroFill(pWRM[TaskLoop] ,Steps+1);
				::ZeroFill(trapQout[TaskLoop], Steps+1); ::ZeroFill(trapSout[TaskLoop], Steps+1);
			}

			Parent=TaskLoop;
			Boy=-1;//ȡQBoy QGirl,��-1,�����ں���֪����û�ҵ�
			Girl=-1;

			CalTime=MPI_Wtime()-CalTime;//����ʱ�����
			cout<<"#"<<processor_name<<","<<rank<<",TCL,"<<CalTime<<":Initialization"<<endl;

			//�����ˮ��ڵ�
			//if(RsvList->find(mBSCode.Length,mBSCode.Value,mBSCode.RegionIndex))
			//{
			//	CalTime=MPI_Wtime();//����ʱ�俪ʼ��ʱ
			//	ReservoirSegment(mBSCode,TaskLoop);
			//	CalTime=MPI_Wtime()-CalTime;//����ʱ�����
			//	cout<<"#"<<processor_name<<","<<rank<<",TCL,"<<CalTime<<":Reservoir"<<endl;
			//}

				//20090918,xiaofc,runoffcalc��������ݿ�϶࣬Ӧ���ڲ�������ʱ
				//����WaterYield��Դ���뵥���޸�
				RunoffCalc(mBSCode,mPara,TaskLoop);//������ɳ����//wh,�Ѿ�������ģ��

				CalTime=MPI_Wtime();//����ʱ�俪ʼ��ʱ

				//2008.2.17,wh,������ת���̣�������Ҫ�������кӶ�������Ϣʱ��ͨ����Ϣ�����������ڵ����������
				if(mTreeNode[TaskLoop].StralherOrder>1)
				{
					for(int j=0;j<TaskLoop;j++)//find which is boy and girl	
					{			
						if((mTreeNode[j].mBSCode.Length==mBSCode.Length+1) && (mTreeNode[j].mBSCode.Value==mBSCode.Value<<1)) { Boy=j;}
						else if((mTreeNode[j].mBSCode.Length==mBSCode.Length+1) && (mTreeNode[j].mBSCode.Value==(mBSCode.Value<<1)+1)) { Girl=j;}		
					}
					if(Boy==-1 && Girl==-1)//��Ҫȥ������
						this->NoChild(mBSCode);

					if((Boy==-1 && Girl!=-1) || (Boy!=-1 && Girl==-1))
						this->OneChild(mBSCode,Boy,Girl);
				}

				ConfluenceCalc(TaskLoop,mBSCode,mPara);//�������㣬�ھ�ȷ��ɢ�����м�����������ʴ���ӿ�ˮ����ٵȡ�

				//davidthu,������ı����ڲ�����ӵ�

				//davidthu,�������������ˮ����������
				if(RsvList->find(mBSCode.Length,mBSCode.Value,mBSCode.RegionIndex))
				{
					//����ҵ�ˮ��ڵ㣬������ˮ����
						ofstream myFileRes;
						ofstream myFileResObs;
					    myFileRes.open("ReservoirModule.txt",ios::app);
					    myFileResObs.open("SegObs.txt",ios::app);
						CString CSSQL;
	                    _bstr_t bSQL;
						float storage;
						CSSQL.Format("Select * from definednodes where regionindex=%I64u and BSValue=%I64u and BSLength=%d",mBSCode.RegionIndex, mBSCode.Value,mBSCode.Length);
			            bSQL=CSSQL.GetString();
						try
						{
							pRstIndicator->Open(bSQL,(ADODB::_Connection*)pCnn,ADODB::adOpenStatic,ADODB::adLockOptimistic,ADODB::adCmdText);
							storage=pRstIndicator->Fields->Item["storage"]->Value;
							storage=storage*10000.0f;
							pRstIndicator->Close();
							myFileRes<<"regionindex="<<mBSCode.RegionIndex<<" bslength="<<mBSCode.Length<<" bsvalue="<<mBSCode.Value<<" lTimeInterval="<<lTimeInterval<<" ReservoirStorage="<<storage<<"\n"<<flush;
							int i=0;
							float DeliveryAmount=(pQout[TaskLoop][i]+pQout[TaskLoop][i]*pSout[TaskLoop][i]/2650.0f)*lTimeInterval;
							while(DeliveryAmount<=storage && i<Steps)
							{
								trapQout[TaskLoop][i]=pQout[TaskLoop][i];
							    trapSout[TaskLoop][i]=pSout[TaskLoop][i];
								pQout[TaskLoop][i]=0.0f;
								pSout[TaskLoop][i]=0.0f;
								if (mBSCode.RegionIndex==1001 && mBSCode.Value==1 && mBSCode.Length==92)
								{
								myFileResObs<<"i="<<i<<" DeliveryAmount="<<DeliveryAmount<<" Storage="<<storage<<" trapQout="<<trapQout[TaskLoop][i]<<" trapSout="<<trapSout[TaskLoop][i]<<" pQout="<<pQout[TaskLoop][i]<<" pSout="<<pSout[TaskLoop][i]<<"\n"<<flush;
								}
								i=i+1;
								DeliveryAmount=DeliveryAmount+(pQout[TaskLoop][i]+pQout[TaskLoop][i]*pSout[TaskLoop][i]/2650.0f)*lTimeInterval;
							}
							if (DeliveryAmount<=storage)
							{
								trapQout[TaskLoop][i]=pQout[TaskLoop][i];
							    trapSout[TaskLoop][i]=pSout[TaskLoop][i];
								pQout[TaskLoop][i]=0.0f;
								pSout[TaskLoop][i]=0.0f;
							    if (mBSCode.RegionIndex==1001 && mBSCode.Value==1 && mBSCode.Length==92)
								{
								myFileResObs<<"i="<<i<<" DeliveryAmount="<<DeliveryAmount<<" Storage="<<storage<<" trapQout="<<trapQout[TaskLoop][i]<<" trapSout="<<trapSout[TaskLoop][i]<<" pQout="<<pQout[TaskLoop][i]<<" pSout="<<pSout[TaskLoop][i]<<"\n"<<flush;
								}

							}

							myFileRes<<"Steps="<<Steps<<" i="<<i<<" DeliveryAmount="<<DeliveryAmount<<"\n"<<flush;
							myFileRes.close();
							myFileResObs.close();
						}
						catch(_com_error e)
						{
							myFileRes<<"regionindex="<<mBSCode.RegionIndex<<" bslength="<<mBSCode.Length<<" bsvalue="<<mBSCode.Value<<" ReservoirStorage="<<storage<<" lTimeInterval="<<lTimeInterval<<" Error!"<<"\n"<<flush;
							myFileRes.close();
							cout<<"Failture Of ReservoirModule."<<rank<<endl;
							myFileRes.close();
							cout<<e.Error()<<endl;
							cout<<e.ErrorMessage()<<endl;
							cout<<(LPCSTR)e.Source()<<endl;        
							cout<<(LPCSTR)e.Description()<<endl;
							MPI_Finalize();
							exit(0);
						}
				}



				CalTime=MPI_Wtime()-CalTime;//����ʱ�����
				cout<<"#"<<processor_name<<","<<rank<<",TCL,"<<CalTime<<":Confluence"<<endl;



			/***********************************************************�������ݿ⼰��ת���̲���*******************************************************/
			DBUpdateTime=MPI_Wtime();//д�����ݿ�ʱ�俪ʼ��ʱ

			//wh,�����ѡ��avpm������������ʴҲ����������㣬����Ҳ����Ȼ���ñ���
			if(BasinModel[ModelSelect][1].MakeLower()=="avpm" && SParameter.bSaveGravityEvents)//����������ʴ�¼�
				SaveGravityEvents(TaskLoop);

			WriteToDischarge(TaskLoop);//�������Ͳ�ɳ���д��discharge��

			DBUpdateTime=MPI_Wtime()-DBUpdateTime;//���ݿ�д��ʱ�����
			cout<<"#"<<processor_name<<","<<rank<<",TDB,"<<DBUpdateTime<<":"<<endl;
			/******************************************************************************************************************************************/

			//����һ���Ӷ�,��ͬģ���ںӶμ䴫�ݵı�����һ����ͬ�����յ���ȻҲ�Ͳ�ͬ
			if(BasinModel[ModelSelect][0].MakeLower()=="hsp")
			{
				if(Boy!=-1)
				{
					delete[] pQout[Boy];  pQout[Boy]=NULL;
					delete[] pDout[Boy];  pDout[Boy]=NULL;	
				}
				if(Girl!=-1)
				{
					delete[] pQout[Girl]; pQout[Girl]=NULL;
					delete[] pDout[Girl]; pDout[Girl]=NULL;	
				}
			}
			else
			{
				delete[] pQin[Parent];  pQin[Parent]=NULL;
				delete[] pSin[Parent];  pSin[Parent]=NULL;
				delete[] pWLM[Parent];  pWLM[Parent]=NULL;
				delete[] pWRM[Parent];  pWRM[Parent]=NULL;
				if(Boy!=-1)
				{
					delete[] pQout[Boy];  pQout[Boy]=NULL;
					delete[] pSout[Boy];  pSout[Boy]=NULL;	
					delete[] trapQout[Boy];  trapQout[Boy]=NULL;
					delete[] trapSout[Boy];  trapSout[Boy]=NULL;	
				}
				if(Girl!=-1)
				{
					delete[] pQout[Girl]; pQout[Girl]=NULL;
					delete[] pSout[Girl]; pSout[Girl]=NULL;	
					delete[] trapQout[Girl];  trapQout[Girl]=NULL;
					delete[] trapSout[Girl];  trapSout[Girl]=NULL;	
				}
			}

			cout<<"#"<<processor_name<<","<<rank<<",CAL,1:"<<mBSCode.RegionIndex<<","<<mBSCode.Value<<","<<mBSCode.Length<<endl;

			lCpuUsage=m_CpuUsage.GetCpuUsageNT();
			iCpuUsage=(int)lCpuUsage;
			cout<<"#"<<processor_name<<","<<rank<<",CPU,"<<iCpuUsage<<":"<<endl<<endl;//20060327,������,����CPU������//renewed by xia	

		}//end for(TaskLoop...)

		//2008.2.17,wh,����ת���̷�����Ϣ
		SendTime = MPI_Wtime();
	    PackSend(TaskCount-1);//wh����ת���̷���"��"�ڵ����������
	    MPI_Send(&(mTreeNode[TaskCount-1].mBSCode),sizeof(BSCode),MPI_BYTE,0,10,MPI_COMM_WORLD);//�����ؽ��̷�����Ϣ

		SendTime = MPI_Wtime() - SendTime;
		cout<<"#"<<processor_name<<","<<rank<<",TCM,"<<SendTime<<":"<<endl;//���������ؽ��̺���ת���̷�����Ϣ��ʱ��

		//����������
		//�ڵ�Ԫ����ѭ�������ת���ǵ�ָ���ջؿռ�
		if(BasinModel[ModelSelect][0].MakeLower()=="hsp")
		{
			delete[] pQout[TaskCount-1]; pQout[TaskCount-1]=NULL;//wh
			delete[] pDout[TaskCount-1]; pDout[TaskCount-1]=NULL;//wh
			delete[] pQout; delete[] pDout;
		}
		else
		{
			delete[] pQout[TaskCount-1]; pQout[TaskCount-1]=NULL;//wh
			delete[] pSout[TaskCount-1]; pSout[TaskCount-1]=NULL;//wh
			delete[] pQin; delete[] pQout;
			delete[] pSin; delete[] pSout;
			delete[] trapQout; delete[] trapSout;
			delete[] pWLM; delete[] pWRM;
		}
		delete [] mTreeNode;

		cout<<"#"<<processor_name<<","<<rank<<",FNS,"<<TaskCount<<":"<<endl<<endl;

	}//end while(1)
	Finalize();	
}
