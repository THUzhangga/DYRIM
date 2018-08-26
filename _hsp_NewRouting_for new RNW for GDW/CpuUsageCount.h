#ifndef CCPUUSAGECOUNT_H
#define CCPUUSAGECOUNT_H

#pragma once
class CCpuUsageCount
{
public:
	CCpuUsageCount(void);
	~CCpuUsageCount(void);
	unsigned long GetCpuUsageNT(void);
	LARGE_INTEGER m_liOldIdleTime;
	LARGE_INTEGER m_liOldSystemTime;
};

#endif


