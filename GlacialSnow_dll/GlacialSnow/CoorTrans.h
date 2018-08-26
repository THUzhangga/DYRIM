#ifndef _COORTRANS_H_INCLUDED 
#define _COORTRANS_H_INCLUDED 
//#include "datastruct.h"
//const double PI = 3.14159265353846;
	struct jwCoord
	{
		float jCoord;
		float wCoord;
	};
	struct xyCoord
	{   double xCoord;
		double yCoord;
		int n;
	};

class PrjPoint 
{ 
private: 
 double L0;    // 中央子午线经度 
 int n;        //分带号
public: 
 bool GetL0(jwCoord jw);
 bool BL2xy(jwCoord jw,xyCoord * xy); 
 bool xy2BL(xyCoord xy,jwCoord * jw); 
protected: 
 double a, f, e2, e12;  // 基本椭球参数 
 double A1, A2, A3, A4; // 用于计算X的椭球参数 

}; 
class PrjPoint_Krasovsky : virtual public PrjPoint 
{ 
public: 
 PrjPoint_Krasovsky()
 {
	a = 6378245; 
	f = 298.3; 
	e2 = 1 - ((f - 1) / f) * ((f - 1) / f); 
	e12 = (f / (f - 1)) * (f / (f - 1)) - 1; 
	A1 = 111134.8611; 
	A2 = -16036.4803; 
	A3 = 16.8281; 
	A4 = -0.0220; 
 } 
 ~PrjPoint_Krasovsky(); 
}; 
class PrjPoint_IUGG1975 : virtual public PrjPoint 
{ 
public: 
 PrjPoint_IUGG1975()
 {
	a = 6378140; 
	f = 298.257; //a/(a-b)
	e2 = 1 - ((f - 1) / f) * ((f - 1) / f); //e2表示e的平方
	e12 = (f / (f - 1)) * (f / (f - 1)) - 1; //e12表示e一撇的平方
	A1 = 111133.0047; 
	A2 = -16038.5282; 
	A3 = 16.8326; 
	A4 = -0.0220; 
 }
 ~PrjPoint_IUGG1975(){} 
}; 
#endif /* ndef _COORTRANS_H_INCLUDED */ 