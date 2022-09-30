#include "stdafx.h"
#include <fstream>     //�ļ��� 
#include <string>      //�ַ���
#include <vector>      //����
#include "Myheader.h"
using namespace std;

double GetGPSTime(int year, int month, int day, int hour, int minute, double second, int& dayofy);

//�������ܣ������ļ���ȡ����ֻ֧��p�ļ���ȡ��
bool ReadPfile(string Filename, vector <Ephemeris_CN_epoch> &EpochNG, vector <Ephemeris_CN_epoch> &EpochNC, vector <Ephemeris_CN_epoch> &EpochNB, vector <Ephemeris_CN_epoch> &EpochNE)
{
	fstream infile;
	infile.open(Filename, ios::in);
	if (!infile)
	{
		printf("�ļ���ʧ��");
		return false;
	}

	string strs;
	string str2;

	//�����ļ�ͷ
	do
	{
		getline(infile, strs);
	} while (strs.substr(60, 13) != "END OF HEADER");

	while (getline(infile, strs))
	{
		string SatType = strs.substr(0, 1);
		if (SatType == "G" || SatType == "C" || SatType == "E")
		{
			Ephemeris_CN_epoch temp_epoch;

			//���ǵ�PRN��
			str2 = strs.substr(1, 2);
			temp_epoch.PRN = atoi(str2.c_str());

			str2 = strs.substr(3, 5);
			temp_epoch.year = atoi(str2.c_str());
			temp_epoch.year = temp_epoch.year;

			str2 = strs.substr(8, 3);
			temp_epoch.month = atoi(str2.c_str());

			str2 = strs.substr(11, 3);
			temp_epoch.day = atoi(str2.c_str());

			str2 = strs.substr(14, 3);
			temp_epoch.hour = atoi(str2.c_str());

			str2 = strs.substr(17, 3);
			temp_epoch.minute = atoi(str2.c_str());

			str2 = strs.substr(20, 3);
			temp_epoch.second = atof(str2.c_str());

			//���������ռ���GPSʱ��
			temp_epoch.GPSTIME = GetGPSTime(temp_epoch.year, temp_epoch.month, temp_epoch.day, temp_epoch.hour, temp_epoch.minute, temp_epoch.second, temp_epoch.doy);
			double GPSTIME_YMD = temp_epoch.GPSTIME;

			str2 = strs.substr(23, 19);
			temp_epoch.a0 = atof(str2.c_str());

			str2 = strs.substr(42, 19);
			temp_epoch.a1 = atof(str2.c_str());

			str2 = strs.substr(61, 19);
			temp_epoch.a2 = atof(str2.c_str());

			//1�㲥���
			getline(infile, strs);//��һ��
			str2 = strs.substr(4, 19);
			temp_epoch.IDOE = atof(str2.c_str());
			str2 = strs.substr(23, 19);
			temp_epoch.Crs = atof(str2.c_str());
			str2 = strs.substr(42, 19);
			temp_epoch.delta_n = atof(str2.c_str());
			str2 = strs.substr(61, 19);
			temp_epoch.M0 = atof(str2.c_str());

			//2
			getline(infile, strs);
			str2 = strs.substr(4, 19);
			temp_epoch.Cuc = atof(str2.c_str());
			str2 = strs.substr(23, 19);
			temp_epoch.e = atof(str2.c_str());
			str2 = strs.substr(42, 19);
			temp_epoch.Cus = atof(str2.c_str());
			str2 = strs.substr(61, 19);
			temp_epoch.sqrtA = atof(str2.c_str());

			//3
			getline(infile, strs);
			str2 = strs.substr(4, 19);
			temp_epoch.TOE = atof(str2.c_str());

			//����Toe����GPSʱ�䣬���μ������ڱȽ�
			temp_epoch.GPSTIME = temp_epoch.TOE;

			str2 = strs.substr(23, 19);
			temp_epoch.Cic = atof(str2.c_str());
			str2 = strs.substr(42, 19);
			temp_epoch.OMEGA = atof(str2.c_str());
			str2 = strs.substr(61, 19);
			temp_epoch.Cis = atof(str2.c_str());

			//4
			getline(infile, strs);
			str2 = strs.substr(4, 19);
			temp_epoch.i0 = atof(str2.c_str());
			str2 = strs.substr(23, 19);
			temp_epoch.Crc = atof(str2.c_str());
			str2 = strs.substr(42, 19);
			temp_epoch.w = atof(str2.c_str());
			str2 = strs.substr(61, 19);
			temp_epoch.OMEGA_DOT = atof(str2.c_str());

			//5
			getline(infile, strs);
			str2 = strs.substr(4, 19);
			temp_epoch.i_DOT = atof(str2.c_str());
			str2 = strs.substr(23, 19);
			temp_epoch.code_L2 = atof(str2.c_str());
			str2 = strs.substr(42, 19);
			temp_epoch.gps_week = atof(str2.c_str());
			str2 = strs.substr(61, 19);
			temp_epoch.mark_code_L2 = atof(str2.c_str());

			//6
			getline(infile, strs);
			str2 = strs.substr(4, 19);
			temp_epoch.pre_sat = atof(str2.c_str());
			str2 = strs.substr(23, 19);
			temp_epoch.hel_sat = atof(str2.c_str());
			str2 = strs.substr(42, 19);
			temp_epoch.TGD = atof(str2.c_str());
			str2 = strs.substr(61, 19);
			temp_epoch.IODC = atof(str2.c_str());

			//7
			getline(infile, strs);
			str2 = strs.substr(4, 19);
			temp_epoch.time_sig_send = atof(str2.c_str());

			if (fabs(temp_epoch.GPSTIME - GPSTIME_YMD) >= 1.0)
				continue;

			//GPS
			if (SatType == "G" && fabs(temp_epoch.hel_sat) < 0.5)
				EpochNG.push_back(temp_epoch);

			//BDS-2
			if (SatType == "C" && fabs(temp_epoch.hel_sat) < 0.5 && temp_epoch.PRN <= 16)
				EpochNC.push_back(temp_epoch);

			//BDS-3
			if (SatType == "C" && fabs(temp_epoch.hel_sat) < 0.5 && temp_epoch.PRN > 16)
				EpochNB.push_back(temp_epoch);

			//Galileo
			if (SatType == "E" && fabs(temp_epoch.hel_sat) < 0.5)
				EpochNE.push_back(temp_epoch);

			continue;
		}
		else
			continue;
	}
	infile.close();
	printf("�������ݶ�����ɣ�\n");
	return true;
}

//��̬���飺�洢12���µ�����
static  int  dinmth[13] = { 0, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };

//�������ܣ�GPSʱ��ת����ת�����ں�ʱ��ΪGPSʱ
double GetGPSTime(int year, int month, int day, int hour, int minute, double second, int& dayofy)
{
	int dayofw, yr, ttlday, m, weekno;
	double gpstime;
	dayofy = 0;

	//�쳣��ݴ���
	if (year < 2000)
		year = year + 2000;

	if (year > 4000)
		year = year - 2000;

	if (year < 1981 || month < 1 || month > 12 || day < 1 || day > 31)
		weekno = 0;

	if (month == 1)
		dayofy = day;
	else
	{
		dayofy = 0;
		for (m = 1; m <= (month - 1); m++)
		{
			dayofy += dinmth[m];
			if (m == 2)
			{
				if (year % 4 == 0 && year % 100 != 0 || year % 400 == 0)
					dayofy += 1;
			}
		}
		dayofy += day;
	}

	ttlday = 360;
	for (yr = 1981; yr <= (year - 1); yr++)
	{
		ttlday += 365;
		if (yr % 4 == 0 && yr % 100 != 0 || yr % 400 == 0)
			ttlday += 1;
	}
	ttlday += dayofy;
	weekno = ttlday / 7;                                                 //������
	dayofw = ttlday - 7 * weekno;                                        //����һ�ܵ�����
	gpstime = double(hour) * 3600 + double(minute) * 60 + second + double(dayofw) * 86400;       //����������ʱ������

	return gpstime;
}