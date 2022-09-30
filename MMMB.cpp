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

//����ReadSPP_KIN��������ȡspp�ļ�����Ա����Զ����spp�ļ���ʽ�����޸ģ�
bool ReadSPP_KIN(string filename, observe_spp& sppfile);

//����OnCycleDetect����������̽��
void OnCycleDetect(Obs_epoch& tempEpoch, Obs_epoch& preEpoch);

//����EpochCommonPro�������Ի�վ������վ���ǹ۲����ݽ��й��Ӵ���
void EpochCommonPro(Obs_epoch tempData1, Obs_epoch tempData2, Obs_epoch& useData);

//����SelectRef������ѡ��ο�����
void SelectRef(Obs_epoch tempData, int& RefNo);

//����InitState��������ʼ��״̬����
void InitState(CMatrix& QX);

//����GetGRCQValue����������ת������
bool GetGRCQValue(int PRN_X[], int PRN_old[], int num_sat, int num_sat_old, int Ref, int Ref_old, CMatrix& TT_Matrix);

//����GetDDR_Peo�����������������ɻ�ȡ��������α�ࣩ
void GetDDR_Peo(Obs_epoch GPSData, vector<int> useprn, int GPSRef, CMatrix& R);

//����GetDDR_Leo�����������������ɻ�ȡ���������ز���
void GetDDR_Leo(Obs_epoch GPSData, vector<int> useprn, int GPSRef, CMatrix& R);

//����Error_Correction������������
double Error_Correction(Sat& tempSat1, Sat& RefSat1, Sat& tempSat2, Sat& RefSat2);

//����ProGRCTrans������ƴ�Ӿ���
void ProGRCTrans(CMatrix& TT_G, CMatrix& TT_C, CMatrix& TT_B, CMatrix& TT_E, CMatrix& TT_ALL)
{
	int Row_G = TT_G.Row;
	int Col_G = TT_G.Col;
	int Row_C = TT_C.Row;
	int Col_C = TT_C.Col;
	int Row_B = TT_B.Row;
	int Col_B = TT_B.Col;
	int Row_E = TT_E.Row;
	int Col_E = TT_E.Col;

	int Tot_Row = Row_G + Row_C + Row_B + Row_E;
	int Tot_Col = Col_G + Col_C + Col_B + Col_E;
	TT_ALL.SetSize(Tot_Row, Tot_Col);

	int i = 0;
	int j = 0;

	//GPS���ָ�ֵ���µ�ת������
	for (i = 0; i < Row_G; i++)
		for (j = 0; j < Col_G; j++)
			TT_ALL[i][j] = TT_G[i][j];

	//BDS-2���ָ�ֵ���µ�ת������
	for (i = 0; i < Row_C; i++)
		for (j = 0; j < Col_C; j++)
			TT_ALL[Row_G + i][Col_G + j] = TT_C[i][j];

	//BDS-3���ָ�ֵ���µ�ת������
	for (i = 0; i < Row_B; i++)
		for (j = 0; j < Col_B; j++)
			TT_ALL[Row_G + Row_C + i][Col_G + Col_C + j] = TT_B[i][j];

	//Galileo���ָ�ֵ���µ�ת������
	for (i = 0; i < Row_E; i++)
		for (j = 0; j < Col_E; j++)
			TT_ALL[Row_G + Row_C + Row_B + i][Col_G + Col_C + Col_B + j] = TT_E[i][j];
}

//����CheckNewSats���������������
void CheckNewSats(int* SatNum, int* oldSatNum, int SatPRN[4][40], int oldSatPRN[4][40], int NewSatPRN[4][40], int* NewSatNum)
{
	for (int i = 0; i < 4; i++)
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

//����UpdataState����������Э����
void UpdataState(CMatrix& QX, CMatrix& X, int* SatNum, int* oldSatNum, int SatPRN[4][40], int oldSatPRN[4][40], int* RefSat, int* OldRef)
{
	CMatrix T[4], ALL_T, Matrix_t;
	int sys = 0, oldsys = 0, ALL_NUM, oldALL_NUM;

	if (SatNum[0] >= 2)
	{
		sys++;
	}
	if (SatNum[1] >= 2)
	{
		sys++;
	}
	if (SatNum[2] >= 2)
	{
		sys++;
	}
	if (SatNum[3] >= 2)
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
	if (oldSatNum[2] >= 2)
	{
		oldsys++;
	}
	if (oldSatNum[3] >= 2)
	{
		oldsys++;
	}

	ALL_NUM = SatNum[0] + SatNum[1] + SatNum[2] + SatNum[3] - sys;
	oldALL_NUM = oldSatNum[0] + oldSatNum[1] + oldSatNum[2] + oldSatNum[3] - oldsys;

	//L1����L1�仯����
	for (int i = 0; i < 4; i++)
	{
		if (SatNum[i] > 1)
		{
			if (oldSatNum[i] > 0)
			{
				T[i].SetSize(SatNum[i] - 1, oldSatNum[i] - 1);//��ǰ��ԪGPS����������һ��ԪGPS������
				GetGRCQValue(SatPRN[i], oldSatPRN[i], SatNum[i], oldSatNum[i], RefSat[i], OldRef[i], T[i]);//����ת������
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
	ProGRCTrans(T[0], T[1], T[2], T[3], ALL_T);//ƴ�Ӿ���

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

//����KalmanFilter���������˲�
void KalmanFilter(CMatrix& B, CMatrix& L, CMatrix& R, CMatrix& Qw, CMatrix& Q, CMatrix& X);

//����Resamb_LAMBDA������Lambda�㷨�̶�ģ����
void Resamb_LAMBDA(CMatrix& Q, CMatrix& X, CMatrix& aXb, double& ratio, CMatrix& fixX);

//�������ܣ�ͨ����ȡspp�ļ������ж�ϵͳRTK����
bool MSMF_RTK(string filename1, string filename2)
{
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
	string name = filename2.substr(0, 3) + "_MSMF_RTK.txt";
	const char* output_filename = name.c_str();
	FILE* fp3;
	fopen_s(&fp3, output_filename, "w");

	int OldSatPRN[4][40] = { 0 };
	int OldRefPRN[4] = { -1,-1,-1,-1 };
	int OldSatNum[4] = { 0 };

	CMatrix Q, X;

	//��������վƥ���׼վ��Ӧ��Ԫ����
	int posk = 0;   //���Լ�¼��վ���ݵ�ƥ��
	//����վ����Ԫ����
	for (int i = 0; i < sppfile2.liyuan_num; i++)
	{
		Obs_epoch tempData;

		//����վ��ǰʱ��
		Obs_epoch tempData2 = sppfile2.epoch[i];

		if (i >= 1)
		{
			Obs_epoch preData2 = sppfile2.epoch[i - 1];
			OnCycleDetect(tempData2, preData2);     //�жϴ˴���Ԫ����һ����Ԫ�е������ز�����ֵ�Ƿ�������,����������ͽ�����Ԫ�е����Ǽ�Ϊ������
		}

		//��׼վ������Ӧʱ�̱�����ǰ
		int j = 0;
		for (j = posk; j < sppfile1.liyuan_num; j++)
		{
			if (sppfile1.epoch[j].GPSTIME <= tempData2.GPSTIME)
				continue;
			else
				break;
		}

		//����һ����Ԫ��posk��ΪҪ�ҵ��������Ԫ������posk��ʱ��С��tempData2��ʱ��
		if (j > 0) posk = --j;
		else
		{
			posk = j;
			continue;
		}

		//�ж�ʱ�������Ƿ��������2s
		if (fabs(sppfile1.epoch[j].GPSTIME - tempData2.GPSTIME) >= 2)
		{
			printf("δƥ�䵽��վ����\n");
			continue;
		}

		//�����׼վ�����ͺ�
		if (sppfile1.epoch[j].GPSTIME > tempData2.GPSTIME)
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

		//����GPS��BDS-2��BDS-3��Galileo���ǣ����ں�����ϵͳ����ϵͳ����
		Obs_epoch GPSdata;
		Obs_epoch BDS2data;
		Obs_epoch BDS3data;
		Obs_epoch Galileodata;

		int SatPRN[4][40] = { 0 };		//��ǰ��Ԫ���Ǻ� ���δ洢
		int SatNum[4] = { 0 };	//��ǰ��Ԫ��ϵͳ������Ŀ
		int RefPRN[4] = { 0 };  //��ǰ��Ԫ��ϵͳ�ο����Ǻ�

		int NewSatNum[4] = { 0 };
		int NewSatPRN[4][40] = { 0 };

		int k = 0;
		int m = 0;
		int gpsnum, bds2num, bds3num, galileonum;
		gpsnum = bds2num = bds3num = galileonum = 0;
		for (m = 0; m < tempData.sat_num; m++)
		{
			if (tempData.sat1[m].sattype == "G")
			{
				GPSdata.sat1.push_back(tempData.sat1[m]);
				GPSdata.sat2.push_back(tempData.sat2[m]);
				SatPRN[0][gpsnum] = tempData.sat1[m].numofsat;
				gpsnum++;
			}

			if (tempData.sat1[m].sattype == "C")
			{
				BDS2data.sat1.push_back(tempData.sat1[m]);
				BDS2data.sat2.push_back(tempData.sat2[m]);
				SatPRN[1][bds2num] = tempData.sat1[m].numofsat;
				bds2num++;
			}

			if (tempData.sat1[m].sattype == "B")
			{
				BDS3data.sat1.push_back(tempData.sat1[m]);
				BDS3data.sat2.push_back(tempData.sat2[m]);
				SatPRN[2][bds3num] = tempData.sat1[m].numofsat;
				bds3num++;
			}

			if (tempData.sat1[m].sattype == "E")
			{
				Galileodata.sat1.push_back(tempData.sat1[m]);
				Galileodata.sat2.push_back(tempData.sat2[m]);
				SatPRN[3][galileonum] = tempData.sat1[m].numofsat;
				galileonum++;
			}
		}

		//������������
		SatNum[0] = gpsnum;
		SatNum[1] = bds2num;
		SatNum[2] = bds3num;
		SatNum[3] = galileonum;

		GPSdata.sat_num = GPSdata.sat1.size();
		BDS2data.sat_num = BDS2data.sat1.size();
		BDS3data.sat_num = BDS3data.sat1.size();
		Galileodata.sat_num = Galileodata.sat1.size();
		tempData.GPSTIME = tempData2.GPSTIME;

		//����������Ҫ4��
		if ((GPSdata.sat_num + BDS2data.sat_num + BDS3data.sat_num + Galileodata.sat_num) < 4)
		{
			printf("����������4��");
			bInit = false;
			continue;
		}

		//��ϵͳ����Ҫ2������,���ֲο��Ǻ���ͨ��
		if (GPSdata.sat_num < 2)
		{
			printf("GPS����������2��\n");
			bInit = false;
			continue;
		}
		if (BDS2data.sat_num < 2)
		{
			printf("BDS2����������2��\n");
			bInit = false;
			continue;
		}
		if (BDS3data.sat_num < 2)
		{
			printf("BDS3����������2��\n");
			bInit = false;
			continue;
		}
		if (Galileodata.sat_num < 2)
		{
			printf("Galileo����������2��\n");
			bInit = false;
			continue;
		}

		//����վ���Ǽ���β��ģ�ͣ�ѡ��ο�����
		int GPSref = -1;
		int BDS2ref = -2;
		int BDS3ref = -3;
		int Galref = -4;

		GPSref = OldRefPRN[0];
		BDS2ref = OldRefPRN[1];
		BDS3ref = OldRefPRN[2];
		Galref = OldRefPRN[3];

		//ѡ�����߶Ƚ�������Ϊ�ο����ǣ���ͬ�������Ƿֿ���
		SelectRef(GPSdata, GPSref);
		SelectRef(BDS2data, BDS2ref);
		SelectRef(BDS3data, BDS3ref);
		SelectRef(Galileodata, Galref);

		RefPRN[0] = GPSref;  //GPS�ο��������
		RefPRN[1] = BDS2ref; //BDS-2�ο��������
		RefPRN[2] = BDS3ref; //BDS-3�ο��������
		RefPRN[3] = Galref;	 //Galileo�ο��������

		//��ǰ��ԪGPS��BDS-2��BDS-3��Galileo��������
		int PRN_X_G[32];
		int PRN_X_B2[20];
		int PRN_X_B3[40];
		int PRN_X_E[40];

		//���г�ʼ��
		memset(PRN_X_G, 0, sizeof(PRN_X_G));
		memset(PRN_X_B2, 0, sizeof(PRN_X_B2));
		memset(PRN_X_B3, 0, sizeof(PRN_X_B3));
		memset(PRN_X_E, 0, sizeof(PRN_X_E));

		//������Ԫ�е���������
		for (k = 0; k < GPSdata.sat_num; k++)
			PRN_X_G[k] = GPSdata.sat1[k].numofsat;
		for (k = 0; k < BDS2data.sat_num; k++)
			PRN_X_B2[k] = BDS2data.sat1[k].numofsat;
		for (k = 0; k < BDS3data.sat_num; k++)
			PRN_X_B3[k] = BDS3data.sat1[k].numofsat;
		for (k = 0; k < Galileodata.sat_num; k++)
			PRN_X_E[k] = Galileodata.sat1[k].numofsat;

		//�����Ǹ���
		int num_X_G = GPSdata.sat_num;
		int num_X_B2 = BDS2data.sat_num;
		int num_X_B3 = BDS3data.sat_num;
		int num_X_E = Galileodata.sat_num;

		//��ϵͳ˫��̸���
		int numgps_x = num_X_G - 1;
		int numbds2_x = num_X_B2 - 1;
		int numbds3_x = num_X_B3 - 1;
		int numgal_x = num_X_E - 1;

		//��ϵͳ�ں�˫��̸��� P1 + L1 �ز��۲ⷽ�̼�α��۲ⷽ��
		int num_x_p = (numgps_x + numbds2_x + numbds3_x + numgal_x) * 2;

		//����������������X����Y����Z��ÿ�����ǵ�ģ����
		int num_x = numgps_x + numbds2_x + numbds3_x + numgal_x + 3;

		//ģ���ȸ���
		int num_n = numgps_x + numbds2_x + numbds3_x + numgal_x;

		//��λ������صľ���
		//1-��ϵͳ
		CMatrix B(num_x_p, num_x);                //ϵ������  
		CMatrix L(num_x_p, 1);                    //�۲�ֵ����
		CMatrix R(num_x_p, num_x_p);              //����������ΪȨ����
		CMatrix Qw(num_x, num_x);                 //��̬������

		//2-��GPS
		CMatrix RG_P(numgps_x, numgps_x);         //����������ΪȨ���� α��
		CMatrix RG_L(numgps_x, numgps_x);         //����������ΪȨ���� �ز�

		//3-��BDS2
		CMatrix RC2_P(numbds2_x, numbds2_x);      //����������ΪȨ���� α��
		CMatrix RC2_L(numbds2_x, numbds2_x);      //����������ΪȨ���� �ز�

		//4-��BDS3
		CMatrix RC3_P(numbds3_x, numbds3_x);      //����������ΪȨ���� α��
		CMatrix RC3_L(numbds3_x, numbds3_x);      //����������ΪȨ���� �ز�

		//5-��Galileo
		CMatrix RE_P(numgal_x, numgal_x);         //����������ΪȨ���� α��
		CMatrix RE_L(numgal_x, numgal_x);         //����������ΪȨ���� �ز�

		//λ�õĶ�̬���������û��������ģ���Ȳ��䣬����ģ���ȵ�����Ϊ0��ֻ��λ������
		for (int idx = 0; idx < 3; idx++) {
			Qw[idx][idx] = 30.0 * 30.0;
		}

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

			//X.MyTRACE();

			UpdataState(Q, X, SatNum, OldSatNum, SatPRN, OldSatPRN, RefPRN, OldRefPRN);
		}

		//������������Ŵ洢����
		vector<int> GPS_PRN;
		vector<int> BDS2_PRN;
		vector<int> BDS3_PRN;
		vector<int> Gal_PRN;

		for (k = 0; k < GPSdata.sat_num; k++)
			GPS_PRN.push_back(PRN_X_G[k]);
		for (k = 0; k < BDS2data.sat_num; k++)
			BDS2_PRN.push_back(PRN_X_B2[k]);
		for (k = 0; k < BDS3data.sat_num; k++)
			BDS3_PRN.push_back(PRN_X_B3[k]);
		for (k = 0; k < Galileodata.sat_num; k++)
			Gal_PRN.push_back(PRN_X_E[k]);

		//α��
		GetDDR_Peo(GPSdata, GPS_PRN, GPSref, RG_P);
		GetDDR_Peo(BDS2data, BDS2_PRN, BDS2ref, RC2_P);
		GetDDR_Peo(BDS3data, BDS3_PRN, BDS3ref, RC3_P);
		GetDDR_Peo(Galileodata, Gal_PRN, Galref, RE_P);

		//�ز�
		GetDDR_Leo(GPSdata, GPS_PRN, GPSref, RG_L);
		GetDDR_Leo(BDS2data, BDS2_PRN, BDS2ref, RC2_L);
		GetDDR_Leo(BDS3data, BDS3_PRN, BDS3ref, RC3_L);
		GetDDR_Leo(Galileodata, Gal_PRN, Galref, RE_L);

		//�ں�ϵͳ��R���󣬷ֿ�ԽǾ���
		//α��
		for (int k1 = 0; k1 < numgps_x; k1++)
			for (int k2 = 0; k2 < numgps_x; k2++)
			{
				R[k1][k2] = RG_P[k1][k2];
			}

		for (int k1 = 0; k1 < numbds2_x; k1++)
			for (int k2 = 0; k2 < numbds2_x; k2++)
			{
				R[k1 + numgps_x][k2 + numgps_x] = RC2_P[k1][k2];
			}

		for (int k1 = 0; k1 < numbds3_x; k1++)
			for (int k2 = 0; k2 < numbds3_x; k2++)
			{
				R[k1 + numgps_x + numbds2_x][k2 + numgps_x + numbds2_x] = RC3_P[k1][k2];
			}

		for (int k1 = 0; k1 < numgal_x; k1++)
			for (int k2 = 0; k2 < numgal_x; k2++)
			{
				R[k1 + numgps_x + numbds2_x + numbds3_x][k2 + numgps_x + numbds2_x + numbds3_x] = RE_P[k1][k2];
			}
		//�ز�
		for (int k1 = 0; k1 < numgps_x; k1++)
			for (int k2 = 0; k2 < numgps_x; k2++)
			{
				R[k1 + numgps_x + numbds2_x + numbds3_x + numgal_x][k2 + numgps_x + numbds2_x + numbds3_x + numgal_x] = RG_L[k1][k2];
			}
		for (int k1 = 0; k1 < numbds2_x; k1++)
			for (int k2 = 0; k2 < numbds2_x; k2++)
			{
				R[k1 + numgps_x * 2 + numbds2_x + numbds3_x + numgal_x][k2 + numgps_x * 2 + numbds2_x + numbds3_x + numgal_x] = RC2_L[k1][k2];
			}
		for (int k1 = 0; k1 < numbds3_x; k1++)
			for (int k2 = 0; k2 < numbds3_x; k2++)
			{
				R[k1 + numgps_x * 2 + numbds2_x * 2 + numbds3_x + numgal_x][k2 + numgps_x * 2 + numbds2_x * 2 + numbds3_x + numgal_x] = RC3_L[k1][k2];
			}
		for (int k1 = 0; k1 < numgal_x; k1++)
			for (int k2 = 0; k2 < numgal_x; k2++)
			{
				R[k1 + numgps_x * 2 + numbds2_x * 2 + numbds3_x * 2 + numgal_x][k2 + numgps_x * 2 + numbds2_x * 2 + numbds3_x * 2 + numgal_x] = RE_L[k1][k2];
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
		//��ʼ��
		int ref_gps = -1;
		int ref_bds2 = -1;
		int ref_bds3 = -1;
		int ref_gal = -1;

		for (k = 0; k < GPSdata.sat_num; k++)
		{
			if (GPSdata.sat1[k].numofsat == GPSref)
			{
				ref_gps = k;
				break;
			}
		}

		for (k = 0; k < BDS2data.sat_num; k++)
		{
			if (BDS2data.sat1[k].numofsat == BDS2ref)
			{
				ref_bds2 = k;
				break;
			}
		}

		for (k = 0; k < BDS3data.sat_num; k++)
		{
			if (BDS3data.sat1[k].numofsat == BDS3ref)
			{
				ref_bds3 = k;
				break;
			}
		}

		for (k = 0; k < Galileodata.sat_num; k++)
		{
			if (Galileodata.sat1[k].numofsat == Galref)
			{
				ref_gal = k;
				break;
			}
		}

		int tp = 0;  //��¼��ϵͳ������
		int tw = 0;  //��¼˫ϵͳ������
		double DDerror = 0;  //��ʼ����������
		//GPS
		for (k = 0; k < GPSdata.sat_num; k++)
		{
			bool Init = true;                      //��¼�Ƿ��оɲο��Ǳ��޳�
			int PRN = GPSdata.sat1[k].numofsat;
			//�ο���������
			if (PRN == GPSref)
			{
				continue;
			}

			if (ref_gps < 0)
			{
				assert(0);
			}

			Sat tempSat1 = GPSdata.sat1[k];
			Sat RefSat1 = GPSdata.sat1[ref_gps];
			Sat tempSat2 = GPSdata.sat2[k];
			Sat RefSat2 = GPSdata.sat2[ref_gps];

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

		//BDS-2
		for (k = 0; k < BDS2data.sat_num; k++)
		{
			bool Init = true;                      //��¼�Ƿ��оɲο��Ǳ��޳�
			int PRN = BDS2data.sat1[k].numofsat;
			//�ο���������
			if (PRN == BDS2ref)
			{
				continue;
			}

			if (ref_bds2 < 0)
			{
				assert(0);
			}

			Sat tempSat1 = BDS2data.sat1[k];
			Sat RefSat1 = BDS2data.sat1[ref_bds2];
			Sat tempSat2 = BDS2data.sat2[k];
			Sat RefSat2 = BDS2data.sat2[ref_bds2];

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
				X[3 + tw][0] = (L[tw + num_n][0] - L[tw][0]) / lambda_L1_C;       //��ʼ��ģ����
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

		//BDS-3
		if (BDS3data.sat_num > 1)
		{
			for (k = 0; k < BDS3data.sat_num; k++)
			{
				bool Init = true;                      //��¼�Ƿ��оɲο��Ǳ��޳�
				int PRN = BDS3data.sat1[k].numofsat;
				//�ο���������
				if (PRN == BDS3ref)
				{
					continue;
				}

				if (ref_bds3 < 0)
				{
					assert(0);
				}

				Sat tempSat1 = BDS3data.sat1[k];
				Sat RefSat1 = BDS3data.sat1[ref_bds3];
				Sat tempSat2 = BDS3data.sat2[k];
				Sat RefSat2 = BDS3data.sat2[ref_bds3];

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
				B[tw + num_n][3 + tw] = lambda_L1_B;

				//������
				DDerror = Error_Correction(tempSat1, RefSat1, tempSat2, RefSat2);

				//˫����ʽ�ļ���������
				double DDgeo = (length_O2 - length_O1 - length_R2 + length_R1) + DDerror;
				double deltp1 = (tempSat2.data[0] - RefSat2.data[0]) - (tempSat1.data[0] - RefSat1.data[0]);  //α��
				double deltL1 = ((tempSat2.data[2] - RefSat2.data[2]) - (tempSat1.data[2] - RefSat1.data[2])) * lambda_L1_B;  //�ز�
				L[tw][0] = deltp1 - DDgeo;         //α����ǰ
				L[tw + num_n][0] = deltL1 - DDgeo; //�ز��ں�

				for (int ii = 0; ii < SatNum[2]; ii++)
				{
					if (OldRefPRN[2] == SatPRN[2][ii])
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
					X[3 + tw][0] = (L[tw + num_n][0] - L[tw][0]) / lambda_L1_B;
					Q[3 + tw][3 + tw] = 100.0 * 100.0;
				}

				if (!bInit)
				{
					X[3 + tw][0] = (L[tw + num_n][0] - L[tw][0]) / lambda_L1_B;       //��ʼ��ģ����
				}

				for (int ii = 0; ii < NewSatNum[2]; ii++)
				{
					if (NewSatPRN[2][ii] == PRN)
					{
						X[3 + tw][0] = (L[tw + num_n][0] - L[tw][0]) / lambda_L1_B;   //��ʼ��ģ����
						Q[3 + tw][3 + tw] = 100.0 * 100.0;
						break;
					}
				}
				tw++;
			}
		}

		//Galileo
		for (k = 0; k < Galileodata.sat_num; k++)
		{
			bool Init = true;                      //��¼�Ƿ��оɲο��Ǳ��޳�
			int PRN = Galileodata.sat1[k].numofsat;
			//�ο���������
			if (PRN == Galref)
			{
				continue;
			}

			if (ref_gal < 0)
			{
				assert(0);
			}

			Sat tempSat1 = Galileodata.sat1[k];
			Sat RefSat1 = Galileodata.sat1[ref_gal];
			Sat tempSat2 = Galileodata.sat2[k];
			Sat RefSat2 = Galileodata.sat2[ref_gal];

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
			B[tw + num_n][3 + tw] = lambda_L1_E;

			//������
			DDerror = Error_Correction(tempSat1, RefSat1, tempSat2, RefSat2);

			//˫����ʽ�ļ���������
			double DDgeo = (length_O2 - length_O1 - length_R2 + length_R1) + DDerror;
			double deltp1 = (tempSat2.data[0] - RefSat2.data[0]) - (tempSat1.data[0] - RefSat1.data[0]);  //α��
			double deltL1 = ((tempSat2.data[2] - RefSat2.data[2]) - (tempSat1.data[2] - RefSat1.data[2])) * lambda_L1_E;  //�ز�
			L[tw][0] = deltp1 - DDgeo;         //α����ǰ
			L[tw + num_n][0] = deltL1 - DDgeo; //�ز��ں�

			for (int ii = 0; ii < SatNum[3]; ii++)
			{
				if (OldRefPRN[3] == SatPRN[3][ii])
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
				X[3 + tw][0] = (L[tw + num_n][0] - L[tw][0]) / lambda_L1_E;
				Q[3 + tw][3 + tw] = 100.0 * 100.0;
			}

			if (!bInit)
			{
				X[3 + tw][0] = (L[tw + num_n][0] - L[tw][0]) / lambda_L1_E;       //��ʼ��ģ����
			}

			for (int ii = 0; ii < NewSatNum[3]; ii++)
			{
				if (NewSatPRN[3][ii] == PRN)
				{
					X[3 + tw][0] = (L[tw + num_n][0] - L[tw][0]) / lambda_L1_E;   //��ʼ��ģ����
					Q[3 + tw][3 + tw] = 100.0 * 100.0;
					break;
				}
			}
			tw++;
		}

		if ((GPSdata.sat_num + BDS2data.sat_num + BDS3data.sat_num + Galileodata.sat_num) >= 4)
		{
			//Ȩ����PG�������̾���NG(�����Ϊ��������)���������(��Ը�������ĸ�����)

			//Bϵ������  L�۲�ֵ����  R������  Qw��̬��������  QЭ�������  X��ʼ��ģ����

			//kalman�˲�
			KalmanFilter(B, L, R, Qw, Q, X);

			double ratio = 1.0;
			CMatrix aXb(3, 1);
			CMatrix fix(num_n, 1);

			//lambda�㷨�̶�ģ����
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

			fprintf(fp3, "EpNum: %2d, %2d, %2d, %10.6f, %2d, %2d, %2d, %2d, %15.8g,%15.8g, %15.8g, %15.8g\n", i + 1, tempData2.hour, tempData2.minute, tempData2.second,
				GPSdata.sat_num, BDS2data.sat_num, BDS3data.sat_num, Galileodata.sat_num, ratio, ErrorX_NEU[0][0], ErrorX_NEU[1][0], ErrorX_NEU[2][0]);
		}

		for (int idx = 0; idx < 4; idx++)
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
	fclose(fp3);
	return true;
}