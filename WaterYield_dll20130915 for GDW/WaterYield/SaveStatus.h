#pragma once

#include "datastruct.h"
#include "CWaterYield.h"
using namespace std;
class SaveStatus
{
public:
	SaveStatus(void);
	~SaveStatus(void);
	// ��ʼ�����򿪼�¼��
	void initialize(BSCode mBSCode,long HourStart,long HoursCount,long StatusTime,ADODB::_ConnectionPtr);
public:
	ADODB::_RecordsetPtr pRstStat;
	ADODB::_ConnectionPtr pCnn;
	bool IsRewrite;//��������������ȫ��ֻ���滻;
	void finalize(void);
	void DoSave(sStatus* mStatus,long StatusTime);//����StatusTime����1������

	CString sccd;//wh,2008.3.24
	_variant_t vtmp;//wh
	long HoursCount;
};
