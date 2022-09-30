#pragma once
#include <vector>      //����
#include <string>      //�ַ���
#include "Matrix.h"

constexpr double C_light = 299792458.458;
constexpr double PI = 3.14159265359;

//GPS�ź�Ƶ��
#define  f1G 1575.42E+6
#define  f2G 1227.60E+6
#define  f5G 1176.45E+6

//BDS-2�ź�Ƶ��
#define  f1C 1561.098E+6
#define  f2C 1207.14E+6
#define  f3C 1268.52E+6

//BDS-3�ź�Ƶ��
#define  f1B 1561.098E+6
#define  f2B 1176.45E+6     //B2aƵ��
#define  f3B 1268.52E+6
#define  f4B 1575.42E+6     //B1cƵ��

//Galileo�ź�Ƶ��
#define f1E 1575.42E+6
#define f2E 1176.45E+6      //E5aƵ��
#define f3E 1207.14E+6      //E5bƵ��
#define f4E 1191.795E+6     //E5a+bƵ��

#define FREQ1       1.57542E9               /* L1/E1  frequency (Hz) */
#define FREQ2       1.22760E9               /* L2     frequency (Hz) */
#define FREQ5       1.17645E9               /* L5/E5a frequency (Hz) */
#define FREQ6       1.27875E9               /* E6/LEX frequency (Hz) */
#define FREQ7       1.20714E9               /* E5b    frequency (Hz) */
#define FREQ8       1.191795E9              /* E5a+b  frequency (Hz) */
#define FREQ9       2.492028E9              /* S      frequency (Hz) */
#define FREQ1_GLO   1.60200E9               /* GLONASS G1 base frequency (Hz) */
#define DFRQ1_GLO   0.56250E6               /* GLONASS G1 bias frequency (Hz/n) */
#define FREQ2_GLO   1.24600E9               /* GLONASS G2 base frequency (Hz) */
#define DFRQ2_GLO   0.43750E6               /* GLONASS G2 bias frequency (Hz/n) */
#define FREQ3_GLO   1.202025E9              /* GLONASS G3 frequency (Hz) */
#define FREQ1_BDS   1.561098E9              /* BeiDou B1 frequency (Hz) */
#define FREQ2_BDS   1.20714E9               /* BeiDou B2 frequency (Hz) */
#define FREQ3_BDS   1.26852E9               /* BeiDou B3 frequency (Hz) */

//�����г����߽���
#define lambda_longwideC1 (C_light / (f3C-f2C))             //BDS2�����0��-1��1������
#define lambda_longwideC2 (C_light/ (f1C+4*f2C-5*f3C))      //BDS2�����1��4��-5������
#define lambda_wideC (C_light / (f1C-f2C))                  //BDS2���1��-1��0������

#define yita0_11 (f1C*f1C*(1/f3C-1/f2C)/(f3C-f2C))         //BDS2��0��-1��1���ز�������ӳ�����
#define yita1_10 (f1C*f1C*(1/f1C-1/f2C)/(f1C-f2C))         //BDS2��1��-1��0���ز�������ӳ�����

#define lambda_longwideB1 (C_light / (f3B-f2B))              //BDS3�����0��1��-1����B1I,B3I,B2a������
#define lambda_longwideB2 (C_light / (f1B-4*f3B+3*f2B))      //BDS3�����1��-4��3����B1I,B3I,B2a������
#define lambda_wideB (C_light / (f1B-f3B))                   //BDS3���1��-1��0����B1I,B3I,B2a������

#define yita01_1 (f1B*f1B*(1/f3B-1/f2B)/(f3B-f2B))         //BDS3��0��1��-1���ز�������ӳ�����
#define yita10_1 (f1B*f1B*(1/f1B-1/f2B)/(f1B-f2B))         //BDS3��1��0��-1���ز�������ӳ�����

#define lambda_L1    (C_light / (FREQ1))		//GPS L1����
#define lambda_L2    (C_light / (FREQ2))		//GPS L2����

#define lambda_L1_C    (C_light / (FREQ1_BDS))   //BDS-2 B1����
#define lambda_L2_C   (C_light / (FREQ2_BDS))    //BDS-2 B2����
#define lambda_L3_C   (C_light / (FREQ3_BDS))    //BDS-2 B3����

#define lambda_L1_B     (C_light / (f1B))    //BDS-3 B1����
#define lambda_L2_B   (C_light / (f2B))      //BDS-3 B2a����
#define lambda_L3_B   (C_light/ (f3B))       //BDS-3 B3����

#define lambda_L1_E     (C_light / (f1E))    //Galileo E1����
#define lambda_L2_E   (C_light / (f2E))     //Galileo E5a����

#define lambda_L_wide     (C_light / (FREQ1 - FREQ2))   //GPS���ﲨ��
#define lambda_L_narrow   (C_light / (FREQ1 + FREQ2))   //GPSխ�ﲨ��

#define lambda_L_wide_C     (C_light / (FREQ1_BDS - FREQ2_BDS))   //BDS-2 ���ﲨ��
#define lambda_L13_wide_C    (C_light / (FREQ1_BDS - FREQ3_BDS))  //BDS-2 13Ƶ���ﲨ��

#define lambda_L_narrow_C    (C_light / (FREQ1_BDS + FREQ2_BDS))  //BDS-2 խ�ﲨ��
#define lambda_L13_narrow_C   (C_light / (FREQ1_BDS + FREQ3_BDS)) //BDS-2 13Ƶխ�ﲨ��

#define lambda_L_wide_B     (C_light / (f1B - f2B))     //BDS-3 ���ﲨ��
#define lambda_L_narrow_B     (C_light / (f1B + f2B))   //BDS-3 խ�ﲨ��

#define lambda_L13_wide_B   (C_light/ (f1B-f3B))        //BDS-3 13Ƶ���ﲨ��
#define lambda_L13_narrow_B   (C_light / (f1B+f3B))     //BDS-3 13Ƶխ�ﲨ��

#define lambda_L_wide_E     (C_light / (f1E - f2E))    //Galileo ���ﲨ��
#define lambda_L_narrow_E     (C_light / (f1E + f2E))  //Galileo խ�ﲨ��

//GPS��BDS���������ݽṹ���ļ�����׺***.**p��
struct Ephemeris_CN_epoch
{
	int PRN;
	int year;
	int month;
	int day;
	int hour;
	int minute;
	double second;
	double GPSTIME;
	double TTLSEC;
	double a0;
	double a1;
	double a2;
	double IDOE;
	double Crs;
	double delta_n;
	double M0;
	double Cuc;
	double e;
	double Cus;
	double sqrtA;
	double TOE;
	double Cic;
	double OMEGA;
	double Cis;
	double i0;
	double Crc;
	double w;
	double OMEGA_DOT;
	double i_DOT;
	double code_L2;
	double gps_week;
	double mark_code_L2;
	double pre_sat;                   //����
	double hel_sat;                   //����״̬
	double TGD;
	double IODC;
	double time_sig_send;             //���ķ���ʱ��
	int doy;                          //����գ����ã�
};

//�������ݽṹ
struct Sat
{
	std::string sattype;             //���ǵ�����
	int numofsat;                    //���ǵ���ţ�PRN��
	double data[20];                 //���ǵĹ۲�ֵ
	double GPSTIME;
	double ttlsec;


	double TGD;                      //TGD����
	double a0;                       //�Ӳ����ϵ��1
	double a1;                       //�Ӳ����ϵ��2
	double a2;                       //�Ӳ����ϵ��3

	double tk;                       //���������ڵ������ʱ��

	double deltt;
	double POS_X;                    //����λ��X
	double POS_Y;                    //����λ��Y
	double POS_Z;                    //����λ��Z

	double r;                        //�������վ�����
	double A;                        //��λ��
	double E;                        //�߶Ƚ�

	int posk;                        //������Ԫ��־
	int health;                      //������־
	int system;                      //����ϵͳ
	int judge_use;                   //�����Ա�־

	double xdl_t;                    //�����ЧӦ��Ӱ�죨ʱ�� s��
	double trop;                     //�������ӳ�1

	double fre_glo;

	//�����
	double Sat_clock;                //�����Ӳ�
	double Trop_Delay;               //�������ӳ�2
	double Trop_Map;                 //������ʪ�ӳ�ͶӰ
	double Relat;                    //�����
	double Sagnac;                   //������ת
	double Tide_Effect;              //��ϫЧӦ
	double Antenna_Height;
	double Sat_Antenna;              //����������λ���ĸ���
	double OffsetL1;                 //L1��λƫ�PCO+PCV��
	double OffsetL2;                 //L2��λƫ�PCO+PCV��
	double Windup;                   //��λ����
};

//O�ļ���Ԫ���ݽṹ
struct  Obs_epoch
{
	int liyuan_num;                  //��Ԫ���
	int year;                        //��
	int month;                       //��
	int day;                         //��
	int hour;                        //ʱ
	int minute;                      //��
	double second;                   //��
	double GPSTIME;                  //GPSʱ��
	double ttlsec;                   //����GPSʱ��ԭ�����ʱ��
	int dayofy;                      //����գ��ڶ���������ʱ���õ�
	int flag;                        //������־
	int sat_num;                     //����Ԫ��������
	std::vector<Sat> sat;
	std::vector<Sat> sat1;           //��ֶ�λ�У����Ӳ�������
	std::vector<Sat> sat2;           //��ֶ�λ�У����Ӳ�������
	double ZHD;                      //�춥�������ӳ٣��ɣ�
	double ZTD;                      //�춥�������ӳ٣��ܣ�
	double RClk[8];                  //���ջ��ӲG/C/R/E/S/J
	int JClk[8];                     //�жϽ��ջ��ӲG/C/R/E/S/J

	double posX;
	double posY;
	double posZ;

	bool ifOK;                       //������Ԫ��ʼ���㶨λ�Ƿ����ɹ�
};

//O�ļ�ͷ���ݽṹ
struct Ofileheader
{
	std::string marker_name;         //��վ����
	double APP_X;                    //�ļ�ͷ�еĸ�������X
	double APP_Y;                    //�ļ�ͷ�еĸ�������Y
	double APP_Z;                    //�ļ�ͷ�еĸ�������Z

	//��γ�ȼ��߳�
	double B;
	double L;
	double DH;

	std::string ANT_TYPE;            //���ջ���������
	double PCO1[3];                  //���ݽ��ջ���������ͨ�����������б�ƥ���POC������Ƶ��1��
	double PCO2[3];                  //���ݽ��ջ���������ͨ�����������б�ƥ���POC������Ƶ��2��
	std::string REC_TYPE;            //���ջ�����
	double H, E, N;                  //�ļ�ͷ�е�����ƫ��
	int FIRST_TIME[6];               //��ʼʱ��
	int LAST_TIME[6];                //����ʱ��
	double INTERVAL;                 //�������
	int LEAPSEC;                     //����
};

struct observe_spp
{
	std::string marker_name;         //��վ����
	double INTERVAL;                 //�������
	double APP_X;                    //��վ��������
	double APP_Y;
	double APP_Z;
	double L;
	double B;
	double DH;
	int liyuan_num;
	std::vector<Obs_epoch> epoch;
};
