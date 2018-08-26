#ifndef MPICOMPUTING_H
#define MPICOMPUTING_H
#pragma once
#include "CommenProcess.h"
#include "TreeList.h"

//�����ʾ���ؽ�����ִ�еĲ������ý��̵�rankΪ0
class MasterProcess : public CommenProcess
{
public:
	MasterProcess(void);
	~MasterProcess(void);

public:
	void ReadBSCode(void);//��RiverSegs�������
	void ReadFromParameter(void);//��Parameter�������
	void FormForest(void);//�����ݿ��еĺӶ����ڴ����齨����
	void DispatchTask(void);//���е�һ�������ɷ�
	void RecvSlaveProcess(void);//ѭ���ȴ���������������Ϣ

private:
	BSCode* GradeTwoCode;//��������ı���
	int GradeTwoCount;//��������ĸ���

	TreeList* TotalList;//ɭ�֣���������������
	TreeList* TreeBranches;//TreeBranches��ÿ��Ԫ�ش洢����Ҫ���͸�����ڵ����������ͬ��Ԫ�ش���ͬ�ļ���ڵ㡣
	
	TreeNode* BranchToSend;//ÿ��Ԫ��Ϊ����һ��С���ĺӶνڵ㣬Ԫ�ظ���Ϊ�����BranchToSendSize
	int BranchToSendSize;
	
	BSCode* NowBSCode;//NowBSCode[iii]��ʾ���͸�iii+1��������ͷ�ڵ��BSCode
	int* TaskFinished;//ͬ�ϣ���ʾ��ÿ��������̷��͵ĺӶ�����
	
	int iWorkingNodes;//20060119������������ʾ���ڹ����Ľڵ���,��ʼ����WordSize-1,ÿ����һ������ڵ��1
	
	//wh,add [5],[5]:parameter���е�RegionIndex��[30]����soiltype��[20]����landuse
	float Matrix_PKH1[5][30][20];
	float Matrix_PKH2[5][30][20];
	float Matrix_PKV0[5][30][20];
	float Matrix_PKV1[5][30][20];
	float Matrix_PKV2[5][30][20];//20060220,Iwish,�����в����������������ϵ��
	float Matrix_UTheta1[5][30][20];
	float Matrix_UTheta2[5][30][20];
	float Matrix_MTheta1[5][30][20];
	float Matrix_MTheta2[5][30][20];
	float Matrix_DTheta1[5][30][20];
	float Matrix_DTheta2[5][30][20];
	float Matrix_UDepth[5][30][20];
	float Matrix_DDepth[5][30][20];
	float Matrix_I0[5][30][20];
	float Matrix_ErosionK[5][30][20];

	float Matrix_ErosionBeta[5][30][20];//wh,20080803

	unsigned long long* ParameterRIndex;//wh
	int IndexCount;//ParameterRIndexԪ�ظ���

private:
	CString GetCalcRegionIndex(void);//wh,�õ�Ҫ�����RegionIndex
	int GetParameterRegionIndex(unsigned long long regionindex);//wh,�õ�Ҫ���õĲ�������parameter�����ĸ�RegionIndex

};

#endif
