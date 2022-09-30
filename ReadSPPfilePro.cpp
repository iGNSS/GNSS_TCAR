#include "stdafx.h"
#include <fstream>     //�ļ��� 
#include <string>      //�ַ���
#include <vector>      //����
#include "Myheader.h"        //���ݽṹ
#include "Matrix.h"          //������
#include <assert.h>          //���ԣ�����������
using namespace std;

#define READ_TXT 0

//�����ⲿ������������ReadOfilePro.cpp�ж���
void OnXYZtoBLH(double X, double Y, double Z, double BLH[]);

//�����ⲿGetGPSTime��������ReadPfile.cpp�ж���
double GetGPSTime(int year, int month, int day, int hour, int minute, double second, int& dayofy);

//�������ܣ���Ա����Զ����spp�ļ���ʽ�����޸�
bool ReadSPP_KIN(string filename, observe_spp& sppfile)
{
	fstream infile;
	infile.open(filename, ios::in);
	if (!infile)
	{
		printf("�ļ���ʧ��");
		return false;
	}
#if READ_TXT
	fstream infileTxt;
	infileTxt.open("snfx_SPP.txt", ios::in);
	if (!infileTxt)
	{
		printf("�ļ���ʧ��");
		return false;
	}
#endif
	string strs;
	string str2;

	double pi = 3.14159265;
	double aa = 6378137.000, bb = 6356752.3142, ee = 0.006694379990;

	do
	{
		getline(infile, strs);
		if (strs.substr(4, 11) == "MARKER_NAME")
		{
			str2 = strs.substr(17, 4);
			sppfile.marker_name = str2;
			continue;
		}
		
		if (strs.substr(7, 8) == "INTERVAL")
		{
			str2 = strs.substr(17, 3);
			sppfile.INTERVAL = atof(str2.c_str());
			continue;
		}

		if (strs.substr(0, 15) == "APPROX_POSITION")
		{
			str2 = strs.substr(17, 15);
			sppfile.APP_X = atof(str2.c_str());
			str2 = strs.substr(33, 15);
			sppfile.APP_Y = atof(str2.c_str());
			str2 = strs.substr(49, 15);
			sppfile.APP_Z = atof(str2.c_str());

			//****�����վ�������**********
			double BLH[3] = {0};
			OnXYZtoBLH(sppfile.APP_X, sppfile.APP_Y, sppfile.APP_Z, BLH);

			sppfile.B = BLH[0];
			sppfile.L = BLH[1];
			sppfile.DH = BLH[2];

			continue;
		}
	} while (strs.substr(5, 10) != "Clock type");                           //ͷ�ļ���ȡ����

	getline(infile, strs);//��ȡGLONASS���Ǳ�־��
	
	//������ȡ3�У�����ֱ�Ӷ�ȡ���۲�����
	getline(infile, strs);
	getline(infile, strs);
	getline(infile, strs);

	//��ʼ��ȡÿ����Ԫ������
	int i = 0;
	while (getline(infile, strs))
	{
		Obs_epoch temp_epochp;
		temp_epochp.liyuan_num = i;

		str2 = strs.substr(17, 3);
		temp_epochp.sat_num = atoi(str2.c_str());

		//ʱ����Ϣ
		int ye, mo, da, ho, mi, se;
		double second;
		str2 = strs.substr(43, 4);
		ye = atoi(str2.c_str());
		str2 = strs.substr(48, 2);
		mo = atoi(str2.c_str());
		str2 = strs.substr(51, 2);
		da = atoi(str2.c_str());
		str2 = strs.substr(54, 2);
		ho = atoi(str2.c_str());
		str2 = strs.substr(57, 2);
		mi = atoi(str2.c_str());
		str2 = strs.substr(60, 10);
		second = atoi(str2.c_str());
		se = floor(second + 0.5);

		temp_epochp.year = ye;
		temp_epochp.month = mo;
		temp_epochp.day = da;
		temp_epochp.hour = ho;
		temp_epochp.minute = mi;
		temp_epochp.second = (double)se;

		//�����
		int dayofy;

		double gpstime = GetGPSTime(ye, mo, da, ho, mi, se, dayofy);
		temp_epochp.GPSTIME = gpstime;

		//��ȡ��Ԫ������Ϣ
		str2 = strs.substr(75, 8);
		temp_epochp.ZTD = atof(str2.c_str());

#if	READ_TXT
		string tmpStr, str3;
		getline(infileTxt, tmpStr);
		str3 = tmpStr.substr(5, 14);
		temp_epochp.posX = atof(str3.c_str());

		str3 = tmpStr.substr(20, 12);
		temp_epochp.posY = atof(str3.c_str());

		str3 = tmpStr.substr(33, 13);
		temp_epochp.posZ = atof(str3.c_str());
#else
		str2 = strs.substr(85, 14);
		temp_epochp.posX = atof(str2.c_str());

		str2 = strs.substr(101, 14);
		temp_epochp.posY = atof(str2.c_str());

		str2 = strs.substr(117, 14);
		temp_epochp.posZ = atof(str2.c_str());
#endif


		for (int k = 0; k<temp_epochp.sat_num; k++)
		{
			Sat temp_sat;
			getline(infile, strs);

			str2 = strs.substr(0, 1);
			temp_sat.sattype = str2.c_str();

			str2 = strs.substr(1, 2);
			temp_sat.numofsat = atoi(str2.c_str());

			str2 = strs.substr(4, 1);
			temp_sat.judge_use = atoi(str2.c_str());

			//���ǵ�λ��
			str2 = strs.substr(6, 15);
			temp_sat.POS_X = atof(str2.c_str());
			str2 = strs.substr(22, 15);
			temp_sat.POS_Y = atof(str2.c_str());
			str2 = strs.substr(38, 15);
			temp_sat.POS_Z = atof(str2.c_str());

			//�����ӡ��߶Ƚǡ���λ��
			str2 = strs.substr(54, 15);
			temp_sat.Sat_clock = atof(str2.c_str());
			str2 = strs.substr(70, 15);
			temp_sat.E = atof(str2.c_str());
			str2 = strs.substr(86, 15);
			temp_sat.A = atof(str2.c_str());

			//α����ز��۲�ֵ��˫Ƶ4����
			str2 = strs.substr(102, 15);
			temp_sat.data[0] = atof(str2.c_str());
			str2 = strs.substr(118, 15);
			temp_sat.data[1] = atof(str2.c_str());
			str2 = strs.substr(134, 15);
			temp_sat.data[2] = atof(str2.c_str());
			str2 = strs.substr(150, 15);
			temp_sat.data[3] = atof(str2.c_str());

			//BDS-2,GPS��Ƶ�۲�ֵ
			if (temp_sat.sattype == "C" || temp_sat.sattype == "G")
			{
				str2 = strs.substr(326, 15);
				temp_sat.data[4] = atof(str2.c_str());
				str2 = strs.substr(342, 15);
				temp_sat.data[5] = atof(str2.c_str());
			}

			//GLONASS���ǵ�FDMAƵ�ʺ�
			if (temp_sat.sattype == "R")
			{
				str2 = strs.substr(326, 15);
				temp_sat.fre_glo = atof(str2.c_str());
			}

			//Galileo��BDS-3����Ƶ����Ƶ�۲�ֵ������
			if ((temp_sat.sattype == "E" || temp_sat.sattype == "B") && strs.length() > 380)
			{
				str2 = strs.substr(326, 15);
				temp_sat.data[4] = atof(str2.c_str());
				str2 = strs.substr(342, 15);
				temp_sat.data[5] = atof(str2.c_str());
				str2 = strs.substr(358, 15);
				temp_sat.data[6] = atof(str2.c_str());
				str2 = strs.substr(374, 15);
				temp_sat.data[7] = atof(str2.c_str());
			}

			//����������
			str2 = strs.substr(166, 15);
			temp_sat.Trop_Delay = atof(str2.c_str());  //���ӳ٣���б����ͶӰ��
			str2 = strs.substr(182, 15);
			temp_sat.Trop_Map = atof(str2.c_str());
			str2 = strs.substr(198, 15);
			temp_sat.Relat = atof(str2.c_str());
			str2 = strs.substr(214, 15);
			temp_sat.Sagnac = atof(str2.c_str());
			str2 = strs.substr(230, 15);
			temp_sat.TGD = atof(str2.c_str());
			str2 = strs.substr(246, 15);
			temp_sat.Antenna_Height = atof(str2.c_str());
			str2 = strs.substr(262, 15);
			temp_sat.Sat_Antenna = atof(str2.c_str());
			str2 = strs.substr(278, 15);
			temp_sat.OffsetL1 = atof(str2.c_str());
			str2 = strs.substr(294, 15);
			temp_sat.OffsetL2 = atof(str2.c_str());
			str2 = strs.substr(310, 15);
			temp_sat.Windup = atof(str2.c_str());

			temp_epochp.sat.push_back(temp_sat);
		}

		temp_epochp.sat_num = temp_epochp.sat.size();
		sppfile.epoch.push_back(temp_epochp);
		temp_epochp.sat.clear();
		printf("��Ԫ%d����\n", i+1);
		i++;
	}//while
	sppfile.liyuan_num = i;
	infile.close();	
#if READ_TXT
	infileTxt.close();
#endif
	printf("sppվ��%s��ȡ����, ����Ԫ����%5d\n", sppfile.marker_name.c_str(), sppfile.liyuan_num);
}

//�������ܣ��Ի�վ������վ���ǹ۲����ݽ��й��Ӵ���
void EpochCommonPro(Obs_epoch tempData1, Obs_epoch tempData2, Obs_epoch &useData)
{
	double CutEle = 10;
	vector<int> SatPRN_GPS;
	vector<int> SatPRN_BDS2;
	vector<int> SatPRN_BDS3;
	vector<int> SatPRN_Galileo;

	int PRNGPS[32] = { 0 };
	int PRNBDS2[20] = { 0 };
	int PRNBDS3[40] = { 0 };
	int PRNGalileo[40] = { 0 };

	for (auto& i : tempData1.sat)
	{
		if (i.sattype == "G" && i.E > CutEle && i.judge_use == 0 && i.data[0] > 0 && i.data[1] > 0 && i.data[2] > 0 && i
			.data[3] > 0 && i.data[4] > 0 && i.data[5] > 0)
			PRNGPS[i.numofsat - 1] += 1;
		if (i.sattype == "C" && i.E > CutEle && i.judge_use == 0 && i.data[0] > 0 && i.data[1] > 0 && i.data[2] > 0 && i
			.data[3] > 0 && i.data[4] > 0 && i.data[5] > 0)
			PRNBDS2[i.numofsat - 1] += 1;
		if (i.sattype == "B" && i.E > CutEle && i.judge_use == 0 && i.data[0] > 0 && i.data[1] > 0 && i.data[2] > 0 && i
			.data[3] > 0 && i.data[4] > 0 && i.data[5] > 0)
			PRNBDS3[i.numofsat - 1] += 1;
		if (i.sattype == "E" && i.E > CutEle && i.judge_use == 0 && i.data[0] > 0 && i.data[1] > 0 && i.data[2] > 0 && i
			.data[3] > 0 && i.data[4] > 0 && i.data[5] > 0)
			PRNGalileo[i.numofsat - 1] += 1;
	}
	for (auto& i : tempData2.sat)
	{
		if (i.sattype == "G" && i.E > CutEle && i.judge_use == 0 && i.data[0] > 0 && i.data[1] > 0 && i.data[2] > 0 && i
			.data[3] > 0 && i.data[4] > 0 && i.data[5] > 0)
			PRNGPS[i.numofsat - 1] += 1;
		if (i.sattype == "C" && i.E > CutEle && i.judge_use == 0 && i.data[0] > 0 && i.data[1] > 0 && i.data[2] > 0 && i
			.data[3] > 0 && i.data[4] > 0 && i.data[5] > 0)
			PRNBDS2[i.numofsat - 1] += 1;
		if (i.sattype == "B" && i.E > CutEle && i.judge_use == 0 && i.data[0] > 0 && i.data[1] > 0 && i.data[2] > 0 && i
			.data[3] > 0 && i.data[4] > 0 && i.data[5] > 0)
			PRNBDS3[i.numofsat - 1] += 1;
		if (i.sattype == "E" && i.E > CutEle && i.judge_use == 0 && i.data[0] > 0 && i.data[1] > 0 && i.data[2] > 0 && i
			.data[3] > 0 && i.data[4] > 0 && i.data[5] > 0)
			PRNGalileo[i.numofsat - 1] += 1;
	}

	int i = 0;
	for (i = 0; i < 32; i++)
		if (PRNGPS[i] == 2)
			SatPRN_GPS.push_back(i + 1);
	for (i = 0; i < 20; i++)
		if (PRNBDS2[i] == 2)
			SatPRN_BDS2.push_back(i + 1);
	for (i = 0; i < 40; i++)
		if (PRNBDS3[i] == 2)
			SatPRN_BDS3.push_back(i + 1);
	for (i = 0; i < 40; i++)
		if (PRNGalileo[i] == 2)
			SatPRN_Galileo.push_back(i + 1);


	for (i = 0; i < 32; i++)
	{
		if (PRNGPS[i] == 2)
		{
			for (auto& j : tempData1.sat)
			{
				if (j.numofsat == i + 1 && j.sattype == "G")
				{
					useData.sat1.push_back(j);
					break;
				}
			}
		}
	}

	for (i = 0; i < 20; i++)
	{
		if (PRNBDS2[i] == 2)
		{
			for (auto& j : tempData1.sat)
			{
				if (j.numofsat == i + 1 && j.sattype == "C")
				{
					useData.sat1.push_back(j);
					break;
				}
			}
		}
	}

	for (i = 0; i < 40; i++)
	{
		if (PRNBDS3[i] == 2)
		{
			for (auto& j : tempData1.sat)
			{
				if (j.numofsat == i + 1 && j.sattype == "B")
				{
					useData.sat1.push_back(j);
					break;
				}
			}
		}
	}

	for (i = 0; i < 40; i++)
	{
		if (PRNGalileo[i] == 2)
		{
			for (auto& j : tempData1.sat)
			{
				if (j.numofsat == i + 1 && j.sattype == "E")
				{
					useData.sat1.push_back(j);
					break;
				}
			}
		}
	}

	int TotSatNum = SatPRN_GPS.size() + SatPRN_BDS2.size() + SatPRN_BDS3.size() + SatPRN_Galileo.size();

	if (useData.sat1.size() != TotSatNum)
		assert(0);

	for (size_t k = 0; k < useData.sat1.size(); k++)
	{
		for (auto& j : tempData2.sat)
		{
			if (j.sattype == useData.sat1[k].sattype && j.numofsat == useData.sat1[k].numofsat)
			{
				useData.sat2.push_back(j);
				break;
			}
		}
	}

	//ƥ����
	if (useData.sat1.size() != useData.sat2.size())
		assert(0);

	for (size_t k = 0; k < useData.sat1.size(); k++)
	{
		if (useData.sat1[k].sattype != useData.sat2[k].sattype)
			assert(0);
		if (useData.sat1[k].numofsat != useData.sat2[k].numofsat)
			assert(0);
	}

	useData.liyuan_num = tempData2.liyuan_num;
	useData.GPSTIME = tempData2.GPSTIME;
	useData.sat_num = useData.sat1.size();
}

//�������ܣ�ѡ��ο�����
void SelectRef(Obs_epoch tempData, int &RefNo)
{
	bool iffind = false;
	//���ڶ���Ԫ������λ�ж�
	if (RefNo > 0)
	{
		for (int i = 0; i < tempData.sat_num; i++)
		{
			if (tempData.sat1[i].numofsat == RefNo && tempData.sat1[i].E > 30 && tempData.sat1[i].judge_use == 0)
			{
				iffind = true;
				return;
			}
		}
	}
	//����Ԫ��λģʽ�������if����,ѡ�����߶Ƚ��������ο���
	if (iffind == false)
	{
		if (tempData.sat_num < 1)
		{
			printf("Error,SATNUM:%d\n", tempData.sat_num);
		}
		RefNo = -1;
		double max_ele = 10.0;
		for (int i = 0; i < tempData.sat_num; i++)
		{
			if (tempData.sat1[i].E >= max_ele && tempData.sat1[i].judge_use == 0)
			{
				RefNo = tempData.sat1[i].numofsat;
				max_ele = tempData.sat1[i].E;
			}
		}
	}

	//���
	bool ifOk = false;
	for (int i = 0; i < tempData.sat_num; i++)
	{
		if (tempData.sat1[i].numofsat == RefNo && tempData.sat1[i].judge_use == 0)
		{
			ifOk = true;
			break;
		}
	}

	if (ifOk == false)
	{
		assert(0);
	}
}

//�������ܣ������������ɻ�ȡ��������
void GetDDR_Peo(Obs_epoch GPSData, vector<int> useprn, int GPSRef, CMatrix &R)
{
	//R�����趨
	int numgps = GPSData.sat_num;
	if (numgps != useprn.size())
		assert(0);
	int numgps_x = numgps - 1;

	CMatrix TTR(numgps_x, numgps);
	CMatrix RT(numgps, numgps);
	int t = 0;
	int k = 0;
	for (k = 0; k < numgps; k++)
	{
		int match_O = -1;
		int PRN = useprn[k];
		for (int m = 0; m < GPSData.sat_num; m++)
		{
			if (GPSData.sat2[m].numofsat == PRN)
			{
				match_O = m;
				break;
			}
		}

		RT[k][k] = pow((0.3 + 0.3*exp(-GPSData.sat2[match_O].E / 10.0)), 2) + pow((0.3 + 0.3*exp(-GPSData.sat1[match_O].E / 10.0)), 2);

		if (PRN != GPSRef)
		{
			TTR[t][k] = 1.0;
			t++;
		}
		else
		{
			for (int m = 0; m < numgps_x; m++)
			{
				TTR[m][k] = -1.0;
			}
		}
	}

	R.first(numgps_x, numgps_x);
	//TTR�����Ŀ���Ǹ����������ɣ������Ǽ���ģʽ�µ���������ʽ
	R = TTR * RT * TTR.T();
}

//�������ܣ���ֶ�λ����
bool SPP_DPOS_Pro(string filename1, string filename2)
{
	//Ĭ�ϵ�һ��վΪ��׼վ���ڶ���վΪ����վ
	observe_spp sppfile1;
	observe_spp sppfile2;
	ReadSPP_KIN(filename1, sppfile1);
	ReadSPP_KIN(filename2, sppfile2);

	//ͶӰ���󣺶���������ϵ���˴���ֱ�Ӳ���ͷ�ļ����꣩
	CMatrix TT(3, 3);
	TT[0][0] = -sin(sppfile2.B)*cos(sppfile2.L);
	TT[0][1] = -sin(sppfile2.B)*sin(sppfile2.L);
	TT[0][2] = cos(sppfile2.B);

	TT[1][0] = -sin(sppfile2.L);
	TT[1][1] = cos(sppfile2.L);
	TT[1][2] = 0;

	TT[2][0] = cos(sppfile2.B)*cos(sppfile2.L);
	TT[2][1] = cos(sppfile2.B)*sin(sppfile2.L);
	TT[2][2] = sin(sppfile2.B);


	//�����ֶ�λ�����
	string name = filename2.substr(0, 4) + "_DPOS.txt";
	const char* output_filename = name.c_str();
	FILE *fp;
	fopen_s(&fp, output_filename, "w");

	//��������վƥ���׼վ��Ӧ��Ԫ����
	int posk = 0;   //���Լ�¼��վ���ݵ�ƥ��
	for (int j = 0; j < sppfile2.liyuan_num; j++)   //����վ����Ԫ����
	{
		Obs_epoch tempData;

		//����վ��ǰʱ��
		Obs_epoch tempData2 = sppfile2.epoch[j];

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

		//�ж�ʱ�������Ƿ��������20s
		if (fabs(sppfile1.epoch[i].GPSTIME - tempData2.GPSTIME) >= 20)
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

		//��ֶ�λ���Ӵ���
		EpochCommonPro(tempData1, tempData2, tempData);

		//����GPS��BDS���ǣ����ں�����ϵͳ��˫ϵͳ����
		Obs_epoch GPSData;
		Obs_epoch BDSData;

		int k = 0;
		int m = 0;
		for (m = 0; m < tempData.sat_num; m++)
		{
			if (tempData.sat1[m].sattype == "G")
			{
				GPSData.sat1.push_back(tempData.sat1[m]);
				GPSData.sat2.push_back(tempData.sat2[m]);
			}

			if (tempData.sat1[m].sattype == "C")
			{
				BDSData.sat1.push_back(tempData.sat1[m]);
				BDSData.sat2.push_back(tempData.sat2[m]);
			}
		}

		GPSData.sat_num = GPSData.sat1.size();
		BDSData.sat_num = BDSData.sat1.size();
		tempData.GPSTIME = tempData2.GPSTIME;

		//��ʱ����GPS��BDS������4�����ǵ����ݣ�����Ӧ��ʱһ��������Ӵ���5�ż���
		if (GPSData.sat_num < 4 || BDSData.sat_num < 4)
		{
			printf("GPS��BDS������С��4��\n");
			continue;
		}

		//����վ���Ǽ���β��ģ�ͣ�ѡ��ο�����
		int GPSRef = -1;
		int BDSRef = -2;
		SelectRef(GPSData, GPSRef);
		SelectRef(BDSData, BDSRef);

		//��ǰ��ԪGPS��BDS��������
		int PRN_X_G[32];
		int PRN_X_B[20];

		memset(PRN_X_G, 0, sizeof(PRN_X_G));
		memset(PRN_X_B, 0, sizeof(PRN_X_B));

		for (k = 0; k < GPSData.sat_num; k++)
			PRN_X_G[k] = GPSData.sat1[k].numofsat;
		for (k = 0; k < BDSData.sat_num; k++)
			PRN_X_B[k] = BDSData.sat1[k].numofsat;

		int num_X_G = GPSData.sat_num;
		int num_X_B = BDSData.sat_num;

		int numgps_x = num_X_G - 1;         //GPS˫��̸���
		int numbds_x = num_X_B - 1;         //BDS˫��̸���
		int num_x_p = numgps_x + numbds_x;  //˫ϵͳ�ں�˫��̸���

		//��λ������صľ���
		//1-˫ϵͳ
		CMatrix B(num_x_p, 3);       //ϵ������
		CMatrix L(num_x_p, 1);       //�۲�ֵ����
		CMatrix R(num_x_p, num_x_p); //����������ΪȨ����

		//2-��GPS
		CMatrix BG(numgps_x, 3);       //ϵ������
		CMatrix LG(numgps_x, 1);       //�۲�ֵ����
		CMatrix RG(numgps_x, numgps_x); //����������ΪȨ����

		//3-��BDS
		CMatrix BC(numbds_x, 3);       //ϵ������
		CMatrix LC(numbds_x, 1);       //�۲�ֵ����
		CMatrix RC(numbds_x, numbds_x); //����������ΪȨ����

		//��ȡGPS��BDS��������

		vector<int> GPS_PRN;
		vector<int> BDS_PRN;

		for (k = 0; k < GPSData.sat_num; k++)
			GPS_PRN.push_back(PRN_X_G[k]);
		for (k = 0; k < BDSData.sat_num; k++)
			BDS_PRN.push_back(PRN_X_B[k]);

		GetDDR_Peo(GPSData, GPS_PRN, GPSRef, RG);
		GetDDR_Peo(BDSData, BDS_PRN, BDSRef, RC);

		//�ں�ϵͳ��R���󣬷ֿ�ԽǾ���
		for (int k1 = 0; k1 < numgps_x; k1++)
			for (int k2 = 0; k2 < numgps_x; k2++)
			{
				R[k1][k2] = RG[k1][k2];
			}

		for (int k1 = 0; k1 < numbds_x; k1++)
			for (int k2 = 0; k2 < numbds_x; k2++)
			{
				R[k1 + numgps_x][k2 + numgps_x] = RC[k1][k2];
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
		for (k = 0; k < GPSData.sat_num; k++)
		{
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

			//ϵ������
			BG[tp][0] = Cof1;
			BG[tp][1] = Cof2;
			BG[tp][2] = Cof3;

			B[tw][0] = Cof1;
			B[tw][1] = Cof2;
			B[tw][2] = Cof3;

			//����������
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
			double DDgeo = (length_O2 - length_O1 - length_R2 + length_R1) + (test_geo2 - Rtest_geo2) - (test_geo1 - Rtest_geo1);
			double deltp1 = (tempSat2.data[0] - RefSat2.data[0]) - (tempSat1.data[0] - RefSat1.data[0]);

			L[tw][0] = deltp1 - DDgeo;
			LG[tp][0] = deltp1 - DDgeo;

			tp++;
			tw++;
		}

		//BDS
		tp = 0;  //��¼��ϵͳ������
		for (k = 0; k < BDSData.sat_num; k++)
		{
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
			BC[tp][0] = Cof1;
			BC[tp][1] = Cof2;
			BC[tp][2] = Cof3;

			B[tw][0] = Cof1;
			B[tw][1] = Cof2;
			B[tw][2] = Cof3;

			//����������
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
			double DDgeo = (length_O2 - length_O1 - length_R2 + length_R1) + (test_geo2 - Rtest_geo2) - (test_geo1 - Rtest_geo1);
			double deltp1 = (tempSat2.data[0] - RefSat2.data[0]) - (tempSat1.data[0] - RefSat1.data[0]);

			L[tw][0] = deltp1 - DDgeo;
			LC[tp][0] = deltp1 - DDgeo;

			tp++;
			tw++;
		}

		//��С���˽���
		//��GPS
		if (GPSData.sat_num >= 4 && BDSData.sat_num >= 4)
		{
			//Ȩ����PG�������̾���NG(�����Ϊ��������)���������(��Ը�������ĸ�����)
			CMatrix PG = RG.InvertGaussJordan();
			CMatrix NG = BG.T()*PG*BG;
			CMatrix XG = NG.InvertGaussJordan() * BG.T()*PG*LG;

			CMatrix PC = RC.InvertGaussJordan();
			CMatrix NC = BC.T()*PC*BC;
			CMatrix XC = NC.InvertGaussJordan() * BC.T()*PC*LC;

			CMatrix P = R.InvertGaussJordan();
			CMatrix N = B.T()*P*B;
			CMatrix X = N.InvertGaussJordan() * B.T()*P*L;

			//�ָ������Ƶ�����ֵ
			CMatrix XG_POS(3, 1); XG_POS[0][0] = POSITION_X2 - XG[0][0]; XG_POS[1][0] = POSITION_Y2 - XG[1][0]; XG_POS[2][0] = POSITION_Z2 - XG[2][0];
			CMatrix XC_POS(3, 1); XC_POS[0][0] = POSITION_X2 - XC[0][0]; XC_POS[1][0] = POSITION_Y2 - XC[1][0]; XC_POS[2][0] = POSITION_Z2 - XC[2][0];
			CMatrix X_POS(3, 1);  X_POS[0][0] = POSITION_X2 - X[0][0];   X_POS[1][0] = POSITION_Y2 - X[1][0];   X_POS[2][0] = POSITION_Z2 - X[2][0];

			//��ʵ����д�ɾ���
			CMatrix XT(3, 1);  XT[0][0] = P_X2; XT[1][0] = P_Y2; XT[2][0] = P_Z2;

			//��׼ȷֵ����õ�XYZ����ϵ�µ����
			CMatrix ErrorXG = XG_POS - XT;
			CMatrix ErrorXC = XC_POS - XT;
			CMatrix ErrorX = X_POS - XT;

			//�����ͶӰ��NEU����������ϵ
			CMatrix ErrorXG_NEU = TT * ErrorXG;
			CMatrix ErrorXC_NEU = TT * ErrorXC;
			CMatrix ErrorX_NEU = TT * ErrorX;

			fprintf(fp, "EpNum: %2d, %2d, %2d, %10.6f, %2d, %2d, %15.8g, %15.8g, %15.8g, %15.8g, %15.8g, %15.8g, %15.8g, %15.8g, %15.8g\n", j + 1, tempData2.hour, tempData2.minute, tempData2.second, GPSData.sat_num, BDSData.sat_num, 
				                                         ErrorXG_NEU[0][0], ErrorXG_NEU[1][0], ErrorXG_NEU[2][0], ErrorXC_NEU[0][0], ErrorXC_NEU[1][0], ErrorXC_NEU[2][0], ErrorX_NEU[0][0], ErrorX_NEU[1][0], ErrorX_NEU[2][0]);
		}
	}
	fclose(fp);
	return true;
}