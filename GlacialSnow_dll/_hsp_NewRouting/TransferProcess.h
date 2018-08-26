#ifndef TRANSFERPROCESS_H
#define TRANSFERPROCESS_H
#pragma once
#include "CommenProcess.h"

//wh,added,2008.3.20,��ת���̣�������Сԭ�����ݿ���discharge��ĸ������������ݿ�ռ�ܿ�����ˡ�
//wh������ʵ�ʼ�⣬���Ļ��ܿ�ͻ����ߣ���ת���̲����ڴ��������ʹ���ܳ�������������������������Ⱥ�ʱ�䲽������СͨѶ����
class TransferProcess;

//��תվ�еĻ������������
class mListNode
{
	friend class TransferProcess;
public:
	mListNode* link;
	unsigned long long RegionIndex;
	unsigned long long BSValue;
	long BSLength;
	float* Q;//ˮ������
	float* S;//ɳ������
	//wh,2008.4.4
	//����ģ�͵����ӣ����ݵ���Ϣ��Խ��Խ�࣬�����϶�Ҫ����"��Ϣ��ʶ"����ʶ��Ҫ��ȡ����Ϣ���࣬��ˮ��ɳ��Ӫ�����Ⱦ��ȡ�
	//��Ϣ��ʶ��ͨ��basinmodel��ֻҪ����ת��������ǰһ������ü��ɡ�
};

//��ת������
class TransferProcess : public CommenProcess
{
public:
	TransferProcess(void);
	~TransferProcess(void);

private:
	float capacity;//��ʱ��δ���ߵĻ���ռ�ݵ�����
	mListNode* first;//�����ͷ�ڵ㣬���ǻ���

	mListNode* FindNode(unsigned long long regionindex, unsigned long long value, long length);//���ݼ�����̷���������Ϣ���������ҵ�����
	void RemoveNode(mListNode* p);//���ѷ��͵Ļ��������ɾ��
	CString GetSize(void);//20080403���õ���ǰ����ռ�ݵ�����,����������λ

public:
	void TransferMain(void);//��ת���̵������������н��̶���һ��ͨ������ת���̺��������̡�

};

#endif
