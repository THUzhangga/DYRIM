#include ".\savestatus.h"

SaveStatus::SaveStatus(void)
{
	pRstStat.CreateInstance(__uuidof(ADODB::Recordset));
}

SaveStatus::~SaveStatus(void)
{
}

// 初始化，打开记录集
void SaveStatus::initialize(BSCode mBSCode,long HourStart,long NumofHours,long StatusTime)
{
	CString SQL,SQL1;
	_bstr_t bSQL,bSQL1;
	//SQL.Format("SELECT * from Status where RegionIndex=%I64u and BSValue=%I64u and BSLength=%d and HourOffset>=%d and HourOffset<%d ORDER BY HourOffset",mBSCode.RegionIndex,mBSCode.Value,mBSCode.Length,HourStart,HourStart+NumofHours);
	//SQL.Format(_T("SELECT * from XAJstatus where sccd=%s and RegionIndex=%I64u and BSValue=%I64u and BSLength=%d and HourOffset>%d and HourOffset<=%d ORDER BY HourOffset"),sccd,mBSCode.RegionIndex,mBSCode.Value,mBSCode.Length,HourStart,HourStart+NumofHours);
	bSQL=SQL.GetString();

	try
	{
		pRstStat->Open(bSQL,(ADODB::_Connection*)pCnn,ADODB:: adOpenKeyset,ADODB::adLockOptimistic,ADODB::adCmdText);
		//pRstStat->MoveFirst();//wh,2008.3.24,当没有记录时执行此句报错

		//2008.3.31,wh,当记录数为0，NumofHours不到24时，做0与0的比较，发现下面的if和else都不执行，总出错
		//因此增加NumofHours>=24的判断
		if(NumofHours>=24 && pRstStat->RecordCount==NumofHours/StatusTime)//数据充足，改写
		{
			IsRewrite=1;
		}
		else //不足，插入
		{
			IsRewrite=0;
			pRstStat->Close();

			//SQL1.Format(_T("DELETE FROM XAJstatus WHERE sccd=%s and RegionIndex=%I64u and BSValue=%I64u and BSLength=%d and HourOffset>%d and HourOffset<=%d"),sccd,mBSCode.RegionIndex,mBSCode.Value,mBSCode.Length,HourStart,HourStart+NumofHours);//wh,add sccd
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


void SaveStatus::DoSave(sStatus* mStatus)
{
	CComVariant tempCom(VT_EMPTY);

	try
	{
		if(IsRewrite)
		{
			pRstStat->Fields->Item["W"]->Value=mStatus->W;
			pRstStat->Fields->Item["QRG"]->Value=mStatus->QRG;
			pRstStat->Fields->Item["QRSS"]->Value=mStatus->QRSS;
		
			pRstStat->Fields->Item["E"]->Value=mStatus->E;//StatusTime内的蒸发量
			pRstStat->Fields->Item["P"]->Value=mStatus->P;//StatusTime内的降雨量

			pRstStat->Update();
			pRstStat->MoveNext();
		}
		else
		{
			pRstStat->AddNew();

			pRstStat->Fields->Item["sccd"]->Value = _variant_t(vtmp);

			tempCom.ChangeType(VT_DECIMAL);
			tempCom.ullVal=mStatus->mBSCode.RegionIndex;
			pRstStat->Fields->Item["RegionIndex"]->Value=tempCom;
			tempCom.ullVal=mStatus->mBSCode.Value;
			pRstStat->Fields->Item["BSValue"]->Value=tempCom;
			pRstStat->Fields->Item["BSLength"]->Value=mStatus->mBSCode.Length;

			pRstStat->Fields->Item["HourOffset"]->Value=mStatus->HourOffset;

			pRstStat->Fields->Item["W"]->Value=mStatus->W;
			pRstStat->Fields->Item["QRG"]->Value=mStatus->QRG;
			pRstStat->Fields->Item["QRSS"]->Value=mStatus->QRSS;

			pRstStat->Fields->Item["E"]->Value=mStatus->E;
			pRstStat->Fields->Item["P"]->Value=mStatus->P;

			pRstStat->Update();
		}
	}
	catch (_com_error &e) 
	{
		cout<<"Error While SaveStatus.DoSave."<<endl;
		//cout<<"sccd="<<sccd<<endl;//wh
		cout<<"RegionIndex="<<mStatus->mBSCode.RegionIndex<<"\tBSValue="<<mStatus->mBSCode.Value<<"\tBSLength="<<mStatus->mBSCode.Length<<endl;
		cout<<"HourOffset="<<mStatus->HourOffset<<endl;
		cout<<"W"<<mStatus->W<<"\tQRG="<<mStatus->QRG<<"\tQRSS="<<mStatus->QRSS<<endl;
		cout<<"E="<<mStatus->E<<"\tP="<<mStatus->P<<endl;
		_bstr_t eTem;
		long el;
		el=e.Error();
		eTem=e.Description();
		cout<<"HRESULT="<<el<<"\tErrMessage="<<eTem<<endl;

		exit(0);
	}
}
