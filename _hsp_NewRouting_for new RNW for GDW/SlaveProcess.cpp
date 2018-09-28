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
//李铁键产流产沙模型dll组件调用
#include "E:\models\DYRIM_raw\WaterYield_dll20130915 for GDW\WaterYield\WaterYield.h"
#include "E:\models\DYRIM_raw\WaterYield_dll20130915 for GDW\WaterYield\WaterYield_i.c"
#include "E:\models\DYRIM_raw\XAJ_dll\新安江模型\新安江模型\My.h"
#include "E:\models\DYRIM_raw\XAJ_dll\新安江模型\新安江模型\My_i.c"

CComQIPtr <IWaterBasin,&IID_IWaterBasin> spWaterBasin;//ltj水沙耦合模型
CComQIPtr <IMy,&IID_IMy> spXAJmodel;//三水源新安江模型
HighSlopeRunoff HSP;//高边坡产流模型

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

//计算进程一些变量的初始化，组织水库节点链表，因为很多变量只有从文本读取后才有值，因此不能放到构造函数中
//每个计算进程都运行一次
void SlaveProcess::SlaveProcessInitialize(void)
{
	///////////模型计算中转变量初始化///////////
	ZeroSerial = new float[Steps+1];   ZeroFill(ZeroSerial,Steps+1);
	pQUpRegion = new float[Steps+1];   ZeroFill(pQUpRegion,Steps+1);
	pSUpRegion = new float[Steps+1];   ZeroFill(pSUpRegion,Steps+1);
	pDUpRegion = new float[Steps+1];   ZeroFill(pDUpRegion,Steps+1);


	//////////////数据库参数初始化/////////////
	pRstData.CreateInstance(__uuidof(ADODB::Recordset));//指向Discharge表
	pRstQ.CreateInstance(__uuidof(ADODB::Recordset));//指向Discharge表
	pRstRsv.CreateInstance(__uuidof(ADODB::Recordset));
	pRstIndicator.CreateInstance(__uuidof(ADODB::Recordset));
	pRstGravityEvents.CreateInstance(__uuidof(ADODB::Recordset));


	//wh added,2008,读入BasinModel表，多模型初始化
	/********************************************************************************************************************/
	CString CSSQL;
	_bstr_t bSQL;

	CSSQL.Format("Select * from basinmodel where sccd=%s order by regionindex desc",SParameter.sccd);//从大到小排列
	bSQL = CSSQL.GetString();
	int i=0;//不同regionindex的个数
	int j=0;//列数，存储模型
	try
	{
		pRstIndicator->Open(bSQL,(ADODB::_Connection*)pCnn,ADODB::adOpenStatic,ADODB::adLockOptimistic,ADODB::adCmdText);

		RowCount = pRstIndicator->RecordCount;
		ColumnCount = 4;//wh,以后模型增加后该值要改。

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
	//读取全流域面积，分配基流
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


	///////////////模型选择初始化//////////////

	//以下对应BasinModel表，表示相应模型是否存在，决定计算进程是否链接该模型的dll,否则如果一次性全链接，缺一个无关紧要的dll，程序都运行不下去//
	//没有dll的无所谓
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
		cout<<"您没有选择任何产流模型，请重新指定."<<endl;
		return;
	}

	if(MLTJYR=="ltjyr")//说明存在，才链接
	{
		CString name=this->processor_name;
		hr = spWaterBasin.CoCreateInstance( CLSID_WaterBasin );
		//20101210,xiaofc, Assert is not good, err message should be given
		//assert(SUCCEEDED(hr));
		if(!SUCCEEDED(hr))
		{
			cout<<"'ltjyr'@"<<this->processor_name<<"初始化失败。\n产流模型模块加载错误，请检查并注册WaterYield.dll。"<<endl;
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
			cout<<"'xaj'@"<<this->processor_name<<"初始化失败。\n产流模型模块加载错误，请检查并注册新安江模型.dll。"<<endl;
			exit(88888);
		}
		hr = spXAJmodel->Initialize(MSRM.AllocSysString(),BasinArea,XAJ,SParameter.sccd.AllocSysString(),SParameter.CSUser.AllocSysString(),SParameter.CSPassword.AllocSysString(),SParameter.CSDatasource.AllocSysString(),SParameter.sRainType.AllocSysString(),SParameter.HourStart,SParameter.NumofHours,SParameter.MSTEP,this->Steps,SParameter.StatusTime);
		assert(SUCCEEDED(hr));
	}
	if(MHSP=="hsp")
	{
		HSP.Initiallize(pCnn,SParameter.sRainType,SParameter.NumofHours,SParameter.HourStart,SParameter.StatusTime,BasinArea,SParameter.MSTEP,this->Steps);
	}

	///////////////水库节点初始化//////////////
	if(SParameter.bCalRsvUp)//该参数如果设为1表示老子要算水库，但是不好意思，现在能力不行。
	{
		cout<<"Rerservoir Codes were not well prepared, please contact developer!"<<endl;//这里以后要加入水库以上汇流和水库过程，结果写入reservoir表中
	}

	//把definednodes表中的RS节点插入到RsvList链表中
	else//不计算水库以上的汇流和水库过程
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
				vtmp = pRstIndicator->Fields->Item["BSValue"]->Value;//读入的是variant类型的变量，所以要先传给vtmp
				vtmp2 = pRstIndicator->Fields->Item["RegionIndex"]->Value;//读入的是variant类型的变量，所以要先传给vtmp
				RsvList->insert(pRstIndicator->Fields->Item["BSLength"]->Value,vtmp.ullVal,-1,vtmp2.ullVal);//水库没有Sorder，所以写为-1
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


//wh,2008，得到河段配置Parameter表哪个RegionIndex的参数
//如果有相等的就配置相等的，没有相等的就配置离它最近一级的长辈。
//输入参数为河段的RegionIndex
int SlaveProcess::GetBasinModelRegionIndex(unsigned long long regionindex)
{
	assert(RowCount>0);
	while(1>0)
	{
		for(int i=0; i<RowCount; i++)
		{
			if(regionindex == BasinModelIndex[i])
				return i;
			if(regionindex>BasinModelIndex[i])//因为从小到大排的
				break;
		}
		regionindex /= SParameter.RegionSystem;//20130904, shy
		//if(regionindex==1)
		if(regionindex<SParameter.CompRegion+1)//shy, regionindex小于CompRegion+1，则已到干流,20140514
			break;
	}//end while
	return 0;
}


//计算进程循环处理主控进程发送的消息，计算完后向主控和中转进程通信
void SlaveProcess::RecvMasterProcess(void)
{
	Para mPara;//中转变量
	BSCode mBSCode;
	int TaskLoop;

	//pFlowB在每个计算进程只需要有一个数组，不需要每接收到一次任务就new一次
	bool BHV_flag=true;

	while(1)
	{
		//////////////////////////////////////////////////////////////////////////
		/////////////////////////从主控节点接收河段数组///////////////////////////
		//////////////////////////////////////////////////////////////////////////
		CommTime=MPI_Wtime();//通讯时间
		cout<<"@"<<processor_name<<","<<rank<<",is ready to receive."<<endl;//is前面的“，”不能删除，很重要的，呵呵。
		MPI_Recv(&TaskCount,1,MPI_INT,0,10,MPI_COMM_WORLD,&Status);

		if( TaskCount==0 )
		{
			cout<<"@"<<processor_name<<","<<rank<<",is ready to exit."<<endl;
			break;
		}	
		else
		{
			mTreeNode = new TreeNode[TaskCount];//一个TreeNode代表一个河段
			MPI_Recv(mTreeNode,TaskCount*sizeof(TreeNode),MPI_BYTE,0,10,MPI_COMM_WORLD,&Status);
		}
		CommTime=MPI_Wtime()-CommTime;
		cout<<"#"<<processor_name<<","<<rank<<",RCV,"<<TaskCount<<":"<<endl;//接收任务的河段数
		cout<<"#"<<processor_name<<","<<rank<<",TCM,"<<CommTime<<":"<<endl<<endl;//接收任务的时间


		//////////////////////////////////////////////////////////////////////////
		/////////////////////2008.3.23，王皓增加多模型判断接口////////////////////
		//////////////////////////////////////////////////////////////////////////
		CalTime=MPI_Wtime();//计算时间开始计时

		ModelSelect = GetBasinModelRegionIndex(mTreeNode[TaskCount-1].mBSCode.RegionIndex);//得到它所隶属的区域在第几行

		//汇流模型判断
		if(((BasinModel[ModelSelect][1].MakeLower()=="avpm") || (BasinModel[ModelSelect][0].MakeLower()=="hsp" && HSP.Min.SaveFlowPattern==true)) && BHV_flag)
		{
			BHV_flag=false;

			//当前计算进程计算始终只new一次
			pFlowB  = new float[Steps+1];  ZeroFill(pFlowB,Steps+1);
			pFlowH  = new float[Steps+1];  ZeroFill(pFlowH,Steps+1);
			pFlow_v = new float[Steps+1];  ZeroFill(pFlow_v,Steps+1);				
		}

		//////////////////////////////////////////////////////////////////////////
		//////////////在单元流域循环前给中转量们的指针个数开辟空间////////////////
		//////////////////////////////////////////////////////////////////////////
		if(BasinModel[ModelSelect][0].MakeLower()=="hsp")
		{
			//wh:每得到一个任务就要new一次，因为每个任务包含河段数是动态不确定的。
			pDout = new float* [TaskCount]; 
			pQout = new float* [TaskCount];
		}
		else
		{
			pQin = new float* [TaskCount];  pQout= new float* [TaskCount];
			pSin = new float* [TaskCount];  pSout= new float* [TaskCount];
			pWLM = new float* [TaskCount];  pWRM = new float* [TaskCount];
		}

		CalTime=MPI_Wtime()-CalTime;//计算时间结束
		cout<<"#"<<processor_name<<","<<rank<<",TCL,"<<CalTime<<":"<<endl;

		//////////////////////////////////////////////////////////////////////////
		//////////////////////////对每一条具体的河段进行计算//////////////////////
		//////////////////////////////////////////////////////////////////////////
		for(TaskLoop=0; TaskLoop<TaskCount; TaskLoop++)
		{
			CalTime=MPI_Wtime();//计算时间开始计时

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

				//wh:需要初始化，否则new的数据每项值不可控，在新安江模型计算中由于没有算沙，pSout的随机值会出现越界，不能写入数据库。
				::ZeroFill(pQin[TaskLoop] ,Steps+1);   ::ZeroFill(pSin[TaskLoop] ,Steps+1);
				::ZeroFill(pQout[TaskLoop],Steps+1);   ::ZeroFill(pSout[TaskLoop],Steps+1);
				::ZeroFill(pWLM[TaskLoop] ,Steps+1);   ::ZeroFill(pWRM[TaskLoop] ,Steps+1);
			}

			Parent=TaskLoop;
			Boy=-1;//取QBoy QGirl,置-1,以能在后来知道是没找到
			Girl=-1;

			CalTime=MPI_Wtime()-CalTime;//计算时间结束
			cout<<"#"<<processor_name<<","<<rank<<",TCL,"<<CalTime<<":Initialization"<<endl;

			//如果是水库节点
			if(RsvList->find(mBSCode.Length,mBSCode.Value,mBSCode.RegionIndex))
			{
				CalTime=MPI_Wtime();//计算时间开始计时
				ReservoirSegment(mBSCode,TaskLoop);
				CalTime=MPI_Wtime()-CalTime;//计算时间结束
				cout<<"#"<<processor_name<<","<<rank<<",TCL,"<<CalTime<<":Reservoir"<<endl;
			}
			else
			{
				//20090918,xiaofc,runoffcalc里访问数据库较多，应在内部单独计时
				//已在WaterYield的源代码单独修改
				RunoffCalc(mBSCode,mPara,TaskLoop);//产流产沙计算//wh,已经包含多模型

				CalTime=MPI_Wtime();//计算时间开始计时

				//2008.2.17,wh,加入中转进程，当不需要保存所有河段流量信息时，通过消息传递子树根节点的流量过程
				if(mTreeNode[TaskLoop].StralherOrder>1)
				{
					for(int j=0;j<TaskLoop;j++)//find which is boy and girl	
					{			
						if((mTreeNode[j].mBSCode.Length==mBSCode.Length+1) && (mTreeNode[j].mBSCode.Value==mBSCode.Value<<1)) { Boy=j;}
						else if((mTreeNode[j].mBSCode.Length==mBSCode.Length+1) && (mTreeNode[j].mBSCode.Value==(mBSCode.Value<<1)+1)) { Girl=j;}		
					}
					if(Boy==-1 && Girl==-1)//需要去找上游
						this->NoChild(mBSCode);

					if((Boy==-1 && Girl!=-1) || (Boy!=-1 && Girl==-1))
						this->OneChild(mBSCode,Boy,Girl);
				}

				ConfluenceCalc(TaskLoop,mBSCode,mPara);//汇流计算，在精确扩散波法中计算了重力侵蚀、河宽、水深、流速等。

				CalTime=MPI_Wtime()-CalTime;//计算时间结束
				cout<<"#"<<processor_name<<","<<rank<<",TCL,"<<CalTime<<":Confluence"<<endl;
			}//end else


			/***********************************************************导入数据库及中转进程部分*******************************************************/
			DBUpdateTime=MPI_Wtime();//写入数据库时间开始计时

			//wh,如果不选择avpm方法，重力侵蚀也根本不会计算，所以也就自然不用保存
			if(BasinModel[ModelSelect][1].MakeLower()=="avpm" && SParameter.bSaveGravityEvents)//保存重力侵蚀事件
				SaveGravityEvents(TaskLoop);

			WriteToDischarge(TaskLoop);//将产流和产沙结果写入discharge表

			DBUpdateTime=MPI_Wtime()-DBUpdateTime;//数据库写入时间结束
			cout<<"#"<<processor_name<<","<<rank<<",TDB,"<<DBUpdateTime<<":"<<endl;
			/******************************************************************************************************************************************/

			//回收一条河段,不同模型在河段间传递的变量不一定相同，回收的自然也就不同
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
			cout<<"#"<<processor_name<<","<<rank<<",CPU,"<<iCpuUsage<<":"<<endl<<endl;//20060327,李铁键,加入CPU利用率//renewed by xia	

		}//end for(TaskLoop...)

		//2008.2.17,wh,往中转进程发送消息
		SendTime = MPI_Wtime();
	    PackSend(TaskCount-1);//wh往中转进程发送"根"节点的流量序列
	    MPI_Send(&(mTreeNode[TaskCount-1].mBSCode),sizeof(BSCode),MPI_BYTE,0,10,MPI_COMM_WORLD);//往主控进程发送消息

		SendTime = MPI_Wtime() - SendTime;
		cout<<"#"<<processor_name<<","<<rank<<",TCM,"<<SendTime<<":"<<endl;//包括往主控进程和中转进程发送消息的时间

		//回收子任务
		//在单元流域循环后给中转量们的指针收回空间
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


//水库节点的处理，此时水库节点的计算结果已经认为完成了，这里只是执行插值和查询操作
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
	//试验一下水库计算模块
	//ofstream myFileReservoir;
	//    myFileReservoir.open("myFileReservoir.txt",ios::trunc);
	//	myFileReservoir.precision(18);

	try
	{
		//从reservior表中取值，houroffset从小到大排列
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

		pRstRsv->MoveNext();//到一下个新值候着

		for(long j=0;j<Steps;j++) 
		{
			calHourOffset = SParameter.HourStart+double(j*lTimeInterval)/3600.0;
			while(calHourOffset>nowHourOffset)//取下一个新值，并使记录集到新的新值候着
			{
				if(pRstRsv->EndOfFile)//记录集里没有新数了
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
			//明天试验一下，这么说的话我在计算开始后一段时间才有水库下泄的话，在下泄前按照下式计算出的pQout就是predischarge了
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


//wh,将产流模块改为dll组件，并增加了模型选择接口
void SlaveProcess::RunoffCalc(BSCode mBSCode,Para mPara,int TaskLoop)
{
	//hsp模型不单独进行产流计算
	if(BasinModel[ModelSelect][0].MakeLower() == "hsp")
		return;

	if(BasinModel[ModelSelect][0].MakeLower() == "ltjyr")
	{
		try
		{
			CString SnowModelType = BasinModel[ModelSelect][3].MakeLower();
			//以下应该是wateryield的坡面产流产沙计算
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

//模型汇流计算,汇流的源代码一个是马斯京根，一个是avpm。
void SlaveProcess::ConfluenceCalc(int TaskLoop,BSCode mBSCode,Para mPara)
{
	if(BasinModel[ModelSelect][0].MakeLower() == "hsp")
	{
		//wh:当前计算河段的上游两河段的"明渠Qu"和"沟道Qd"水流量过程
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

		//最核心的模型计算
		HSP.Calc(mBSCode,&mPara,pQu1,pQu2,pQd1,pQd2,pQout[Parent],pDout[Parent],pFlowB,pFlowH,pFlow_v);

		return;
	}

	//以下是以前的模型，为了不搞混，上面单独写hsp有关的。
	if(mTreeNode[TaskLoop].StralherOrder == 1)//无需算汇流
	{
		ZeroFill(pQout[Parent],Steps+1);//Parent=TaskLoop;
		ZeroFill(pSout[Parent],Steps+1);
		SwapQp(&pQout[Parent],&pQin[Parent]);
		SwapQp(&pSout[Parent],&pSin[Parent]);

		//这就是为什么最靠近流域边界的坡面单元的河段Qinput和Sinput是0而不是坡面产流产沙量。
		//互换是因为Qout还需要作为河段出口过程为下游河段提供边界条件呢。
	}
	else //算汇流
	{
		//给两部分入流赋值，pQout1代表girl，pQout2代表boy
		float* pQout1; float* pQout2;
		float* pSout1; float* pSout2;

		if(mBSCode.RegionIndex==1122012001 && mBSCode.Value==0 && mBSCode.Length==940)
			int xxx=0;

		//david,怀疑以下是造成一条河段avpm calc()函数计算上游俩河段入汇时girl河段水沙为0的原因，还得顺藤摸瓜仔细看。
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
		if(BasinModel[ModelSelect][1].MakeLower() == "muskingum")//马斯京根汇流法//wh
		{
			Transform(pQout1,pQout2,pQin[Parent],pQout[Parent],lTimeInterval,Steps,&mPara);
		}
		else if(BasinModel[ModelSelect][1].MakeLower() == "avpm")//精确扩散波法//wh
		{
			try
			{
				AVPM mAVPM(SParameter.SediTransCapF,SParameter.MSTEP,SParameter.sccd);//wh

				mAVPM.iTSteps=Steps;
				mAVPM.isDebug=false;

				//20060316,李铁键,统一了是否计算重力侵蚀的处理
				//if(SParameter.iCalGravityErosion==0)      
				mAVPM.bCalcGravity=false;	

				/*else*/ if(SParameter.iCalGravityErosion==1) 
					if(mPara.DrainingArea<100000000)	//20080306,xiaofc,100km2以上的汇水面积下不发生重力侵蚀
						mAVPM.bCalcGravity=true;

					else if(SParameter.iCalGravityErosion==2)
					{
						if(mBSCode.RegionIndex==0 && mBSCode.Value==0)//20051212 李铁键,AVPM增加控制是否计算重力侵蚀的参数,依家宏意见，头道拐至龙门干流不发生重力侵蚀
							mAVPM.bCalcGravity=false;
						else
							mAVPM.bCalcGravity=true;
					}

					mAVPM.mBSCode = mBSCode;//wh

					mAVPM.initialize(mPara);
					mAVPM.alphaE = SParameter.fAlphaErosion;//20060317,李铁键,恢复饱和系数
					mAVPM.alphaD = SParameter.fAlphaDeposition;
					mAVPM.P2 = SParameter.GravityErosioinP2;//20060324,李铁键,发生重力侵蚀的纵向范围的概率

					//20060327,李铁键,为存储重力侵蚀信息用
					if(SParameter.bSaveGravityEvents){ mAVPM.pGravityEvents=&GravityEvents; }	
					else{ mAVPM.pGravityEvents=NULL; }

					mAVPM.delta_t=lTimeInterval;
					mAVPM.WL=pWLM[Parent];    mAVPM.WR=pWRM[Parent];
					mAVPM.Qout=pQout[Parent]; mAVPM.Sout=pSout[Parent];
					mAVPM.Qin1=pQout1;        mAVPM.Qin2=pQout2;         mAVPM.Qin3=pQin[Parent];
					mAVPM.SinL=pSout1;        mAVPM.SinR=pSout2;
					mAVPM.SinMe=pSin[Parent];

					//20080303,xiaofc,为保存壮态增加
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
					//20080303,xiaofc,存状态
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
	}//end of 算汇流
}


//wh,往中转进程发送根节点的流量、沙量序列以及RegionIndex、BSValue、BSLength（发货）
void SlaveProcess::PackSend(int k)
{
	int len[5];
	MPI_Aint disp[5];
	MPI_Datatype type[5],newtype;

	int position=0;//打包开始的位置
	int size=2*(Steps+1)*sizeof(float)+2*sizeof(unsigned long long)+sizeof(long);
	char* buff = new char[size];

	//设置新类型包含数据的个数
	len[0] = len[1] = len[2] = 1;
	len[3] = len[4] = Steps+1;

	MPI_Address(&mTreeNode[k].mBSCode.RegionIndex,disp);//mTreeNode[k].mBSCode.RegionIndex相对于MPI_BOTTOM的偏移，存储到disp+0中
	MPI_Address(&mTreeNode[k].mBSCode.Value,disp+1);
	MPI_Address(&mTreeNode[k].mBSCode.Length,disp+2);
	MPI_Address(pQout[k],disp+3);

	//这不是解决问题长久之计，长久之际是还要传递一个模型类型判别变量，并该变中转进程
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

	MPI_Type_struct(5,len,disp,type,&newtype);//定义新的数据类型
	MPI_Type_commit(&newtype);//新类型提交

	MPI_Pack(MPI_BOTTOM,1,newtype,buff,size,&position,MPI_COMM_WORLD);//数据打包
	MPI_Send(buff,position,MPI_PACKED,TransferProcessRank,10,MPI_COMM_WORLD);//发送打包数据到中转进程。
	delete[] buff;
}


//wh为了找到河段子节点的出口流量，为汇流服务。这里河段的子节点肯定是其他计算进程曾经分到的小树的根节点，
//NoChild有两种情况，因为分任务产生的分离和因为不同RegionIndex产生的分离,但是NoChild可能有两个或一个孩子。
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

	if(i == RCCount)//不是连接段，两个或一个子节点因为分任务产生的分离
	{
		int len[4];
		MPI_Aint disp[4];
		MPI_Datatype type[4],newtype;
		int position=0;//打包开始的位置
		int size = 3 * sizeof(unsigned long long) + sizeof(long);
		char* buff = new char[size];

		unsigned long long BoyBsvalue = mBSCode.Value<<1;
		unsigned long long GirlBsvalue = ((mBSCode.Value<<1)+(unsigned long long)1);
		mBSCode.Length++;//wh,2008.3.26修正,发送的是子节点的BSlength，不是当前节点的

		len[0] = len[1] = len[2] = len[3] = 1;
		MPI_Address(&mBSCode.RegionIndex,disp);//mBSCode.RegionIndex相对于MPI_BOTTOM的偏移，存储到disp+0中
		MPI_Address(&BoyBsvalue,disp+1);
		MPI_Address(&GirlBsvalue,disp+2);
		MPI_Address(&mBSCode.Length,disp+3);

		type[0] = type[1] = type[2] = MPI_UNSIGNED_LONG_LONG;
		type[3] = MPI_LONG;

		MPI_Type_struct(4,len,disp,type,&newtype);//定义新的数据类型
		MPI_Type_commit(&newtype);//新类型提交

		MPI_Pack(MPI_BOTTOM,1,newtype,buff,size,&position,MPI_COMM_WORLD);//数据打包
		MPI_Send(buff,position,MPI_PACKED,TransferProcessRank,11,MPI_COMM_WORLD);//发送两个货物的标签，将其打包发送到中转进程，告诉你我要提货了。
		delete[] buff;

		MPI_Recv(pQUpRegion,Steps+1,MPI_FLOAT,TransferProcessRank,11,MPI_COMM_WORLD,&Status);//从中转进程接收流量序列

		//wh
		if(BasinModel[ModelSelect][0].MakeLower()=="hsp")
		{
			MPI_Recv(pDUpRegion,Steps+1,MPI_FLOAT,TransferProcessRank,11,MPI_COMM_WORLD,&Status);//从中转进程接收地下水量序列
		}
		else
		{
			MPI_Recv(pSUpRegion,Steps+1,MPI_FLOAT,TransferProcessRank,11,MPI_COMM_WORLD,&Status);//从中转进程接收沙量序列
		}
	}

	else//是连接段，去上一个region里找，只有一个子节点
	{
		MPI_Send(&RegionConnection[i].RegionIndex,1,MPI_UNSIGNED_LONG_LONG,TransferProcessRank,12,MPI_COMM_WORLD);//bsvalue一定是0，所以不用发送
		
		//shy, 20130831,region出口的bslength不再从1开始，继承上一级region的继续增加，所以要取数据的时候要指明bslength
		long UpRegionBSLength;
		UpRegionBSLength=RegionConnection[i].Length+1;
		//david,然而旧河网bslength还是从1开始
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

//wh只能在内存中找到当前节点的一个子节点，不过因为节点可能有一个或两个孩子，所以子节点在中转进程中可能有，也可能没有。
void SlaveProcess::OneChild(BSCode mBSCode,int Boy,int Girl)
{
	ofstream myFile888;
	myFile888.open("OneChild.txt",ios::app);
	assert((Boy==-1 && Girl!=-1) || (Boy!=-1 && Girl==-1));

	int len[3];
	MPI_Aint disp[3];
	MPI_Datatype type[3],newtype;
	int position=0;//打包开始的位置
	int size = 2 * sizeof(unsigned long long) + sizeof(long);
	char* buff = new char[size];

	unsigned long long bsvalue;

	if(mBSCode.RegionIndex==1 && mBSCode.Value==0 && mBSCode.Length==218)
		int xxx=0;

	//判断是否连接段,20131126,shy
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
	
	if(i == RCCount)//不是连接段，两个或一个子节点因为分任务产生的分离
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
		mBSCode.Length++;//wh,2008.3.26修正,发送的是子节点的BSlength，不是当前节点的
		
		len[0] = len[1] = len[2] = 1;
		MPI_Address(&mBSCode.RegionIndex,disp);//mBSCode.RegionIndex相对于MPI_BOTTOM的偏移，存储到disp+0中
		MPI_Address(&bsvalue,disp+1);
		MPI_Address(&mBSCode.Length,disp+2);

		type[0] = type[1] = MPI_UNSIGNED_LONG_LONG;
		type[2] = MPI_LONG;

		MPI_Type_struct(3,len,disp,type,&newtype);//定义新的数据类型
		MPI_Type_commit(&newtype);//新类型提交

		MPI_Pack(MPI_BOTTOM,1,newtype,buff,size,&position,MPI_COMM_WORLD);//数据打包
		MPI_Send(buff,position,MPI_PACKED,TransferProcessRank,13,MPI_COMM_WORLD);//发送两个货物的标签，将其打包发送到中转进程，告诉你我要提货了。
		delete buff;

		MPI_Recv(pQUpRegion,Steps+1,MPI_FLOAT,TransferProcessRank,13,MPI_COMM_WORLD,&Status);//从中转进程接收流量序列

		//wh
		if(BasinModel[ModelSelect][0].MakeLower()=="hsp")
		{
			MPI_Recv(pDUpRegion,Steps+1,MPI_FLOAT,TransferProcessRank,13,MPI_COMM_WORLD,&Status);//从中转进程接收沙量序列
		}
		else
		{
			MPI_Recv(pSUpRegion,Steps+1,MPI_FLOAT,TransferProcessRank,13,MPI_COMM_WORLD,&Status);//从中转进程接收沙量序列
		}
	}
	else
    {
		myFile888<<"i != RCCount\n"<<flush;
		//shy, 20130831,region出口的bslength不再从1开始，继承上一级region的继续增加，所以要取数据的时候要指明bslength
		long UpRegionBSLength;
		UpRegionBSLength=RegionConnection[i].Length+1;
		//david,然而旧河网bslength还是从1开始
		UpRegionBSLength=1;

		len[0] = len[1] = len[2] = 1;
		MPI_Address(&RegionConnection[i].RegionIndex,disp);//mBSCode.RegionIndex相对于MPI_BOTTOM的偏移，存储到disp+0中
		MPI_Address(&RegionConnection[i].Value,disp+1);
		MPI_Address(&UpRegionBSLength,disp+2);

		type[0] = type[1] = MPI_UNSIGNED_LONG_LONG;
		type[2] = MPI_LONG;

		MPI_Type_struct(3,len,disp,type,&newtype);//定义新的数据类型
		MPI_Type_commit(&newtype);//新类型提交

		MPI_Pack(MPI_BOTTOM,1,newtype,buff,size,&position,MPI_COMM_WORLD);//数据打包
		MPI_Send(buff,position,MPI_PACKED,TransferProcessRank,13,MPI_COMM_WORLD);//发送两个货物的标签，将其打包发送到中转进程，告诉你我要提货了。
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


//往数据库中写入重力侵蚀事件，前面没涉及到写数据库，下面才开始。
void SlaveProcess::SaveGravityEvents(int TaskLoop)
{
	//20060327,李铁键,写重力侵蚀事件入数据库
	//ADODB::_RecordsetPtr pRstGravityEvents;
	//pRstGravityEvents.CreateInstance(__uuidof(ADODB::Recordset));
	unsigned long iVecLoop;
	CString cSQL;
	_bstr_t SQL;
	try
	{
		//先删掉同时期的旧数据，因为这个东西是不可重复的
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
	GravityEvents.clear();//写入数据库完成之后,清空这次的重力侵蚀记录
}


//wh2009：其实该函数不应该放在这里，也是模型依赖的，应放在模型的代码里面，因为并不是所有模型的输出结果都
//写入discharge表，但是一般情况下输出都是一致的，因为都是出口的水沙流量序列。
//将产流产沙结果写入数据库
void SlaveProcess::WriteToDischarge(int TaskLoop)
{
	//ADODB::_RecordsetPtr pRstIndicator;//指向DefinedNodes表
	//pRstIndicator.CreateInstance(__uuidof(ADODB::Recordset));
	long j=0;
	CString cSQL;
	_bstr_t SQL;
	try
	{
		cSQL.Format("Select * from DefinedNodes where RegionIndex=%I64u and BSValue=%I64u and BSLength=%d and nodetype in ('CS','GS','RS')",mTreeNode[TaskLoop].mBSCode.RegionIndex,mTreeNode[TaskLoop].mBSCode.Value,mTreeNode[TaskLoop].mBSCode.Length);
		SQL=cSQL.GetString();
		pRstIndicator->Open(SQL,(ADODB::_Connection*)pCnn,ADODB::adOpenStatic,ADODB::adLockOptimistic,ADODB::adCmdText);

		//wh,判断是否需要写数据,下面的意思是只写收到的子树的根节点以及在definednodes中的CS、GS点。
		//if(pRstIndicator->EndOfFile && TaskLoop!=TaskCount-1)
		//2008.1.17,wh,只有definednodes写，或者将来加入全都写（此时也就不用什么definednodes了），如果通过内存传递根节点信息，那根节点就没必要往数据库里面写了。
		if(pRstIndicator->EndOfFile && SParameter.bSaveAllDischarge==0)
			pRstIndicator->Close();
		else//写入
		{
			pRstIndicator->Close();

			//20051209,xiaofc,为消除因无河道初始流量引起的preparehours内的数据无效，不能入库,修改了写库的时间起点从HourStart改为HourStart+PrepareHours
			//20051209晚上,如果这个河段以前没算过，那么即便是错的流量也要写进去，因为下游的还要用这个数儿呢
			CComVariant comSQL;//老陈说的新用法
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

			//wh,前preparehours模型空转，但是记录已经写入到了数据库,当上游河段preparehours内记录都存在时，则从preparehours后开始取数据
			long lNoStorageHours;//起存时间
			if(lDataCount<SParameter.PrepareHours*60/SParameter.MSTEP){ lNoStorageHours=0;}
			else{ lNoStorageHours=SParameter.PrepareHours;}

			//20051219 李铁键 改进为写数据时能update则不delete+addnew,以减小数据库负担
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
			bIsRewrite=(lDataCount==(SParameter.NumofHours-lNoStorageHours)*60/SParameter.MSTEP);//证明discharge中已经有从
			pCnn->BeginTrans();//开始事务，避免过多的commit

			if(bIsRewrite)//改写
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
						
						//看看到底是算了多少步,时间对不对得上
						//cout<<"===============lNoStorageHours="<<lNoStorageHours<<"Steps="<<Steps<<"========"<<endl;
						pRstData->Fields->Item["QOutput"]->Value=pQout[Parent][j];

						if(SParameter.iCalcSediTrans){ pRstData->Fields->Item["SOutput"]->Value=pSout[Parent][j]; }//20070608,xiaofc,增加是否保存输沙结果	
						else{ pRstData->Fields->Item["SOutput"]->Value=0.0f;}

						pRstData->Fields->Item["QInput"]->Value=pQin[Parent][j];

						pRstData->Fields->Item["SInput"]->Value=pSin[Parent][j];
						pRstData->Fields->Item["trapsout"]->Value=trapSout[Parent][j];
						pRstData->Fields->Item["trapqout"]->Value=trapQout[Parent][j];
						//catch(...)
						//{
						//	cout<<"============errror==========="<<endl;
						//}

						//wh,增加模型判断,也就是说如果bSaveFlowPattern为1但是没有选择avpm照样不可以
						//因为计算BHV的程序写在了AVPM里，这样做是合理的。
						if(BasinModel[ModelSelect][1].MakeLower()=="avpm" && SParameter.bSaveFlowPattern)//20070622, xiaofc, save flow pattern
						{
							pRstData->Fields->Item["B"]->Value=pFlowB[j];
							pRstData->Fields->Item["H"]->Value=pFlowH[j];
							pRstData->Fields->Item["v"]->Value=pFlow_v[j];
						}

						//20110112, xiaofc, 不依赖数据库中转流量数据, 以下复杂操作取消
						if(j%(1000+rank*10)==0)
						{
							pRstData->UpdateBatch(ADODB::adAffectAll);
							pCnn->CommitTrans();
							pCnn->BeginTrans();
							//pRstData->Move(j-lNoStorageHours*60/SParameter.MSTEP);20110113,以上3行不改变当前位置，move会向后移动相应行数，造成错误，画蛇添足
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
			else//插入
			{
				//删除旧记录			
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

						//20070608,xiaofc,增加是否保存输沙结果
						//2008.3.23,wh,增加模型判断机制
						//wh,added,sccd,2008.3.24
						vtmp = SParameter.sccd;
						pRstData->Fields->Item["sccd"]->Value = _variant_t(vtmp);

						pRstData->Fields->Item["QOutput"]->Value=pQout[Parent][j];
						pRstData->Fields->Item["trapqout"]->Value=trapQout[Parent][j];

						if(SParameter.iCalcSediTrans) {pRstData->Fields->Item["SOutput"]->Value=pSout[Parent][j];pRstData->Fields->Item["trapsout"]->Value=trapSout[Parent][j];}
						else { pRstData->Fields->Item["SOutput"]->Value=0.0f;pRstData->Fields->Item["trapsout"]->Value=0.0f;}

						pRstData->Fields->Item["QInput"]->Value=pQin[Parent][j];
						pRstData->Fields->Item["SInput"]->Value=pSin[Parent][j];	

						//2008.3.23,增加模型判断机制
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
					//把if拿出来是为了加快速度，减少循环过程中逻辑判断的次数
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
			}//end else插入
			pCnn->CommitTrans();//结束事务
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


//子节点计算完成后的收尾工作//组件的释放，数据库的关闭，堆空间释放
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
		spWaterBasin->Finalize(this->MSRM.AllocSysString());//释放SRM的dll，应该是在产流dll都结束以后才释放，否则融雪就无法算。
		spWaterBasin.Release();	//正解，组件释放
	}
	if(MXAJ=="xaj")
	{
		spXAJmodel->Finalize(this->MSRM.AllocSysString());
		spXAJmodel.Release();
	}
}


void SlaveProcess::RecvMasterProcess_reservoir(void)
{
	Para mPara;//中转变量
	BSCode mBSCode;
	int TaskLoop;

	//pFlowB在每个计算进程只需要有一个数组，不需要每接收到一次任务就new一次
	bool BHV_flag=true;

	while(1)
	{
		//////////////////////////////////////////////////////////////////////////
		/////////////////////////从主控节点接收河段数组///////////////////////////
		//////////////////////////////////////////////////////////////////////////
		CommTime=MPI_Wtime();//通讯时间
		cout<<"@"<<processor_name<<","<<rank<<",is ready to receive."<<endl;//is前面的“，”不能删除，很重要的，呵呵。
		MPI_Recv(&TaskCount,1,MPI_INT,0,10,MPI_COMM_WORLD,&Status);

		if( TaskCount==0 )
		{
			cout<<"@"<<processor_name<<","<<rank<<",is ready to exit."<<endl;
			break;
		}	
		else
		{
			mTreeNode = new TreeNode[TaskCount];//一个TreeNode代表一个河段
			MPI_Recv(mTreeNode,TaskCount*sizeof(TreeNode),MPI_BYTE,0,10,MPI_COMM_WORLD,&Status);
		}
		CommTime=MPI_Wtime()-CommTime;
		cout<<"#"<<processor_name<<","<<rank<<",RCV,"<<TaskCount<<":"<<endl;//接收任务的河段数
		cout<<"#"<<processor_name<<","<<rank<<",TCM,"<<CommTime<<":"<<endl<<endl;//接收任务的时间


		//////////////////////////////////////////////////////////////////////////
		/////////////////////2008.3.23，王皓增加多模型判断接口////////////////////
		//////////////////////////////////////////////////////////////////////////
		CalTime=MPI_Wtime();//计算时间开始计时

		ModelSelect = GetBasinModelRegionIndex(mTreeNode[TaskCount-1].mBSCode.RegionIndex);//得到它所隶属的区域在第几行

		//汇流模型判断
		if(((BasinModel[ModelSelect][1].MakeLower()=="avpm") || (BasinModel[ModelSelect][0].MakeLower()=="hsp" && HSP.Min.SaveFlowPattern==true)) && BHV_flag)
		{
			BHV_flag=false;

			//当前计算进程计算始终只new一次
			pFlowB  = new float[Steps+1];  ZeroFill(pFlowB,Steps+1);
			pFlowH  = new float[Steps+1];  ZeroFill(pFlowH,Steps+1);
			pFlow_v = new float[Steps+1];  ZeroFill(pFlow_v,Steps+1);				
		}

		//////////////////////////////////////////////////////////////////////////
		//////////////在单元流域循环前给中转量们的指针个数开辟空间////////////////
		//////////////////////////////////////////////////////////////////////////
		if(BasinModel[ModelSelect][0].MakeLower()=="hsp")
		{
			//wh:每得到一个任务就要new一次，因为每个任务包含河段数是动态不确定的。
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


		CalTime=MPI_Wtime()-CalTime;//计算时间结束
		cout<<"#"<<processor_name<<","<<rank<<",TCL,"<<CalTime<<":"<<endl;

		//////////////////////////////////////////////////////////////////////////
		//////////////////////////对每一条具体的河段进行计算//////////////////////
		//////////////////////////////////////////////////////////////////////////
		for(TaskLoop=0; TaskLoop<TaskCount; TaskLoop++)
		{
			CalTime=MPI_Wtime();//计算时间开始计时

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

				//wh:需要初始化，否则new的数据每项值不可控，在新安江模型计算中由于没有算沙，pSout的随机值会出现越界，不能写入数据库。
				::ZeroFill(pQin[TaskLoop] ,Steps+1);   ::ZeroFill(pSin[TaskLoop] ,Steps+1);
				::ZeroFill(pQout[TaskLoop],Steps+1);   ::ZeroFill(pSout[TaskLoop],Steps+1);
				::ZeroFill(pWLM[TaskLoop] ,Steps+1);   ::ZeroFill(pWRM[TaskLoop] ,Steps+1);
				::ZeroFill(trapQout[TaskLoop], Steps+1); ::ZeroFill(trapSout[TaskLoop], Steps+1);
			}

			Parent=TaskLoop;
			Boy=-1;//取QBoy QGirl,置-1,以能在后来知道是没找到
			Girl=-1;

			CalTime=MPI_Wtime()-CalTime;//计算时间结束
			cout<<"#"<<processor_name<<","<<rank<<",TCL,"<<CalTime<<":Initialization"<<endl;

			//如果是水库节点
			//if(RsvList->find(mBSCode.Length,mBSCode.Value,mBSCode.RegionIndex))
			//{
			//	CalTime=MPI_Wtime();//计算时间开始计时
			//	ReservoirSegment(mBSCode,TaskLoop);
			//	CalTime=MPI_Wtime()-CalTime;//计算时间结束
			//	cout<<"#"<<processor_name<<","<<rank<<",TCL,"<<CalTime<<":Reservoir"<<endl;
			//}

				//20090918,xiaofc,runoffcalc里访问数据库较多，应在内部单独计时
				//已在WaterYield的源代码单独修改
				RunoffCalc(mBSCode,mPara,TaskLoop);//产流产沙计算//wh,已经包含多模型

				CalTime=MPI_Wtime();//计算时间开始计时

				//2008.2.17,wh,加入中转进程，当不需要保存所有河段流量信息时，通过消息传递子树根节点的流量过程
				if(mTreeNode[TaskLoop].StralherOrder>1)
				{
					for(int j=0;j<TaskLoop;j++)//find which is boy and girl	
					{			
						if((mTreeNode[j].mBSCode.Length==mBSCode.Length+1) && (mTreeNode[j].mBSCode.Value==mBSCode.Value<<1)) { Boy=j;}
						else if((mTreeNode[j].mBSCode.Length==mBSCode.Length+1) && (mTreeNode[j].mBSCode.Value==(mBSCode.Value<<1)+1)) { Girl=j;}		
					}
					if(Boy==-1 && Girl==-1)//需要去找上游
						this->NoChild(mBSCode);

					if((Boy==-1 && Girl!=-1) || (Boy!=-1 && Girl==-1))
						this->OneChild(mBSCode,Boy,Girl);
				}

				ConfluenceCalc(TaskLoop,mBSCode,mPara);//汇流计算，在精确扩散波法中计算了重力侵蚀、河宽、水深、流速等。

				//davidthu,输出到文本用于测试添加的

				//davidthu,算完后，再来考虑水库拦蓄作用
				if(RsvList->find(mBSCode.Length,mBSCode.Value,mBSCode.RegionIndex))
				{
					//如果找到水库节点，读入蓄水容量
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



				CalTime=MPI_Wtime()-CalTime;//计算时间结束
				cout<<"#"<<processor_name<<","<<rank<<",TCL,"<<CalTime<<":Confluence"<<endl;



			/***********************************************************导入数据库及中转进程部分*******************************************************/
			DBUpdateTime=MPI_Wtime();//写入数据库时间开始计时

			//wh,如果不选择avpm方法，重力侵蚀也根本不会计算，所以也就自然不用保存
			if(BasinModel[ModelSelect][1].MakeLower()=="avpm" && SParameter.bSaveGravityEvents)//保存重力侵蚀事件
				SaveGravityEvents(TaskLoop);

			WriteToDischarge(TaskLoop);//将产流和产沙结果写入discharge表

			DBUpdateTime=MPI_Wtime()-DBUpdateTime;//数据库写入时间结束
			cout<<"#"<<processor_name<<","<<rank<<",TDB,"<<DBUpdateTime<<":"<<endl;
			/******************************************************************************************************************************************/

			//回收一条河段,不同模型在河段间传递的变量不一定相同，回收的自然也就不同
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
			cout<<"#"<<processor_name<<","<<rank<<",CPU,"<<iCpuUsage<<":"<<endl<<endl;//20060327,李铁键,加入CPU利用率//renewed by xia	

		}//end for(TaskLoop...)

		//2008.2.17,wh,往中转进程发送消息
		SendTime = MPI_Wtime();
	    PackSend(TaskCount-1);//wh往中转进程发送"根"节点的流量序列
	    MPI_Send(&(mTreeNode[TaskCount-1].mBSCode),sizeof(BSCode),MPI_BYTE,0,10,MPI_COMM_WORLD);//往主控进程发送消息

		SendTime = MPI_Wtime() - SendTime;
		cout<<"#"<<processor_name<<","<<rank<<",TCM,"<<SendTime<<":"<<endl;//包括往主控进程和中转进程发送消息的时间

		//回收子任务
		//在单元流域循环后给中转量们的指针收回空间
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
