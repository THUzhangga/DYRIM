#pragma once
#include <stdio.h>
#include <iostream>
#include "DataStruct.h"
using namespace std;

List::List()
{
	first =new ListNode(); 
	first->BSLength = -1;
	first->BSValue = -1;
	first->SOrder=-1;
	first->tmp=-1;
	first->link = NULL;
}

int List::find(long length,unsigned long long value)//找节点，返回非零值SOrder
{
	if(first==NULL)
		return 0;
	
	ListNode *p = new ListNode(); 
	p = first;
	while(p->link) 
	{
		p=p->link;
		if(length == p->BSLength && value == p->BSValue)
			return p->SOrder;
	}
	return 0;
}

int List::insert(long length,unsigned long long value,long SOrder)
{
	ListNode *p = new ListNode();
	p->BSLength = length;
	p->BSValue = value;
	p->SOrder=SOrder;
	p->link = first->link;//第一次运行时，first->link为NULL，见构造函数，即头节点链接到NULL
	first->link = p;
	return 1;
}

int List::insert(long length,unsigned long long value,long SOrder,long tmp)
{
	ListNode *p = new ListNode();
	p->BSLength = length;
	p->BSValue = value;
	p->SOrder=SOrder;
	p->tmp=tmp;
	p->link = first->link;
	first->link = p;
	return 1;
}

int List::remove(ListNode *p)
{
	ListNode *q;
	q=first;
	while (q->link) 
	{
		if (q->link == p) 
		{
			q->link=p->link;
			delete p;
			p=q->link;
			return 1;
		}
		else
			q=q->link;
	}
	return 0;
}

ListNode *List::N_find(long length,unsigned long long  value)
{
	ListNode *p; 
	p = first;
	while(p->link) 
	{
		p=p->link;
		if(length == p->BSLength && value == p->BSValue)
			return p;
	}
	return NULL;
}

ListNode *List::N_find(int tmp)
{
	ListNode *p; 
	p = first;
	while(p->link) 
	{
		p=p->link;
		if(tmp == p->tmp)
			return p;
	}
	return NULL;
}

void List::free(void)
{
	ListNode * ln;
	while(first)
	{
		ln=first;
		first=first->link;
		delete ln;
	}
}

