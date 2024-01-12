# SJTU CS2612 程序语言与编译原理课程大作业：寄存器分配

## 项目简介
本次项目作业利用课上所讲授的liveness分析方法,在一段程序已经做好基本快生成的情况下，进行寄存器分配，具体实现的功能有：  
* liveness分析
* 寄存器分配
* 汇编代码生成
* 在考虑系统调用的函数情况下对caller与callee saved寄存器进行了处理

## 如何使用
* 主程序为```backend_complier.cpp```，待分配程序以```output.txt```文档的形式给出，直接编译运行```backend_complier.cpp```即可对```output.txt```内的程序进行寄存器分配
* 
