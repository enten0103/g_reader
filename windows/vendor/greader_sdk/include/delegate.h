#pragma once
#ifndef __DELEGATE_H__
#define __DELEGATE_H__


typedef struct
{
	unsigned char Epc[1024]; //16����EPC�ַ���
	unsigned char Pc[5]; //PCֵ, 16�����ַ���
	unsigned char AntId; //���߱��
	unsigned char Rssi; //�ź�ǿ��
	unsigned char Result; //��ǩ��ȡ��� 0��Ϊ��ȡ�ɹ�����0Ϊʧ�� 1����ǩ����Ӧ 2��CRC���� 3�������������� 4����������� 5������������� 6��������ǩ���� 7��������д������
	unsigned char Tid[512]; //16����TID�ַ���
	unsigned char Userdata[1024]; //16����Userdata�ַ���
	unsigned char Reserved[512]; //16���Ʊ������ַ���
	unsigned char SubAntId;	//�����ߺ�
	int ReadUtcSecTime;	//UTCʱ����
	int ReadUtcMicroSecTime;	//UTCʱ��΢��
	int Point;	//��ǰƵ��
	unsigned char Phase;	//��ǰ��ǩ��λ
	unsigned char EpcBank[1024];	//EPC������
	int CtesiusLtu27;	//CTESIUS LTU27�¶ȴ�������
	int CtesiusLtu31;	//CTESIUS LTU31�¶ȴ�������
	int Quanray;	//�����¶ȴ�������
	short RSSI_dBm;
	unsigned char CRC[3];
	int ReplySerialNumber;	//��ǩӦ������
	char readerSerialNumber[21];	//�豸���к�
}LogBaseEpcInfo;
typedef void(__stdcall* delegateTagEpcLog)(char* readerName, LogBaseEpcInfo msg);
typedef struct {
	char readerSerialNumber[21];	//�豸���к�
}LogBaseEpcOver;
typedef void(__stdcall* delegateTagEpcOver)(char* readerName, LogBaseEpcOver msg);

typedef struct {
	unsigned char Tid[18];
	unsigned char AntId;
	unsigned char Rssi;
	unsigned char Result;
	unsigned char Userdata[512];
	char readerSerialNumber[21];	//�豸���к�
}LogBase6bInfo;
typedef void(__stdcall* delegateTag6bLog)(char* readerName, LogBase6bInfo msg);
typedef struct {
	char readerSerialNumber[21];	//�豸���к�
}LogBase6bOver;
typedef void(__stdcall* delegateTag6bOver)(char* readerName, LogBase6bOver msg);

typedef struct {
	unsigned char Epc[1024];
	unsigned char Pc[3];
	unsigned char AntId;
	unsigned char Rssi;
	unsigned char Result;
	unsigned char Tid[512];
	unsigned char Userdata[1024];
	char readerSerialNumber[21];	//�豸���к�
}LogBaseGbInfo;
typedef void(__stdcall* delegateTagGbLog)(char* readerName, LogBaseGbInfo msg);
typedef struct {
	char readerSerialNumber[21];	//�豸���к�
}LogBaseGbOver;
typedef void(__stdcall* delegateTagGbOver)(char* readerName, LogBaseGbOver msg);

typedef struct {
	unsigned char Epc[1024];
	unsigned char Pc[3];
	unsigned char AntId;
	unsigned char Rssi;
	unsigned char Result;
	unsigned char Tid[512];
	unsigned char Userdata[1024];
	char readerSerialNumber[21];	//�豸���к�
}LogBaseGjbInfo;
typedef void(__stdcall* delegateTagGjbLog)(char* readerName, LogBaseGjbInfo msg);
typedef struct {
	char readerSerialNumber[21];	//�豸���к�
}LogBaseGjbOver;
typedef void(__stdcall* delegateTagGjbOver)(char* readerName, LogBaseGjbOver msg);


typedef struct {
	int tagType;
	unsigned char Epc[1024];
	unsigned char Tid[1024];
	int AntId;
	int Rssi;
	unsigned char SubAntId;	//�����ߺ�
	int ReadUtcSecTime; //UTCʱ����
	int ReadUtcMicroSecTime; //UTCʱ��΢��
	int Point; //��ǰƵ��
	unsigned char Phase; //��ǰ��ǩ��λ
	int RSSI_dBm;
	int ReplySerialNumber;	//��ǩӦ������
	char readerSerialNumber[21]; //�豸���к�
}LogBaseTLInfo;
typedef void(__stdcall* delegateTagTLLog)(char* readerName, LogBaseTLInfo msg);
typedef struct {
	char readerSerialNumber[21]; //�豸���к�
}LogBaseTLOver;
typedef void(__stdcall* delegateTagTLOver)(char* readerName, LogBaseTLOver msg);

typedef struct {
	int tagType;
	unsigned char Epc[1024];
	int AntId;
	int Rssi;
	unsigned char SubAntId;	//�����ߺ�
	int ReadUtcSecTime; //UTCʱ����
	int ReadUtcMicroSecTime; //UTCʱ��΢��
	int Point; //��ǰƵ��
	unsigned char Phase; //��ǰ��ǩ��λ
	int RSSI_dBm;
	int ReplySerialNumber;	//��ǩӦ������
	char readerSerialNumber[21]; //�豸���к�
}LogBase6DInfo;
typedef void(__stdcall* delegateTag6DLog)(char* readerName, LogBase6DInfo msg);
typedef struct {
	char readerSerialNumber[21]; //�豸���к�
}LogBase6DOver;
typedef void(__stdcall* delegateTag6DOver)(char* readerName, LogBase6DOver msg);


typedef struct {
	unsigned char GpiPort;
	unsigned char Level;
	char TriggerTime[32];
	char readerSerialNumber[21];	//�豸���к�
}LogAppGpiStart;
typedef void(__stdcall* delegateGpiStart)(char* readerName, LogAppGpiStart msg);

typedef struct {
	unsigned char GpiPort;
	unsigned char Level;
	char TriggerTime[32];
	char readerSerialNumber[21];	//�豸���к�
}LogAppGpiOver;
typedef void(__stdcall* delegateGpiOver)(char* readerName, LogAppGpiOver msg);


typedef struct {
	unsigned char reason;
}LogTestWorkingModeInit;
typedef void(__stdcall* delegateWorkingModeInit)(char* readerName, LogTestWorkingModeInit msg);

typedef void(__stdcall* delegateTcpDisconnected)(char* readerName);

typedef void(__stdcall* delegateUsbHidRemoved)(char* readerName);

#endif // DELEGATE_H
