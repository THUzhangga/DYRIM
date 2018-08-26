#ifndef DATASTRUCT_H
#define DATASTRUCT_H
#pragma	once
#include <string>
#include <stdio.h>
#include <stdlib.h>
#include <atlstr.h>
#include <iostream>
#include <fstream>
#include <windows.h>
#include <math.h>
#include <time.h>
#include <float.h>
#include <vector>//20060327,李铁键,为存储重力侵蚀信息用
using namespace std;

#import "c:\program files\common files\system\ado\msado25.tlb" rename("EOF","EndOfFile")

////wh,预定义宏，用来捕捉数据库操作中的错误
//#ifndef CATCH_ERROR
//#define CATCH_ERROR											\
//		{													\
//			CString strComError;							\
//			strComError.Format("错误编号: %08lx\n错误信息: %s\n错误源: %s\n错误描述: %s", \
//								e.Error(),                  \
//								e.ErrorMessage(),           \
//								(LPCSTR) e.Source(),        \
//								(LPCSTR) e.Description());  \
//			::MessageBox(NULL,strComError,"错误",MB_ICONEXCLAMATION);	\
//		}
//#endif

//定义分钟步长长度
#define PI 3.1415926538

//20060109,李铁键,把两个雨量类里的这个时间起点宏迁到这里
#define YearSTD    1950
#define MonthSTD   1
#define DaySTD     1
#define HourSTD    0

//重力侵蚀的结构体,20060327,李铁键,为存储重力侵蚀信息用
struct GravityEvent
{
	char LR;
	long TimeStep;
	float FD;
	float FR;
	float Probability;
	float Amount;
};

class List;
class ListNode {
	friend class List;
public:
	long BSLength;
	unsigned long long BSValue;
	long SOrder;
	int tmp;
	ListNode *link;		
};

class List {	                 	
public:
	List();
	int  insert(long length,unsigned long long value,long SOrder);
	int  insert(long length,unsigned long long value,long SOrder,long rank);
	int  remove(ListNode *);
	int  find(long,unsigned long long);
	ListNode *N_find(long,unsigned long long);
	ListNode *N_find(int);
public:
	ListNode *first;
	void free(void);
};


struct BSCode 
{
	long RegionGrade;
	unsigned long long RegionIndex;
	long Length;//ToLength
	unsigned long long Value;//ToValue
};

struct Para
{
	//河段的固定参数的结构体，不含时变参数，LAI,WaterEva
	//constanat p muskingum
	float WaveCoefficient; //波速系数
	float x;//坦化系数//流量比重因子，无因次，即汇流模型中W = K[xI+(1-x)O]中的x；
	float WetRadius; //湿周

	//Middle X
	float X;
	//Middle Y
	float Y;

	//沟道
	float StreamSlope; //坡度
	float StreamLength; //长
	float Manning; //n
	float RiverManning;//20051218 李铁键 沟道的糙率
	float m;//边坡系数,最初取4

	//20080306,xiaofc,增加沟坡坡度字段，专门用于计算重力侵蚀
	float GullySlope; //角度
	float DrainingArea;//汇水面积 

	//土壤土质
	float D50;
	int LandUse;
	int SoilType;
	
	//需要通过矩阵转换的
	float Sita1,Sita2,MSita1,MSita2,DSita1,DSita2;//孔隙率
	float UDepth,DDepth;
	float PKH1,PKH2,PKV0,PKV1,PKV2;//渗透率
	float I0;
	float ErosionK;
	float ErosionBeta;

	//左
	float SlopeL;//正切值
	float LengthL;
	float AreaL;

	//右
	float SlopeR;
	float LengthR;
	float AreaR;

	//源
	float SlopeS;
	float LengthS;
	float AreaS;

	float A;
	float UElevation;
	float DElevation;
};

//代表一条河段
struct TreeNode 
{
	TreeNode * Parent;
	TreeNode * Boy;
	TreeNode * Girl;
	BSCode mBSCode;
	Para mPara;

	//wh解读：TaskCount是此点上游还没计算或者正在计算河段数，包括自己。
	int TaskCount;

	//wh解读：UnderCalc大于0时，表示该河段的上游存在河段不在当前TreeList中，两种情况：1是因为分任务产生的分离，2是因为在其他的RegionIndex中。
	//这个参数作用是表示该河段能否从TreeList中切割出，只有等于0可以。
	int UnderCalc;//是否正在被计算

	int StralherOrder;
};


//2008.11.5,WH,新安江模型
typedef struct XAJParameter
{
	int WUM;
	int WLM;
	int WDM;
	float C;
	float B;
	float IMP;
	int SM;
	float EX;
	float KG;
	float KSS;
	float KKG;
	float KKSS;

	int WU0;
	int WL0;
	int WD0;
	int S0;
	int QRS0;
	int QRSS0;
	int QRG0;

}XAJParameter;


struct sStatus
{
	BSCode mBSCode;
	long HourOffset;
	float E;
	float P;
	float W;
	float QRG; 
	float QRSS;
};



//2009.4.7,WH,高边坡产流模型(HSPUSEPARA表)
typedef struct HSParameter
{
	//梯形沟道的两个边角
	float theta1,theta2;
	
	//全流域出口河段的平滩河宽(GH)和流量
	float B0,Q0;

	//夏季最大日蒸发和冬季最小日蒸发，如果为负值则使用海匀默认公式，否则按该公式计算
	float Emax,Emin;
	
	//最大基岩埋深m（圆弧底与河床底的竖直距离）
	float sDeep;

	//岩层的渗透系数,基岩的给水度(0,1)
	float Kr,mu;
	
	//hType:高程值类型(relative or absolute),sShape:坡面形状(rec,tri,par)
	string hType,sShape;
	
	//dt1:地上时间步长,dt2:地下时间步长
	float dt1,dt2;
	
	//沿坡面和非饱和土壤垂向最小空间步长(因为变化)
	float dx,dz;

	float sRatio;//相对不透水层坡脚处厚度比TG/T0G
	float wRatio;//确定饱和土壤水和沟道地下水初始水位：NL/HL
	float rRatio;//确定岩石裂隙水水位：H1T0/OT0,位于T0T下方，该值为负(-oo,1)

	//hspdischarge表中是否保存B，H，V的值，false不保存，true保存
	int SaveFlowPattern;

} HSParameter;



#endif