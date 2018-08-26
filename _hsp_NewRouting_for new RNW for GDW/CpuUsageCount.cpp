#include "windows.h"
#include ".\cpuusagecount.h"

#define SystemBasicInformation       0
#define SystemPerformanceInformation 2
#define SystemTimeInformation        3

#define Li2Double(x) ((double)((x).HighPart) * 4.294967296E9 + (double)((x).LowPart))

typedef struct
{
    DWORD   dwUnknown1;
    ULONG   uKeMaximumIncrement;
    ULONG   uPageSize;
    ULONG   uMmNumberOfPhysicalPages;
    ULONG   uMmLowestPhysicalPage;
    ULONG   uMmHighestPhysicalPage;
    ULONG   uAllocationGranularity;
    PVOID   pLowestUserAddress;
    PVOID   pMmHighestUserAddress;
    ULONG   uKeActiveProcessors;
    BYTE    bKeNumberProcessors;
    BYTE    bUnknown2;
    WORD    wUnknown3;
} SYSTEM_BASIC_INFORMATION;

typedef struct
{
    LARGE_INTEGER   liIdleTime;
    DWORD           dwSpare[76];
} SYSTEM_PERFORMANCE_INFORMATION;

typedef struct
{
    LARGE_INTEGER liKeBootTime;
    LARGE_INTEGER liKeSystemTime;
    LARGE_INTEGER liExpTimeZoneBias;
    ULONG         uCurrentTimeZoneId;
    DWORD         dwReserved;
} SYSTEM_TIME_INFORMATION;

//CCpuUsageCount::CCpuUsageCount(void)
//: m_liOldIdleTime(0)
//, m_liOldSystemTime(0)
//{
//}

CCpuUsageCount::CCpuUsageCount(void)
{
	m_liOldIdleTime.QuadPart   = 0;
    m_liOldSystemTime.QuadPart = 0;

}

CCpuUsageCount::~CCpuUsageCount(void)
{
}

unsigned long CCpuUsageCount::GetCpuUsageNT(void)
{
	SYSTEM_PERFORMANCE_INFORMATION SysPerfInfo;
    SYSTEM_TIME_INFORMATION        SysTimeInfo;
    SYSTEM_BASIC_INFORMATION       SysBaseInfo;
    double                         dbIdleTime=40;
    double                         dbSystemTime;
    LONG                           status;
    typedef LONG (WINAPI *PROCNTQSI)(UINT,PVOID,ULONG,PULONG);
    PROCNTQSI NtQuerySystemInformation;

	NtQuerySystemInformation = (PROCNTQSI)GetProcAddress(
                                          GetModuleHandle("ntdll"),
                                         "NtQuerySystemInformation"
                                         );
	if (!NtQuerySystemInformation)
	{
        return 0;
	}

    // get number of processors in the system
    status = NtQuerySystemInformation(SystemBasicInformation,
		                              &SysBaseInfo,sizeof(SysBaseInfo),NULL);

    if (status != NO_ERROR)
	{
        return 0;
	}

	 status = NtQuerySystemInformation(SystemTimeInformation,
		                               &SysTimeInfo,sizeof(SysTimeInfo),0);
     if (status!=NO_ERROR)
	 {
          return 0;
	 }

     // get new CPU's idle time
     status = NtQuerySystemInformation(SystemPerformanceInformation,
		                               &SysPerfInfo,sizeof(SysPerfInfo),NULL);

     if (status != NO_ERROR)
	 {
          return 0;
	 }
	  if (m_liOldIdleTime.QuadPart != 0)
     {
         // CurrentValue = NewValue - OldValue
         dbIdleTime = Li2Double(SysPerfInfo.liIdleTime) - Li2Double(m_liOldIdleTime);
         dbSystemTime = Li2Double(SysTimeInfo.liKeSystemTime) - Li2Double(m_liOldSystemTime);

          // CurrentCpuIdle = IdleTime / SystemTime
          dbIdleTime = dbIdleTime / dbSystemTime;

          // CurrentCpuUsage% = 100 - (CurrentCpuIdle * 100) / NumberOfProcessors
          dbIdleTime = 100.0 - dbIdleTime * 100.0 / (double)SysBaseInfo.bKeNumberProcessors + 0.5;

          
     }

     // store new CPU's idle and system time
     m_liOldIdleTime = SysPerfInfo.liIdleTime;
     m_liOldSystemTime = SysTimeInfo.liKeSystemTime;

	return (UINT)dbIdleTime;
}
