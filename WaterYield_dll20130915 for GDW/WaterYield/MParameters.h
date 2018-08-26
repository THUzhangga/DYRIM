#ifndef PARAMETERS_H
#define PARAMETERS_H

#pragma once
#include "stdafx.h"
#include "datastruct.h"

//2008,�Դ���idl�и����Զ���ָ��Ĵ��ݣ�������־Ͳ���ʹ��Parameters��������벻ͨ����������ʾ��Ӧ���Ǹ�ADO�е�xxx�����ˣ����Զ��ĳ���MParameters��
class MParameters
{
public:

	ADODB::_ConnectionPtr pTabFileCnn;//���Ӷ���

	//20070122,xiaofc,��ΪҪ��ζ�����tvarparameter�ļ�¼����������˽����Ϊ���ñ���
	ADODB::_RecordsetPtr pStatRst;

	//MParameters ����ʱ��ı�Ĳ���
	float RouteLength;
	float Roughnessn;
	float SlopeJ;
	float Area;
	//��ɳ����
	float SediD;
	
	float PKV0;//ÿСʱ��������ˮ����������������
	float PKV1;//��������в���֮��Ĵ�ֱ��͸ϵ��
	float PKV2;//�в����������֮��Ĵ�ֱ��͸ϵ��addbyiwish20060220

	float PKH1; //Ǳˮ������ˮƽ��͸ϵ��
	float PKH2;

	float USita1; //Ǳˮ���϶�ʵ�����ˮ����
	float USita2; //Ǳˮ���϶�ʵ�����ˮ����
	float MSita1;
	float MSita2;
	float DSita1;	//�������϶�ʵ�����ˮ����
	float DSita2;  //�������϶�ʵ�����ˮ����
	
	float UDepth;
	float DSDepth;  //���������
	float ErosionK;//20051219 Ѧ�� �����ɳ��ʽ�е�k
	float ErosionBeta;

	//David �����ɳģ�Ͳ���
	float ErosionK1;
	float ErosionK2;
	float ErosionBeta1;
	float ErosionBeta2;

	//addbyiwish 20060222
	float au,am,ad;//�����ƹ�ʽ��ϵ����
	float bu,bm,bd;//�����ƹ�ʽ��ָ����

	//status ��ʱ��仯��״̬
	float SoilW;//Ǳˮ���ʼˮ��
	float MidSoilW;
	float DSW; //������ĳ�ʼˮ��
	float LeafW; //Ҷ���ˮ�m
	float LAI,WaterEVA,EPI;

	//���㺬ˮ��
	//һ��һ���������ŵ�status�������ֲ��״��Ҳ����д
	//L��R��S�����ң�Դ��U,M,D:���У���
	float WLU,WLM,WLD,WRU,WRM,WRD,WSU,WSM,WSD;
	float WLL,WRL,WSL;
	long lHourStart;//������ʼ��ʱ��

	//20070122,xiaofc,Ϊ��֤��LAI��E0����ȷ�ԣ������������±���
	short YearStart;
	short MonthStart;
	short YearEnd;
	short MonthEnd;

	//altered by weihong
	float UpInitWaterContent;
	float MidInitWaterContent;
	float DownInitWaterContent;
	CString sccd;//wh,2008.3.24

	_variant_t vtmp;//wh

	//altered by weihong
	MParameters(float upw, float midw, float downw,CString Sccd){
		UpInitWaterContent = upw;
		MidInitWaterContent = midw;
		DownInitWaterContent = downw;
		sccd = Sccd;
	}

public:
	MParameters(void);
	~MParameters(void);
	bool GetParameters(struct Para *pPara,char Position,BSCode mBSCode);
	// ��״̬�����ļ�
	bool SaveStatusFile(void);
	//�����㺬ˮ��
	bool GetWaterInSoil(BSCode mBSCode,Para myPara);
	//��LAI��EPI
	//20070122,xiaofc������һ��LAI,EPI��¼
	void ReadLE(BSCode mBSCode);
	//20070122,xiaofc,�������仯��LAI,EPI��¼
	void ReadLE(BSCode mBSCode,short Year,short Month);

};
#endif
