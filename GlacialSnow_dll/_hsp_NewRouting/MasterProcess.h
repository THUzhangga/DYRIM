#ifndef MPICOMPUTING_H
#define MPICOMPUTING_H
#pragma once
#include "CommenProcess.h"
#include "TreeList.h"

//该类表示主控进程所执行的操作，该进程的rank为0
class MasterProcess : public CommenProcess
{
public:
	MasterProcess(void);
	~MasterProcess(void);

public:
	void ReadBSCode(void);//从RiverSegs表读参数
	void ReadFromParameter(void);//从Parameter表读参数
	void FormForest(void);//将数据库中的河段在内存中组建成树
	void DispatchTask(void);//进行第一次任务派发
	void RecvSlaveProcess(void);//循环等待并处理计算进程消息

private:
	BSCode* GradeTwoCode;//二级区块的编码
	int GradeTwoCount;//二级区块的个数

	TreeList* TotalList;//森林，即整个最大的流域
	TreeList* TreeBranches;//TreeBranches的每个元素存储的是要发送给计算节点的子树，不同的元素代表不同的计算节点。
	
	TreeNode* BranchToSend;//每个元素为上面一棵小树的河段节点，元素个数为下面的BranchToSendSize
	int BranchToSendSize;
	
	BSCode* NowBSCode;//NowBSCode[iii]表示发送给iii+1进程子树头节点的BSCode
	int* TaskFinished;//同上，表示往每个计算进程发送的河段总数
	
	int iWorkingNodes;//20060119，李铁键，表示正在工作的节点数,开始等于WordSize-1,每结束一个计算节点减1
	
	//wh,add [5],[5]:parameter表中的RegionIndex，[30]代表soiltype，[20]代表landuse
	float Matrix_PKH1[5][30][20];
	float Matrix_PKH2[5][30][20];
	float Matrix_PKV0[5][30][20];
	float Matrix_PKV1[5][30][20];
	float Matrix_PKV2[5][30][20];//20060220,Iwish,增加中层土向深层土的入渗系数
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
	int IndexCount;//ParameterRIndex元素个数

private:
	CString GetCalcRegionIndex(void);//wh,得到要计算的RegionIndex
	int GetParameterRegionIndex(unsigned long long regionindex);//wh,得到要配置的参数属于parameter表中哪个RegionIndex

};

#endif
