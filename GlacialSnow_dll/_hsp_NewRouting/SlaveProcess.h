#ifndef SLAVEPROCESS_H
#define SLAVEPROCESS_H
#pragma once
#include "DataStruct.h"
#include "CommenProcess.h"
#include ".\avpm.h"

//该类用来表示计算进程的执行过程。
class SlaveProcess : public CommenProcess
{
public:
	SlaveProcess(void);
	~SlaveProcess(void);

public:
	void SlaveProcessInitialize(void);
	void RecvMasterProcess(void);//计算进程循环处理主控进程发送的消息，汇总各个子函数

private:
	void ReservoirSegment(BSCode mBSCode,int TaskLoop);//水库节点的计算
	void RunoffCalc(BSCode mBSCode,Para mPara,int TaskLoop);//非水库节点的产流计算
	
	void NoChild(BSCode mBSCode);//找上游河段的出口流量
	void OneChild(BSCode mBSCode,int Boy,int Girl);
	
	void ConfluenceCalc(int TaskLoop,BSCode mBSCode,Para mPara);//汇流计算
	
	void SaveGravityEvents(int TaskLoop);//保存重力侵蚀事件
	void WriteToDischarge(int TaskLoop);//将产流产沙结果写入discharge表
	void PackSend(int k);//将根节点信息打包发送到中转进程1
	void Finalize(void);

private:
	HRESULT hr;

	int TaskCount;//子树共有河段的个数
	
	TreeNode* mTreeNode;
	int Parent,Boy,Girl;//三者在数组里的索引

	vector<GravityEvent> GravityEvents;//20060327,李铁键,为存储重力侵蚀信息用

	//中转数据
	float** pQin; float** pQout;
	float** pSin; float** pSout;
	float** pWLM; float** pWRM;
	//2009:HSP模型传递的沟道地下水流量过程
	float** pDout;
	
	float* ZeroSerial;
	float* pQUpRegion;
	float* pSUpRegion;
	float* pDUpRegion;//for HSP模型沟道地下水

	float* pFlowB;
	float* pFlowH;
	float* pFlow_v;
	
	double DBUpdateTime;//写数据库时间
	double CommTime;//接收时间
    double CalTime;//计算时间
	double SendTime;//发送时间

	ADODB::_RecordsetPtr pRstData;//指向Discharge表
	ADODB::_RecordsetPtr pRstQ;//指向Discharge表
	ADODB::_RecordsetPtr pRstRsv;//指向Reservoir表
	ADODB::_RecordsetPtr pRstIndicator;//指向DefinedNodes表
	ADODB::_RecordsetPtr pRstGravityEvents;//重力侵蚀事件
		
	List* RsvList;
	_variant_t vtmp;	

private:
   
	//------------------------wh,2008.3.23 added--------------------------------
	//用来处理多模型接口
	unsigned long long BasinModelIndex[20];
	CString BasinModel[20][10];
	int RowCount;//BasinModel行数
	int ColumnCount;
	int GetBasinModelRegionIndex(unsigned long long regionindex);//得到该区域要使用那些模型
	int ModelSelect;//得到此次接受的任务从BasinModel中的第几行取数据
	//------------------------wh,2008.3.23 added--------------------------------

	//wh,2008,12.9，通过basinmodel表用来判断是否需要链接相应的dll，因为Finalize函数中需要根据
	//此进行release，所以变量声明放到类定义里来了。
	CString MLTJYR,MXAJ,MSRM,MHSP; 
};

#endif