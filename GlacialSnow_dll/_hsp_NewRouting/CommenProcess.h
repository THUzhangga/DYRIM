#pragma once
#include "DataStruct.h"
#include "CpuUsageCount.h"//20060327,������,����CPU������,CpuUsageCount���������ṩ
#include <iostream>
#undef SEEK_SET
#undef SEEK_END
#undef SEEK_CUR
#include "mpi.h"
using namespace std;


//wh�������������ݿ���HydroUsePara�����в���������ģ������Ҫ�Ĳ������洢�ڸñ��У���Щ������FilePath.ini�еĲ�����Ӧ
typedef struct SystemParameter
{
	long HourStart;
	long NumofHours;
	long PrepareHours;//׼������ʱ�䣬������������ʱ�Σ���������û�кӵ��ڳ�ʼ������Ӱ��
	long StatusTime;//20080406��wh������status��洢���ݵ����ڣ�ԭ����1d1�棬�������Ƚϴ�

	//CString m_RainFile;
	CString CSDatasource;
	CString CSUser;
	CString CSPassword;

	//CString sRoutingMethod;//��������
    CString sRainType;//���������ݵ�����	
	CString CompRegions;

	int TaskUnitSize;   //����������Ԫ��������С
	int MinTaskUnitSize;//�ܳ��ܵ���С���������Ĵ�С
	int iCalGravityErosion;//20050316,������,�����Ƿ����������ʴ�Ĳ���.0.������;1.ȫ����;2.��Regionindex==0&&BSValue==0֮��ļ���
	int iCalcSediTrans;//20070608,xiaofc,�Ƿ������ɳ���ͣ��ݼ�Ϊ�Ƿ񱣴�

	bool bCalRsvUp;//�Ƿ����ˮ������
	bool bSaveGravityEvents;//20060327,������,�Ƿ񱣴�������ʴ�¼�
	bool bSaveFlowPattern;//20070622,xiaofc,Save the flow pattern, namely B, H, v
	bool bSaveAllDischarge;//wh,20080217�Ƿ񱣴����кӶε�������Ϣ��Ϊ0ʱֻ����definednodes�ж���ĺӶ�
	
	float GravityErosioinP2;//20060324,������,����������ʴ������Χ�ĸ���
	float fAlphaErosion;//��ˢʱ�Ļָ�����ϵ��
	float fAlphaDeposition;//�ٻ�ʱ�Ļָ�����ϵ��
	CString SediTransCapF;//"Fei""Zhang"

	// coefficients needed in "parameters"
	float UpInitWaterContent;
	float MidInitWaterContent;
	float DownInitWaterContent;

	// coefficients needed in "evapotranspiration"
	CString emethod;
	float thetab;//ë�ܶ��Ѻ�ˮ��
	float thetaw;//��ή��ˮ��
	int N;//ָ��
	float E0_a; //E0/a

	CString sccd;//wh���������
	int MSTEP;//wh��ʱ�䲽��

    CString SoilErosionEquation;//wh,20080803
	
}SystemParameter;



//wh,����ʵ��MasterProcess��SlaveProcess��TransferProcess��һЩ��������
class CommenProcess
{
public:
	CommenProcess(void);
	~CommenProcess(void);

public:
	void ProcessInitialize(int worldsize,int Rank,CString user,CString password,CString sid);
	void ReadHydroUsePara(void);//�����ݿ��ж�ȡϵͳ����
	void ReadRegionConnection(void);//��RegionConnection���ж�ȡ��Ϣ

protected://�������Щ���������̶���Ҫ�õ��ģ�ע���ˡ�
	
	//---MPI�������---//
	int WorldSize;//�������ô��:)
	int rank;
	int namelen;
	int TransferProcessRank;//��ת���̵ı��
	char processor_name[MPI_MAX_PROCESSOR_NAME];
	MPI_Status Status;

	//---CPU�����ʹ������--//
	CCpuUsageCount m_CpuUsage;//20060327,������,����CPU������
	unsigned long lCpuUsage;
	int iCpuUsage;

	//---���ݿ�ָ�빲�����--//
	ADODB::_ConnectionPtr pCnn;//����Oracle���ݿ�
	ADODB::_RecordsetPtr pRst;//����RiverSegs��

	//---�ڴ�����������---//
	SystemParameter SParameter;//HydroUsePara�����
	XAJParameter XAJ;//��ӦXAJUSEPARA��
	BSCode* RegionConnection;
	CComVariant tempCom;//20070204,xiaofc,�ֶ�ֵ��win2000�²����Զ�תΪunsigned long long,������ʱComVariant
	int RCCount;
	int GradeTwoLoop;

	long Steps;
	long lTimeInterval;

};
