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
	//wh，为了不依赖于FilePath.ini，用户名、密码、sid必须通过命令行传递
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
	
	//ADO是com,需要初始化环境
	::CoInitialize(NULL);
	
	int rank;
	MPI_Init(&argc,&argv);
	MPI_Comm_rank(MPI_COMM_WORLD,&rank);
	//cout<<"The rank of process is:"<<rank<<endl;

	int WorldSize;
	MPI_Comm_size(MPI_COMM_WORLD,&WorldSize);

	if(rank == 0)//主控进程
	{
		MasterProcess MP;

		//下面三行是公共操作，计算进程也同样执行
		MP.ProcessInitialize(WorldSize,rank,user,password,sid);//配置MPI的一些参数
		MP.ReadHydroUsePara();
		MP.ReadRegionConnection();//从RegionConnection表读取信息

		MP.ReadBSCode();//读入不同的RegionIndex,BSValue,BSLength信息
		MP.ReadFromParameter();//从parameter表读入参数
		MP.FormForest();//从Oracle中将树读入内存
		MP.DispatchTask();//往各个计算进程发送子树
		MP.RecvSlaveProcess();//循环等待并处理计算进程发送来的消息
	}
	
	else if(rank == (WorldSize-1))//最后一个进程作为中转进程，这样源程序改动比较小
	{
		TransferProcess TP;
		TP.ProcessInitialize(WorldSize,rank,user,password,sid);
		TP.ReadHydroUsePara();
		TP.TransferMain();

		cout<<"***Ready to Bye! Thank you for using TUD-Basin.***"<<endl;
	}
	
	else//计算进程
	{
		SlaveProcess SP;

		SP.ProcessInitialize(WorldSize,rank,user,password,sid);
		SP.ReadHydroUsePara();
		SP.ReadRegionConnection();

		SP.SlaveProcessInitialize();//变量初始化等
		SP.RecvMasterProcess();//处理主控进程发送来的任务
	}

	//cout<<"Ready to Bye!"<<"  rank="<< rank<<"  Thank you for using DWM."<<endl;
	
	MPI_Finalize();
	exit(0);//wh,加上此句，否则newrouting.exe并不真正结束，在进程管理器中还存在。
};
