
#include "stdafx.h"
#include ".\savestatus.h"

SaveStatus::SaveStatus(void)
{
	pRstStat.CreateInstance(__uuidof(ADODB::Recordset));
}

SaveStatus::~SaveStatus(void)
{
}

// 初始化，打开记录集
//HoursCount即是NumOfHours
void SaveStatus::initialize(BSCode mBSCode,long HourStart,long mHoursCount,long StatusTime,ADODB::_ConnectionPtr mpCnn)//考虑StatusTime大于1天的情况,shy
{
	HoursCount=mHoursCount;
	pCnn=mpCnn;
	CString SQL,SQL1;
	_bstr_t bSQL,bSQL1;
	//SQL.Format(_T("SELECT * from Status where RegionIndex=%I64u and BSValue=%I64u and BSLength=%d and HourOffset>=%d and HourOffset<%d ORDER BY HourOffset"),mBSCode.RegionIndex,mBSCode.Value,mBSCode.Length,HourStart,HourStart+HoursCount);
	SQL.Format(_T("SELECT * from Status where sccd='%s' and RegionIndex=%I64u and BSValue=%I64u and BSLength=%d and HourOffset>%d and HourOffset<=%d ORDER BY HourOffset"),sccd,mBSCode.RegionIndex,mBSCode.Value,mBSCode.Length,HourStart,HourStart+HoursCount);//wh,add sccd
	bSQL=SQL.GetString();

	try
	{
		pRstStat->Open(bSQL,(ADODB::_Connection*)pCnn,ADODB:: adOpenKeyset,ADODB::adLockOptimistic,ADODB::adCmdText);
		//pRstStat->MoveFirst();//wh,2008.3.24,当没有记录时执行此句报错

		//2008.3.31,wh,当记录数为0，HoursCount不到24时，做0与0的比较，发现下面的if和else都不执行，总出错
		//因此增加HoursCount>=24的判断
		long myCount=pRstStat->RecordCount;
		//if(HoursCount>=24 && pRstStat->RecordCount==HoursCount/24)//数据充足，改写
		if(HoursCount>=StatusTime && pRstStat->RecordCount==HoursCount/StatusTime)//数据充足，改写
		{
			IsRewrite=1;
		}
		else //不足，插入
		{
			IsRewrite=0;

			pRstStat->Close();

			SQL1.Format(_T("DELETE FROM Status WHERE SCCD='%s' and RegionIndex=%I64u and BSValue=%I64u and BSLength=%d and HourOffset>%d and HourOffset<=%d"),sccd, mBSCode.RegionIndex,mBSCode.Value,mBSCode.Length,HourStart,HourStart+HoursCount);//wh,add sccd
			bSQL1=SQL1.GetString();
			pCnn->Execute(bSQL1,NULL,ADODB::adCmdText);

			pRstStat->Open(bSQL,(ADODB::_Connection*)pCnn,ADODB:: adOpenKeyset,ADODB::adLockOptimistic,ADODB::adCmdText);
		}
	}
	
	catch(_com_error &er)
	{
		cout << "error in WaterYield.dll:SavaStatus:initialize"<<endl; 
		cout<<er.ErrorMessage() << endl;
		cout << er.Source() << endl;
		cout<<er.Description()<<endl;
		return;
	}
	
}

void SaveStatus::finalize(void)
{
	pRstStat->Close();
}

void SaveStatus::DoSave(sStatus* mStatus,long StatusTime)//考虑StatusTime大于1天的情况,shy
{
	long i;
	long DayCount;
	//DayCount=HoursCount/24+1;
	DayCount=HoursCount/StatusTime+1;//考虑StatusTime大于1天的情况,shy
	CComVariant tempCom(VT_EMPTY);
	try
	{
		if(IsRewrite)
		{
			for(i=1;i<DayCount;i++)
			{
				pRstStat->Fields->Item["WLL"]->Value=mStatus[i].WLL;
				pRstStat->Fields->Item["WLU"]->Value=mStatus[i].WLU;
				pRstStat->Fields->Item["WLM"]->Value=mStatus[i].WLM;
				pRstStat->Fields->Item["WLD"]->Value=mStatus[i].WLD;
				pRstStat->Fields->Item["WRL"]->Value=mStatus[i].WRL;
				
				pRstStat->Fields->Item["WRU"]->Value=mStatus[i].WRU;
				pRstStat->Fields->Item["WRM"]->Value=mStatus[i].WRM;
				pRstStat->Fields->Item["WRD"]->Value=mStatus[i].WRD;
				pRstStat->Fields->Item["WSL"]->Value=mStatus[i].WSL;
				pRstStat->Fields->Item["WSU"]->Value=mStatus[i].WSU;
				pRstStat->Fields->Item["WSM"]->Value=mStatus[i].WSM;
				pRstStat->Fields->Item["WSD"]->Value=mStatus[i].WSD;
				
				pRstStat->Fields->Item["E"]->Value=mStatus[i].E;//20070116,xiaofc
				pRstStat->Fields->Item["P"]->Value=mStatus[i].P;//20070202,xiaofc,日降雨量
	            
				pRstStat->Fields->Item["SlopeEro"]->Value=mStatus[i].SlopeErosion;//20080303,xiaofc,坡面侵蚀量

				/*for(i=5;i<20;i++)
				{
					temp=pRstStat->Fields->Item[i]->Value;
					if(temp<-1e-6) pRstStat->Fields->Item[i]->Value=-1.0;
				}*/
				pRstStat->MoveNext();
			}

			pRstStat->UpdateBatch(ADODB::adAffectAll);
			//pRstStat->Update();
		}
		else
		{
			for(i=1;i<DayCount;i++)
			{
				pRstStat->AddNew();

				//wh,added,sccd,2008.3.24
				//vtmp = sccd;
				pRstStat->Fields->Item["sccd"]->Value = 1;

				tempCom.ChangeType(VT_DECIMAL);
				tempCom.ullVal=mStatus[i].StatusBSCode.RegionIndex;
				pRstStat->Fields->Item["RegionIndex"]->Value=tempCom;

				tempCom.ullVal=mStatus[i].StatusBSCode.Value;
				pRstStat->Fields->Item["BSValue"]->Value=tempCom;

				pRstStat->Fields->Item["BSLength"]->Value=mStatus[i].StatusBSCode.Length;

				pRstStat->Fields->Item["HourOffset"]->Value=mStatus[i].HourOffset;

				pRstStat->Fields->Item["WLL"]->Value=mStatus[i].WLL;

				pRstStat->Fields->Item["WLU"]->Value=mStatus[i].WLU;

				pRstStat->Fields->Item["WLM"]->Value=mStatus[i].WLM;

				pRstStat->Fields->Item["WLD"]->Value=mStatus[i].WLD;

				pRstStat->Fields->Item["WRL"]->Value=mStatus[i].WRL;

				pRstStat->Fields->Item["WRU"]->Value=mStatus[i].WRU;

				pRstStat->Fields->Item["WRM"]->Value=mStatus[i].WRM;

				pRstStat->Fields->Item["WRD"]->Value=mStatus[i].WRD;

				pRstStat->Fields->Item["WSL"]->Value=mStatus[i].WSL;

				pRstStat->Fields->Item["WSU"]->Value=mStatus[i].WSU;

				pRstStat->Fields->Item["WSM"]->Value=mStatus[i].WSM;

				pRstStat->Fields->Item["WSD"]->Value=mStatus[i].WSD;
			
				pRstStat->Fields->Item["E"]->Value=mStatus[i].E;//20070116,xiaofc

				pRstStat->Fields->Item["P"]->Value=mStatus[i].P;//20070202,xiaofc,日降雨量
	        
				pRstStat->Fields->Item["SlopeEro"]->Value=mStatus[i].SlopeErosion;//20080303,xiaofc,坡面侵蚀量

				//int column=pRstStat->Fields->Count;//列数，一共22
				/*for(i=5;i<20;i++)
				{
					temp=pRstStat->Fields->Item[i]->Value;
					if(temp<-1e-6) pRstStat->Fields->Item[i]->Value=-1.0;
				}*/
			}
			pRstStat->UpdateBatch(ADODB::adAffectAll);
		}
	}
	catch (_com_error &e) 
	{
		cout<<"Error While SaveStatus.DoSave."<<endl;
		cout<<"sccd="<<sccd<<endl;//wh
		cout<<"RegionIndex="<<mStatus->StatusBSCode.RegionIndex<<"\tBSValue="<<mStatus->StatusBSCode.Value<<"\tBSLength="<<mStatus->StatusBSCode.Length<<endl;
		cout<<"HourOffset="<<mStatus->HourOffset<<endl;
		cout<<"WLL="<<mStatus->WLL<<"\tWLU="<<mStatus->WLU<<"\tWLM="<<mStatus->WLM<<"\tWLD="<<mStatus->WLD<<endl;
		cout<<"WRL="<<mStatus->WRL<<"\tWRU="<<mStatus->WRU<<"\tWRM="<<mStatus->WRM<<"\tWRD="<<mStatus->WRD<<endl;
		cout<<"WSL="<<mStatus->WSL<<"\tWSU="<<mStatus->WSU<<"\tWSM="<<mStatus->WSM<<"\tWSD="<<mStatus->WSD<<endl;
		cout<<"E="<<mStatus->E<<"\tP="<<mStatus->P<<"\tSlopeErosion="<<mStatus->SlopeErosion<<endl;//20070202,xiaofc,日降雨量; 20080303,坡面侵蚀量
	
		_bstr_t eTem,eTem2;
		long el;
		el=e.Error();
		eTem=e.Description();
		cout<<"HRESULT="<<el<<"\tErrMessage="<<eTem<<endl;
	    
		exit(0);
	}
}
