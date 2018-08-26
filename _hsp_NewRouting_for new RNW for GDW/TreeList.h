#ifndef TREELIST_H
#define TREELIST_H
#pragma	once
#include <stdlib.h>
#include "DataStruct.h"
#include "CommenProcess.h"

//wh，一个TreeList代表一个RegionIndex
class TreeList
{
public:
	//必段按BSLength从小到大的顺序加
	void Add(BSCode mBSCode, Para mPara, int SOrder);
	//初始化时输入表示连接关系的BSCode指针
	void initialize(BSCode * RegionCnn, int mRCCount,TreeList * mTotalList,int mGradeTwoCount);
	//得到规模大致为BranchSize的一个子树
	TreeList GetBranch(int BranchSize);
	// 向ToList内存区展平
	void ExpandTo(TreeNode * ToList);
	//更新完成任务后的树,用于一树的一枝
	void FinishBranch(BSCode mBSCode,int TaskFinished);
	//更新完成任务后的树,用于完成一树
	void FinishBranch(BSCode mBSCode,TreeList * TotalList, int GradeTwoCount);

	TreeNode *First;
	BSCode *RegionConnection;
	int RCCount;

	//20060904,xiaofc，增加树对于森林状况的认知
	TreeList *TotalList;
	int GradeTwoCount;//整个计算区域不同RegionIndex的个数
	SystemParameter MySParameter;//HydroUsePara表变量, shy, 20130904
private:
	bool isCnnNode(BSCode mBSCode);//20060904Updated,xiaofc
	TreeList GetBranch(TreeNode* StartPoint, int Max, int Min);
	void ExpandTo(TreeNode * ToList,TreeNode * CurrentNode);
	//找到一个编码对应的节点的父节点地址
	TreeNode * FindParentPosit(BSCode mBSCode);

private:
	int Loop;
};

#endif