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

//��һ��RegionIndex������һ���Ӷ�
//newBSCode1�������Ϣ��Para��������Ϣ��SOrder��S�ּ���
void TreeList::Add(BSCode newBSCode1, Para newPara, int SOrder)
{
	BSCode newBSCode=newBSCode1;
	TreeNode * TreePointer;
	TreeNode * ParentNode;
	
	bool isCalcLine;//���ĩ�ҽڵ�Ϊ���ӵ㣬�����ϴ�����Ҫ�����һ�ߣ��ô˱�����ǡ�

	if(First==NULL)//�����ĸ�
	{
		First=new TreeNode;

		//���µĽڵ㸳ֵ
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
	else//���Ѿ����˸��ڵ�
	{
		//�������ҵ�λ��
		ParentNode=FindParentPosit(newBSCode);

		TreePointer=new TreeNode;

		if(newBSCode.Value & 1)
			ParentNode->Girl=TreePointer;//����BsvalueΪgirl�ڵ�
		else
			ParentNode->Boy=TreePointer;

		//���µĽڵ㸳ֵ
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

		//����������TaskCount
		//wh�����沿�ָ������Ч�����ǵõ������TreeList�л����ܡ�����������ĺӶΣ���Щ�Ӷα�������������
		//RegionIndex�ļ����������Щ�Ӷ���UnderCalc��ʶ��
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

//20060904,xiaofc,�����ж���Regionconnection�е����ӹ�ϵ�������ж������������Ƿ��ڱ��μ��������ڡ�
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
		//20060904,xiaofc,��ʹ���δ������򣬽����ж������������Ƿ��ڱ��μ���������
		//20080406��wh����ʵ��������϶��ڱ��μ��������ڣ���Ϊ����HydroUsePara�е�CalcRegionΪ"��RegionIndex",�����ε�����RegionIndex�ᱻ�Զ�ѡ���������Ǻ���ġ�
		j=0;
		while(j<GradeTwoCount && TotalList[j].First)
		{
			if(TotalList[j].First->mBSCode.RegionGrade==RegionConnection[i].RegionGrade && TotalList[j].First->mBSCode.RegionIndex==RegionConnection[i].RegionIndex)
				return 1;
			j++;
		}
		return 0;//�����������ӣ����䲻���κ�һ�����μ����������
	}
	return 0;
}

//���þ��Ǽ���Min������������Ǹ�GetBranch����
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

//StartPoint����RegionIndex��ͷ�ڵ㣬�ú��������þ��Ǵ�ͷ�ڵ�������ҵ�һ������Min��Max֮�����
TreeList TreeList::GetBranch(TreeNode *StartPoint,int Max, int Min)
{
	TreeList BranchList;
	TreeNode *TreePoint;
	TreeNode *ParentNode;
	BranchList.initialize(RegionConnection,RCCount,TotalList,GradeTwoCount);

	if(!StartPoint)
		return BranchList;

	TreePoint=StartPoint;

	//�ظ�����������������������������û��ĩ��
	while(TreePoint->Boy && TreePoint->TaskCount>Max)
	{
		TreePoint=TreePoint->Boy;
	}

	//����ѵ��Ľڵ㲻��С����û���ڱ����㣬�жϣ��ֳ�BranchList
	//if(TreePoint->TaskCount>=Min && TreePoint->TaskCount<=Max && TreePoint->UnderCalc==0)//20060906,xiaofc,�����ж�����>max��ԭ�����
	//20070702,xiaofc:
	//�ڱȽ϶��������ͬʱ���������£�ĳһ��������ļ�������Ѿ��ӽ����ڣ����Ӷ���С��Min
	//�ھɵĹ����£���һ����Ҫ�ȴ�������֧����������,ֱ��Min���͵�����²��ֳܷ�����
	//����û�����Ч�ʣ�����������ԭ�����Ĵ���.
	//��˸�Ϊ����������ڸ����ĺӶΣ���СֻҪ����<=Max�������ȷֳ���
	//wh��TreePoint==First����ʾͷ�ڵ��TaskCount�Ѿ�<=Max�ˣ���ʱ�������ǲ��Ǵ��ڵ���Min���ᱻ�г�����Ȼǰ����UnderCalc=0��
	//wh��TreePoint->UnderCalc==0���ж���������Ҫ�������Ϊ0��ðȻ�г�������ת�������᲻������
	if(( TreePoint->TaskCount>=Min || TreePoint==First ) && TreePoint->TaskCount<=Max && TreePoint->UnderCalc==0)//20060906,xiaofc,�����ж�����>max��ԭ�����
	{
		//if(TreePoint==First)
		//{
		//	cout<<"TreePoint->"<<TreePoint<<endl;
		//	cout<<"TreePoint->BSCode: "<<TreePoint->mBSCode.RegionIndex<<"\t"<<TreePoint->mBSCode.Value<<"\t"<<TreePoint->mBSCode.Length<<endl;
		//	cout<<"Parent->"<<TreePoint->Parent<<endl;
		//}

		ParentNode=TreePoint->Parent;
		
		if(ParentNode)
			//�е��ļȿ�����Boy,Ҳ������Girl,����Ҫ�Ӵ��ж�
			if(ParentNode->Boy==TreePoint)
				ParentNode->Boy=NULL;
			else
				ParentNode->Girl=NULL;
		else //�����������ֳ�ȥ�ˣ������ÿ�
			First=NULL;

		TreePoint->Parent=NULL;
		BranchList.First=TreePoint;

		//�������г����Ǹ�����ִ�б��״̬�Ĺ��ܣ������ڵ�
		//���е����ϱ��UnderCalc״̬
		while(ParentNode)
		{
			ParentNode->UnderCalc=ParentNode->UnderCalc+1;//wh��good
			ParentNode=ParentNode->Parent;
		}
	}
	//20060906:xiaofc,ʣ�����������Ǵ˵��BoyΪNULL,���Ǵ˵��TaskCountȴ����Max
	//wh��boyΪNULL����Ϊ����������ķ���
	else if(1/*TreePoint!=StartPoint 20070523,���ܼӴ���������Ϊ���ܸո��ϴδ�(0,2)���꣬���ȷʵ����(0,1)�����߲���ȥ*/)//������ܱ��жϣ��������ã�ִ��GetBranch
	{
		//TreePoint=TreePoint->Parent;Remarked on 20060906,��һ֧��BoyΪNULL,����Girl>Maxʱ��������Girl�˾�ᵼ�¹�ģ��Max
		while(TreePoint && BranchList.First==NULL)//���������Ҳ�Ҳ����������丸�����ã����ҵ���Ϊֹ
		{
 			//����ӷ�֧�һص��˿�ʼ�ĳ����㣬���˳������ؿ���
			if(TreePoint==StartPoint && TreePoint->UnderCalc>0)//20070523�����������������Ǹ��ڵ㣬����Ҫ����Ϊundercalc�ŷ���NULL
				break;

			if(TreePoint->Girl && TreePoint->Girl->TaskCount>=Min)
				 BranchList=GetBranch(TreePoint->Girl,Max,Min);
			TreePoint=TreePoint->Parent;
		}
	}

	return BranchList;
}

//��ToList�ڴ���չƽ
//wh��ToList��Ԫ�صĸ���Ϊ�и����һ����������ӵ�еĺӶ���
void TreeList::ExpandTo(TreeNode * ToList)
{
	if(First==NULL)
		return;

	//20060621����������ÿ��չ��ǰҪȷ��չ��Ŀ��������±�ָʾ(Loop)Ϊ0
	Loop=0;
	ExpandTo(ToList,First);
}

//wh�����ToList:ʢװ�г����������������׵�ַ��CurrentNode���и������ĸ����ڵ㣬��ʼֵΪͷ�ڵ㣬������������
void TreeList::ExpandTo(TreeNode * ToList,TreeNode * CurrentNode)
{
	if(CurrentNode==NULL)
		return;

	//������
	if(CurrentNode->Boy)
		ExpandTo(ToList,CurrentNode->Boy);
	if(CurrentNode->Girl)
		ExpandTo(ToList,CurrentNode->Girl);
	
	//������
	ToList[Loop]=*CurrentNode;//��ֵ

	if(CurrentNode!=First)//�жϸ��ӻ�Ů��ϵ
		CurrentNode->Parent->Boy==CurrentNode?CurrentNode->Parent->Boy=NULL:CurrentNode->Parent->Girl=NULL;
	else
		First=NULL;
	//�ջ��ڴ�
	delete CurrentNode;//wh����Ϊ�����Ѿ������˸�ֵ��������δ�RegionIndex�н���Щ�Ӷ�delete�����ͷ��ڴ档
	//����ָ��ƫ��
	Loop++;
	return;
}

//�������������һ��������ʱ����ͨ��������RegionIndex��û�����꣩�����FinishBranch�������ͨ���������������RegionIndex��
//TaskCount��UnderCalc����Ϣ����
void TreeList::FinishBranch(BSCode mBSCode,int TaskFinished)
{
	TreeNode * TreePointer;

	TreePointer=FindParentPosit(mBSCode);//�ҵ�λ��

	//�����ɵ�����ͨ����֦
	//������Ϣ��undercalc,TaskCount
	while(TreePointer->Parent)
	{
		TreePointer->TaskCount-=TaskFinished;
		TreePointer->UnderCalc--;
		TreePointer=TreePointer->Parent;
	}
	
	TreePointer->TaskCount-=TaskFinished;
	TreePointer->UnderCalc--;

}

//���������FinishBranch����һ��RegionIndex�����������RegionIndex��UnderCalc��Ϣ����
void TreeList::FinishBranch(BSCode mBSCode, TreeList *TotalList, int GradeTwoCount)
{
	int i;

	//����Ѿ����ܸ����ˣ�ֱ�ӷ���
	//wh������Ѿ��Ǹ���(RegionIndex=0)����ʱ�Ѿ�û�����ε�RegionIndex�ˣ���Ȼ�����ٸ��£�ֱ�ӷ��ؼ��ɡ�ֻ��RegionIndex=0�ĺӶ�RegionGrade��Ϊ1
	if(mBSCode.RegionGrade==1)
		return;

	//�����ҵ������ӵ����Σ��޸�UnderCalc
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
	//��ֻ���㲿��֧��ʱ��ĳһ��֧��������RegionConnection��������Σ�������β�����GradeTwoCount�ļ���֮��
	//��������£���һ��forѭ����i=GradeTwoCount��û��break
	//�������TotalList[GradeTwoCount]Խ�磬�������if�ж�
	if(i<GradeTwoCount)//xiaofc added
		TotalList[i].FinishBranch(UpRegion,0);

}

//����������ؽ��̶���RegionIndex��ʱ���ǰ���RegionGrade��С�����˳�����ģ�
//��˶���ĳ���ڵ�ʱ�丸��һ�����ڡ�
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