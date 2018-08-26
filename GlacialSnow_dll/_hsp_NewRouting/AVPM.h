#pragma once
#include <fstream>
#include "DataStruct.h"
using namespace std;
class AVPM
{
//public:
	//AVPM();
//public:
//	~AVPM(void);

private:
	//常数
	static const float g;
	static const float gamma;
	static const float nu;
	// 本数组存储安全系数从0.29到1.5的六种方差（0.05,0.1,0.15,0.2,0.25,0.3）的模糊概率积分值
	static const float fuzzpro[122][7]; 
	
	ofstream myFile;
	
	long DayLoop;

public:
	AVPM(void);
	AVPM(CString ssmethod,int mstep,CString sccd);//wh
	int MSTEP;//wh
	CString sccd;//wh

public:
	//计算
	float nano;//极小值，用于能坡J
	long HourStart;//20080303,xiaofc,计算开始时间
	long iTSteps;//20070611,xiaofc,疑int表示范围过小，改为long
	long lTotalT;
	int delta_t;
	long t;//时间下标

	ADODB::_ConnectionPtr pCnn;//20080303,xiaofc,数据库连接
	BSCode mBSCode; //河段编号
	float delta_x;
	bool isDebug;
	bool bCalcGravity;//是否计算重力侵蚀的标志

	//水流
	float *Q;
	float *Qout;//由调用方给这块内存
	float h[4];
	float u[4];	
	float B[4];
	float m;//B=2mh,A=mh^2
	float eta;//ph/px项在涨水期的有效系数0.8,退水期为1-eta=0.2
	
	//20070828,xiaofc,考虑小流量下变糙率
	float n;//沟道的manning系数,这个是动态的，变化之后的
	float n0;//这是从参数表里取来的
	
	float L;
	float S;

	//泥沙
	bool bIsDep;//是淤积
	float *mySin;
	float *SinL;//左
	float *SinR;//右的含沙量
	float *SinMe;//自已河段的产沙量
	float *Sout;
	float gammaS;//2650kg/m3
	float D50;//中径，m
	float alphaE;//冲刷时的恢复饱和系数 
	float alphaD;//淤积时的
	float Svm;//极限体积比含沙量，只与D50相关

	float D90; //百分比占 90％ 的颗粒粒径，m  heli
	CString SSMethod;//计算挟沙力的方法，"Zhang","Fei"

	float * Qin1;
	float * Qin2;
	float * Qin3;
	
	Para mPara;

	float *WL;
	float *WR;

	//20070622,xiaofc, flow pattern
	bool SaveFlowPattern;
	float * FlowB;
	float * FlowH;
	float * Flow_v;
	
	//20080303,xiaofc, save status for avpm
	float * ChannelErosion;
	float * GravityErosion;

public: //下面是薛海添加的重力侵蚀部分用到的变量	

	//薛 以下是重力侵蚀需要用到的部分参数
	float P2;//20060317,李铁键,一个河段的纵向距离内，发生重力侵蚀的纵向范围的概率
	vector <GravityEvent> * pGravityEvents;		//20060327,李铁键,为存储重力侵蚀信息用

	float Tao;   //沟坡水流切应力
	float Tao_c; //坡脚土体被冲刷的启动切应力，用唐存本公式
	float Delt_B; // 坡脚由于冲刷而展宽的横向
	float Angle_beta; //沟坡的初次失稳的角度
	float Angle_i;    //沟坡d然坡角，和下边的Angle_downslope相等
	float Angle_down,Angle_downL,Angle_downR; 
	float Height1,Height1L,Height1R;    //临空面以上的高度
	float Height_t,Height_tL,Height_tR;   //裂隙深度
	float Length_down,Length_downL,Length_downR;
	float T;         //裂隙中水平水压力
	float Gama_wm;    //某个含水量下的土壤容重
	float Gama_d;     //土体干容重
	float Wt;         //沟坡滑坡体的重力
	float FR,FD;      //沟坡抗滑力和下滑力
	float Cohesive_origin,Cohesive_addi; //  原始粘聚力（饱和时的）和附加粘聚力
	float Angle_Fai;     //土体内摩擦角
	float W_cont; //土体含水量%
	float a,b;           //附加粘聚力与含水量关系中的系数和指数
	float F1,F2,Fs;      //模糊隶属函数中安全系数的上下界即其本身
	float H_Average;         //用于把小飞虫相邻四个水深平均，作为我的输入
	float Hc;      //小飞虫的沟道的某个高度
	float k;        //裂隙与坡高的比
	float Probability;  //稳定概率
	float SlideAmount;  // 崩塌量
	//************* 下面这些是需要小飞虫从库里取的数据
	float mL,mR;  //沟坡陡坡的边坡值
	float Height,HeightL,HeightR;     //沟坡高度
	float Angle_slope; //家宏给出的坡面角（正切值）
	float Slope_A,Slope_L,Slope_AL,Slope_LL,Slope_AR,Slope_LR;    //坡面的面积与长度
	float Length_all;// 类似上边 ,最后是家宏给的坡面长度
	float Stream_J;      //沟道的坡度
	float mytemp;

	//下面是函数

	void InitGravityPara();
	void ConvertTao(float H_avg);   //将小飞虫的宽浅沟道转化到较陡的沟坡
	void InitGullyShap(Para mPara);       //将家宏给出的简单坡面转化为一个缓坡和一个陡坡，确定陡、缓坡的坡度，坡长等。
	void CalFs(int LR);     //计算沟坡稳定性，得到安全系数
	void CalFuzzProbi(float Cigema);  //计算对应于某一个Fs的模糊概率。第一个参数为安全系数，第二个系数为方差
	inline float GenRand(float min,float max);  //产生一个[min,max]的随机数
	void HowMuchFailed(int LR);                //根据计算出的失稳概率计算滑坡的数量

	//薛***


public:
	void Calc(void);//主计算函数
	float GetSS(float Sv,float h,float U,float *omega);//计算挟沙力,最后一个参数用来返回个沉速
	void SediCalc(void); //不平衡输沙计算,放在Calc的时间循环里调用
	void initialize(Para mPara);
	void finalize(void);

	float GetSSFei(float Sv,float h,float U,float *omega);//计算挟沙力,最后一个参数用来返回个沉速 //heli

	// 保存到status表的数据
	void SaveStatus(void);
};
