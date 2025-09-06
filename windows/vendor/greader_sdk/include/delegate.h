#pragma once
#ifndef __DELEGATE_H__
#define __DELEGATE_H__


typedef struct
{
	unsigned char Epc[1024]; //16进制EPC字符串
	unsigned char Pc[5]; //PC值, 16进制字符串
	unsigned char AntId; //天线编号
	unsigned char Rssi; //信号强度
	unsigned char Result; //标签读取结果 0，为读取成功，非0为失败 1，标签无响应 2，CRC错误 3，数据区被锁定 4，数据区溢出 5，访问密码错误 6，其他标签错误 7，其他读写器错误
	unsigned char Tid[512]; //16进制TID字符串
	unsigned char Userdata[1024]; //16进制Userdata字符串
	unsigned char Reserved[512]; //16进制保留区字符串
	unsigned char SubAntId;	//子天线号
	int ReadUtcSecTime;	//UTC时间秒
	int ReadUtcMicroSecTime;	//UTC时间微秒
	int Point;	//当前频点
	unsigned char Phase;	//当前标签相位
	unsigned char EpcBank[1024];	//EPC区数据
	int CtesiusLtu27;	//CTESIUS LTU27温度传感数据
	int CtesiusLtu31;	//CTESIUS LTU31温度传感数据
	int Quanray;	//坤锐温度传感数据
	short RSSI_dBm;
	unsigned char CRC[3];
	int ReplySerialNumber;	//标签应答包序号
	char readerSerialNumber[21];	//设备序列号
}LogBaseEpcInfo;
typedef void(__stdcall* delegateTagEpcLog)(char* readerName, LogBaseEpcInfo msg);
typedef struct {
	char readerSerialNumber[21];	//设备序列号
}LogBaseEpcOver;
typedef void(__stdcall* delegateTagEpcOver)(char* readerName, LogBaseEpcOver msg);

typedef struct {
	unsigned char Tid[18];
	unsigned char AntId;
	unsigned char Rssi;
	unsigned char Result;
	unsigned char Userdata[512];
	char readerSerialNumber[21];	//设备序列号
}LogBase6bInfo;
typedef void(__stdcall* delegateTag6bLog)(char* readerName, LogBase6bInfo msg);
typedef struct {
	char readerSerialNumber[21];	//设备序列号
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
	char readerSerialNumber[21];	//设备序列号
}LogBaseGbInfo;
typedef void(__stdcall* delegateTagGbLog)(char* readerName, LogBaseGbInfo msg);
typedef struct {
	char readerSerialNumber[21];	//设备序列号
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
	char readerSerialNumber[21];	//设备序列号
}LogBaseGjbInfo;
typedef void(__stdcall* delegateTagGjbLog)(char* readerName, LogBaseGjbInfo msg);
typedef struct {
	char readerSerialNumber[21];	//设备序列号
}LogBaseGjbOver;
typedef void(__stdcall* delegateTagGjbOver)(char* readerName, LogBaseGjbOver msg);


typedef struct {
	int tagType;
	unsigned char Epc[1024];
	unsigned char Tid[1024];
	int AntId;
	int Rssi;
	unsigned char SubAntId;	//子天线号
	int ReadUtcSecTime; //UTC时间秒
	int ReadUtcMicroSecTime; //UTC时间微秒
	int Point; //当前频点
	unsigned char Phase; //当前标签相位
	int RSSI_dBm;
	int ReplySerialNumber;	//标签应答包序号
	char readerSerialNumber[21]; //设备序列号
}LogBaseTLInfo;
typedef void(__stdcall* delegateTagTLLog)(char* readerName, LogBaseTLInfo msg);
typedef struct {
	char readerSerialNumber[21]; //设备序列号
}LogBaseTLOver;
typedef void(__stdcall* delegateTagTLOver)(char* readerName, LogBaseTLOver msg);

typedef struct {
	int tagType;
	unsigned char Epc[1024];
	int AntId;
	int Rssi;
	unsigned char SubAntId;	//子天线号
	int ReadUtcSecTime; //UTC时间秒
	int ReadUtcMicroSecTime; //UTC时间微秒
	int Point; //当前频点
	unsigned char Phase; //当前标签相位
	int RSSI_dBm;
	int ReplySerialNumber;	//标签应答包序号
	char readerSerialNumber[21]; //设备序列号
}LogBase6DInfo;
typedef void(__stdcall* delegateTag6DLog)(char* readerName, LogBase6DInfo msg);
typedef struct {
	char readerSerialNumber[21]; //设备序列号
}LogBase6DOver;
typedef void(__stdcall* delegateTag6DOver)(char* readerName, LogBase6DOver msg);


typedef struct {
	unsigned char GpiPort;
	unsigned char Level;
	char TriggerTime[32];
	char readerSerialNumber[21];	//设备序列号
}LogAppGpiStart;
typedef void(__stdcall* delegateGpiStart)(char* readerName, LogAppGpiStart msg);

typedef struct {
	unsigned char GpiPort;
	unsigned char Level;
	char TriggerTime[32];
	char readerSerialNumber[21];	//设备序列号
}LogAppGpiOver;
typedef void(__stdcall* delegateGpiOver)(char* readerName, LogAppGpiOver msg);


typedef struct {
	unsigned char reason;
}LogTestWorkingModeInit;
typedef void(__stdcall* delegateWorkingModeInit)(char* readerName, LogTestWorkingModeInit msg);

typedef void(__stdcall* delegateTcpDisconnected)(char* readerName);

typedef void(__stdcall* delegateUsbHidRemoved)(char* readerName);

#endif // DELEGATE_H
