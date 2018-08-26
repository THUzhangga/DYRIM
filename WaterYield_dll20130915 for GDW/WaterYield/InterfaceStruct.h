

//wh,2008.1.14
//为了组件的接口函数能够传递结构体，定义了InterfaceStruct.idl文件，组件在
//编译的时候，#include "InterfaceStruct.h"语句会被写到WaterYield.h中，而且该文件
//会被链接到exe，而BSCode等结构体在exe中已经有定义了，因次在exe编译的时候会出现
//结构体的重定义的错误，这也是为什么构造一个空InterfaceStruct.h的原因，如果
//不存在该文件组件编译的时候还会报错。
