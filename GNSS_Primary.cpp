// GNSS_Primary.cpp : �������̨Ӧ�ó������ڵ㡣
// GNSS_Primary��������ʵ��GNSS��׼���㶨λ��GPS+BDS˫ϵͳ��Ƶģʽ

#include "stdafx.h"
#include <string>      //�ַ���
#include <vector>      //����
#include "Function.h"
using namespace std;

//˵���˳���Ϊwin32���򣬽�������ѧϰʹ�ã�������������ΪMFC��ܳ��򣬿ɽ���Ҫ������ļ�·��ѡ���
int _tmain(int argc, _TCHAR* argv[])
{
	//����1�����ǻ�������ͱ�׼���㶨λ,ִ�й���1ʱ����������2�´���ע�͵�
	//����洢������������EpochNG��EpochNC��EpochNB
	
	//vector <Ephemeris_CN_epoch> EpochNG; 
	//vector <Ephemeris_CN_epoch> EpochNC;
	//vector <Ephemeris_CN_epoch> EpochNB;
	//vector <Ephemeris_CN_epoch> EpochNE;

	//string pFilename = "20210709p.rnx";  //Ĭ�ϵ�ǰ����·��
	//bool pfileread = ReadPfile(pFilename, EpochNG, EpochNC, EpochNB, EpochNE);
	//if (!pfileread)
	//	exit(1);

	//string oFilename = "20210709-2.21O";  //Ĭ�ϵ�ǰ����·��
	//bool ofileread = ReadOfileProcess(oFilename, EpochNG, EpochNC, EpochNB, EpochNE);

	//����2����������spp�ļ�����α���ֶ�λ,ִ�й���2ʱ����������1�´���ע�͵�
	string sppFilename1 = "SINA_error.txt";  //Ĭ�ϵ�ǰ����·��
	string sppFilename2 = "mov7_error.txt";  //Ĭ�ϵ�ǰ����·��
	//SPP_DPOS_Pro(sppFilename1, sppFilename2);
	//SPP_Kinematic_Pro(sppFilename1, sppFilename2);
	//MSMF_RTK(sppFilename1, sppFilename2);
	//TCAR_fix(sppFilename1, sppFilename2);
	//TCAR_move(sppFilename1, sppFilename2);
	TCAR_test(sppFilename1, sppFilename2);

	return 0;
}