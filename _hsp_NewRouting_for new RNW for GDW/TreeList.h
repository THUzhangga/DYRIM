#ifndef TREELIST_H
#define TREELIST_H
#pragma	once
#include <stdlib.h>
#include "DataStruct.h"
#include "CommenProcess.h"

//wh��һ��TreeList����һ��RegionIndex
class TreeList
{
public:
	//�ضΰ�BSLength��С�����˳���
	void Add(BSCode mBSCode, Para mPara, int SOrder);
	//��ʼ��ʱ�����ʾ���ӹ�ϵ��BSCodeָ��
	void initialize(BSCode * RegionCnn, int mRCCount,TreeList * mTotalList,int mGradeTwoCount);
	//�õ���ģ����ΪBranchSize��һ������
	TreeList GetBranch(int BranchSize);
	// ��ToList�ڴ���չƽ
	void ExpandTo(TreeNode * ToList);
	//���������������,����һ����һ֦
	void FinishBranch(BSCode mBSCode,int TaskFinished);
	//���������������,�������һ��
	void FinishBranch(BSCode mBSCode,TreeList * TotalList, int GradeTwoCount);

	TreeNode *First;
	BSCode *RegionConnection;
	int RCCount;

	//20060904,xiaofc������������ɭ��״������֪
	TreeList *TotalList;
	int GradeTwoCount;//������������ͬRegionIndex�ĸ���
	SystemParameter MySParameter;//HydroUsePara�����, shy, 20130904
private:
	bool isCnnNode(BSCode mBSCode);//20060904Updated,xiaofc
	TreeList GetBranch(TreeNode* StartPoint, int Max, int Min);
	void ExpandTo(TreeNode * ToList,TreeNode * CurrentNode);
	//�ҵ�һ�������Ӧ�Ľڵ�ĸ��ڵ��ַ
	TreeNode * FindParentPosit(BSCode mBSCode);

private:
	int Loop;
};

#endif