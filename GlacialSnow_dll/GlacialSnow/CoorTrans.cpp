#include "stdafx.h"
#include "CoorTrans.h" 
#include <math.h> 
bool PrjPoint::BL2xy(jwCoord jw,xyCoord * xy) 
{ 
 double B=jw.wCoord*PI/180,L=jw.jCoord*PI/180; 
 double X, N, t, t2, m, m2, ng2; 
 double sinB, cosB; 
 double x,y;
 //bool ok=GetL0(jw);
 //if (!ok)  return false;
 try 
 {
	X = A1 * B * 180.0 / PI + A2 * sin(2 * B) + A3 * sin(4 * B) + A4 * sin(6 *B); 
	sinB = sin(B); 
	cosB = cos(B); 
	t = tan(B); 
	t2 = t * t; 
	N = a / sqrt(1 - e2 * sinB * sinB); 
	m = cosB * (L - L0); 
	m2 = m * m; 
	ng2 = cosB * cosB * e2 / (1 - e2); 
	x = X + N * t * ((0.5 + ((5 - t2 + 9 * ng2 + 4 * ng2 * ng2) / 24.0 + (61 -58 * t2 + t2 * t2) * m2 / 720.0) * m2) * m2); 
	y = N * m * ( 1 + m2 * ( (1 - t2 + ng2) / 6.0 + m2 * ( 5 - 18 * t2 + t2 * t2 + 14 * ng2 - 58 * ng2 * t2 ) / 120.0)); 
	xy->xCoord=y;
	xy->yCoord=x;
	return true; 
 }
 catch(...)
 {
     return false;
 }
} 
bool PrjPoint::xy2BL(xyCoord xy,jwCoord * jw) 
{ 
 double x=xy.yCoord,y=xy.xCoord;
 double sinB, cosB, t, t2, N ,ng2, V, yN; 
 double preB0, B0,B,L; 
 double eta; 
 try
 {
	y -= 500000; 
	B0 = x / A1; 
	do 
	{ 
		preB0 = B0; 
		B0 = B0 * PI / 180.0; 
		B0 = (x - (A2 * sin(2 * B0) + A3 * sin(4 * B0) + A4 * sin(6 * B0))) / A1; 
		eta = fabs(B0 - preB0); 
	}while(eta > 0.000000001); 
	B0 = B0 * PI / 180.0; 
	sinB = sin(B0); 
	cosB = cos(B0); 
	t = tan(B0); 
	t2 = t * t; 
	N = a / sqrt(1 - e2 * sinB * sinB); 
	ng2 = cosB * cosB * e2 / (1 - e2); 
	V = sqrt(1 + ng2); 
	yN = y / N; 
	B = B0 - (yN * yN - (5 + 3 * t2 + ng2 - 9 * ng2 * t2) * yN * yN * yN * yN /12.0 + (61 + 90 * t2 + 45 * t2 * t2) * yN * yN * yN * yN * yN * yN / 360.0) * V * V * t / 2; 
	L = L0 + (yN - (1 + 2 * t2 + ng2) * yN * yN * yN / 6.0 + (5 + 28 * t2 + 24* t2 * t2 + 6 * ng2 + 8 * ng2 * t2) * yN * yN * yN * yN * yN / 120.0) / cosB; 
	jw->jCoord=L;
	jw->wCoord =B;
	return true; 
 }
 catch(...)
 {
      return false;
 }
} 
bool  PrjPoint::GetL0(jwCoord jw)     //按6度带投影转换
{   bool stopOrNot=false;
    int Num=0;
	while(!stopOrNot)
	{
		if(jw.jCoord>=6*Num && jw.jCoord<=6*(Num+1))   //此处只能处理东半球，经度为西半球
		{
			L0=(6*Num+3)*PI/180;//将-3修改为+3，王皓
			stopOrNot=true;
		}
		else
		{
			if (Num>=30) 
			{
				stopOrNot=true;
			    return false;
			}
		}
         Num+=1;
	}
 return true; 
} 

  