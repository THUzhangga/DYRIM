#pragma once
#include "datastruct.h"
using namespace std;

class SaveStatus
{
public:
	SaveStatus(void);
	~SaveStatus(void);
	
public:
	ADODB::_RecordsetPtr pRstStat;
	ADODB::_ConnectionPtr pCnn;
	bool IsRewrite;//计算期内数据齐全，只需替换;
	
	CString sccd;
	_variant_t vtmp;

	void initialize(BSCode mBSCode,long HourStart,long NumofHours,long StatusTime);// 初始化，打开记录集
	void finalize(void);
	void DoSave(sStatus* mStatus);
	
};
