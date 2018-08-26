#include "DataStruct.h"
#include "TreeList.h"
#include <iostream>
#include <fstream>
#include <assert.h>
#include <time.h>
#include "MasterProcess.h"
#include "functions.h"
//��������Ҫ���ϣ�Ϊ������mpich2��һ��bug
#undef SEEK_SET
#undef SEEK_END
#undef SEEK_CUR
#include "mpi.h"
using namespace std;

MasterProcess::MasterProcess(void)
{
}

MasterProcess::~MasterProcess(void)
{
}


//wh added,2008.3.23,HydroUsePara���е�calcregion��"��regionindex"���ú���Ϊ���ҵ�Ҫ���������RegionIndex
//��RegionIndex��ȷ����ʽ��BasinModel�е���СRegionIndex��
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
		unsigned long long RootRegionIndex=_strtoui64((LPCTSTR)(_bstr_t)SParameter.CompRegions,NULL,10);//�ı�ת��Ϊunsigned long long
		unsigned long long temp;
		CString tempstring;
		CalcIndex = SParameter.CompRegions;//��ʼֵ�Ѿ�������regionindex�ˡ�
		while(!pRst->EndOfFile)
		{
			tempCom=pRst->Fields->Item["regionindex"]->Value;
			temp = tempCom.ullVal;

			//���ӽڵ㣬���������ڵ�
			while(temp > RootRegionIndex)
			{
				temp /= 100;
				if(temp == RootRegionIndex)
				{
					//����Ҫ�������·�ʽ�����ֱ����һ��tempstring.format("%s,%I64u"...)���ֲ�����ʱ�����ܳ���stack��������
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

//�������RegionIndex,RegionGrade����Ϣ��GradeTwoCode��̬������
void MasterProcess::ReadBSCode(void)
{
	CString cSQL;
	_bstr_t SQL;

	//20060904,xiaofc,����Region��Χ�޶�,CompRegions�Ƿ����ΪSQL���������ж�
	//��ȡ����������Ϣ
	if(SParameter.CompRegions=="" || SParameter.CompRegions=="0")
		cSQL.Format("select regiongrade,regionindex from riversegs group by regiongrade,regionindex order by regiongrade desc");
	else
	{
		CString CalcRegionIndex = this->GetCalcRegionIndex();//wh,ѡ��Ҫ��������������Ǻ���ģ���Ϊ��ֻ��һ�����ڣ�û����ڣ�ȫ���
		cSQL.Format("select regiongrade,regionindex from riversegs where regionindex in (%s) group by regiongrade,regionindex order by regiongrade desc",CalcRegionIndex);//wh
	}

	SQL=cSQL.GetString();
	try
	{
		pRst->Open(SQL,(ADODB::_Connection*)pCnn,ADODB::adOpenStatic,ADODB::adLockOptimistic,ADODB::adCmdText);
		GradeTwoCount=pRst->RecordCount;//GradeTwoCount:���������в�ͬRegionIndex�ĸ���
		pRst->MoveFirst();

		GradeTwoCode=new BSCode[GradeTwoCount];

		for(GradeTwoLoop=0;GradeTwoLoop<GradeTwoCount;GradeTwoLoop++)
		{
			GradeTwoCode[GradeTwoLoop].RegionGrade=pRst->Fields->Item["regiongrade"]->Value;

			tempCom=pRst->Fields->Item["regionindex"]->Value;
			GradeTwoCode[GradeTwoLoop].RegionIndex=tempCom.ullVal;

			pRst->MoveNext();
		}
		pRst->Close();//��������CommenProcess��
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


//���ܣ������ݿ��е�Parameter���ж����������ͺ�����������Ϣ
//wh��Ĭ������רҵģ��Ҫ�Ĳ������ᳬ��ԭ��parameter�����ֶ�
//wh2009ע��ʵ������Master�����������Parameter���ǲ����ʵģ���ΪParameter�������ھ����ģ�ͣ�������ģ�Ͳ���Ҫ�����
//�����ⲿ�ִ���Ӧ�÷ŵ�ģ�ʹ�������������һ��������ͨѶ������ȡ����ģ�ʹ����ж�ȡParameter������ݿ⿪��������һ
//�����ʡʱ�䣬ͬʱ��������ģ�ͼ���ʱ������Parameter����Ϣ�������ഫ���ˣ����仰˵Master����ֻ�贫�ݡ����������롱��
//�У�riversegs�����ϢҲ����д��ģ�͵Ĵ����������Riversegs��ļ�����Ϣ������ģ�Ͷ���Ҫ�õ��ģ���˷ŵ����ؽ�����Ҳ
//��ȫû�����⣬����ͨ���Եġ����Ǹ������е��鷳������ʱ���ˣ���ʱ�����Ż���
void MasterProcess::ReadFromParameter(void)
{
	//����ת��������������+��������=>13����ز���
	int iLandUse;//������
	int iSoilType;//������
	CString cSQL;
	_bstr_t SQL;

	_variant_t VTmp;//wh,20080803
	
	//SQL="Select * from parameter order by LandUse,SoilType";

	//wh added,2008
	//=================================================================================================================================
	cSQL.Format("Select distinct regionindex from parameter where sccd=%s order by regionindex desc",SParameter.sccd);//�Ӵ�С����
	SQL = cSQL.GetString();
	int i=0;//��ͬregionindex�ĸ���
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

		//wh�������˷������Լ��ֲ�ʽ����ʹ�õ�RegionIndex������Ҫ��ס��RegionIndexΪ��RegionIndex
		//regionindex�������ǰ��
		try
		{
			pRst->Open(SQL,(ADODB::_Connection*)pCnn,ADODB::adOpenStatic,ADODB::adLockOptimistic,ADODB::adCmdText);

			//д�����
			while(!pRst->EndOfFile)
			{
				iSoilType=pRst->Fields->Item["SoilType"]->Value;
				iLandUse=pRst->Fields->Item["LandUse"]->Value;
				//iLandUse -= 1;//6����Ϊ1~6//wh,�Ժ�landuse���Ǵ�0��ʼ�棬��soiltypeͳһ,���Բ����ټ�1��

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

				//��������,wh,20080803
				VTmp=pRst->Fields->Item["ErosionBeta"]->Value;
			    Matrix_ErosionBeta[k][iSoilType][iLandUse]=VTmp.vt==VT_NULL?0.0f:pRst->Fields->Item["ErosionBeta"]->Value;

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


//��RiverSegs��������кӶΣ���ɴ���ȫ�����ɭ��
void MasterProcess::FormForest(void)
{
	_variant_t VTmp;
	BSCode TmpBSCode;
	Para TmpPara;	
	int TmpSOrder;
	int count1=0;

	CString cSQL;
	_bstr_t SQL;
	TotalList=new TreeList[GradeTwoCount]; //һ��TreeList����һ��RegionIndex����

	//20060905,xiaofc,Ҫ��initialize,��ֹTotalList[i].First==NULL����������������жϵĴ���
	for(GradeTwoLoop=0;GradeTwoLoop<GradeTwoCount;GradeTwoLoop++)
		TotalList[GradeTwoLoop].initialize(RegionConnection,RCCount,TotalList,GradeTwoCount);

	//��δ򿪶���������ѭ��
	cout<<"Process 0 is forming forest..."<<endl;
	for(GradeTwoLoop=0;GradeTwoLoop<GradeTwoCount;GradeTwoLoop++)
	{
		//wh:һ��forѭ������Ĳ����ǽ�һ��RegionIndex��֯��һ������forѭ�����ˣ�ɭ��Ҳ��OK�ˡ�
		try
		{
			cSQL.Format("select * from riversegs where regiongrade=%d and regionindex=%I64u order by bslength",GradeTwoCode[GradeTwoLoop].RegionGrade,GradeTwoCode[GradeTwoLoop].RegionIndex); 
			SQL=cSQL.GetString();
			pRst->Open(SQL,(ADODB::_Connection*)pCnn,ADODB::adOpenStatic,ADODB::adLockOptimistic,ADODB::adCmdText);

			count1 += pRst->RecordCount;//count1��¼����������Ŀ
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

				//wh,20080803(20080306,xiaofc,���ӹ����¶��ֶΣ�ר�����ڼ���������ʴ)
				TmpPara.GullySlope=pRst->Fields->Item["GullySlope"]->Value;
				TmpPara.DrainingArea=pRst->Fields->Item["CatchmentArea"]->Value;

				TmpPara.LandUse=pRst->Fields->Item["LandUse"]->Value;
				//TmpPara.LandUse=TmpPara.LandUse-1;//wh,LandUse�����ݿ��б�ʾ��1to6

				TmpPara.SoilType=pRst->Fields->Item["SoilType"]->Value;
				TmpPara.D50=pRst->Fields->Item["SoilD50"]->Value;

				TmpPara.Manning=pRst->Fields->Item["Manning"]->Value;
				TmpPara.RiverManning=pRst->Fields->Item["RiverManning"]->Value;

				TmpPara.A=pRst->Fields->Item["A"]->Value;
				TmpPara.UElevation=pRst->Fields->Item["UElevation"]->Value;
				TmpPara.DElevation=pRst->Fields->Item["DElevation"]->Value;

				TmpPara.m=4.0f;//����ϵ��//wh,����Ҫ��ɷֲ�ʽ�Ĳ��������׻��
				TmpPara.WaveCoefficient=1.0f;
				TmpPara.WetRadius=5.0f;
				TmpPara.x=0.2f;

				int i = GetParameterRegionIndex(TmpBSCode.RegionIndex);//added by wanghao

				//wh��basinmodel���а�����ϵ�򲻸���
				//�ް�����ϵ�����
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

				TmpSOrder=pRst->Fields->Item["STRALHERORDER"]->Value;//Get StralherOrder

				TotalList[GradeTwoLoop].Add(TmpBSCode,TmpPara,TmpSOrder);//Add to Tree

				pRst->MoveNext();
			}//end while
			//cout<<TotalList[0].First->TaskCount<<endl;
			
			pRst->Close();
		}//end try

		catch(_com_error e)
		{
			cout<<"Failture Of Reading From Table RiverSegs When Forming Forest."<<rank<<endl;
			
			cout<<e.Error()<<endl;
			cout<<e.ErrorMessage()<<endl;
			cout<<(LPCSTR)e.Source()<<endl;        
			cout<<(LPCSTR)e.Description()<<endl;  

			MPI_Finalize();
			exit(0);
		}

	}//������ѭ���Ľ���
	delete[] ParameterRIndex;
	cout<<"End of Forming Forest"<<endl;
	cout<<"#"<<processor_name<<","<<rank<<",TTL,"<<count1<<":"<<endl;//TTL:��������Ŀ	
}

//wh,2008���õ��Ӷ�����Parameter���ĸ�RegionIndex�Ĳ���
//�������ȵľ�������ȵģ�û����ȵľ������������һ���ĳ�����
//�������Ϊ�Ӷε�RegionIndex
int MasterProcess::GetParameterRegionIndex(unsigned long long regionindex)
{
	assert(IndexCount>0);
	while(1>0)
	{
		for(int i=0; i<IndexCount; i++)
		{
			if(regionindex == ParameterRIndex[i])
				return i;
			if(regionindex>ParameterRIndex[i])//��Ϊ��С�����ŵģ����Ժ���Ŀ϶�С
				break;
		}
		regionindex /= 100;
		if(regionindex==0)
			break;
	}//end while
	return 0;
}


//���������̽��е�һ���������ɷ�
void MasterProcess::DispatchTask(void)
{
	//WorldSize = 1;//wh,�þ�ֻ�Ƿ�����һ�����������ԡ�
	TreeBranches=new TreeList[WorldSize];//WorldSize��������̵ĸ���
	NowBSCode=new BSCode[WorldSize];
	TaskFinished=new int[WorldSize];
	iWorkingNodes = WorldSize;//WorldSize�Ѿ��Ǵ�������̵���Ŀ��

	//�����ѭ����ʾ��ÿ��������̵�һ��������
	for(int iii=0;iii<WorldSize;iii++)
	{
		TreeBranches[iii].initialize(TotalList[0].RegionConnection,TotalList[0].RCCount,TotalList,GradeTwoCount);
		TreeBranches[iii] = GetBranchInForest(TotalList,SParameter.TaskUnitSize,SParameter.MinTaskUnitSize,GradeTwoCount);

		//�����һ������ĺӶζ��Ҳ�����
		if(TreeBranches[iii].First==NULL)
		{
			//һ�����Ҳ�����Ҫ�����¸���
			TaskFinished[iii]=0;
			MPI_Send(&TaskFinished[iii],1,MPI_INT,iii+1,10,MPI_COMM_WORLD);		
			iWorkingNodes--;
		}
		else
		{
			BranchToSendSize = TreeBranches[iii].First->TaskCount;
			//cout<<BranchToSendSize<<endl;
			BranchToSend = new TreeNode[BranchToSendSize];
			TreeBranches[iii].ExpandTo(BranchToSend);//����������չ��Ϊ����
			
			//������ڵ��BSCode�ͷ��������ĺӶ�����
			NowBSCode[iii] = BranchToSend[BranchToSendSize-1].mBSCode;//չƽ����ڵ������
			TaskFinished[iii] = BranchToSend[BranchToSendSize-1].TaskCount;
			//cout<<NowBSCode[iii].RegionGrade<<"\t"<<NowBSCode[iii].RegionIndex<<"\t"<<NowBSCode[iii].Length<<"\t"<<NowBSCode[iii].Value<<"\t"<<TaskFinished[iii]<<endl;

			MPI_Send(&TaskFinished[iii],1,MPI_INT,iii+1,10,MPI_COMM_WORLD);	//�������̷���
			MPI_Send(BranchToSend,BranchToSendSize*sizeof(TreeNode),MPI_BYTE,iii+1,10,MPI_COMM_WORLD);
			delete[] BranchToSend;
		}
	}//end for
}


//ѭ������Ӽ�����̽��յ�������
void MasterProcess::RecvSlaveProcess(void)
{
	int jjj;
	BSCode TmpCode;
	while(1)
	{
		MPI_Recv(&TmpCode,sizeof(BSCode),MPI_BYTE,MPI_ANY_SOURCE,10,MPI_COMM_WORLD,&Status);//�ҵ���������rank
		int iii=Status.MPI_SOURCE-1;
		NowBSCode[iii]=TmpCode;

		if(IsEmptyForest(TotalList,GradeTwoCount))//����ɭ�ֿ��ˣ�����������ȫ�����˵����
		{
			TaskFinished[iii]=0;
			MPI_Send(&TaskFinished[iii],1,MPI_INT,iii+1,10,MPI_COMM_WORLD);			
			MPI_Send(&TaskFinished[iii],1,MPI_INT,this->TransferProcessRank,14,MPI_COMM_WORLD);	//wh,֪ͨ��ת����һ�ж������ˡ�
			//cout<<"The Tree Is Empty."<<endl;

			//wh,����ǰ��hydroscheme���еķ���״̬��Ϊ1����ʾ�㶨�ˡ�
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
	
		//����ɵ����������Ŀ���
		for(jjj=0;jjj<GradeTwoCount;jjj++)
		{
			if( (TotalList[jjj].First) && NowBSCode[iii].RegionIndex==TotalList[jjj].First->mBSCode.RegionIndex )
			{
				TotalList[jjj].FinishBranch(NowBSCode[iii],TaskFinished[iii]);
				break;
			}
		}

		//�ĸ���û�ҵ����������һ�������ĸ�
		//wh����ʱFirst��ȻΪNULL
		if(jjj==GradeTwoCount)
		try
		{
			TotalList[0].FinishBranch(NowBSCode[iii],TotalList,GradeTwoCount);
		}
		catch (...)
		{
			cout<<"Error while FinishBranch for the root part of a branch."<<endl;
		}

		//�ҵ�������
		//20060119,������,�����ʣһ������ڵ㣬��ȡ����С����Ԫ�����ƣ���ֹ���С����С����Ԫ�ĸ����ֲ���ȥ
		if(iWorkingNodes>1)
			TreeBranches[iii]=GetBranchInForest(TotalList,SParameter.TaskUnitSize,SParameter.MinTaskUnitSize,GradeTwoCount);
		else
			TreeBranches[iii]=GetBranchInForest(TotalList,SParameter.TaskUnitSize,1,GradeTwoCount);

		//һ�����Ҳ�����Ҫ�����¸���
		if(TreeBranches[iii].First==NULL)//�Ҳ������ʹ�ģ������������ɮ�����ٵ����
		{
			//NowBSCode[iii].RegionIndex=-1;//����������û������Ͱ����ֵ��-1,�����һ��ѭ����ȥFinishBranch,����
			TaskFinished[iii]=0;
			MPI_Send(&TaskFinished[iii],1,MPI_INT,iii+1,10,MPI_COMM_WORLD);	
			iWorkingNodes--;
			//cout<<"Send 0 to "<<iii+1<<endl;
		}
		else//��else����ͬ��
		{
			BranchToSendSize=TreeBranches[iii].First->TaskCount;
			BranchToSend=new TreeNode[BranchToSendSize];
			TreeBranches[iii].ExpandTo(BranchToSend);
		
			NowBSCode[iii]=BranchToSend[BranchToSendSize-1].mBSCode;
			TaskFinished[iii]=BranchToSend[BranchToSendSize-1].TaskCount;

			MPI_Send(&TaskFinished[iii],1,MPI_INT,iii+1,10,MPI_COMM_WORLD);
			MPI_Send(BranchToSend,BranchToSendSize*sizeof(TreeNode),MPI_BYTE,iii+1,10,MPI_COMM_WORLD);
			delete [] BranchToSend;
		}

		iii++;
		if(iii==WorldSize){ iii=0;}

		//20060327,������,����CPU������//renewed by xia
		lCpuUsage=m_CpuUsage.GetCpuUsageNT();
		iCpuUsage=(int)lCpuUsage;
		cout<<"#"<<processor_name<<","<<rank<<",CPU,"<<iCpuUsage<<":"<<endl;				

	}//end of while

	//�����ڴ�
	delete[] NowBSCode;
	delete[] TaskFinished;

	delete[] RegionConnection;
	delete[] TotalList;
	delete[] GradeTwoCode;
	delete[] TreeBranches;

}




