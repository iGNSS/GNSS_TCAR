// Matrix.h: interface for the CMatrix class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_MATRIX_H__98E364C6_FC5D_42A7_919C_F374BBA24341__INCLUDED_)
#define AFX_MATRIX_H__98E364C6_FC5D_42A7_919C_F374BBA24341__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CMatrix  
{
private:
    double *p;
public:
	int Row;
    int Col;
	void ones(int n);
	void ones(int row,int col);
	CMatrix combine(CMatrix C);

	void first(int row,int col);
	CMatrix InvertGaussJordan();
	double GetAt(int row,int col);
	double Tri();
	CMatrix Conver();
	bool LTDL(CMatrix & LL,CMatrix &DD);
	bool LDLT(CMatrix &,CMatrix &);
	void SetSize(int row,int col);
	
	CMatrix();
	virtual ~CMatrix();
    int GetRows(void);
    int GetCols(void);
    CMatrix(int n);
    CMatrix(int row ,int col);
    CMatrix(double *arrAddress ,int col);//�����������ֵ
    CMatrix(double *arrAddress ,int arrWidth ,int arrHeight);
    CMatrix(const CMatrix &m);//�������캯��
    bool IsVector(void);
	double Maxabs();//�����Ԫ�ؾ���ֱ�ĵ����ֵ
    CMatrix SubMatrix(int offset);
    CMatrix RudMatrix(int offset);
	void EyeMatrix(int offset,double m);
	void SetArr(double* m,int p,int q);//�޸ľ���
	double Arg(void);//���������ʽ
    double * operator[](int heightPos);
    bool IsPtv(void);//�жϷ����Ƿ��������
    CMatrix T(void);
	//    CMatrix operator *(CMatrix & m1);
	//    CMatrix operator *(double alpha ,CMatrix &m1);
    CMatrix operator * (double alpha);
    CMatrix operator / (double alpha);
	CMatrix operator + (CMatrix &m1);
    CMatrix operator - (CMatrix &m1);
    CMatrix operator * ( CMatrix &m1);
    //CMatrix operator / (CMatrix &m1);
    CMatrix& operator =(const CMatrix &m);
	
    CMatrix Inver();
	CMatrix Sol(CMatrix &l);//����ԪGauss��ȥ��
	CMatrix Sol2(CMatrix &l,double w,int &n);//��γ��ɳڵ�����
	void DeleteP();
	void MyTRACE(void);
};

#endif // !defined(AFX_MATRIX_H__98E364C6_FC5D_42A7_919C_F374BBA24341__INCLUDED_)
