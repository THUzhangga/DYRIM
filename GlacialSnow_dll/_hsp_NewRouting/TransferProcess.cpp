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

//wh,20080403,Ϊ�˵õ���ת�����Ѿ����˶��ٻ���������ѻ�û�����죬ʱ�䳤�˻�����ġ�
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
		capacity = float( int( (capacity *= 10) + 0.5 ) ) / 10;//Ϊ�˱���һλС��
		return "KB";
	}
	else if(capacity>=1024*1024 && capacity<pow(1024.0,3.0) )
	{
		capacity = capacity/1024/1024;
		capacity = float( int( (capacity *= 10) + 0.5 ) ) / 10;//Ϊ�˱���һλС��
		return "MB";
	}
	else if(capacity>=pow(1024.0,3.0) && capacity<pow(1024.0,4.0))
	{
		capacity = capacity/pow(1024.0,3.0);
		capacity = float( int( (capacity *= 100 ) + 0.5 ) ) / 100;//Ϊ�˱�����λС��
		return "GB";
	}
	else
		cout<<"��ת����̫�������޷����ܣ�"<<endl;

}


//wh����ת���̵���ִ�к����������������ȡ��ʱ�ܱ�֤����һ�����ڡ���Ϊ�ڱ�֤���κӶμ�����ɣ�ͷ�ڵ���Ϣ���ͳɹ��󣬽���0�Ž�һ���и�������
void TransferProcess::TransferMain(void)
{
	//position,size,buff:
	int position=0;
	int size = 2*(Steps+1)*sizeof(float)+2*sizeof(unsigned long long)+sizeof(long);
	char* buff = new char[size];
	
	char buffer[28];//3��unsigned long long��һ��long
	unsigned long long RegionIndex,BoyBsvalue,GirlBsvalue;
	long BSLength;
	mListNode *Boy=NULL,*Girl=NULL;

	int flag=0;

	//wh20080403,ͳ�ƽ����������������Լ�����ռ�ݿռ�
	long GoodsIn=0,GoodsOut=0;
	CString unit;//�����ĵ�λ

	while(1)
	{
		//���յ�����ڵ㷢�����ĸ��ڵ�������Ϣ����װ��������ͬ����Ϣ��tag����
		//���û�ж�Ӧ��Ϣ���Խ��գ�����Ϣ��������
		MPI_Iprobe(MPI_ANY_SOURCE,10,MPI_COMM_WORLD,&flag,&Status);
		if(flag == true)
		{
			MPI_Recv(buff,size,MPI_PACKED,Status.MPI_SOURCE,10,MPI_COMM_WORLD,&Status);

			mListNode *p = new mListNode();
			p->Q = new float[Steps+1];
			p->S = new float[Steps+1];//wh,���ڴ��ݵ���ˮ��ɳ����Ϣ�����Ŷ�ģ�͵ķ�չ����ת���̴��ݵ���ϢҲ����֮�仯��

			//�����յ�����Ϣ���
			MPI_Unpack(buff,size,&position,&(p->RegionIndex),1,MPI_UNSIGNED_LONG_LONG,MPI_COMM_WORLD);
			MPI_Unpack(buff,size,&position,&(p->BSValue),1,MPI_UNSIGNED_LONG_LONG,MPI_COMM_WORLD);
			MPI_Unpack(buff,size,&position,&(p->BSLength),1,MPI_LONG,MPI_COMM_WORLD);
			MPI_Unpack(buff,size,&position,p->Q,Steps+1,MPI_FLOAT,MPI_COMM_WORLD);
			MPI_Unpack(buff,size,&position,p->S,Steps+1,MPI_FLOAT,MPI_COMM_WORLD);

			position = 0;
			flag = false;

			p->link = first->link;//ÿ���½��յ��Ļ���ŵ�first֮��
			first->link = p;

			//Ψһ�����ĵط�,��������յ�ȫ�ֵĸ��ڵ�
			cout<<"#"<<processor_name<<","<<rank<<",IMPORT,1:"<<p->RegionIndex<<","<<p->BSValue<<","<<p->BSLength<<endl;

			GoodsIn++;
			cout<<"#"<<processor_name<<","<<rank<<",GOODSIN,"<<GoodsIn<<":"<<endl;//20080403

			capacity = (GoodsIn-GoodsOut)*size;//B
			unit = this->GetSize();//������λ����B��KB��MB...
			cout<<"#"<<processor_name<<","<<rank<<",CAPACITY,"<<capacity<<unit<<":"<<endl;//20080403

		}


		//���յ���Ҫ�����ӽڵ���Ϣ���е���Ϣ�������ӽڵ���Ϊ���񻮷ֲ����ķ��룬boy��girl��Ϊ-1
		MPI_Iprobe(MPI_ANY_SOURCE,11,MPI_COMM_WORLD,&flag,&Status);
		if(flag == true)
		{
			//���յ�����ڵ�ȡ������Ϣ������ȡ����ǩ���
			MPI_Recv(buffer,28,MPI_PACKED,Status.MPI_SOURCE,11,MPI_COMM_WORLD,&Status);

			MPI_Unpack(buffer,28,&position,&RegionIndex,1,MPI_UNSIGNED_LONG_LONG,MPI_COMM_WORLD);
			MPI_Unpack(buffer,28,&position,&BoyBsvalue,1,MPI_UNSIGNED_LONG_LONG,MPI_COMM_WORLD);
			MPI_Unpack(buffer,28,&position,&GirlBsvalue,1,MPI_UNSIGNED_LONG_LONG,MPI_COMM_WORLD);
			MPI_Unpack(buffer,28,&position,&BSLength,1,MPI_LONG,MPI_COMM_WORLD);

			Boy  = FindNode(RegionIndex,BoyBsvalue,BSLength);
			Girl = FindNode(RegionIndex,GirlBsvalue,BSLength);

			//wh,2008.3.25,�����Ϊ����ĳ���Ӷ�ֻ��һ���ӽڵ��ʱ�򣬵����ӽڵ�ǡ����Ϊ����������˷��룬��ô��
			//slaveprocess��Ҳ����ʾΪboy��girl��Ϊ-1����ʱ���boyΪnull����girlһ����Ϊnull����Ϊ����Ӷε�S�ּ�
			//����1�����girlΪnull����ôboyһ����Ϊnull����ʵֻҪ����һ��������ɣ���Ϊboy�ڵ��bsvalueΪż����
			//girl�ڵ��Ϊ����
			if(NULL==Boy && NULL==Girl)
			{
				//wh,�������Ӧ���ǲ����ܵģ�������ĳ���Ӷε�S�ּ�����1������û�����κӶΣ�Ϊ����;��ͣ��ֱ�ӷ���ȫ0���ݡ�
				cout<<"û���ҵ���Ӧ<˫��>��"<<RegionIndex<<","<<BoyBsvalue<<","<<BSLength<<" and "<<RegionIndex<<","<<GirlBsvalue<<","<<BSLength<<","<<"tag=11"<<endl;
				
				float* zeroQ = new float[Steps+1];//ˮ������//��תû���ӽڵ�����
				float* zeroS = new float[Steps+1];//ɳ������//��תû���ӽڵ�����
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

				//��boy��girlˮ����ͺ��͸��������
				for(int k=0;k<Steps+1;k++){ temp[k] = Boy->Q[k] + Girl->Q[k];}
				MPI_Send(temp,Steps+1,MPI_FLOAT,Status.MPI_SOURCE,11,MPI_COMM_WORLD);

				//��boy��girlɳ����ͺ��͸��������
				for(int k=0;k<Steps+1;k++){ temp[k] = Boy->S[k] + Girl->S[k];}
				MPI_Send(temp,Steps+1,MPI_FLOAT,Status.MPI_SOURCE,11,MPI_COMM_WORLD);

				delete[] temp;

				cout<<"#"<<processor_name<<","<<rank<<",EXPORT,1:"<<RegionIndex<<","<<BoyBsvalue<<","<<BSLength<<endl;
				cout<<"#"<<processor_name<<","<<rank<<",EXPORT,1:"<<RegionIndex<<","<<GirlBsvalue<<","<<BSLength<<endl;
				GoodsOut += 2;
			}

			if(NULL!=Boy)  RemoveNode(Boy);//���ڵ��������ɾ��
			if(NULL!=Girl) RemoveNode(Girl);

			position = 0;
			flag = false;	

			if(NULL!=Boy || NULL!=Girl)
			{
				cout<<"#"<<processor_name<<","<<rank<<",GOODSOUT,"<<GoodsOut<<":"<<endl;//20080403

				capacity = (GoodsIn-GoodsOut)*size;//B
				unit = this->GetSize();//������λ����B��KB��MB...
				cout<<"#"<<processor_name<<","<<rank<<",CAPACITY,"<<capacity<<unit<<":"<<endl;//20080403
			}
		}


		//���Ҫȡ�Ļ�����RegionIndex��ͷ�ڵ㣬��Ϊ��ʱ�͸�ͷ�ڵ�һ�����ӣ����Ե����ó�������Ӧ��boy=girl=-1��һ�����
		MPI_Iprobe(MPI_ANY_SOURCE,12,MPI_COMM_WORLD,&flag,&Status);
		if(flag == true)
		{
			MPI_Recv(&RegionIndex,1,MPI_UNSIGNED_LONG_LONG,Status.MPI_SOURCE,12,MPI_COMM_WORLD,&Status);
			
			Boy  = FindNode(RegionIndex,0,1);//bsvalueһ����0��bslengthһ����1

			//assert(NULL!=Boy);
			MPI_Send(Boy->Q,Steps+1,MPI_FLOAT,Status.MPI_SOURCE,12,MPI_COMM_WORLD);
			MPI_Send(Boy->S,Steps+1,MPI_FLOAT,Status.MPI_SOURCE,12,MPI_COMM_WORLD);

			RemoveNode(Boy);
			flag = false;

			cout<<"#"<<processor_name<<","<<rank<<",EXPORT,1:"<<RegionIndex<<","<<"0"<<","<<"1"<<endl;
			
			GoodsOut++;
			cout<<"#"<<processor_name<<","<<rank<<",GOODSOUT,"<<GoodsOut<<":"<<endl;//20080403

			capacity = (GoodsIn-GoodsOut)*size;//B
			unit = this->GetSize();//������λ����B��KB��MB...
			cout<<"#"<<processor_name<<","<<rank<<",CAPACITY,"<<capacity<<unit<<":"<<endl;//20080403

		}


		//���յ���Ҫ��һ�ӽڵ���Ϣ���е���Ϣ
		MPI_Iprobe(MPI_ANY_SOURCE,13,MPI_COMM_WORLD,&flag,&Status);
		if(flag == true)
		{
			MPI_Recv(buffer,28,MPI_PACKED,Status.MPI_SOURCE,13,MPI_COMM_WORLD,&Status);
			
			MPI_Unpack(buffer,28,&position,&RegionIndex,1,MPI_UNSIGNED_LONG_LONG,MPI_COMM_WORLD);
			MPI_Unpack(buffer,28,&position,&BoyBsvalue,1,MPI_UNSIGNED_LONG_LONG,MPI_COMM_WORLD);
			MPI_Unpack(buffer,28,&position,&BSLength,1,MPI_LONG,MPI_COMM_WORLD);

			Boy  = FindNode(RegionIndex,BoyBsvalue,BSLength);

			//wh,2008.3.25������ĳ���Ӷξ���һ���ӽڵ㣬��ʱҲ��Ӧ��boy!=-1��girl=-1��������������if�жϣ�boyΪNULL����ȻBoy->Q�޷�����
			if(NULL == Boy)
			{
				//cout<<"û���ҵ���Ӧ<����>:"<<RegionIndex<<","<<BoyBsvalue<<","<<BSLength<<","<<"tag=13"<<endl;

				float* zeroQ = new float[Steps+1];//ˮ������//��תû���ӽڵ�����
				float* zeroS = new float[Steps+1];//ɳ������//��תû���ӽڵ�����
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
				unit = this->GetSize();//������λ����B��KB��MB...
				cout<<"#"<<processor_name<<","<<rank<<",CAPACITY,"<<capacity<<unit<<":"<<endl;//20080403
			}

			position = 0;
			flag = false;

		}


		//���յ�����0�������˳�����Ϣ
		MPI_Iprobe(0,14,MPI_COMM_WORLD,&flag,&Status);
		if(flag == true)
		{
            MPI_Recv(buffer,28,MPI_INT,0,14,MPI_COMM_WORLD,&Status);//�ӵ�����0Ҫ���˳�����Ϣ
			
			if(NULL != first->link)//���ȫ����ĸ��ڵ���Ϣ�ᷢ�͵���ת���̣����������Ѿ��������ˣ���������ͽ���ж����
			{
				cout<<"#"<<processor_name<<","<<rank<<",EXPORT,1:"<<first->link->RegionIndex<<","<<first->link->BSValue<<","<<first->link->BSLength<<endl;

				GoodsOut++;
				cout<<"#"<<processor_name<<","<<rank<<",GOODSOUT,"<<GoodsOut<<":"<<endl;//20080403

				capacity = (GoodsIn-GoodsOut)*size;//B
				unit = this->GetSize();//������λ����B��KB��MB...
				cout<<"#"<<processor_name<<","<<rank<<",CAPACITY,"<<capacity<<unit<<":"<<endl;//20080403

				RemoveNode(first->link);
			}

			break;
		}
	
	}//end while
	delete[] buff;
}


//���ݻ����ʶ���ҵ�����ڵ���Ҫ�Ļ���
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
	//cout<<"��ת����û���ҵ�����"<<"RegionIndex:"<<regionindex<<","<<"BSValue:"<<value<<","<<"BSLength:"<<length<<endl;
	return NULL;
}


//���Ѿ����͵Ļ����������ɾ��
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
	//cout<<"��ת�����޷�ɾ������"<<endl;
	return;
}
