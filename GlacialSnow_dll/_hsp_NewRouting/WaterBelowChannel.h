#ifndef WATERBELOWCHANNEL_H
#define WATERBELOWCHANNEL_H
#pragma once

//�����µĵ���ˮ�˶�
class WaterBelowChannel
{
public:
	WaterBelowChannel(void);
public:
	~WaterBelowChannel(void);

public:
	//��T0TΪ��׼��Ĳ�ѹ��ˮͷ(M,N��)
	float H,W;

public:
	//����dW�����������²��֣�����������ˮ��.
	float WtoH(float dW);
	
	//����ˮλΪH0ʱ����KL֮���ˮ��m3
	float HtoW(float H0);

public:
	//HH����H�㣬��Ϊ������ˮ��H������������������ΪHH��
	//B��C���е�����,���������ж�G��H�·���Ӧ����Բ���λ���ֱ�߶�
	//A��D�ֱ�λ������ֱ���ϣ���������ȷ��ֱ�߷���
	float G[2],HH[2],E[2],F[2],K[2],L[2],Center[2],B[2],C[2],A[2],D[2];
	
	//R����ʴ���뾶��Length:�ӵ���
	float R,Length;

	//��ǰ�Ӷ��ܴ洢�����ˮ��
	float Wmax;

};

#endif