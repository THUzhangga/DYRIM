
//wh:����������ʱ��û��ʹ��

#ifndef SAVESTATUS_H
#define SAVESTATUS_H
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
	bool IsRewrite;//��������������ȫ��ֻ���滻;
	
	_variant_t vtmp;

	void initialize(BSCode mBSCode,long HourStart,long NumofHours,long StatusTime);// ��ʼ�����򿪼�¼��
	void finalize(void);

	void DoSave(sStatus* mStatus);


};
#endif
