#include "DataStruct.h"
#include "TreeList.h"
#include <iostream>
#include <fstream>
#include <assert.h>
#include <time.h>
#include "MasterProcess.h"
#include "functions.h"
//以下三句要加上，为了修正mpich2的一个bug
#undef SEEK_SET
#undef SEEK_END
#undef SEEK_CUR
#include "C:\Program Files\MPICH2\include\mpi.h"
using namespace std;

MasterProcess::MasterProcess(void)
{
}

MasterProcess::~MasterProcess(void)
{
}


//wh added,2008.3.23,HydroUsePara表中的calcregion是"根regionindex"，该函数为了找到要计算的所有RegionIndex
//根RegionIndex的确定方式是BasinModel中的最小RegionIndex。
CString MasterProcess::GetCalcRegionIndex(void)
{
	CString cSQL;
	_bstr_t SQL;
	cSQL = "select distinct regionindex from riversegs";
	SQL=cSQL.GetString();
	try
	{
		pRst->Open(SQL,(ADODB::_Connection*)pCnn,ADODB::adOpenStatic,ADODB::adLockOptimistic,ADODB::adCmdText);

		CString CalcIndex;
		unsigned long long RootRegionIndex=_strtoui64((LPCTSTR)(_bstr_t)SParameter.CompRegions,NULL,10);//文本转化为unsigned long long
		unsigned long long temp;
		CString tempstring;
		CalcIndex = SParameter.CompRegions;//初始值已经包括根regionindex了。
		while(!pRst->EndOfFile)
		{
			tempCom=pRst->Fields->Item["regionindex"]->Value;
			temp = tempCom.ullVal;

			//找子节点，不包括根节点
			while(temp > RootRegionIndex)
			{
				temp /= SParameter.RegionSystem;//20130904, shy
				if(temp == RootRegionIndex)
				{
					//这里要采用如下方式，如果直接用一个tempstring.format("%s,%I64u"...)，分步调试时这里总出现stack不足的情况
					tempstring.Format("%I64u",tempCom.ullVal);
					CalcIndex = CalcIndex +","+tempstring;

					break;
				}//end if
				if(temp < RootRegionIndex)
					break;
			}//end while

			pRst->MoveNext();
		}//end while

		pRst->Close();

		return CalcIndex;

	}//end try
	catch(_com_error e)
	{
		cout<<"Failture Of Selecting Distinct RegionIndex From Table RiverSegs."<<rank<<endl;
		
		cout<<e.Error()<<endl;
		cout<<e.ErrorMessage()<<endl;
	    cout<<(LPCSTR)e.Source()<<endl;        
		cout<<(LPCSTR)e.Description()<<endl;  

		MPI_Finalize();
		exit(0);
	}
}

//读入各个RegionIndex,RegionGrade的信息到GradeTwoCode动态数组中
void MasterProcess::ReadBSCode(void)
{
	CString cSQL;
	_bstr_t SQL;
	ofstream myFile444;
	myFile444.open("ReadBSCode.txt",ios::trunc);
	myFile444 <<"CompRegions="<<SParameter.CompRegions<<" CompRegion="<<SParameter.CompRegion<<".\n";
	//20060904,xiaofc,增加Region范围限定,CompRegions是否空作为SQL语句成生的判断
	//获取二级区的信息
	cout<<"aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"<<endl;
	if(SParameter.CompRegions=="" || (SParameter.CompRegions=="%s",SParameter.CompRegion))//20140515,shy
		cSQL.Format("select regiongrade,regionindex from riversegs group by regiongrade,regionindex order by regiongrade desc");
	else
	{
		CString CalcRegionIndex = this->GetCalcRegionIndex();//wh,选择要计算的树，这样是合理的，因为树只有一个出口，没有入口，全封闭
		cSQL.Format("select regiongrade,regionindex from riversegs where regionindex in (%s) group by regiongrade,regionindex order by regiongrade desc",CalcRegionIndex);//wh
		//cSQL.Format("select regiongrade,regionindex from riversegs where regionindex in (%s) group by regiongrade,regionindex order by regiongrade desc",SParameter.CompRegions);//xiaofc,这样可以减少计算，但是程序会出错
		//cSQL.Format("select regiongrade,regionindex from riversegs group by regiongrade,regionindex order by regiongrade desc");
		myFile444 <<"CompRegions="<<SParameter.CompRegions<<" CompRegion="<<SParameter.CompRegion<<".\n";
	}
	cout<<"bbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbb"<<endl;

		
	SQL=cSQL.GetString();
	try
	{
		pRst->Open(SQL,(ADODB::_Connection*)pCnn,ADODB::adOpenStatic,ADODB::adLockOptimistic,ADODB::adCmdText);
		GradeTwoCount=pRst->RecordCount;//GradeTwoCount:计算区域中不同RegionIndex的个数
		pRst->MoveFirst();

		GradeTwoCode=new BSCode[GradeTwoCount];

		for(GradeTwoLoop=0;GradeTwoLoop<GradeTwoCount;GradeTwoLoop++)
		{
			GradeTwoCode[GradeTwoLoop].RegionGrade=pRst->Fields->Item["regiongrade"]->Value;

			tempCom=pRst->Fields->Item["regionindex"]->Value;
			GradeTwoCode[GradeTwoLoop].RegionIndex=tempCom.ullVal;
			myFile444 <<"Regiongrade="<<GradeTwoCode[GradeTwoLoop].RegionGrade<<" Regionindex="<<GradeTwoCode[GradeTwoLoop].RegionIndex<<".\n";

			pRst->MoveNext();
		}
		pRst->Close();//创建是在CommenProcess中
		myFile444.close();
	}
	catch(_com_error e)
	{
		cout<<"Failture Of Selecting RegionGrade,RegionIndex From Table RiverSegs."<<rank<<endl;
		
		cout<<e.Error()<<endl;
		cout<<e.ErrorMessage()<<endl;
	    cout<<(LPCSTR)e.Source()<<endl;        
		cout<<(LPCSTR)e.Description()<<endl;  

		MPI_Finalize();
		exit(0);
	}
}


//功能：从数据库中的Parameter表中读入土壤类型和土壤利用信息
//wh，默认所有专业模型要的参数不会超出原来parameter已有字段
//wh2009注：实际上在Master进程里面读入Parameter表是不合适的，因为Parameter表依赖于具体的模型，其他的模型不需要这个表，
//所以这部分代码应该放到模型代码里，如果放这里一方面是用通讯开销来取代在模型代码中读取Parameter表的数据库开销，并不一
//定会节省时间，同时在用其他模型计算时，传递Parameter表信息属于冗余传递了；换句话说Master进程只需传递“二叉树编码”就
//行，riversegs表的信息也可以写到模型的代码里，但由于Riversegs表的几何信息是所有模型都需要用到的，因此放到主控进程里也
//完全没有问题，具有通用性的。但是改起来有点麻烦死，暂时算了，有时间再优化。
void MasterProcess::ReadFromParameter(void)
{
	//参数转换矩阵：土地利用+土壤类型=>13个相关参数
	int iLandUse;//矩阵列
	int iSoilType;//矩阵行
	CString cSQL;
	_bstr_t SQL;

	_variant_t VTmp;//wh,20080803
	
	//SQL="Select * from parameter order by LandUse,SoilType";

	//wh added,2008
	//=================================================================================================================================
	cSQL.Format("Select distinct regionindex from parameter where sccd=%s order by regionindex desc",SParameter.sccd);//从大到小排列
	SQL = cSQL.GetString();
	int i=0;//不同regionindex的个数
	try
	{
		pRst->Open(SQL,(ADODB::_Connection*)pCnn,ADODB::adOpenStatic,ADODB::adLockOptimistic,ADODB::adCmdText);
		
		IndexCount = pRst->RecordCount;
        ParameterRIndex = new unsigned long long[IndexCount];

		while(!pRst->EndOfFile)
		{
			tempCom = pRst->Fields->Item["regionindex"]->Value;
			ParameterRIndex[i] = tempCom.ullVal;

		    //cout<<ParameterRIndex[i]<<endl;

			pRst->MoveNext();
			i++;
		}
		pRst->Close();
	}
	catch(_com_error e)
	{
		cout<<"Failture Of Reading Distinct RegionIndex From Table Parameter."<<rank<<endl;
		
		cout<<e.Error()<<endl;
		cout<<e.ErrorMessage()<<endl;
	    cout<<(LPCSTR)e.Source()<<endl;        
		cout<<(LPCSTR)e.Description()<<endl;  

		MPI_Finalize();
		exit(0);
	}
	//=================================================================================================================================

	for(int k=0;k<i;k++)
	{
		cSQL.Format("Select * from parameter where sccd=%s and regionindex=%I64u order by LandUse,SoilType",SParameter.sccd,ParameterRIndex[k]);
		SQL = cSQL.GetString();

		//wh，增加了方案号以及分布式调参使用的RegionIndex，不过要记住该RegionIndex为根RegionIndex
		//regionindex大的排在前面
		try
		{
			pRst->Open(SQL,(ADODB::_Connection*)pCnn,ADODB::adOpenStatic,ADODB::adLockOptimistic,ADODB::adCmdText);

			//写入矩阵
			while(!pRst->EndOfFile)
			{
				iSoilType=pRst->Fields->Item["SoilType"]->Value;
				iLandUse=pRst->Fields->Item["LandUse"]->Value;
				//iLandUse -= 1;//6个数为1~6//wh,以后landuse都是从0开始存，跟soiltype统一,所以不用再减1了

				Matrix_PKH1[k][iSoilType][iLandUse]=pRst->Fields->Item["PKH1"]->Value;
				Matrix_PKH2[k][iSoilType][iLandUse]=pRst->Fields->Item["PKH2"]->Value;
				Matrix_PKV0[k][iSoilType][iLandUse]=pRst->Fields->Item["PKV0"]->Value;
				Matrix_PKV1[k][iSoilType][iLandUse]=pRst->Fields->Item["PKV1"]->Value;
				Matrix_PKV2[k][iSoilType][iLandUse]=pRst->Fields->Item["PKV2"]->Value;//20060220,Iwish
				Matrix_UTheta1[k][iSoilType][iLandUse]=pRst->Fields->Item["UTheta1"]->Value;
				Matrix_UTheta2[k][iSoilType][iLandUse]=pRst->Fields->Item["UTheta2"]->Value;
				Matrix_MTheta1[k][iSoilType][iLandUse]=pRst->Fields->Item["MTheta1"]->Value;
				Matrix_MTheta2[k][iSoilType][iLandUse]=pRst->Fields->Item["MTheta2"]->Value;
				Matrix_DTheta1[k][iSoilType][iLandUse]=pRst->Fields->Item["DTheta1"]->Value;
				Matrix_DTheta2[k][iSoilType][iLandUse]=pRst->Fields->Item["DTheta2"]->Value;
				Matrix_UDepth[k][iSoilType][iLandUse]=pRst->Fields->Item["UDepth"]->Value;
				Matrix_DDepth[k][iSoilType][iLandUse]=pRst->Fields->Item["DDepth"]->Value;
				Matrix_I0[k][iSoilType][iLandUse]=pRst->Fields->Item["I0"]->Value;
				Matrix_ErosionK[k][iSoilType][iLandUse]=pRst->Fields->Item["ErosionK"]->Value;

				//以下两行,wh,20080803
				VTmp=pRst->Fields->Item["ErosionBeta"]->Value;
			    Matrix_ErosionBeta[k][iSoilType][iLandUse]=VTmp.vt==VT_NULL?0.0f:pRst->Fields->Item["ErosionBeta"]->Value;

				//David
				Matrix_ErosionK1[k][iSoilType][iLandUse]=pRst->Fields->Item["ErosionK1"]->Value;
				Matrix_ErosionK2[k][iSoilType][iLandUse]=pRst->Fields->Item["ErosionK2"]->Value;
				Matrix_ErosionBeta1[k][iSoilType][iLandUse]=pRst->Fields->Item["ErosionBeta1"]->Value;
				Matrix_ErosionBeta2[k][iSoilType][iLandUse]=pRst->Fields->Item["ErosionBeta2"]->Value;
				
				pRst->MoveNext();
			}
			pRst->Close();
		}
		catch(_com_error e)
		{
			cout<<"Failture Of Reading From Table Parameter."<<rank<<endl;
			
			cout<<e.Error()<<endl;
			cout<<e.ErrorMessage()<<endl;
			cout<<(LPCSTR)e.Source()<<endl;        
			cout<<(LPCSTR)e.Description()<<endl;

			MPI_Finalize();
			exit(0);
		}
	}//end for
}


//从RiverSegs表读入所有河段，组成代表全流域的森林
void MasterProcess::FormForest(void)
{
	ofstream myFile444;
	myFile444.open("FormForest.txt",ios::trunc);
	_variant_t VTmp;
	BSCode TmpBSCode;
	Para TmpPara;	
	int TmpSOrder;
	int count1=0;

	CString cSQL;
	_bstr_t SQL;
	TotalList=new TreeList[GradeTwoCount]; //一个TreeList代表一个RegionIndex子树

	//20060905,xiaofc,要先initialize,防止TotalList[i].First==NULL，造成上下游依赖判断的错误
	for(GradeTwoLoop=0;GradeTwoLoop<GradeTwoCount;GradeTwoLoop++)
	{
		TotalList[GradeTwoLoop].initialize(RegionConnection,RCCount,TotalList,GradeTwoCount);
		TotalList[GradeTwoLoop].MySParameter=SParameter;//20130904, shy
	}

	//逐次打开二级区作总循环
	cout<<"Process 0 is forming forest..."<<endl;
	for(GradeTwoLoop=0;GradeTwoLoop<GradeTwoCount;GradeTwoLoop++)
	{
		//wh:一个for循环里面的操作是将一个RegionIndex组织成一棵树，for循环完了，森林也就OK了。
		//try
		//{
			cSQL.Format("select * from riversegs where regiongrade=%d and regionindex=%I64u order by bslength",GradeTwoCode[GradeTwoLoop].RegionGrade,GradeTwoCode[GradeTwoLoop].RegionIndex); 
			SQL=cSQL.GetString();
			pRst->Open(SQL,(ADODB::_Connection*)pCnn,ADODB::adOpenStatic,ADODB::adLockOptimistic,ADODB::adCmdText);

			count1 += pRst->RecordCount;//count1记录河流的总数目
			pRst->MoveFirst();

			int k=0;
			while(!pRst->EndOfFile)
			{
				TmpBSCode.Length = pRst->Fields->Item["bslength"]->Value;

				tempCom = pRst->Fields->Item["bsvalue"]->Value;
				TmpBSCode.Value = tempCom.ullVal;

				TmpBSCode.RegionGrade=GradeTwoCode[GradeTwoLoop].RegionGrade;
				TmpBSCode.RegionIndex=GradeTwoCode[GradeTwoLoop].RegionIndex;

				VTmp=pRst->Fields->Item["MiddleX"]->Value;
				TmpPara.X=VTmp.vt==VT_NULL?110.0f:pRst->Fields->Item["MiddleX"]->Value;

				VTmp=pRst->Fields->Item["MiddleY"]->Value;
				TmpPara.Y=VTmp.vt==VT_NULL?37.0f:pRst->Fields->Item["MiddleY"]->Value;

				TmpPara.AreaL=pRst->Fields->Item["AreaLeft"]->Value;
				TmpPara.AreaR=pRst->Fields->Item["AreaRight"]->Value;
				TmpPara.AreaS=pRst->Fields->Item["AreaSource"]->Value;

				TmpPara.LengthL=pRst->Fields->Item["LengthLeft"]->Value;
				TmpPara.LengthR=pRst->Fields->Item["LengthRight"]->Value;
				TmpPara.LengthS=pRst->Fields->Item["LengthSource"]->Value;
				TmpPara.StreamLength=pRst->Fields->Item["SegLength"]->Value;

				TmpPara.SlopeL=pRst->Fields->Item["SlopeLeft"]->Value;
				if(abs(TmpPara.SlopeL)<1e-5) TmpPara.SlopeL=1e-5f;
				TmpPara.SlopeR=pRst->Fields->Item["SlopeRight"]->Value;
				if(abs(TmpPara.SlopeR)<1e-5) TmpPara.SlopeR=1e-5f;
				TmpPara.SlopeS=pRst->Fields->Item["SlopeSource"]->Value;
				if(abs(TmpPara.SlopeS)<1e-5) TmpPara.SlopeS=1e-5f;
				TmpPara.StreamSlope=pRst->Fields->Item["Slope"]->Value;

				//wh,20080803(20080306,xiaofc,增加沟坡坡度字段，专门用于计算重力侵蚀)
				TmpPara.GullySlope=pRst->Fields->Item["GullySlope"]->Value;
				TmpPara.DrainingArea=pRst->Fields->Item["CatchmentArea"]->Value;

				TmpPara.LandUse=pRst->Fields->Item["LandUse"]->Value;
				//TmpPara.LandUse=TmpPara.LandUse-1;//wh,LandUse在数据库中表示是1to6

				TmpPara.SoilType=pRst->Fields->Item["SoilType"]->Value;
				TmpPara.D50=pRst->Fields->Item["SoilD50"]->Value;

				TmpPara.Manning=pRst->Fields->Item["Manning"]->Value;
				TmpPara.RiverManning=pRst->Fields->Item["RiverManning"]->Value;

				TmpPara.A=pRst->Fields->Item["A"]->Value;

				TmpPara.UElevation=pRst->Fields->Item["UElevation"]->Value;
				TmpPara.DElevation=pRst->Fields->Item["DElevation"]->Value;

				TmpPara.m=4.0f;//边坡系数//wh,将来要变成分布式的参数，容易获得
				//if(TmpBSCode.Value==0 && TmpBSCode.RegionIndex==0)
					//TmpPara.m=20.0f;

				TmpPara.WaveCoefficient=1.0f;
				TmpPara.WetRadius=5.0f;
				TmpPara.x=0.2f;

				int i = GetParameterRegionIndex(TmpBSCode.RegionIndex);//added by wanghao

				//wh，basinmodel中有包含关系则不覆盖
				//无包含关系则遍历
				TmpPara.DDepth=Matrix_DDepth[i][TmpPara.SoilType][TmpPara.LandUse];
				TmpPara.DSita1=Matrix_DTheta1[i][TmpPara.SoilType][TmpPara.LandUse];
				TmpPara.DSita2=Matrix_DTheta2[i][TmpPara.SoilType][TmpPara.LandUse];
				TmpPara.I0=Matrix_I0[i][TmpPara.SoilType][TmpPara.LandUse];
				TmpPara.MSita1=Matrix_MTheta1[i][TmpPara.SoilType][TmpPara.LandUse];
				TmpPara.MSita2=Matrix_MTheta2[i][TmpPara.SoilType][TmpPara.LandUse];
				TmpPara.PKH1=Matrix_PKH1[i][TmpPara.SoilType][TmpPara.LandUse];
				TmpPara.PKH2=Matrix_PKH2[i][TmpPara.SoilType][TmpPara.LandUse];
				TmpPara.PKV0=Matrix_PKV0[i][TmpPara.SoilType][TmpPara.LandUse];
				TmpPara.PKV1=Matrix_PKV1[i][TmpPara.SoilType][TmpPara.LandUse];
				TmpPara.PKV2=Matrix_PKV2[i][TmpPara.SoilType][TmpPara.LandUse];//20060220,Iwish
				TmpPara.Sita1=Matrix_UTheta1[i][TmpPara.SoilType][TmpPara.LandUse];
				TmpPara.Sita2=Matrix_UTheta2[i][TmpPara.SoilType][TmpPara.LandUse];
				TmpPara.UDepth=Matrix_UDepth[i][TmpPara.SoilType][TmpPara.LandUse];
				TmpPara.ErosionK=Matrix_ErosionK[i][TmpPara.SoilType][TmpPara.LandUse];
				TmpPara.ErosionBeta=Matrix_ErosionBeta[i][TmpPara.SoilType][TmpPara.LandUse];//20080803

				//David
				TmpPara.ErosionK1=Matrix_ErosionK1[i][TmpPara.SoilType][TmpPara.LandUse];
				TmpPara.ErosionK2=Matrix_ErosionK2[i][TmpPara.SoilType][TmpPara.LandUse];
				TmpPara.ErosionBeta1=Matrix_ErosionBeta1[i][TmpPara.SoilType][TmpPara.LandUse];
				TmpPara.ErosionBeta2=Matrix_ErosionBeta2[i][TmpPara.SoilType][TmpPara.LandUse];

				TmpSOrder=pRst->Fields->Item["STRALHERORDER"]->Value;//Get StralherOrder

				if(TmpBSCode.RegionIndex==1122 && TmpBSCode.Length==603)
					int xxx=0;

				TotalList[GradeTwoLoop].Add(TmpBSCode,TmpPara,TmpSOrder);//Add to Tree

				pRst->MoveNext();
			}//end while
			//cout<<TotalList[0].First->TaskCount<<endl;
			
			pRst->Close();
		//}//end try

		//catch(_com_error e)
		//{
		//	cout<<"Failture Of Reading From Table RiverSegs When Forming Forest."<<rank<<endl;
		//	
		//	cout<<e.Error()<<endl;
		//	cout<<e.ErrorMessage()<<endl;
		//	cout<<(LPCSTR)e.Source()<<endl;        
		//	cout<<(LPCSTR)e.Description()<<endl;  

		//	MPI_Finalize();
		//	exit(0);
		//}

	}//二级区循环的结束
	delete[] ParameterRIndex;
	cout<<"End of Forming Forest"<<endl;
	cout<<"#"<<processor_name<<","<<rank<<",TTL,"<<count1<<":"<<endl;//TTL:河流总数目	
	myFile444 <<"ForestFormed.\n";
	myFile444.close();
}

//wh,2008，得到河段配置Parameter表哪个RegionIndex的参数
//如果有相等的就配置相等的，没有相等的就配置离它最近一级的长辈。
//输入参数为河段的RegionIndex
int MasterProcess::GetParameterRegionIndex(unsigned long long regionindex)
{
	assert(IndexCount>0);
	while(1>0)
	{
		for(int i=0; i<IndexCount; i++)
		{
			if(regionindex == ParameterRIndex[i])
				return i;
			if(regionindex>ParameterRIndex[i])//因为从小到大排的，所以后面的肯定小
				break;
		}
		regionindex /= SParameter.RegionSystem;//20130904, shy;
		//if(regionindex==1)
		if(regionindex<SParameter.CompRegion+1)//shy, regionindex小于CompRegion+1，则已到干流,20140514
			break;
	}//end while
	return 0;
}


//向各计算进程进行第一次子任务派发
void MasterProcess::DispatchTask(void)
{
	ofstream myFile444;
	myFile444.open("DispatchTask.txt",ios::trunc);
	//WorldSize = 1;//wh,该句只是方便用一个进程来调试。
	TreeBranches=new TreeList[WorldSize];//WorldSize：计算进程的个数
	NowBSCode=new BSCode[WorldSize];
	TaskFinished=new int[WorldSize];
	iWorkingNodes = WorldSize;//WorldSize已经是纯计算进程的数目了

	//下面的循环表示对每个计算进程第一次任务发送
	for(int iii=0;iii<WorldSize;iii++)
	{
		TreeBranches[iii].initialize(TotalList[0].RegionConnection,TotalList[0].RCCount,TotalList,GradeTwoCount);
		TreeBranches[iii] = GetBranchInForest(TotalList,SParameter.TaskUnitSize,SParameter.MinTaskUnitSize,GradeTwoCount);

		//如果连一个待算的河段都找不到了
		if(TreeBranches[iii].First==NULL)
		{
			myFile444<<"进程"<<iii<<"下岗.\n";
			//一个都找不到，要有人下岗了
			TaskFinished[iii]=0;
			MPI_Send(&TaskFinished[iii],1,MPI_INT,iii+1,10,MPI_COMM_WORLD);		
			iWorkingNodes--;
		}
		else
		{
			BranchToSendSize = TreeBranches[iii].First->TaskCount;
			//cout<<BranchToSendSize<<endl;
			BranchToSend = new TreeNode[BranchToSendSize];
			TreeBranches[iii].ExpandTo(BranchToSend);//将发送子树展开为数组
			
			//记忆根节点的BSCode和发送子树的河段总数
			NowBSCode[iii] = BranchToSend[BranchToSendSize-1].mBSCode;//展平后根节点在最后
			TaskFinished[iii] = BranchToSend[BranchToSendSize-1].TaskCount;
			//cout<<NowBSCode[iii].RegionGrade<<"\t"<<NowBSCode[iii].RegionIndex<<"\t"<<NowBSCode[iii].Length<<"\t"<<NowBSCode[iii].Value<<"\t"<<TaskFinished[iii]<<endl;
			myFile444<<"进程"<<iii<<"接收到新的任务("<<NowBSCode[iii].RegionIndex<<","<<NowBSCode[iii].Length<<","<<NowBSCode[iii].Value<<")\n"<<flush;
			MPI_Send(&TaskFinished[iii],1,MPI_INT,iii+1,10,MPI_COMM_WORLD);	//向计算进程发送
			MPI_Send(BranchToSend,BranchToSendSize*sizeof(TreeNode),MPI_BYTE,iii+1,10,MPI_COMM_WORLD);
			delete[] BranchToSend;
		}
	}//end for
	myFile444 <<"TaskDispatched.\n";
	myFile444.close();
}


//循环处理从计算进程接收到的任务
void MasterProcess::RecvSlaveProcess(void)
{
	ofstream myFile555;
	myFile555.open("RecvSlaveProcess.txt",ios::trunc);
	myFile555 <<"TaskDispatched.\n"<<flush;
	myFile555 <<"Now Observing RecvSlaveProcess.\n"<<flush;


	int jjj;
	BSCode TmpCode;
	while(1)
	{
		myFile555<<"计算过程信息开始收集..."<<".\n"<<flush;
		MPI_Recv(&TmpCode,sizeof(BSCode),MPI_BYTE,MPI_ANY_SOURCE,10,MPI_COMM_WORLD,&Status);//找到完成任务的rank
		int iii=Status.MPI_SOURCE-1;
		NowBSCode[iii]=TmpCode;
		myFile555<<"进程"<<iii<<"完成了任务("<<NowBSCode[iii].RegionIndex<<","<<NowBSCode[iii].Length<<","<<NowBSCode[iii].Value<<")\n"<<flush;

		if(TmpCode.RegionIndex==116010101 && TmpCode.Length==289 && TmpCode.Value==0)
	    {
		int xx=0;
	    }

		if(IsEmptyForest(TotalList,GradeTwoCount))//整个森林空了，适用于真正全算完了的情况
		{
			TaskFinished[iii]=0;
			MPI_Send(&TaskFinished[iii],1,MPI_INT,iii+1,10,MPI_COMM_WORLD);			
			MPI_Send(&TaskFinished[iii],1,MPI_INT,this->TransferProcessRank,14,MPI_COMM_WORLD);	//wh,通知中转进程一切都结束了。
			//cout<<"The Tree Is Empty."<<endl;

			//wh,结束前把hydroscheme表中的方案状态改为1，表示搞定了。
			CString SQL;
			_bstr_t sql;
			SQL.Format("update hydroscheme set status=1 where sccd=%s",SParameter.sccd);
			sql=SQL.GetString();
			try
			{
				pCnn->Execute(sql,NULL,ADODB::adCmdText);
				pCnn->Close();
			}
			catch(_com_error e)
			{
				cout<<"Failture Of Updating Status Of Table HydroScheme."<<rank<<endl;

				cout<<e.Error()<<endl;
				cout<<e.ErrorMessage()<<endl;
				cout<<(LPCSTR)e.Source()<<endl;        
				cout<<(LPCSTR)e.Description()<<endl; 

				MPI_Finalize();
				exit(0);
			}
			break;
		}
	
		//找完成的任务属于哪棵树
		for(jjj=0;jjj<GradeTwoCount;jjj++)
		{
			if( (TotalList[jjj].First) && NowBSCode[iii].RegionIndex==TotalList[jjj].First->mBSCode.RegionIndex )
			{
					//ofstream myFile666;
	    //      myFile666.open("FindParentPosit.txt",ios::app);
	    //      myFile666 <<"NowBSCode[iii].Length-TotalList[jjj].First->mBSCode.Length-1="<<NowBSCode[iii].Length-TotalList[jjj].First->mBSCode.Length-1<<".\n";
	    //      myFile666.close();
				TotalList[jjj].FinishBranch(NowBSCode[iii],TaskFinished[iii]);
				myFile555<<"进程"<<iii<<"完成的任务属于区域"<<TotalList[jjj].First->mBSCode.RegionIndex<<"\n"<<flush;
				break;
			}
		}

		//哪个都没找到，则完成了一根子树的根
		//wh：此时First当然为NULL
		if(jjj==GradeTwoCount)
		try
		{
			myFile555<<"进程"<<iii<<"完成了一根子树的根!"<<"\n"<<flush;
			TotalList[0].FinishBranch(NowBSCode[iii],TotalList,GradeTwoCount);
		}
		catch (...)
		{
			cout<<"Error while FinishBranch for the root part of a branch."<<endl;
		}
		myFile555<<"给进程"<<iii<<"分配新任务中.\n"<<flush;
		//找到新任务
		//20060119,李铁键,如果仅剩一个计算节点，则取消最小任务单元的限制，防止最后小于最小任务单元的根部分不出去
		if(iWorkingNodes>1)
			TreeBranches[iii]=GetBranchInForest(TotalList,SParameter.TaskUnitSize,SParameter.MinTaskUnitSize,GradeTwoCount);
		else
			TreeBranches[iii]=GetBranchInForest(TotalList,SParameter.TaskUnitSize,1,GradeTwoCount);

		//一个都找不到，要有人下岗了
		if(TreeBranches[iii].First==NULL)//找不到合适规模的树，适用于僧多粥少的情况
		{
			//NowBSCode[iii].RegionIndex=-1;//暂这样处理，没有任务就把这个值置-1,免得下一次循环还去FinishBranch,出错
			TaskFinished[iii]=0;
			MPI_Send(&TaskFinished[iii],1,MPI_INT,iii+1,10,MPI_COMM_WORLD);	
			myFile555<<"进程"<<iii<<"找不到合适规模的树.\n"<<flush;
			iWorkingNodes--;
			//cout<<"Send 0 to "<<iii+1<<endl;
		}
		else//该else部分同上
		{
			BranchToSendSize=TreeBranches[iii].First->TaskCount;
			BranchToSend=new TreeNode[BranchToSendSize];
			TreeBranches[iii].ExpandTo(BranchToSend);
		
			NowBSCode[iii]=BranchToSend[BranchToSendSize-1].mBSCode;
			TaskFinished[iii]=BranchToSend[BranchToSendSize-1].TaskCount;
			myFile555<<"进程"<<iii<<"接收到新的任务("<<NowBSCode[iii].RegionIndex<<","<<NowBSCode[iii].Length<<","<<NowBSCode[iii].Value<<")\n"<<flush;
			MPI_Send(&TaskFinished[iii],1,MPI_INT,iii+1,10,MPI_COMM_WORLD);
			MPI_Send(BranchToSend,BranchToSendSize*sizeof(TreeNode),MPI_BYTE,iii+1,10,MPI_COMM_WORLD);
			delete [] BranchToSend;
		}
		myFile555<<"进程"<<iii<<"本轮任务收发结束.\n"<<flush;

		iii++;
		if(iii==WorldSize){ iii=0;}

		//20060327,李铁键,加入CPU利用率//renewed by xia
		lCpuUsage=m_CpuUsage.GetCpuUsageNT();
		iCpuUsage=(int)lCpuUsage;
		cout<<"#"<<processor_name<<","<<rank<<",CPU,"<<iCpuUsage<<":"<<endl;				

	}//end of while

	//清理内存
	delete[] NowBSCode;
	delete[] TaskFinished;

	delete[] RegionConnection;
	delete[] TotalList;
	delete[] GradeTwoCode;
	delete[] TreeBranches;

	myFile555 <<"Calculation Finished.\n";
	myFile555.close();
}




