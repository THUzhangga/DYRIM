#pragma once
#include "MParameters.h"

class Leaf
{
private:
	float I0; //����������������m^3
	float Area;//���������
	float LAI; //Leaf Area Index
public:
	float W; //��ˮ��,m
	bool isDebug;

public:
	Leaf(void);
	~Leaf(void);
	float NextHour(float WaterAdded); //��������꣬ˮ�m������SurfaceWater
	bool initialize(Para *,MParameters *,char); //��������ǰ����������ͨ��MParameters��õ���������ֱ�Ӵ�MetaBasins����ȥȡ��
};
