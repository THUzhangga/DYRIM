#include "TreeList.h"
#include ".\treelist.h"
using namespace std;

void TreeList::initialize(BSCode *RegionCnn,int mRCCount,TreeList *mTotalList,int mGradeTwoCount)
{
	First=NULL;
	RegionConnection=RegionCnn;
	RCCount=mRCCount;
	Loop=0;
	TotalList=mTotalList;
	GradeTwoCount=mGradeTwoCount;
}

//往一个RegionIndex中增加一条河段
//newBSCode1：身份信息；Para：参数信息；SOrder：S分级；
void TreeList::Add(BSCode newBSCode1, Para newPara, int SOrder)
{
	BSCode newBSCode=newBSCode1;
	TreeNode * TreePointer;
	TreeNode * ParentNode;
	
	bool isCalcLine;//如果末梢节点为连接点，则其上处于需要计算的一线，用此变量标记。

	if(First==NULL)//是树的根
	{
		First=new TreeNode;

		//给新的节点赋值
		First->Boy=NULL;
		First->Girl=NULL;
		First->Parent=NULL;
		First->mBSCode=newBSCode;
		First->mPara=newPara;
		First->StralherOrder=SOrder;
		First->UnderCalc=0;
		First->TaskCount=1;
		//cout<<First->TaskCount<<endl;
	}
	else//树已经有了根节点
	{
		//向下至找到位置
		ParentNode=FindParentPosit(newBSCode);

		TreePointer=new TreeNode;

		if(newBSCode.Value & 1)
			ParentNode->Girl=TreePointer;//奇数Bsvalue为girl节点
		else
			ParentNode->Boy=TreePointer;

		//给新的节点赋值
		TreePointer->Boy=NULL;
		TreePointer->Girl=NULL;
		TreePointer->Parent=ParentNode;
		TreePointer->mBSCode=newBSCode;
		TreePointer->mPara=newPara;
		TreePointer->StralherOrder=SOrder;
		TreePointer->TaskCount=1;

		try
		{
			isCalcLine=isCnnNode(newBSCode);
		}
		catch (...) 
		{
			cout<<"Err while isCnnNode"<<endl;
			exit(0);
		}

		if(isCalcLine)
			TreePointer->UnderCalc=1;
		else
			TreePointer->UnderCalc=0;

		//向上至更新TaskCount
		//wh，下面部分更新完后，效果就是得到了这个TreeList中还不能“独立”计算的河段，这些河段必须依赖于上游
		//RegionIndex的计算输出，这些河段用UnderCalc标识。
		TreePointer=TreePointer->Parent;
		while(TreePointer)
		{
			TreePointer->TaskCount=TreePointer->TaskCount+1;
			if(isCalcLine)
				TreePointer->UnderCalc=TreePointer->UnderCalc+1;

			TreePointer=TreePointer->Parent;
		}
	}
}

//20060904,xiaofc,不仅判断在Regionconnection中的连接关系，补充判断其上游区域是否在本次计算任务内。
bool TreeList::isCnnNode(BSCode mBSCode)
{
	int i;
	int j;
	for(i=0;i<RCCount;i++)
	{
		if(mBSCode.RegionGrade!=RegionConnection[i].RegionGrade-1)
			continue;
		if(mBSCode.RegionIndex!=unsigned long long(RegionConnection[i].RegionIndex/100))
			continue;
		if(mBSCode.Length!=RegionConnection[i].Length)
			continue;
		if(mBSCode.Value!=RegionConnection[i].Value)
			continue;
		//20060904,xiaofc,即使上游存在区域，进而判断其上游区域是否在本次计算任务内
		//20080406，wh，其实上游区域肯定在本次计算任务内，因为现在HydroUsePara中的CalcRegion为"根RegionIndex",其上游的所有RegionIndex会被自动选出，这样是合理的。
		j=0;
		while(j<GradeTwoCount && TotalList[j].First)
		{
			if(TotalList[j].First->mBSCode.RegionGrade==RegionConnection[i].RegionGrade && TotalList[j].First->mBSCode.RegionIndex==RegionConnection[i].RegionIndex)
				return 1;
			j++;
		}
		return 0;//存在上游连接，但其不在任何一个本次计算的区域内
	}
	return 0;
}

//作用就是计算Min，调用下面的那个GetBranch函数
TreeList TreeList::GetBranch(int BranchSize)
{
	TreeList BranchList;
	int iMin;
	iMin=int((BranchSize-1)/2);

	try
	{
		BranchList=GetBranch(First,BranchSize,iMin);
	}
	catch (...)
	{
		cout<<"Error while Treelist.GetBrach()."<<endl;
	}

	return BranchList;
}

//StartPoint就是RegionIndex的头节点，该函数的作用就是从头节点出发，找到一个介于Min和Max之间的树
TreeList TreeList::GetBranch(TreeNode *StartPoint,int Max, int Min)
{
	TreeList BranchList;
	TreeNode *TreePoint;
	TreeNode *ParentNode;
	BranchList.initialize(RegionConnection,RCCount,TotalList,GradeTwoCount);

	if(!StartPoint)
		return BranchList;

	TreePoint=StartPoint;

	//沿干流向上游搜索，至子树不超大且没到末稍
	while(TreePoint->Boy && TreePoint->TaskCount>Max)
	{
		TreePoint=TreePoint->Boy;
	}

	//如果搜到的节点不过小，且没正在被计算，切断，分出BranchList
	//if(TreePoint->TaskCount>=Min && TreePoint->TaskCount<=Max && TreePoint->UnderCalc==0)//20060906,xiaofc,增加判断条件>max，原因见下
	//20070702,xiaofc:
	//在比较多的子流域同时计算的情况下，某一个子流域的计算可能已经接近出口，但河段数小于Min
	//在旧的规则下，这一部分要等待到其他支流继续计算,直至Min降低的情况下才能分出任务
	//这样没有提高效率，反而引出了原因不明的错误.
	//因此改为，子流域出口附近的河段，大小只要满足<=Max，即优先分出。
	//wh，TreePoint==First，表示头节点的TaskCount已经<=Max了，此时不管它是不是大于等于Min都会被切出，当然前提是UnderCalc=0。
	//wh，TreePoint->UnderCalc==0的判断条件很重要，如果不为0就冒然切出，在中转进程中提不到货物
	if(( TreePoint->TaskCount>=Min || TreePoint==First ) && TreePoint->TaskCount<=Max && TreePoint->UnderCalc==0)//20060906,xiaofc,增加判断条件>max，原因见上
	{
		//if(TreePoint==First)
		//{
		//	cout<<"TreePoint->"<<TreePoint<<endl;
		//	cout<<"TreePoint->BSCode: "<<TreePoint->mBSCode.RegionIndex<<"\t"<<TreePoint->mBSCode.Value<<"\t"<<TreePoint->mBSCode.Length<<endl;
		//	cout<<"Parent->"<<TreePoint->Parent<<endl;
		//}

		ParentNode=TreePoint->Parent;
		
		if(ParentNode)
			//切掉的既可能是Boy,也可能是Girl,所以要加此判断
			if(ParentNode->Boy==TreePoint)
				ParentNode->Boy=NULL;
			else
				ParentNode->Girl=NULL;
		else //整棵树都被分出去了，树根置空
			First=NULL;

		TreePoint->Parent=NULL;
		BranchList.First=TreePoint;

		//真正被切出的那个函数执行标记状态的功能，至根节点
		//分切点以上标记UnderCalc状态
		while(ParentNode)
		{
			ParentNode->UnderCalc=ParentNode->UnderCalc+1;//wh，good
			ParentNode=ParentNode->Parent;
		}
	}
	//20060906:xiaofc,剩余的情况可能是此点的Boy为NULL,但是此点的TaskCount却大于Max
	//wh，boy为NULL是因为分任务产生的分离
	else if(1/*TreePoint!=StartPoint 20070523,不能加此条件，因为可能刚刚上次从(0,2)分完，这次确实落在(0,1)点上走不下去*/)//如果不能被切断，找其妹妹，执行GetBranch
	{
		//TreePoint=TreePoint->Parent;Remarked on 20060906,当一支的Boy为NULL,而其Girl>Max时，因错过此Girl此句会导致规模超Max
		while(TreePoint && BranchList.First==NULL)//如果其妹妹也找不到，再找其父的妹妹，至找到根为止
		{
 			//如果从分支找回到了开始的出发点，则退出，返回空树
			if(TreePoint==StartPoint && TreePoint->UnderCalc>0)//20070523，增加条件：不仅是根节点，而且要是因为undercalc才返回NULL
				break;

			if(TreePoint->Girl && TreePoint->Girl->TaskCount>=Min)
				 BranchList=GetBranch(TreePoint->Girl,Max,Min);
			TreePoint=TreePoint->Parent;
		}
	}

	return BranchList;
}

//向ToList内存区展平
//wh，ToList中元素的个数为切割出的一个子任务中拥有的河段数
void TreeList::ExpandTo(TreeNode * ToList)
{
	if(First==NULL)
		return;

	//20060621，李铁键：每次展开前要确认展开目的数组的下标指示(Loop)为0
	Loop=0;
	ExpandTo(ToList,First);
}

//wh解读：ToList:盛装切出“子树”的数组首地址，CurrentNode：切割子树的各个节点，初始值为头节点，放在数组的最后。
void TreeList::ExpandTo(TreeNode * ToList,TreeNode * CurrentNode)
{
	if(CurrentNode==NULL)
		return;

	//先左，右
	if(CurrentNode->Boy)
		ExpandTo(ToList,CurrentNode->Boy);
	if(CurrentNode->Girl)
		ExpandTo(ToList,CurrentNode->Girl);
	
	//处理结点
	ToList[Loop]=*CurrentNode;//赋值

	if(CurrentNode!=First)//切断父子或父女关系
		CurrentNode->Parent->Boy==CurrentNode?CurrentNode->Parent->Boy=NULL:CurrentNode->Parent->Girl=NULL;
	else
		First=NULL;
	//收回内存
	delete CurrentNode;//wh，因为上面已经进行了赋值操作，因次从RegionIndex中将这些河段delete掉，释放内存。
	//序列指针偏移
	Loop++;
	return;
}

//王皓解读：当算完一个子任务时（普通的子任务，RegionIndex并没有算完），这个FinishBranch负责该普通任务算完后其所在RegionIndex的
//TaskCount和UnderCalc的信息更新
void TreeList::FinishBranch(BSCode mBSCode,int TaskFinished)
{
	TreeNode * TreePointer;

	TreePointer=FindParentPosit(mBSCode);//找到位置

	//如果完成的是普通的树枝
	//更新信息，undercalc,TaskCount
	while(TreePointer->Parent)
	{
		TreePointer->TaskCount-=TaskFinished;
		TreePointer->UnderCalc--;
		TreePointer=TreePointer->Parent;
	}
	
	TreePointer->TaskCount-=TaskFinished;
	TreePointer->UnderCalc--;

}

//王皓解读：该FinishBranch负责当一个RegionIndex算完后，其下游RegionIndex的UnderCalc信息更新
void TreeList::FinishBranch(BSCode mBSCode, TreeList *TotalList, int GradeTwoCount)
{
	int i;

	//如果已经是总干流了，直接返回
	//wh，如果已经是干流(RegionIndex=0)，此时已经没有下游的RegionIndex了，当然不用再更新，直接返回即可。只有RegionIndex=0的河段RegionGrade才为1
	if(mBSCode.RegionGrade==1)
		return;

	//否则，找到其连接的上游，修改UnderCalc
	BSCode UpRegion;

	for(i=0;i<RCCount;i++)
		if(RegionConnection[i].RegionIndex==mBSCode.RegionIndex)
			break;
	if(i==RCCount)
		return;

	UpRegion.Value=RegionConnection[i].Value;
	UpRegion.Length=RegionConnection[i].Length;

	UpRegion.RegionIndex=unsigned long long(mBSCode.RegionIndex/100);
	UpRegion.RegionGrade=mBSCode.RegionGrade-1;

	for(i=0;i<GradeTwoCount;i++)
		if(TotalList[i].First && TotalList[i].First->mBSCode.RegionIndex==UpRegion.RegionIndex)
			break;
	
	//2007073,xiaofc:
	//当只计算部分支流时，某一个支流可能在RegionConnection里存在上游，这个上游并不在GradeTwoCount的计数之内
	//这种情况下，上一个for循环到i=GradeTwoCount而没有break
	//下面调用TotalList[GradeTwoCount]越界，因此增加if判断
	if(i<GradeTwoCount)//xiaofc added
		TotalList[i].FinishBranch(UpRegion,0);

}

//王皓解读：主控进程读入RegionIndex的时候，是按照RegionGrade从小到大的顺序读入的，
//因此读入某个节点时其父亲一定存在。
TreeNode * TreeList::FindParentPosit(BSCode mBSCode)
{
	TreeNode *TreePointer;
	int ii;
	TreePointer=First;

	for(ii=0;ii<mBSCode.Length-2;ii++)
	{
		if(mBSCode.Value & ((mBSCode.Length-2-ii)>=64?0:(unsigned long long)1<<(mBSCode.Length-2-ii)))//wh:very good
			TreePointer=TreePointer->Girl;
		else
			TreePointer=TreePointer->Boy;
	}
	return TreePointer;

}