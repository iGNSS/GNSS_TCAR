#include <fstream>     //�ļ��� 
#include <string>      //�ַ���
#include <vector>      //����
#include "Myheader.h"

//�����������ݣ�p�ļ������洢
bool ReadPfile(std::string Filename, std::vector <Ephemeris_CN_epoch> &EpochNG, std::vector <Ephemeris_CN_epoch> &EpochNC, std::vector <Ephemeris_CN_epoch> &EpochNB, std::vector < Ephemeris_CN_epoch> &EpochNE);

//����۲����ݣ�o�ļ�����������Ӧ������͵��㶨λs
bool ReadOfileProcess(std::string Filename, std::vector <Ephemeris_CN_epoch> &EpochNG, std::vector <Ephemeris_CN_epoch> &EpochNC, std::vector <Ephemeris_CN_epoch> &EpochNB, std::vector < Ephemeris_CN_epoch>& EpochNE);

//��������spp�ļ����в�ֶ�λ���㣬�����齨�Լ�����ʱ�������ȡ����o�ļ�ֱ�Ӵ���
bool SPP_DPOS_Pro(std::string filename1, std::string filename2);

//��������spp�ļ�����RTK����
bool SPP_Kinematic_Pro(std::string filename1, std::string filename2);

//��������spp�ļ����ж�ϵͳRTK����
bool MSMF_RTK(std::string filename1, std::string filename2);

//��������spp�ļ����ж�ϵͳRTK���㣨��BDS3�⣩
bool trisys_RTK(std::string filename1, std::string filename2);

//�������ܣ�TCAR���̶�ģ����,���ü������ģ��,��׼վ
void TCAR_fix(std::string& filename1, std::string& filename2);

//�������ܣ�TCAR���̶�ģ����,���ü������ģ��,����վ
void TCAR_move(std::string& filename1, std::string& filename2);

//�������ܣ�TCAR���̶�ģ����,���ü������ģ��,����վ,��������
void TCAR_test(std::string& filename1, std::string& filename2);