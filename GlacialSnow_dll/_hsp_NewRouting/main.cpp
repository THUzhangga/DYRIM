#pragma once
#undef SEEK_SET
#undef SEEK_END
#undef SEEK_CUR
#include "mpi.h"
#include "CommenProcess.h"
#include "MasterProcess.h"
#include "SlaveProcess.h"
#include "TransferProcess.h"


void main(int argc, char *argv[])
{
	//wh��Ϊ�˲�������FilePath.ini���û��������롢sid����ͨ�������д���
	CString user,password,sid;

	if(argc != 4)
	{
		cout<<"NewRouting.exe needs three commandline parameter: User,Password,sid \n\
			   Program exiting now."<<endl;
		exit(-1);
	}
	else
	{
		user = argv[1];
		password = argv[2];
		sid = argv[3];
	}
	//user="hydro";password="hydro";sid="hydro";
	
	//ADO��com,��Ҫ��ʼ������
	::CoInitialize(NULL);
	
	int rank;
	MPI_Init(&argc,&argv);
	MPI_Comm_rank(MPI_COMM_WORLD,&rank);
	//cout<<"The rank of process is:"<<rank<<endl;

	int WorldSize;
	MPI_Comm_size(MPI_COMM_WORLD,&WorldSize);

	if(rank == 0)//���ؽ���
	{
		MasterProcess MP;

		//���������ǹ����������������Ҳͬ��ִ��
		MP.ProcessInitialize(WorldSize,rank,user,password,sid);//����MPI��һЩ����
		MP.ReadHydroUsePara();
		MP.ReadRegionConnection();//��RegionConnection���ȡ��Ϣ

		MP.ReadBSCode();//���벻ͬ��RegionIndex,BSValue,BSLength��Ϣ
		MP.ReadFromParameter();//��parameter��������
		MP.FormForest();//��Oracle�н��������ڴ�
		MP.DispatchTask();//������������̷�������
		MP.RecvSlaveProcess();//ѭ���ȴ������������̷���������Ϣ
	}
	
	else if(rank == (WorldSize-1))//���һ��������Ϊ��ת���̣�����Դ����Ķ��Ƚ�С
	{
		TransferProcess TP;
		TP.ProcessInitialize(WorldSize,rank,user,password,sid);
		TP.ReadHydroUsePara();
		TP.TransferMain();

		cout<<"***Ready to Bye! Thank you for using TUD-Basin.***"<<endl;
	}
	
	else//�������
	{
		SlaveProcess SP;

		SP.ProcessInitialize(WorldSize,rank,user,password,sid);
		SP.ReadHydroUsePara();
		SP.ReadRegionConnection();

		SP.SlaveProcessInitialize();//������ʼ����
		SP.RecvMasterProcess();//�������ؽ��̷�����������
	}

	//cout<<"Ready to Bye!"<<"  rank="<< rank<<"  Thank you for using DWM."<<endl;
	
	MPI_Finalize();
	exit(0);//wh,���ϴ˾䣬����newrouting.exe���������������ڽ��̹������л����ڡ�
};
