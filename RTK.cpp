#include "stdafx.h"
#include "Myheader.h"
#include "Matrix.h"
#include "Lambda.h"
#include <cmath>
#include <vector>
#include <fstream>
#include <string>
#include <assert.h>

using namespace std;

//�����ⲿGetGPSTime��������ReadPfile.cpp�ж���
double GetGPSTime(int year, int month, int day, int hour, int minute, double second, int& dayofy);

//����EpochCommonPro�������Ի�վ������վ���ǹ۲����ݽ��й��Ӵ���
void EpochCommonPro(Obs_epoch tempData1, Obs_epoch tempData2, Obs_epoch& useData);

//����SelectRef������ѡ��ο�����
void SelectRef(Obs_epoch tempData, int& RefNo);

//����ReadSPP_KIN��������ȡspp�ļ�����Ա����Զ����spp�ļ���ʽ�����޸ģ�
bool ReadSPP_KIN(string filename, observe_spp& sppfile);

//����GetDDR_Peo�����������������ɻ�ȡ��������α�ࣩ
void GetDDR_Peo(Obs_epoch GPSData, vector<int> useprn, int GPSRef, CMatrix& R);

// �������ܣ������������ɻ�ȡ���������ز���
void GetDDR_Leo(Obs_epoch GPSData, vector<int> useprn, int GPSRef, CMatrix& R) {
	//R�����趨
	int numgps = GPSData.sat_num;
	if (numgps != useprn.size()) {
		assert(0);
	}
	int numgps_x = numgps - 1;
	CMatrix TTR(numgps_x, numgps);
	CMatrix RT(numgps, numgps);
	int t = 0;
	int k = 0;
	for (k = 0; k < numgps; k++) {
		int match_O = -1;
		int PRN = useprn[k];
		for (int m = 0; m < GPSData.sat_num; m++) {
			if (GPSData.sat2[m].numofsat == PRN) {
				match_O = m;
				break;
			}
		}

		RT[k][k] = pow((0.003 + 0.003 * exp(-GPSData.sat2[match_O].E / 10.0)), 2) + pow((0.003 + 0.003 * exp(-GPSData.sat1[match_O].E / 10.0)), 2);

		if (PRN != GPSRef) {
			TTR[t][k] = 1.0;
			t++;
		}
		else {
			for (int m = 0; m < numgps_x; m++) {
				TTR[m][k] = -1.0;
			}
		}
	}

	R.first(numgps_x, numgps_x);
	//TTR�����Ŀ���Ǹ����������ɣ������Ǽ���ģʽ�µ���������ʽ
	R = TTR * RT * TTR.T();

}

//�������ܣ��������˲�
void KalmanFilter(CMatrix& B, CMatrix& L, CMatrix& R, CMatrix& Qw, CMatrix& Q, CMatrix& X) {
	int r = X.Row;
	CMatrix I, F;
	I.SetSize(r, r);
	F.SetSize(r, r);

	for (int i = 0; i < r; i++) {
		I[i][i] = 1.0;
		F[i][i] = 1.0;
	}
	CMatrix M = F * Q * F.T() + Qw;
	CMatrix K = M * B.T() * (B * M * B.T() + R).InvertGaussJordan();
	Q = (I - K * B) * M * (I - B.T() * K.T()) + K * R * K.T();
	CMatrix V = L - B * X;
	X = X + K * V;
}

//�������ܣ�����̽��
void OnCycleDetect(Obs_epoch& tempEpoch, Obs_epoch& preEpoch) {
	double slip1 = 0;
	double slip2 = 0;
	for (int i = 0; i < tempEpoch.sat_num; i++) {
		Sat* sat = &tempEpoch.sat[i];
		if (sat->judge_use)
			continue;
		if ((sat->sattype != "G") && (sat->sattype != "C") && (sat->sattype != "B") && (sat->sattype != "E"))
			continue;
		for (int j = 0; j < preEpoch.sat_num; j++) {
			if ((tempEpoch.sat[i].sattype == preEpoch.sat[j].sattype) && (tempEpoch.sat[i].numofsat == preEpoch.sat[j].numofsat)) {
				Sat* presat = &preEpoch.sat[j];
				if (presat->judge_use)
					continue;
				if ((presat->sattype != "G") && (presat->sattype != "C") && (presat->sattype != "B") && (presat->sattype != "E"))
					continue;
				//GPS
				if (presat->sattype == "G") {
					slip1 = sat->data[2] - presat->data[2] - (FREQ1 / FREQ2) * (sat->data[3] - presat->data[3]);//�����в�
					slip2 = (-(sat->data[2] - sat->data[3]) + 1 / lambda_L_wide * lambda_L_narrow * (sat->data[0] / lambda_L1 + sat->data[1] / lambda_L2))
						- (-(presat->data[2] - presat->data[3]) + 1 / lambda_L_wide * lambda_L_narrow * (presat->data[0] / lambda_L1 + presat->data[1] / lambda_L2)); //MW
				}
				//BDS-2
				else if (presat->sattype == "C") {
					slip1 = sat->data[2] - presat->data[2] - (FREQ1_BDS / FREQ2_BDS) * (sat->data[3] - presat->data[3]);//�����в�
					slip2 = (-(sat->data[2] - sat->data[3]) + 1 / lambda_L_wide_C * lambda_L_narrow_C * (sat->data[0] / lambda_L1_C + sat->data[1] / lambda_L2_C))
						- (-(presat->data[2] - presat->data[3]) + 1 / lambda_L_wide_C * lambda_L_narrow_C * (presat->data[0] / lambda_L1_C + presat->data[1] / lambda_L2_C)); //MW
				}
				//BDS-3
				else if (presat->sattype == "B") {
					slip1 = sat->data[2] - presat->data[2] - (f1B / f2B) * (sat->data[3] - presat->data[3]);//�����в�
					slip2 = (-(sat->data[2] - sat->data[3]) + 1 / lambda_L_wide_B * lambda_L_narrow_B * (sat->data[0] / lambda_L1_B + sat->data[1] / lambda_L2_B))
						- (-(presat->data[2] - presat->data[3]) + 1 / lambda_L_wide_B * lambda_L_narrow_B * (presat->data[0] / lambda_L1_B + presat->data[1] / lambda_L2_B));//MW
				}
				//Galileo
				else if (presat->sattype == "E") {
					slip1 = sat->data[2] - presat->data[2] - (f1E / f2E) * (sat->data[3] - presat->data[3]);//�����в�
					slip2 = (-(sat->data[2] - sat->data[3]) + 1 / lambda_L_wide_E * lambda_L_narrow_E * (sat->data[0] / lambda_L1_E + sat->data[1] / lambda_L2_E))
						- (-(presat->data[2] - presat->data[3]) + 1 / lambda_L_wide_E * lambda_L_narrow_E * (presat->data[0] / lambda_L1_E + presat->data[1] / lambda_L2_E)); //MW
				}
				if (fabs(slip1) > 0.05 || fabs(slip2) > 10) {
					sat->judge_use = 1;
				}
				break;
			}
		}
	}
}

//�������ܣ�����ת������
bool GetGRCQValue(int PRN_X[], int PRN_old[], int num_sat, int num_sat_old, int Ref, int Ref_old, CMatrix& TT_Matrix)
{
	int i, j;
	int M_PRN[40];

	int num_same = 0;
	for (i = 0; i < num_sat_old; i++)
	{
		for (j = 0; j < num_sat; j++)
		{
			if (PRN_old[i] == PRN_X[j])
			{
				M_PRN[num_same] = PRN_old[i];
				num_same++;
				break;
			}
		}
	}

	//�Ƚ��л��Ǽ��
	if (Ref != Ref_old)
	{
		bool iffind_old = false;
		bool iffind_new = false;
		for (i = 0; i < num_sat_old; i++)
		{
			if (Ref_old == PRN_old[i])
			{
				iffind_old = true;
			}
			if (Ref == PRN_old[i])
			{
				iffind_new = true;
			}
		}
		if (iffind_old == false || iffind_new == false)
		{
			return false;
		}
	}

	CMatrix TMatrix1;
	TMatrix1.SetSize(num_sat_old - 1, num_sat_old - 1);
	for (int tx = 0; tx < num_sat_old - 1; tx++)
	{
		TMatrix1[tx][tx] = 1;
	}

	//�Ƚ��вο��Ǹ���
	if (Ref != Ref_old)
	{
		int match_old = -1;
		int match_new = -1;
		bool iffind_old = false;
		bool iffind_new = false;
		for (i = 0; i < num_sat_old; i++)
		{
			if (Ref_old == PRN_old[i])
			{
				match_old = i;
				iffind_old = true;
			}
			if (Ref == PRN_old[i])
			{
				match_new = i;
				iffind_new = true;
			}
		}
		if (iffind_old == false)
		{
			assert(false);
		}
		if (iffind_new == false)
		{
			Ref = Ref_old;    //����µĲο���������һ����Ԫ�Ҳ�������ά�ֲο����ǲ���
		}
		else
		{
			CMatrix TT(num_sat_old - 1, num_sat_old - 1);
			CMatrix TC(num_sat_old - 1, num_sat_old - 1);
			for (i = 0; i < num_sat_old - 1; i++)
			{
				TT[i][i] = 1.0;
			}
			if (match_new < match_old)
			{
				for (i = 0; i < num_sat_old - 1; i++)
				{
					TT[i][match_new] = -1.0;
				}
				for (i = 0; i < match_new; i++)
				{
					TC[i][i] = 1.0;
				}
				for (i = match_new; i < match_old - 1; i++)
				{
					TC[i][i + 1] = 1.0;
				}
				TC[match_old - 1][match_new] = 1.0;
				for (i = match_old; i < num_sat_old - 1; i++)
				{
					TC[i][i] = 1.0;
				}
			}
			else if (match_new > match_old)
			{
				for (i = 0; i < num_sat_old - 1; i++)
				{
					TT[i][match_new - 1] = -1.0;
				}
				for (i = 0; i < match_old; i++)
				{
					TC[i][i] = 1.0;
				}
				TC[match_old][match_new - 1] = 1.0;
				for (i = match_old + 1; i < match_new; i++)
				{
					TC[i][i - 1] = 1.0;
				}
				for (i = match_new; i < num_sat_old - 1; i++)
				{
					TC[i][i] = 1.0;
				}
			}
			TMatrix1 = TC * TT;
		}
	}

	CMatrix TMatrix2;
	TMatrix2.SetSize(num_same - 1, num_sat_old - 1);
	if (num_same < num_sat_old)//������ʧ
	{
		CMatrix TT(num_same - 1, num_sat_old - 1);
		int t_col = 0;
		int t_raw = 0;
		for (i = 0; i < num_sat_old; i++)
		{
			if (PRN_old[i] == Ref)
			{
				continue;
			}
			bool iffind = false;
			for (j = 0; j < num_same; j++)
			{
				if (M_PRN[j] == PRN_old[i])
				{
					iffind = true;
					break;
				}
			}
			if (iffind == true)
			{
				TT[t_col][t_raw] = 1.0;
				t_col++;
				t_raw++;
			}
			else if (iffind == false)
			{
				t_raw++;
			}
		}
		if (t_col != num_same - 1)
		{
			assert(false);
		}
		if (t_raw != num_sat_old - 1)
		{
			assert(false);
		}

		TMatrix2 = TT * TMatrix1;
	}
	else if (num_same == num_sat_old)
	{
		TMatrix2 = TMatrix1;
	}
	else
	{
		assert(false);
	}

	CMatrix TMatrix3;
	TMatrix3.SetSize(num_sat - 1, num_sat_old - 1);
	if (num_same < num_sat)//��������
	{
		CMatrix TT(num_sat - 1, num_same - 1);
		CMatrix MAX_Q(num_sat - 1, num_sat - 1);

		int t_col = 0;
		int t_raw = 0;
		for (i = 0; i < num_sat; i++)
		{
			if (PRN_X[i] == Ref)
			{
				continue;
			}
			bool iffind = false;
			for (j = 0; j < num_same; j++)
			{
				if (M_PRN[j] == PRN_X[i])
				{
					iffind = true;
					break;
				}
			}
			if (iffind == true)
			{
				TT[t_col][t_raw] = 1.0;

				t_col++;
				t_raw++;
			}
			else if (iffind == false)
			{
				for (int i = 0; i < num_sat - 1; i++)
				{
					if (i == t_col)
					{
						MAX_Q[t_col][t_col] = 1.0E10;
					}
				}
				t_col++;
			}
		}

		if (t_col != num_sat - 1)
		{
			assert(false);
		}
		if (t_raw != num_same - 1)
		{
			assert(false);
		}
		TMatrix3 = TT * TMatrix2;
	}
	else if (num_same == num_sat)
	{
		TMatrix3 = TMatrix2;
	}
	else
	{
		assert(false);
	}

	//����ת������+ת��������к���+������Э��������
	TT_Matrix = TMatrix3;
	return true;
}

//�������ܣ�ƴ�Ӿ���
void ProGRCTrans(CMatrix& TT_G, CMatrix& TT_C, CMatrix& TT_ALL)
{
	int Row_G = TT_G.Row;
	int Col_G = TT_G.Col;
	int Row_C = TT_C.Row;
	int Col_C = TT_C.Col;


	int Tot_Row = Row_G + Row_C;
	int Tot_Col = Col_G + Col_C;
	TT_ALL.SetSize(Tot_Row, Tot_Col);

	int i = 0;
	int j = 0;

	//GPS���ָ�ֵ���µ�ת������
	for (i = 0; i < Row_G; i++)
		for (j = 0; j < Col_G; j++)
			TT_ALL[i][j] = TT_G[i][j];

	//BDS���ָ�ֵ���µ�ת������
	for (i = 0; i < Row_C; i++)
		for (j = 0; j < Col_C; j++)
			TT_ALL[Row_G + i][Col_G + j] = TT_C[i][j];
}

//�������ܣ���ʼ��״̬����
void InitState(CMatrix& QX)
{
	int num_x = QX.Row;
	for (int idx = 0; idx < num_x; idx++)
	{
		if (idx <= 2)
		{
			QX[idx][idx] = 10.0 * 10.0;    //��ʼ��λ��
		}
		else
		{
			QX[idx][idx] = 10.0 * 10.0;  //��ʼ��ģ����
		}
	}
}

//�������ܣ�����Э����
void UpdataState(CMatrix& QX, CMatrix& X, int* SatNum, int* oldSatNum, int SatPRN[2][20], int oldSatPRN[2][20], int* RefSat, int* OldRef)
{
	CMatrix T[2], ALL_T, Matrix_t;
	int sys = 0, oldsys = 0, ALL_NUM, oldALL_NUM;
	if (SatNum[0] >= 2)
	{
		sys++;
	}
	if (SatNum[1] >= 2)
	{
		sys++;
	}
	if (oldSatNum[0] >= 2)
	{
		oldsys++;
	}
	if (oldSatNum[1] >= 2)
	{
		oldsys++;
	}
	ALL_NUM = SatNum[0] + SatNum[1] - sys;
	oldALL_NUM = oldSatNum[0] + oldSatNum[1] - oldsys;
	//L1����L1�仯����
	for (int i = 0; i < 2; i++)
	{
		if (SatNum[i] > 1)
		{
			if (oldSatNum[i] > 0)
			{
				T[i].SetSize(SatNum[i] - 1, oldSatNum[i] - 1);//��ǰ��ԪGPS����������һ��ԪGPS������
				GetGRCQValue(SatPRN[i], oldSatPRN[i], SatNum[i], oldSatNum[i], RefSat[i], OldRef[i], T[i]);
			}
			else
			{
				T[i].SetSize(SatNum[i] - 1, 0);
			}
		}
		else
		{
			T[i].SetSize(0, 0);
			if (oldSatNum[i] != 0)
			{
				T[i].SetSize(0, oldSatNum[i] - 1);
			}
		}
	}

	ALL_T.SetSize(ALL_NUM, oldALL_NUM);
	ProGRCTrans(T[0], T[1], ALL_T);

	Matrix_t.SetSize(ALL_NUM + 3, oldALL_NUM + 3);
	for (int j = 0; j < 3; j++)
	{
		Matrix_t[j][j] = 1.0;
	}

	for (int ii = 0; ii < ALL_NUM; ii++)
	{
		for (int jj = 0; jj < oldALL_NUM; jj++)
		{
			Matrix_t[ii + 3][jj + 3] = ALL_T[ii][jj];
		}
	}

	QX = Matrix_t * QX * Matrix_t.T();
	X = Matrix_t * X;
}

//�������ܣ����������
void CheckNewSats(int* SatNum, int* oldSatNum, int SatPRN[2][20], int oldSatPRN[2][20], int NewSatPRN[2][20], int* NewSatNum)
{
	for (int i = 0; i < 2; i++)
	{
		int newsatsnum = 0;
		for (int j = 0; j < SatNum[i]; j++)
		{
			bool bFind = false;
			for (int k = 0; k < oldSatNum[i]; k++)
			{
				if (SatPRN[i][j] == oldSatPRN[i][k])
				{
					bFind = true;
					break;
				}
			}

			if (!bFind)
			{
				NewSatPRN[i][newsatsnum] = SatPRN[i][j];
				newsatsnum++;
			}
		}
		NewSatNum[i] = newsatsnum;
	}
}

//�������ܣ�Lambda�㷨�̶�ģ����
void Resamb_LAMBDA(CMatrix& Q, CMatrix& X, CMatrix& aXb, double& ratio, CMatrix& fixX)
{
	CMatrix Qbb, Qaa, Qab, Xa, Xb, fixXa, Q_LAM, X_LAM;
	int Num_X = X.Row - 3;
	Qbb.SetSize(3, 3);
	Qaa.SetSize(Num_X, Num_X);
	Qab.SetSize(3, Num_X);
	Q_LAM.SetSize(Num_X, Num_X);
	X_LAM.SetSize(Num_X, 1);
	fixXa.SetSize(Num_X, 1);
	Xa.SetSize(Num_X, 1);
	Xb.SetSize(3, 1);

	for (int i = 0; i < Num_X; i++)
	{
		for (int j = 0; j < Num_X; j++)
		{
			Q_LAM[i][j] = Q[i + 3][j + 3];
		}
		X_LAM[i][0] = X[i + 3][0];
		Xa[i][0] = X[i + 3][0];
	}
	for (int ii = 0; ii < 3; ii++)
	{
		Xb[ii][0] = X[ii][0];
	}

	for (int ii = 0; ii < 3; ii++)
	{
		for (int jj = 0; jj < 3; jj++)
		{
			Qbb[ii][jj] = Q[ii][jj];
		}
	}

	for (int ii = 0; ii < 3; ii++)
	{
		for (int jj = 0; jj < Num_X; jj++)
		{
			Qab[ii][jj] = Q[ii][jj + 3];
		}
	}

	Qaa = Q_LAM;
	CLambda lambda;
	lambda.afloat.SetSize(Num_X, 1);
	lambda.Qahat.SetSize(Num_X, Num_X);
	lambda.Qahat = Q_LAM;
	lambda.afloat = X_LAM;
	lambda.lambda2(lambda.afloat, lambda.Qahat);

	//�̶���
	for (int ii = 0; ii < Num_X; ii++)
	{
		fixXa[ii][0] = lambda.afixed[ii][0];
	}

	aXb = Xb - Qab * Qaa.InvertGaussJordan() * (Xa - fixXa);
	Qbb = Qbb - Qab * Qaa.InvertGaussJordan() * Qab.T();
	ratio = lambda.ratio;
	fixX = fixXa;
}

//�������ܣ�������
double Error_Correction(Sat& tempSat1, Sat& RefSat1, Sat& tempSat2, Sat& RefSat2) {
	//��վ-�ǲο�����
	double error_satclock1 = tempSat1.Sat_clock;
	double error_trop1 = tempSat1.Trop_Delay;
	double error_relat1 = tempSat1.Relat;
	double error_sagnac1 = tempSat1.Sagnac;
	double error_tgd1 = tempSat1.TGD;
	double error_antenna_height1 = tempSat1.Antenna_Height;
	double error_sat_antenna1 = tempSat1.Sat_Antenna;
	double test_geo1 = -error_satclock1 + error_trop1 - error_relat1 + error_sagnac1 + error_tgd1 - error_sat_antenna1;

	//��վ-�ο�����
	double Rerror_satclock1 = RefSat1.Sat_clock;
	double Rerror_trop1 = RefSat1.Trop_Delay;
	double Rerror_relat1 = RefSat1.Relat;
	double Rerror_sagnac1 = RefSat1.Sagnac;
	double Rerror_tgd1 = RefSat1.TGD;
	double Rerror_antenna_height1 = RefSat1.Antenna_Height;
	double Rerror_sat_antenna1 = RefSat1.Sat_Antenna;
	double Rtest_geo1 = -Rerror_satclock1 + Rerror_trop1 - Rerror_relat1 + Rerror_sagnac1 + Rerror_tgd1 - Rerror_sat_antenna1;

	//�ο�վ2-�ǲο�����
	double error_satclock2 = tempSat2.Sat_clock;
	double error_trop2 = tempSat2.Trop_Delay;
	double error_relat2 = tempSat2.Relat;
	double error_sagnac2 = tempSat2.Sagnac;
	double error_tgd2 = tempSat2.TGD;
	double error_antenna_height2 = tempSat2.Antenna_Height;
	double error_sat_antenna2 = tempSat2.Sat_Antenna;
	double test_geo2 = -error_satclock2 + error_trop2 - error_relat2 + error_sagnac2 + error_tgd2 - error_sat_antenna2;

	//�ο�վ2-�ο�����
	double Rerror_satclock2 = RefSat2.Sat_clock;
	double Rerror_trop2 = RefSat2.Trop_Delay;
	double Rerror_relat2 = RefSat2.Relat;
	double Rerror_sagnac2 = RefSat2.Sagnac;
	double Rerror_tgd2 = RefSat2.TGD;
	double Rerror_antenna_height2 = RefSat2.Antenna_Height;
	double Rerror_sat_antenna2 = RefSat2.Sat_Antenna;
	double Rtest_geo2 = -Rerror_satclock2 + Rerror_trop2 - Rerror_relat2 + Rerror_sagnac2 + Rerror_tgd2 - Rerror_sat_antenna2;

	//˫����ʽ�ļ���������
	const double DDerror = (test_geo2 - Rtest_geo2) - (test_geo1 - Rtest_geo1);
	return DDerror;
}

//�������ܣ�����B,L����
/*void Get_BL(Obs_epoch& Data, int k, int ref_gps, int tw, int num_n, double POSITION_X1, double POSITION_Y1, double POSITION_Z1, double POSITION_X2, double POSITION_Y2, double POSITION_Z2, CMatrix& B, CMatrix& L) {
	Sat tempSat1 = Data.sat1[k];
	Sat RefSat1 = Data.sat1[ref_gps];
	Sat tempSat2 = Data.sat2[k];
	Sat RefSat2 = Data.sat2[ref_gps];

	double length_O1, length_R1, length_O2, length_R2;
	double DDerror = 0;

	//�������ǵ���վ���루4�����룩
	length_O1 = sqrt(pow((tempSat1.POS_X - POSITION_X1), 2) + pow((tempSat1.POS_Y - POSITION_Y1), 2) + pow((tempSat1.POS_Z - POSITION_Z1), 2));
	length_R1 = sqrt(pow((RefSat1.POS_X - POSITION_X1), 2) + pow((RefSat1.POS_Y - POSITION_Y1), 2) + pow((RefSat1.POS_Z - POSITION_Z1), 2));

	length_O2 = sqrt(pow((tempSat2.POS_X - POSITION_X2), 2) + pow((tempSat2.POS_Y - POSITION_Y2), 2) + pow((tempSat2.POS_Z - POSITION_Z2), 2));
	length_R2 = sqrt(pow((RefSat2.POS_X - POSITION_X2), 2) + pow((RefSat2.POS_Y - POSITION_Y2), 2) + pow((RefSat2.POS_Z - POSITION_Z2), 2));

	//��������ϵ��
	double Cof1 = (tempSat2.POS_X - POSITION_X2) / length_O2 - (RefSat2.POS_X - POSITION_X2) / length_R2;
	double Cof2 = (tempSat2.POS_Y - POSITION_Y2) / length_O2 - (RefSat2.POS_Y - POSITION_Y2) / length_R2;
	double Cof3 = (tempSat2.POS_Z - POSITION_Z2) / length_O2 - (RefSat2.POS_Z - POSITION_Z2) / length_R2;

	B[tw][0] = Cof1;
	B[tw][1] = Cof2;
	B[tw][2] = Cof3;         //α����ǰ

	B[tw + num_n][0] = Cof1;
	B[tw + num_n][1] = Cof2;
	B[tw + num_n][2] = Cof3;  //�ز��ں�
	if(tempSat1.sattype=="G" && tempSat2.sattype=="G")
		B[tw + num_n][3 + tw] = lambda_L1;
	else
		B[tw + num_n][3 + tw] = lambda_L1_C;

	//������
	Error_Correction(tempSat1, RefSat1, tempSat2, RefSat2, DDerror);

	//˫����ʽ�ļ���������
	double DDgeo = (length_O2 - length_O1 - length_R2 + length_R1) + DDerror;
	double deltp1 = (tempSat2.data[0] - RefSat2.data[0]) - (tempSat1.data[0] - RefSat1.data[0]);  //α��
	double deltL1 = 0;
	if (tempSat1.sattype == "G" && tempSat2.sattype == "G")
		double deltL1 = ((tempSat2.data[2] - RefSat2.data[2]) - (tempSat1.data[2] - RefSat1.data[2])) * lambda_L1; //�ز�
	else
		double deltL1 = ((tempSat2.data[2] - RefSat2.data[2]) - (tempSat1.data[2] - RefSat1.data[2])) * lambda_L1_C; //�ز�
	L[tw][0] = deltp1 - DDgeo;         //α����ǰ
	L[tw + num_n][0] = deltL1 - DDgeo; //�ز��ں�
}*/

//�������ܣ�ͨ����ȡspp�ļ�������RTK����
bool SPP_Kinematic_Pro(string filename1, string filename2) {
	//Ĭ�ϵ�һ��վΪ��׼վ���ڶ���Ϊ����վ
	observe_spp sppfile1;
	observe_spp sppfile2;
	ReadSPP_KIN(filename1, sppfile1);
	ReadSPP_KIN(filename2, sppfile2);

	//ͶӰ���󣺶���������ϵ���˴���ֱ�Ӳ���ͷ�ļ����꣩
	CMatrix TT(3, 3);
	TT[0][0] = -sin(sppfile2.B) * cos(sppfile2.L);
	TT[0][1] = -sin(sppfile2.B) * sin(sppfile2.L);
	TT[0][2] = cos(sppfile2.B);

	TT[1][0] = -sin(sppfile2.L);
	TT[1][1] = cos(sppfile2.L);
	TT[1][2] = 0;

	TT[2][0] = cos(sppfile2.B) * cos(sppfile2.L);
	TT[2][1] = cos(sppfile2.B) * sin(sppfile2.L);
	TT[2][2] = sin(sppfile2.B);

	bool bInit = false;//��ʼ��

	//�����ֶ�λ�����
	string name = filename2.substr(0, 3) + "_RTK.txt";
	const char* output_filename = name.c_str();
	FILE* fp;
	fopen_s(&fp, output_filename, "w");

	int OldSatPRN[2][20] = { 0 };
	int OldRefPRN[2] = { -1,-1 };
	int OldSatNum[2] = { 0 };

	CMatrix Q, X;

	//��������վƥ���׼վ��Ӧ��Ԫ����
	int posk = 0;   //���Լ�¼��վ���ݵ�ƥ��
	for (int j = 0; j < sppfile2.liyuan_num; j++)   //����վ����Ԫ����
	{
		Obs_epoch tempData;

		//����վ��ǰʱ��
		Obs_epoch tempData2 = sppfile2.epoch[j];

		if (j >= 1) {
			Obs_epoch preData2 = sppfile2.epoch[j - 1];
			OnCycleDetect(tempData2, preData2);     //�жϴ˴���Ԫ����һ����Ԫ�е������ز�����ֵ�Ƿ�������,����������ͽ�����Ԫ�е����Ǽ�Ϊ������
		}

		//��׼վ������Ӧʱ�̱�����ǰ
		int i = 0;
		for (i = posk; i < sppfile1.liyuan_num; i++)
		{
			if (sppfile1.epoch[i].GPSTIME <= tempData2.GPSTIME)
				continue;
			else
				break;
		}

		//����һ����Ԫ��posk��ΪҪ�ҵ��������Ԫ������posk��ʱ��С��tempData2��ʱ��
		if (i > 0) posk = --i;
		else
		{
			posk = i;
			continue;
		}

		//�ж�ʱ�������Ƿ��������1s
		if (fabs(sppfile1.epoch[i].GPSTIME - tempData2.GPSTIME) >= 1)
		{
			printf("δƥ�䵽��վ����\n");
			continue;
		}

		//�����׼վ�����ͺ�
		if (sppfile1.epoch[i].GPSTIME > tempData2.GPSTIME)
		{
			printf("��վ����ƥ�����\n");
			exit(1);
		}

		//��û�վ����������Ԫ����
		Obs_epoch tempData1 = sppfile1.epoch[posk];

		//�Ի�վ��������̽��
		if (posk >= 1)
		{
			Obs_epoch preData1 = sppfile1.epoch[posk - 1];
			OnCycleDetect(tempData1, preData1);
		}

		//��ֶ�λ���Ӵ���
		EpochCommonPro(tempData1, tempData2, tempData);

		//����GPS��BDS���ǣ����ں�����ϵͳ��˫ϵͳ����
		Obs_epoch GPSData;
		Obs_epoch BDSData;

		int SatPRN[2][20] = { 0 };		//��ǰ��Ԫ���Ǻ� ���δ洢
		int SatNum[2] = { 0 };	//��ǰ��Ԫ������Ŀ
		int RefPRN[2] = { 0 };  //�ο����Ǻ�

		int NewSatNum[2] = { 0 };
		int NewSatPRN[2][20] = { 0 };

		int k = 0;
		int m = 0;
		int gpsnum, bdsnum;
		gpsnum = bdsnum = 0;
		for (m = 0; m < tempData.sat_num; m++)
		{
			if (tempData.sat1[m].sattype == "G")
			{
				GPSData.sat1.push_back(tempData.sat1[m]);
				GPSData.sat2.push_back(tempData.sat2[m]);
				SatPRN[0][gpsnum] = tempData.sat1[m].numofsat;
				gpsnum++;
			}

			if (tempData.sat1[m].sattype == "C")
			{
				BDSData.sat1.push_back(tempData.sat1[m]);
				BDSData.sat2.push_back(tempData.sat2[m]);
				SatPRN[1][bdsnum] = tempData.sat1[m].numofsat;
				bdsnum++;
			}
		}

		SatNum[0] = gpsnum;		//��ǰ��Ԫ��������Ŀ GPS/BDS
		SatNum[1] = bdsnum;

		GPSData.sat_num = GPSData.sat1.size();
		BDSData.sat_num = BDSData.sat1.size();
		tempData.GPSTIME = tempData2.GPSTIME;

		//��ʱ����GPS��BDS������4�����ǵ����ݣ�����Ӧ��ʱһ��������Ӵ���5�ż���
		if (GPSData.sat_num < 4 || BDSData.sat_num < 4)
		{
			printf("GPS��BDS������С��4��\n");
			bInit = false;
			continue;
		}

		//����վ���Ǽ���β��ģ�ͣ�ѡ��ο�����
		int GPSRef = -1;
		int BDSRef = -2;
		GPSRef = OldRefPRN[0];
		BDSRef = OldRefPRN[1];
		SelectRef(GPSData, GPSRef);  //ѡ�����߶Ƚ�������Ϊ�ο����� 
		SelectRef(BDSData, BDSRef);  //���Ƿֿ���

		RefPRN[0] = GPSRef;  //GPS�ο��������
		RefPRN[1] = BDSRef;	 //BDS�ο��������

		//��ǰ��ԪGPS��BDS��������
		int PRN_X_G[32];
		int PRN_X_B[20];

		memset(PRN_X_G, 0, sizeof(PRN_X_G));	//���г�ʼ��
		memset(PRN_X_B, 0, sizeof(PRN_X_B));

		for (k = 0; k < GPSData.sat_num; k++)
			PRN_X_G[k] = GPSData.sat1[k].numofsat;
		for (k = 0; k < BDSData.sat_num; k++)
			PRN_X_B[k] = BDSData.sat1[k].numofsat;

		int num_X_G = GPSData.sat_num;	          //GPS���Ǹ���
		int num_X_B = BDSData.sat_num;	          //BDS���Ǹ���

		int numgps_x = num_X_G - 1;               //GPS˫��̸���
		int numbds_x = num_X_B - 1;               //BDS˫��̸���

		int num_x_p = (numgps_x + numbds_x) * 2;  //˫ϵͳ�ں�˫��̸���  P1 + L1		�ز��۲ⷽ�̼�α��۲ⷽ��

		int num_x = numgps_x + numbds_x + 3;      //����������������X����Y����Z��ÿ�����ǵ�ģ����
		int num_n = numgps_x + numbds_x;          //ģ���ȸ���

		//��λ������صľ���	
		//1-˫ϵͳ
		CMatrix B(num_x_p, num_x);                //ϵ������  
		CMatrix L(num_x_p, 1);                    //�۲�ֵ����
		CMatrix R(num_x_p, num_x_p);              //����������ΪȨ����
		CMatrix Qw(num_x, num_x);                 //��̬������

		//λ�õĶ�̬���������û��������ģ���Ȳ�����������Ϊ0
		for (int idx = 0; idx < 3; idx++) {
			Qw[idx][idx] = 30.0 * 30.0;
		}

		//2-��GPS
		CMatrix RG_P(numgps_x, numgps_x); //����������ΪȨ���� α��
		CMatrix RG_L(numgps_x, numgps_x); //����������ΪȨ���� �ز�

		//3-��BDS
		CMatrix RC_P(numbds_x, numbds_x); //����������ΪȨ���� α��
		CMatrix RC_L(numbds_x, numbds_x); //����������ΪȨ���� �ز�

		//��ȡGPS��BDS��������
		if (!bInit)
		{
			Q.SetSize(num_x, num_x);
			X.SetSize(num_x, 1);
			InitState(Q);
		}
		else
		{
			//���������
			CheckNewSats(SatNum, OldSatNum, SatPRN, OldSatPRN, NewSatPRN, NewSatNum);

			//����Э������
			UpdataState(Q, X, SatNum, OldSatNum, SatPRN, OldSatPRN, RefPRN, OldRefPRN);
		}

		vector<int> GPS_PRN;	//������Ŵ洢����
		vector<int> BDS_PRN;

		for (k = 0; k < GPSData.sat_num; k++)
			GPS_PRN.push_back(PRN_X_G[k]);
		for (k = 0; k < BDSData.sat_num; k++)
			BDS_PRN.push_back(PRN_X_B[k]);

		//α��
		GetDDR_Peo(GPSData, GPS_PRN, GPSRef, RG_P);
		GetDDR_Peo(BDSData, BDS_PRN, BDSRef, RC_P);
		//�ز�
		GetDDR_Leo(GPSData, GPS_PRN, GPSRef, RG_L);
		GetDDR_Leo(BDSData, BDS_PRN, BDSRef, RC_L);

		//�ں�ϵͳ��R���󣬷ֿ�ԽǾ���
		//α��
		for (int k1 = 0; k1 < numgps_x; k1++)
			for (int k2 = 0; k2 < numgps_x; k2++)
			{
				R[k1][k2] = RG_P[k1][k2];
			}

		for (int k1 = 0; k1 < numbds_x; k1++)
			for (int k2 = 0; k2 < numbds_x; k2++)
			{
				R[k1 + numgps_x][k2 + numgps_x] = RC_P[k1][k2];
			}

		//�ز�
		for (int k1 = 0; k1 < numgps_x; k1++)
			for (int k2 = 0; k2 < numgps_x; k2++)
			{
				R[k1 + numgps_x + numbds_x][k2 + numgps_x + numbds_x] = RG_L[k1][k2];
			}
		for (int k1 = 0; k1 < numbds_x; k1++)
			for (int k2 = 0; k2 < numbds_x; k2++)
			{
				R[k1 + numgps_x * 2 + numbds_x][k2 + numgps_x * 2 + numbds_x] = RC_L[k1][k2];
			}

		//��ʼ������Ϣ����վ����spp�ļ�ͷ�����꣨�̶���������վ����ÿ����Ԫ�ĸ������꣨����ʵ��Ӧ���������Ϊ��̬�����
		//���������У�����վΪ��̬��������ļ�ͷ������Ҳ��ȡ�������ڱȽ�
		//��վ���꣺ͷ�ļ�����
		double POSITION_X1 = sppfile1.APP_X;
		double POSITION_Y1 = sppfile1.APP_Y;
		double POSITION_Z1 = sppfile1.APP_Z;

		//����վ���꣺��ÿ����Ԫ���㶨λ���������
		double POSITION_X2 = tempData2.posX;
		double POSITION_Y2 = tempData2.posY;
		double POSITION_Z2 = tempData2.posZ;

		//����վ�Ƚ�ֵ������Ϊ��ֵ��ͷ�ļ����꣩
		double P_X2 = sppfile2.APP_X;
		double P_Y2 = sppfile2.APP_Y;
		double P_Z2 = sppfile2.APP_Z;

		//�ҵ��ο������±�
		int ref_gps = -1;
		int ref_bds = -1;
		for (k = 0; k < GPSData.sat_num; k++)
		{
			if (GPSData.sat1[k].numofsat == GPSRef)
			{
				ref_gps = k;
				break;
			}
		}

		for (k = 0; k < BDSData.sat_num; k++)
		{
			if (BDSData.sat1[k].numofsat == BDSRef)
			{
				ref_bds = k;
				break;
			}
		}

		//GPS
		int tp = 0;  //��¼��ϵͳ������
		int tw = 0;  //��¼˫ϵͳ������
		double DDerror = 0;  //��ʼ����������
		for (k = 0; k < GPSData.sat_num; k++)
		{
			bool Init = true;                      //��¼�Ƿ��оɲο��Ǳ��޳�
			int PRN = GPSData.sat1[k].numofsat;

			//�ο���������
			if (PRN == GPSRef)
			{
				continue;
			}

			if (ref_gps < 0)
			{
				assert(0);
			}

			Sat tempSat1 = GPSData.sat1[k];
			Sat RefSat1 = GPSData.sat1[ref_gps];
			Sat tempSat2 = GPSData.sat2[k];
			Sat RefSat2 = GPSData.sat2[ref_gps];

			double length_O1, length_R1, length_O2, length_R2;

			//�������ǵ���վ���루4�����룩
			length_O1 = sqrt(pow((tempSat1.POS_X - POSITION_X1), 2) + pow((tempSat1.POS_Y - POSITION_Y1), 2) + pow((tempSat1.POS_Z - POSITION_Z1), 2));
			length_R1 = sqrt(pow((RefSat1.POS_X - POSITION_X1), 2) + pow((RefSat1.POS_Y - POSITION_Y1), 2) + pow((RefSat1.POS_Z - POSITION_Z1), 2));

			length_O2 = sqrt(pow((tempSat2.POS_X - POSITION_X2), 2) + pow((tempSat2.POS_Y - POSITION_Y2), 2) + pow((tempSat2.POS_Z - POSITION_Z2), 2));
			length_R2 = sqrt(pow((RefSat2.POS_X - POSITION_X2), 2) + pow((RefSat2.POS_Y - POSITION_Y2), 2) + pow((RefSat2.POS_Z - POSITION_Z2), 2));

			//��������ϵ��
			double Cof1 = (tempSat2.POS_X - POSITION_X2) / length_O2 - (RefSat2.POS_X - POSITION_X2) / length_R2;
			double Cof2 = (tempSat2.POS_Y - POSITION_Y2) / length_O2 - (RefSat2.POS_Y - POSITION_Y2) / length_R2;
			double Cof3 = (tempSat2.POS_Z - POSITION_Z2) / length_O2 - (RefSat2.POS_Z - POSITION_Z2) / length_R2;

			B[tw][0] = Cof1;
			B[tw][1] = Cof2;
			B[tw][2] = Cof3;         //α����ǰ

			B[tw + num_n][0] = Cof1;
			B[tw + num_n][1] = Cof2;
			B[tw + num_n][2] = Cof3;  //�ز��ں�
			B[tw + num_n][3 + tw] = lambda_L1;

			//������
			DDerror = Error_Correction(tempSat1, RefSat1, tempSat2, RefSat2);

			//˫����ʽ�ļ���������
			double DDgeo = (length_O2 - length_O1 - length_R2 + length_R1) + DDerror;
			double deltp1 = (tempSat2.data[0] - RefSat2.data[0]) - (tempSat1.data[0] - RefSat1.data[0]);  //α��
			double deltL1 = ((tempSat2.data[2] - RefSat2.data[2]) - (tempSat1.data[2] - RefSat1.data[2])) * lambda_L1; //�ز�

			L[tw][0] = deltp1 - DDgeo;         //α����ǰ
			L[tw + num_n][0] = deltL1 - DDgeo; //�ز��ں�

			for (int ii = 0; ii < SatNum[0]; ii++)
			{
				if (OldRefPRN[0] == SatPRN[0][ii])
				{
					Init = true;
					break;
				}
				else
					Init = false;
			}

			//���ɲο��Ǳ��޳������ʼ��ģ����
			if (!Init)
			{
				X[3 + tw][0] = (L[tw + num_n][0] - L[tw][0]) / lambda_L1;
				Q[3 + tw][3 + tw] = 100.0 * 100.0;
			}

			if (!bInit)
			{
				X[3 + tw][0] = (L[tw + num_n][0] - L[tw][0]) / lambda_L1;       //��ʼ��ģ����
			}

			for (int ii = 0; ii < NewSatNum[0]; ii++)
			{
				if (NewSatPRN[0][ii] == PRN)
				{
					X[3 + tw][0] = (L[tw + num_n][0] - L[tw][0]) / lambda_L1;   //��ʼ��ģ����
					Q[3 + tw][3 + tw] = 100.0 * 100.0;
					break;
				}
			}
			tw++;
		}

		//BDS
		tp = 0;  //��¼��ϵͳ������
		DDerror = 0;
		for (k = 0; k < BDSData.sat_num; k++)
		{
			bool Init = true;                      //��¼�Ƿ��оɲο��Ǳ��޳�
			int PRN = BDSData.sat1[k].numofsat;

			//�ο���������
			if (PRN == BDSRef)
			{
				continue;
			}

			if (ref_bds < 0)
			{
				assert(0);
			}

			Sat tempSat1 = BDSData.sat1[k];
			Sat RefSat1 = BDSData.sat1[ref_bds];
			Sat tempSat2 = BDSData.sat2[k];
			Sat RefSat2 = BDSData.sat2[ref_bds];

			double length_O1, length_R1, length_O2, length_R2;

			//�������ǵ���վ���루4�����룩
			length_O1 = sqrt(pow((tempSat1.POS_X - POSITION_X1), 2) + pow((tempSat1.POS_Y - POSITION_Y1), 2) + pow((tempSat1.POS_Z - POSITION_Z1), 2));
			length_R1 = sqrt(pow((RefSat1.POS_X - POSITION_X1), 2) + pow((RefSat1.POS_Y - POSITION_Y1), 2) + pow((RefSat1.POS_Z - POSITION_Z1), 2));

			length_O2 = sqrt(pow((tempSat2.POS_X - POSITION_X2), 2) + pow((tempSat2.POS_Y - POSITION_Y2), 2) + pow((tempSat2.POS_Z - POSITION_Z2), 2));
			length_R2 = sqrt(pow((RefSat2.POS_X - POSITION_X2), 2) + pow((RefSat2.POS_Y - POSITION_Y2), 2) + pow((RefSat2.POS_Z - POSITION_Z2), 2));

			//��������ϵ��
			double Cof1 = (tempSat2.POS_X - POSITION_X2) / length_O2 - (RefSat2.POS_X - POSITION_X2) / length_R2;
			double Cof2 = (tempSat2.POS_Y - POSITION_Y2) / length_O2 - (RefSat2.POS_Y - POSITION_Y2) / length_R2;
			double Cof3 = (tempSat2.POS_Z - POSITION_Z2) / length_O2 - (RefSat2.POS_Z - POSITION_Z2) / length_R2;

			//ϵ������
			B[tw][0] = Cof1;
			B[tw][1] = Cof2;
			B[tw][2] = Cof3;
			B[tw + num_n][0] = Cof1;
			B[tw + num_n][1] = Cof2;
			B[tw + num_n][2] = Cof3;
			B[tw + num_n][3 + tw] = lambda_L1_C;

			//������
			DDerror = Error_Correction(tempSat1, RefSat1, tempSat2, RefSat2);

			//˫����ʽ�ļ���������
			double DDgeo = (length_O2 - length_O1 - length_R2 + length_R1) + DDerror;
			double deltp1 = (tempSat2.data[0] - RefSat2.data[0]) - (tempSat1.data[0] - RefSat1.data[0]);  //α��
			double deltL1 = ((tempSat2.data[2] - RefSat2.data[2]) - (tempSat1.data[2] - RefSat1.data[2])) * lambda_L1_C;  //�ز�
			L[tw][0] = deltp1 - DDgeo;         //α����ǰ
			L[tw + num_n][0] = deltL1 - DDgeo; //�ز��ں�

			for (int ii = 0; ii < SatNum[1]; ii++)
			{
				if (OldRefPRN[1] == SatPRN[1][ii])
				{
					Init = true;
					break;
				}
				else
					Init = false;
			}

			//���ɲο��Ǳ��޳������ʼ��ģ����
			if (!Init)
			{
				X[3 + tw][0] = (L[tw + num_n][0] - L[tw][0]) / lambda_L1_C;
				Q[3 + tw][3 + tw] = 100.0 * 100.0;
			}

			if (!bInit)
			{
				X[3 + tw][0] = (L[tw + num_n][0] - L[tw][0]) / lambda_L1_C;   //��ʼ��ģ����
			}

			for (int ii = 0; ii < NewSatNum[1]; ii++)
			{
				if (NewSatPRN[1][ii] == PRN)
				{
					X[3 + tw][0] = (L[tw + num_n][0] - L[tw][0]) / lambda_L1_C;   //��ʼ��ģ����
					Q[3 + tw][3 + tw] = 100.0 * 100.0;
					break;
				}
			}
			tw++;
		}

		if (GPSData.sat_num >= 4 && BDSData.sat_num >= 4)
		{
			//Ȩ����PG�������̾���NG(�����Ϊ��������)���������(��Ը�������ĸ�����)

			//Bϵ������  L�۲�ֵ����  R������  Qw��̬��������  QЭ�������  Xλ�ú�����ģ����

			KalmanFilter(B, L, R, Qw, Q, X);

			double ratio = 1.0;

			CMatrix aXb(3, 1);
			CMatrix fix(num_n, 1);
			Resamb_LAMBDA(Q, X, aXb, ratio, fix);

			//�ָ������Ƶ�����ֵ
			CMatrix X_POS(3, 1);
			X_POS[0][0] = POSITION_X2 - aXb[0][0];
			X_POS[1][0] = POSITION_Y2 - aXb[1][0];
			X_POS[2][0] = POSITION_Z2 - aXb[2][0];

			//��ʵ����д�ɾ���
			CMatrix XT(3, 1);
			XT[0][0] = P_X2;
			XT[1][0] = P_Y2;
			XT[2][0] = P_Z2;

			//��׼ȷֵ����õ�XYZ����ϵ�µ����
			CMatrix ErrorX = X_POS - XT;

			//�����ͶӰ��NEU����������ϵ
			CMatrix ErrorX_NEU = TT * ErrorX;

			fprintf(fp, "EpNum: %2d, %2d, %2d, %10.6f, %2d, %2d,%15.8g,%15.8g, %15.8g, %15.8g\n", j + 1, tempData2.hour, tempData2.minute, tempData2.second, GPSData.sat_num, BDSData.sat_num, ratio,
				ErrorX_NEU[0][0], ErrorX_NEU[1][0], ErrorX_NEU[2][0]);
		}

		for (int idx = 0; idx < 2; idx++)
		{
			OldRefPRN[idx] = RefPRN[idx];
			for (int ii = 0; ii < OldSatNum[idx]; ii++)
			{
				OldSatPRN[idx][ii] = 0;
			}
			OldSatNum[idx] = SatNum[idx];
			for (int ii = 0; ii < SatNum[idx]; ii++)
			{
				OldSatPRN[idx][ii] = SatPRN[idx][ii];
			}
			for (int ii = 0; ii < SatNum[idx]; ii++)
			{
				SatPRN[idx][ii] = 0;
			}
			SatNum[idx] = 0;
			for (int ii = 0; ii < NewSatNum[idx]; ii++)
			{
				NewSatPRN[idx][ii] = 0;
			}
			NewSatNum[idx] = 0;
		}
		bInit = true;  //��ʼ���ɹ�
	}
	fclose(fp);
	return true;
}

