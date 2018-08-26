#pragma once

#include "datastruct.h"
#include "CWaterYield.h"
using namespace std;
class SaveStatus
{
public:
	SaveStatus(void);
	~SaveStatus(void);
	// 初始化，打开记录集
	void initialize(BSCode mBSCode,long HourStart,long HoursCount,long StatusTime,ADODB::_ConnectionPtr);
public:
	ADODB::_RecordsetPtr pRstStat;
	ADODB::_ConnectionPtr pCnn;
	bool IsRewrite;//计算期内数据齐全，只需替换;
	void finalize(void);
	void DoSave(sStatus* mStatus,long StatusTime);//考虑StatusTime大于1天的情况

	CString sccd;//wh,2008.3.24
	_variant_t vtmp;//wh
	long HoursCount;
};
