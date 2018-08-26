#include "TransferProcess.h"
#include "functions.h"
#undef SEEK_SET
#undef SEEK_END
#undef SEEK_CUR
#include "mpi.h"
#include <math.h>

TransferProcess::TransferProcess(void)
{
	first =new mListNode(); 
	first->BSLength = -1;
	first->BSValue = -1;
	first->RegionIndex = -1;
	first->Q = NULL;
	first->S = NULL;
	first->link = NULL;

	this->capacity = 0;
}

TransferProcess::~TransferProcess(void)
{
}

//wh,20080403,为了得到中转进程已经存了多少货物，如果货物堆积没人认领，时间长了会溢出的。
CString TransferProcess::GetSize(void)
{
	if(capacity<1024)
	{
		capacity = int(capacity);
		return "B";
	}
	else if(capacity>=1024 && capacity<1024*1024)
	{
		capacity /= 1024;
		capacity = float( int( (capacity *= 10) + 0.5 ) ) / 10;//为了保留一位小数
		return "KB";
	}
	else if(capacity>=1024*1024 && capacity<pow(1024.0,3.0) )
	{
		capacity = capacity/1024/1024;
		capacity = float( int( (capacity *= 10) + 0.5 ) ) / 10;//为了保留一位小数
		return "MB";
	}
	else if(capacity>=pow(1024.0,3.0) && capacity<pow(1024.0,4.0))
	{
		capacity = capacity/pow(1024.0,3.0);
		capacity = float( int( (capacity *= 100 ) + 0.5 ) ) / 100;//为了保留两位小数
		return "GB";
	}
	else
		cout<<"中转容量太大，我已无法忍受！"<<endl;

}


//wh，中转进程的主执行函数，当计算进程来取货时能保证货物一定存在。因为在保证上游河段计算完成，头节点消息发送成功后，进程0才进一步切割子树。
void TransferProcess::TransferMain(void)
{
	//position,size,buff:
	int position=0;
	int size = 2*(Steps+1)*sizeof(float)+2*sizeof(unsigned long long)+sizeof(long);
	char* buff = new char[size];
	
	char buffer[28];//3个unsigned long long，一个long
	unsigned long long RegionIndex,BoyBsvalue,GirlBsvalue;
	long BSLength;
	mListNode *Boy=NULL,*Girl=NULL;

	int flag=0;

	//wh20080403,统计进货数、出货数，以及货物占据空间
	long GoodsIn=0,GoodsOut=0;
	CString unit;//容量的单位

	while(1)
	{
		//接收到计算节点发送来的根节点序列信息，组装成链表，不同的消息靠tag区分
		//如果没有对应消息可以接收，该消息立即返回
		MPI_Iprobe(MPI_ANY_SOURCE,10,MPI_COMM_WORLD,&flag,&Status);
		if(flag == true)
		{
			MPI_Recv(buff,size,MPI_PACKED,Status.MPI_SOURCE,10,MPI_COMM_WORLD,&Status);

			mListNode *p = new mListNode();
			p->Q = new float[Steps+1];
			p->S = new float[Steps+1];//wh,现在传递的是水和沙的信息，随着多模型的发展，中转进程传递的信息也将随之变化。

			//将接收到的消息解包
			MPI_Unpack(buff,size,&position,&(p->RegionIndex),1,MPI_UNSIGNED_LONG_LONG,MPI_COMM_WORLD);
			MPI_Unpack(buff,size,&position,&(p->BSValue),1,MPI_UNSIGNED_LONG_LONG,MPI_COMM_WORLD);
			MPI_Unpack(buff,size,&position,&(p->BSLength),1,MPI_LONG,MPI_COMM_WORLD);
			MPI_Unpack(buff,size,&position,p->Q,Steps+1,MPI_FLOAT,MPI_COMM_WORLD);
			MPI_Unpack(buff,size,&position,p->S,Steps+1,MPI_FLOAT,MPI_COMM_WORLD);

			position = 0;
			flag = false;

			p->link = first->link;//每个新接收到的货物放到first之后
			first->link = p;

			//唯一进货的地方,最后会额外收到全局的根节点
			cout<<"#"<<processor_name<<","<<rank<<",IMPORT,1:"<<p->RegionIndex<<","<<p->BSValue<<","<<p->BSLength<<endl;

			GoodsIn++;
			cout<<"#"<<processor_name<<","<<rank<<",GOODSIN,"<<GoodsIn<<":"<<endl;//20080403

			capacity = (GoodsIn-GoodsOut)*size;//B
			unit = this->GetSize();//容量单位，是B，KB，MB...
			cout<<"#"<<processor_name<<","<<rank<<",CAPACITY,"<<capacity<<unit<<":"<<endl;//20080403

		}


		//接收到需要两个子节点信息序列的消息，两个子节点因为任务划分产生的分离，boy和girl都为-1
		MPI_Iprobe(MPI_ANY_SOURCE,11,MPI_COMM_WORLD,&flag,&Status);
		if(flag == true)
		{
			//接收到计算节点取货的消息，并将取货标签解包
			MPI_Recv(buffer,28,MPI_PACKED,Status.MPI_SOURCE,11,MPI_COMM_WORLD,&Status);

			MPI_Unpack(buffer,28,&position,&RegionIndex,1,MPI_UNSIGNED_LONG_LONG,MPI_COMM_WORLD);
			MPI_Unpack(buffer,28,&position,&BoyBsvalue,1,MPI_UNSIGNED_LONG_LONG,MPI_COMM_WORLD);
			MPI_Unpack(buffer,28,&position,&GirlBsvalue,1,MPI_UNSIGNED_LONG_LONG,MPI_COMM_WORLD);
			MPI_Unpack(buffer,28,&position,&BSLength,1,MPI_LONG,MPI_COMM_WORLD);

			Boy  = FindNode(RegionIndex,BoyBsvalue,BSLength);
			Girl = FindNode(RegionIndex,GirlBsvalue,BSLength);

			//wh,2008.3.25,如果认为存在某个河段只有一个子节点的时候，当该子节点恰好因为分任务产生了分离，那么在
			//slaveprocess中也会显示为boy和girl都为-1，此时如果boy为null，那girl一定不为null，因为这个河段的S分级
			//大于1，如果girl为null，那么boy一定不为null，其实只要考虑一中情况即可，因为boy节点的bsvalue为偶数，
			//girl节点的为奇数
			if(NULL==Boy && NULL==Girl)
			{
				//wh,这种情况应该是不可能的，除非有某条河段的S分级大于1，但是没有上游河段，为了中途不停，直接发送全0数据。
				cout<<"没有找到相应<双货>："<<RegionIndex<<","<<BoyBsvalue<<","<<BSLength<<" and "<<RegionIndex<<","<<GirlBsvalue<<","<<BSLength<<","<<"tag=11"<<endl;
				
				float* zeroQ = new float[Steps+1];//水量序列//中转没有子节点的情况
				float* zeroS = new float[Steps+1];//沙量序列//中转没有子节点的情况
				ZeroFill(zeroQ,Steps+1);
				ZeroFill(zeroS,Steps+1);

				MPI_Send(zeroQ,Steps+1,MPI_FLOAT,Status.MPI_SOURCE,11,MPI_COMM_WORLD);
				MPI_Send(zeroS,Steps+1,MPI_FLOAT,Status.MPI_SOURCE,11,MPI_COMM_WORLD);

				delete[] zeroQ;
				delete[] zeroS;
			}

			else if(NULL==Boy && NULL!=Girl)
			{
				MPI_Send(Girl->Q,Steps+1,MPI_FLOAT,Status.MPI_SOURCE,11,MPI_COMM_WORLD);
				MPI_Send(Girl->S,Steps+1,MPI_FLOAT,Status.MPI_SOURCE,11,MPI_COMM_WORLD);

				cout<<"#"<<processor_name<<","<<rank<<",EXPORT,1:"<<RegionIndex<<","<<GirlBsvalue<<","<<BSLength<<endl;
				GoodsOut++;
			}

			else if(NULL==Girl && NULL!=Boy)
			{
				MPI_Send(Boy->Q,Steps+1,MPI_FLOAT,Status.MPI_SOURCE,11,MPI_COMM_WORLD);
				MPI_Send(Boy->S,Steps+1,MPI_FLOAT,Status.MPI_SOURCE,11,MPI_COMM_WORLD);	

				cout<<"#"<<processor_name<<","<<rank<<",EXPORT,1:"<<RegionIndex<<","<<BoyBsvalue<<","<<BSLength<<endl;
				GoodsOut++;
			}

			else
			{
				float* temp = new float[Steps+1];

				//将boy和girl水量求和后发送给计算进程
				for(int k=0;k<Steps+1;k++){ temp[k] = Boy->Q[k] + Girl->Q[k];}
				MPI_Send(temp,Steps+1,MPI_FLOAT,Status.MPI_SOURCE,11,MPI_COMM_WORLD);

				//将boy和girl沙量求和后发送给计算进程
				for(int k=0;k<Steps+1;k++){ temp[k] = Boy->S[k] + Girl->S[k];}
				MPI_Send(temp,Steps+1,MPI_FLOAT,Status.MPI_SOURCE,11,MPI_COMM_WORLD);

				delete[] temp;

				cout<<"#"<<processor_name<<","<<rank<<",EXPORT,1:"<<RegionIndex<<","<<BoyBsvalue<<","<<BSLength<<endl;
				cout<<"#"<<processor_name<<","<<rank<<",EXPORT,1:"<<RegionIndex<<","<<GirlBsvalue<<","<<BSLength<<endl;
				GoodsOut += 2;
			}

			if(NULL!=Boy)  RemoveNode(Boy);//将节点从链表中删除
			if(NULL!=Girl) RemoveNode(Girl);

			position = 0;
			flag = false;	

			if(NULL!=Boy || NULL!=Girl)
			{
				cout<<"#"<<processor_name<<","<<rank<<",GOODSOUT,"<<GoodsOut<<":"<<endl;//20080403

				capacity = (GoodsIn-GoodsOut)*size;//B
				unit = this->GetSize();//容量单位，是B，KB，MB...
				cout<<"#"<<processor_name<<","<<rank<<",CAPACITY,"<<capacity<<unit<<":"<<endl;//20080403
			}
		}


		//如果要取的货物是RegionIndex的头节点，因为此时就该头节点一个孩子，所以单独拿出来，对应于boy=girl=-1的一种情况
		MPI_Iprobe(MPI_ANY_SOURCE,12,MPI_COMM_WORLD,&flag,&Status);
		if(flag == true)
		{
			MPI_Recv(&RegionIndex,1,MPI_UNSIGNED_LONG_LONG,Status.MPI_SOURCE,12,MPI_COMM_WORLD,&Status);
			
			Boy  = FindNode(RegionIndex,0,1);//bsvalue一定是0，bslength一定是1

			//assert(NULL!=Boy);
			MPI_Send(Boy->Q,Steps+1,MPI_FLOAT,Status.MPI_SOURCE,12,MPI_COMM_WORLD);
			MPI_Send(Boy->S,Steps+1,MPI_FLOAT,Status.MPI_SOURCE,12,MPI_COMM_WORLD);

			RemoveNode(Boy);
			flag = false;

			cout<<"#"<<processor_name<<","<<rank<<",EXPORT,1:"<<RegionIndex<<","<<"0"<<","<<"1"<<endl;
			
			GoodsOut++;
			cout<<"#"<<processor_name<<","<<rank<<",GOODSOUT,"<<GoodsOut<<":"<<endl;//20080403

			capacity = (GoodsIn-GoodsOut)*size;//B
			unit = this->GetSize();//容量单位，是B，KB，MB...
			cout<<"#"<<processor_name<<","<<rank<<",CAPACITY,"<<capacity<<unit<<":"<<endl;//20080403

		}


		//接收到需要单一子节点信息序列的消息
		MPI_Iprobe(MPI_ANY_SOURCE,13,MPI_COMM_WORLD,&flag,&Status);
		if(flag == true)
		{
			MPI_Recv(buffer,28,MPI_PACKED,Status.MPI_SOURCE,13,MPI_COMM_WORLD,&Status);
			
			MPI_Unpack(buffer,28,&position,&RegionIndex,1,MPI_UNSIGNED_LONG_LONG,MPI_COMM_WORLD);
			MPI_Unpack(buffer,28,&position,&BoyBsvalue,1,MPI_UNSIGNED_LONG_LONG,MPI_COMM_WORLD);
			MPI_Unpack(buffer,28,&position,&BSLength,1,MPI_LONG,MPI_COMM_WORLD);

			Boy  = FindNode(RegionIndex,BoyBsvalue,BSLength);

			//wh,2008.3.25，存在某个河段就有一个子节点，此时也对应着boy!=-1而girl=-1，如果不加下面的if判断，boy为NULL，自然Boy->Q无法发送
			if(NULL == Boy)
			{
				//cout<<"没有找到相应<单货>:"<<RegionIndex<<","<<BoyBsvalue<<","<<BSLength<<","<<"tag=13"<<endl;

				float* zeroQ = new float[Steps+1];//水量序列//中转没有子节点的情况
				float* zeroS = new float[Steps+1];//沙量序列//中转没有子节点的情况
				ZeroFill(zeroQ,Steps+1);
				ZeroFill(zeroS,Steps+1);

				MPI_Send(zeroQ,Steps+1,MPI_FLOAT,Status.MPI_SOURCE,13,MPI_COMM_WORLD);
				MPI_Send(zeroS,Steps+1,MPI_FLOAT,Status.MPI_SOURCE,13,MPI_COMM_WORLD);

				delete[] zeroQ;
				delete[] zeroS;
			}

			else
			{
				MPI_Send(Boy->Q,Steps+1,MPI_FLOAT,Status.MPI_SOURCE,13,MPI_COMM_WORLD);
				MPI_Send(Boy->S,Steps+1,MPI_FLOAT,Status.MPI_SOURCE,13,MPI_COMM_WORLD);

				RemoveNode(Boy);

				cout<<"#"<<processor_name<<","<<rank<<",EXPORT,1:"<<RegionIndex<<","<<BoyBsvalue<<","<<BSLength<<endl;

				GoodsOut++;
				cout<<"#"<<processor_name<<","<<rank<<",GOODSOUT,"<<GoodsOut<<":"<<endl;//20080403

				capacity = (GoodsIn-GoodsOut)*size;//B
				unit = this->GetSize();//容量单位，是B，KB，MB...
				cout<<"#"<<processor_name<<","<<rank<<",CAPACITY,"<<capacity<<unit<<":"<<endl;//20080403
			}

			position = 0;
			flag = false;

		}


		//接收到进程0发来的退出的消息
		MPI_Iprobe(0,14,MPI_COMM_WORLD,&flag,&Status);
		if(flag == true)
		{
            MPI_Recv(buffer,28,MPI_INT,0,14,MPI_COMM_WORLD,&Status);//接到进程0要求退出的消息
			
			if(NULL != first->link)//最后全流域的根节点信息会发送到中转进程，但是所有已经计算完了，所以这里就将其卸载了
			{
				cout<<"#"<<processor_name<<","<<rank<<",EXPORT,1:"<<first->link->RegionIndex<<","<<first->link->BSValue<<","<<first->link->BSLength<<endl;

				GoodsOut++;
				cout<<"#"<<processor_name<<","<<rank<<",GOODSOUT,"<<GoodsOut<<":"<<endl;//20080403

				capacity = (GoodsIn-GoodsOut)*size;//B
				unit = this->GetSize();//容量单位，是B，KB，MB...
				cout<<"#"<<processor_name<<","<<rank<<",CAPACITY,"<<capacity<<unit<<":"<<endl;//20080403

				RemoveNode(first->link);
			}

			break;
		}
	
	}//end while
	delete[] buff;
}


//根据货物标识，找到计算节点需要的货物
mListNode* TransferProcess::FindNode(unsigned long long regionindex, unsigned long long value, long length)
{
	mListNode* p; 
	p = first;
	while(p->link) 
	{
		p = p->link;
		if(length == p->BSLength && value == p->BSValue && regionindex == p->RegionIndex)
			return p;
	}
	//cout<<"中转进程没有找到货物"<<"RegionIndex:"<<regionindex<<","<<"BSValue:"<<value<<","<<"BSLength:"<<length<<endl;
	return NULL;
}


//将已经发送的货物从链表中删除
void TransferProcess::RemoveNode(mListNode* p)
{
	mListNode* q;
	q = first;
	while (q->link) 
	{
		if (q->link == p) 
		{
			q->link=p->link;

			delete[] p->Q;
			delete[] p->S;
			delete p;
			
			return;
		}
		else
			q=q->link;
	}
	//cout<<"中转进程无法删除货物"<<endl;
	return;
}
