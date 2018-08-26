#ifndef SLAVEPROCESS_H
#define SLAVEPROCESS_H
#pragma once
#include "DataStruct.h"
#include "CommenProcess.h"
#include ".\avpm.h"

//����������ʾ������̵�ִ�й��̡�
class SlaveProcess : public CommenProcess
{
public:
	SlaveProcess(void);
	~SlaveProcess(void);

public:
	void SlaveProcessInitialize(void);
	void RecvMasterProcess(void);//�������ѭ���������ؽ��̷��͵���Ϣ�����ܸ����Ӻ���

private:
	void ReservoirSegment(BSCode mBSCode,int TaskLoop);//ˮ��ڵ�ļ���
	void RunoffCalc(BSCode mBSCode,Para mPara,int TaskLoop);//��ˮ��ڵ�Ĳ�������
	
	void NoChild(BSCode mBSCode);//�����κӶεĳ�������
	void OneChild(BSCode mBSCode,int Boy,int Girl);
	
	void ConfluenceCalc(int TaskLoop,BSCode mBSCode,Para mPara);//��������
	
	void SaveGravityEvents(int TaskLoop);//����������ʴ�¼�
	void WriteToDischarge(int TaskLoop);//��������ɳ���д��discharge��
	void PackSend(int k);//�����ڵ���Ϣ������͵���ת����1
	void Finalize(void);

private:
	HRESULT hr;

	int TaskCount;//�������кӶεĸ���
	
	TreeNode* mTreeNode;
	int Parent,Boy,Girl;//�����������������

	vector<GravityEvent> GravityEvents;//20060327,������,Ϊ�洢������ʴ��Ϣ��

	//��ת����
	float** pQin; float** pQout;
	float** pSin; float** pSout;
	float** pWLM; float** pWRM;
	//2009:HSPģ�ʹ��ݵĹ�������ˮ��������
	float** pDout;
	
	float* ZeroSerial;
	float* pQUpRegion;
	float* pSUpRegion;
	float* pDUpRegion;//for HSPģ�͹�������ˮ

	float* pFlowB;
	float* pFlowH;
	float* pFlow_v;
	
	double DBUpdateTime;//д���ݿ�ʱ��
	double CommTime;//����ʱ��
    double CalTime;//����ʱ��
	double SendTime;//����ʱ��

	ADODB::_RecordsetPtr pRstData;//ָ��Discharge��
	ADODB::_RecordsetPtr pRstQ;//ָ��Discharge��
	ADODB::_RecordsetPtr pRstRsv;//ָ��Reservoir��
	ADODB::_RecordsetPtr pRstIndicator;//ָ��DefinedNodes��
	ADODB::_RecordsetPtr pRstGravityEvents;//������ʴ�¼�
		
	List* RsvList;
	_variant_t vtmp;	

private:
   
	//------------------------wh,2008.3.23 added--------------------------------
	//���������ģ�ͽӿ�
	unsigned long long BasinModelIndex[20];
	CString BasinModel[20][10];
	int RowCount;//BasinModel����
	int ColumnCount;
	int GetBasinModelRegionIndex(unsigned long long regionindex);//�õ�������Ҫʹ����Щģ��
	int ModelSelect;//�õ��˴ν��ܵ������BasinModel�еĵڼ���ȡ����
	//------------------------wh,2008.3.23 added--------------------------------

	//wh,2008,12.9��ͨ��basinmodel�������ж��Ƿ���Ҫ������Ӧ��dll����ΪFinalize��������Ҫ����
	//�˽���release�����Ա��������ŵ��ඨ�������ˡ�
	CString MLTJYR,MXAJ,MSRM,MHSP; 
};

#endif