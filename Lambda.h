#pragma once
#include <vector>
#include <iostream>
#include "Matrix.h"

class CLambda
{
public:
	CLambda(void);
	~CLambda(void);
	CMatrix afloat;//ģ���ȸ����
	CMatrix Qahat;//Э������
	CMatrix sqnorm;
	CMatrix Z;
	CMatrix afixed;//�̶���
	CMatrix D;
	CMatrix L;
	CMatrix z;
	double ratio;
	void lambda2(CMatrix afloat, CMatrix Qahat);
	void decorrel(CMatrix Q, CMatrix a);//ȥ���
	double chistart(CMatrix D, CMatrix L, CMatrix afloat, int ncands);
	void lsearch(CMatrix afloat, CMatrix L, CMatrix D, double Chi2, int ncands);

	double PsCaltor(CMatrix D, int num);
	double Normpdf(double u);
	double round(double r);
};


