#ifndef TRANSFERPROCESS_H
#define TRANSFERPROCESS_H
#pragma once
#include "CommenProcess.h"

//wh,added,2008.3.20,中转进程，用来减小原来数据库中discharge表的负担，否则数据库空间很快就满了。
//wh，经过实际检测，进的货很快就会提走，中转进程不会内存溢出，即使可能出现溢出情况，可以增大计算粒度和时间步长，减小通讯量。
class TransferProcess;

//中转站中的货物，用链表串起来
class mListNode
{
	friend class TransferProcess;
public:
	mListNode* link;
	unsigned long long RegionIndex;
	unsigned long long BSValue;
	long BSLength;
	float* Q;//水量序列
	float* S;//沙量序列
	//wh,2008.4.4
	//随着模型的增加，传递的信息会越来越多，将来肯定要增加"信息标识"，标识我要提取的信息种类，是水、沙、营养物、污染物等。
	//信息标识就通过basinmodel表，只要在中转进程运行前一次性配好即可。
};

//中转进程类
class TransferProcess : public CommenProcess
{
public:
	TransferProcess(void);
	~TransferProcess(void);

private:
	float capacity;//此时尚未提走的货物占据的容量
	mListNode* first;//链表的头节点，并非货物

	mListNode* FindNode(unsigned long long regionindex, unsigned long long value, long length);//根据计算进程发送来的消息在链表中找到货物
	void RemoveNode(mListNode* p);//将已发送的货物从链表删除
	CString GetSize(void);//20080403，得到当前货物占据的容量,返回容量单位

public:
	void TransferMain(void);//中转进程的主函数，所有进程都在一个通信域，中转进程号是最大进程。

};

#endif
