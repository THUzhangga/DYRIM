#ifndef WATERBELOWCHANNEL_H
#define WATERBELOWCHANNEL_H
#pragma once

//沟道下的地下水运动
class WaterBelowChannel
{
public:
	WaterBelowChannel(void);
public:
	~WaterBelowChannel(void);

public:
	//以T0T为基准面的测压管水头(M,N点)
	float H,W;

public:
	//返回dW填满沟道地下部分，进入明渠的水量.
	float WtoH(float dW);
	
	//返回水位为H0时，和KL之间的水量m3
	float HtoW(float H0);

public:
	//HH就是H点，因为与上面水深H重名，所以这里命名为HH。
	//B和C是切点坐标,用来辅助判断G、H下方对应的是圆弧段还是直线段
	//A和D分别位于两条直线上，用来辅助确定直线方程
	float G[2],HH[2],E[2],F[2],K[2],L[2],Center[2],B[2],C[2],A[2],D[2];
	
	//R：侵蚀弧半径；Length:河道长
	float R,Length;

	//当前河段能存储的最大水量
	float Wmax;

};

#endif