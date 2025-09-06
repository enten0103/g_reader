#pragma once
#ifndef MESSAGE_H
#define MESSAGE_H

#define REGION(msg) 1

#ifdef __cplusplus
extern "C" {
#endif

	typedef enum {
		AntennaNo_1 = 0x1,
		AntennaNo_2 = 0x2,
		AntennaNo_3 = 0x4,
		AntennaNo_4 = 0x8,
		AntennaNo_5 = 0x10,
		AntennaNo_6 = 0x20,
		AntennaNo_7 = 0x40,
		AntennaNo_8 = 0x80,
		AntennaNo_9 = 0x100,
		AntennaNo_10 = 0x200,
		AntennaNo_11 = 0x400,
		AntennaNo_12 = 0x800,
		AntennaNo_13 = 0x1000,
		AntennaNo_14 = 0x2000,
		AntennaNo_15 = 0x4000,
		AntennaNo_16 = 0x8000,
		AntennaNo_17 = 0x10000,
		AntennaNo_18 = 0x20000,
		AntennaNo_19 = 0x40000,
		AntennaNo_20 = 0x80000,
		AntennaNo_21 = 0x100000,
		AntennaNo_22 = 0x200000,
		AntennaNo_23 = 0x400000,
		AntennaNo_24 = 0x800000,
		AntennaNo_25 = 0x1000000,
		AntennaNo_26 = 0x2000000,
		AntennaNo_27 = 0x4000000,
		AntennaNo_28 = 0x8000000,
		AntennaNo_29 = 0x10000000,
		AntennaNo_30 = 0x20000000,
		AntennaNo_31 = 0x40000000,
		AntennaNo_32 = 0x80000000,
	}AntennaNo;

	typedef enum {
		InventoryMode_Single = 0,
		InventoryMode_Inventory = 1,
	}InventoryMode;

	typedef enum {
		InitInventory = 0,
		InitUpgrade = 1,
	}ConnectionAttemptEventStatusType;

	typedef struct {
		int RtCode;
		char RtMsg[128];
	}Result;

#if REGION("��ѯ��д����Ϣ")

	typedef struct {
		char AuthKey;
		char InventoryAuth;
		char UpgradeAuth;
	}WorkingState;

	typedef struct {
		Result rst;
		char SerialNum[21]; //��д����ˮ��
		char PowerOnTime[32]; //�ϵ�ʱ��
		char BaseBuildDate[64]; //��������ʱ��
		char AppVersion[16]; //Ӧ������汾���磺��0.1.0.0����
		char AppBuildDate[32]; //Ӧ�ñ���ʱ��
		char SystemVersion[64]; //����ϵͳ�汾
		char TuyaPID[64];
		char TuyaUUID[64];
		char TuyaAuthKey[512];
		char TuyaShortUrl[512];
		char BasePowerOnTime[32];
		WorkingState workingState;
	}MsgAppGetReaderInfo;
#endif

#if REGION("�����汾��")
	typedef struct {
		Result rst;
		char BaseVersion[16];
	}MsgAppGetBaseVersion;
#endif

#if REGION("���ڲ���")
	typedef struct {
		Result rst;
		char BaudrateIndex;
	}MsgAppSetSerialParam;

	typedef struct {
		Result rst;
		char BaudrateIndex;
	}MsgAppGetSerialParam;
#endif

#if REGION("���ö�д����̫��IP����")
	typedef struct {
		Result rst;
		unsigned char autoIp;
		char ip[32];
		char mask[32];
		char gateway[32];
		char dns1[32];
		char dns2[32];
	}MsgAppSetEthernetIP;

	typedef struct {
		Result rst;
		unsigned char autoIp;
		char ip[32];
		char mask[32];
		char gateway[32];
		char dns1[32];
		char dns2[32];
	}MsgAppGetEthernetIP;
#endif

#if REGION("��ѯ��д����̫��MAC")
	typedef struct {
		Result rst;
		char mac[32];
	}MsgAppGetEthernetMac;
#endif

#if REGION("���úͲ�ѯ������/�ͻ���ģʽ����")
	typedef struct {
		Result rst;
		unsigned char tcpMode;
		unsigned short serverPort;
		char clientIp[32];
		unsigned short clientPort;
	}MsgAppSetTcpMode;

	typedef struct {
		Result rst;
		unsigned char tcpMode;
		unsigned short serverPort;
		char clientIp[32];
		unsigned short clientPort;
	}MsgAppGetTcpMode;
#endif

#if REGION("����GPO")
	typedef struct {
		int GpoIndex;//Gpo1-32
		unsigned char Gpo;//0, �̵����Ͽ���1���̵����պ�
	}DictionaryGpo;

	typedef struct {
		Result rst;
		DictionaryGpo DicGpo[128];
		unsigned char DicGpoCount;
	}MsgAppSetGpo;
#endif

#if REGION("��ȡGPI״̬")
	typedef struct {
		int GpiIndex;//Gpi1-16
		unsigned char Gpi;//0, �͵�ƽ��1���ߵ�ƽ
	}DictionaryGpi;

	typedef struct {
		Result rst;
		DictionaryGpi DicGpi[128];
		unsigned char DicGpiCount;
	}MsgAppGetGpiState;
#endif 

#if REGION("GPI��������")
	typedef struct {
		Result rst;
		char GpiPort; //GPI �˿ںţ�������0 ��ʼ
		char TriggerStart; //������ʼ��0 �����رգ�1 �͵�ƽ������2 �ߵ�ƽ������3 �����ش�����4 �½��ش�����5 ������ش�����
		char TriggerCommand[128]; //���������Hex�ַ�����,�ɵ������ͷ�
		char TriggerOver; //����ֹͣ��0 ��ֹͣ��1 �͵�ƽ������2 �ߵ�ƽ������3 �����ش�����4 �½��ش�����5 ������ش�����6 ��ʱֹͣ��
		unsigned short OverDelayTime; //��ʱֹͣʱ�䣨����ֹͣ����Ϊ����ʱֹͣ����Ч��, ��10msΪ��λ
		char LevelUploadSwitch; //������ֹͣʱIO ��ƽ�仯�ϴ����أ�0 ���ϴ���1 �ϴ���
	}MsgAppSetGpiTrigger;

	typedef struct {
		Result rst;
		char GpiPort; //GPI �˿ںţ�������0 ��ʼ
		char TriggerStart; //������ʼ��0 �����رգ�1 �͵�ƽ������2 �ߵ�ƽ������3 �����ش�����4 �½��ش�����5 ������ش�����
		char TriggerCommand[128]; //���������Hex�ַ�����
		char TriggerOver; //����ֹͣ��0 ��ֹͣ��1 �͵�ƽ������2 �ߵ�ƽ������3 �����ش�����4 �½��ش�����5 ������ش�����6 ��ʱֹͣ��
		unsigned short OverDelayTime; //��ʱֹͣʱ�䣨����ֹͣ����Ϊ����ʱֹͣ����Ч��, ��10msΪ��λ
		char LevelUploadSwitch; //������ֹͣʱIO ��ƽ�仯�ϴ����أ�0 ���ϴ���1 �ϴ���
	}MsgAppGetGpiTrigger;
#endif

#if REGION("Τ��ͨ�Ų���")
	typedef struct {
		Result rst;
		char Switch;
		char Format;
		char Content;
		short NegativePulseWidth;
		unsigned short PulseInterval;
	}MsgAppSetWiegand;

	typedef struct {
		Result rst;
		char Switch;
		char Format;
		char Content;
		short NegativePulseWidth;
		unsigned short PulseInterval;
	}MsgAppGetWiegand;
#endif

#if REGION("������д��")
	typedef struct {

	}MsgAppReboot;
#endif

#if REGION("��д��ϵͳʱ��")
	typedef struct {
		Result rst;
		int UtcSecond;
		int UtcMicrosecond;
	}MsgAppSetReaderTime;

	typedef struct {
		Result rst;
		int UtcSecond;
		int UtcMicrosecond;
	}MsgAppGetReaderTime;
#endif

#if REGION("��д��MAC")
	typedef struct {
		Result rst;
		unsigned char Mac[6];
	}MsgAppSetReaderMac;
#endif

#if REGION("�ָ���д��Ĭ������")
	typedef struct {
		Result rst;

	}MsgAppRestoreDefault;
#endif

#if REGION("��д��RS485����")
	typedef struct {
		Result rst;
		char Address;
		char BaudRate;
	}MsgAppSetRs485;

	typedef struct {
		Result rst;
		char Address;
		char BaudRate;
	}MsgAppGetRs485;
#endif

#if REGION("��д�ϵ���������")
	typedef struct {
		Result rst;
		char Switch;
	}MsgAppSetBreakpointResume;

	typedef struct {
		Result rst;
		char Switch;
	}MsgAppGetBreakpointResume;
#endif

#if REGION("��д�������ǩ����")
	typedef struct {
		Result rst;
	}MsgAppGetCacheTagData;

	typedef struct {
		Result rst;
	}MsgAppClearCacheTagData;

	typedef struct {
		Result rst;
		int SerialNumber;
	}MsgAppTagDataReply;
#endif

#if REGION("������")
	typedef struct {
		Result rst;
		char Switch;
	}MsgAppSetBuzzerSwitch;

	typedef struct {
		Result rst;
		char Status;
		char Mode;
	}MsgAppSetBuzzerCtrl;
#endif

#if REGION("����������")
	typedef struct {
		Result rst;
		int PacketNumber;
		unsigned char PacketContent[512];
		int PacketLen;
	}MsgAppGetWhiteList;

	typedef struct {
		Result rst;
		int PacketNumber;
		unsigned char PacketContent[512];
		int PacketLen;
	}MsgAppImportWhiteList;

	typedef struct {
		Result rst;

	}MsgAppDelWhiteList;

	typedef struct {
		Result rst;
		char RelayNo;
		short RelayCloseTime;
	}MsgAppSetWhiteListAction;

	typedef struct {
		Result rst;
		char RelayNo;
		short RelayCloseTime;
	}MsgAppGetWhiteListAction;

	typedef struct {
		Result rst;
		char Switch;
		char FilterArea;
	}MsgAppSetWhiteListSwitch;

	typedef struct {
		Result rst;
		char Switch;
		char FilterArea;
	}MsgAppGetWhiteListSwitch;
#endif

#if REGION("UDP�ϱ�����")
	typedef struct {
		Result rst;
		char Switch;
		char ip[32];
		int port;
		int period;
	}MsgAppSetUdpParam;

	typedef struct {
		Result rst;
		char Switch;
		char ip[32];
		int port;
		int period;
	}MsgAppGetUdpParam;
#endif

#if REGION("HTTP�ϱ�����")
	typedef struct {
		Result rst;
		char Switch;
		int period;
		char format;
		int timeout;
		char cache;
		int addressLen;
		char address[512];
	}MsgAppSetHttpParam;

	typedef struct {
		Result rst;
		char Switch;
		int period;
		char format;
		int timeout;
		char cache;
		int addressLen;
		char address[512];
	}MsgAppGetHttpParam;
#endif

#if REGION("USB���̿���")
	typedef struct {
		Result rst;
		char operate;
		char kbdSwitch;
	}MsgAppUsbKbdSwitch;
#endif


#if REGION("WIFI����")
	typedef struct {
		Result rst;
	}MsgAppSetWifiHotspotSearch;

	typedef struct {
		Result rst;

	}WifiHotspotInfo;

	typedef struct {
		Result rst;
		int PacketNumber;
		char PacketContent[1024];
	}MsgAppGetWifiHotspotSearch;

	typedef struct {
		Result rst;
		int ESSIDLen;
		char ESSID[512];
		int pwdLen;
		char password[512];
		bool setCTFlag;
		char certificationType;
		bool setEAFlag;
		char encryptionAlgorithm;
	}MsgAppSetWifiHotspot;

	typedef struct {
		Result rst;
		int ESSIDLen;
		char ESSID[512];
	}MsgAppGetWifiConnectStatus;

	typedef struct {
		Result rst;
		char autoIp;
		char ip[32];
		char mask[32];
		char gateway[32];
		char dns1[32];
		char dns2[32];
		int ESSIDLen;
		char ESSID[512];
	}MsgAppSetWifiIp;

	typedef struct {
		Result rst;
		int ESSIDLen;
		char ESSID[512];
		char autoIp;
		char ip[32];
		char mask[32];
		char gateway[32];
		char dns1[32];
		char dns2[32];
	}MsgAppGetWifiIp;

	typedef struct {
		Result rst;
		char Switch;
	}MsgAppSetWifiOnOff;

	typedef struct {
		Result rst;
		char Switch;
	}MsgAppGetWifiOnOff;
#endif

#if REGION("EASƥ�䱨������")
	typedef struct {
		short keepTime;
		char setGpo1;
		char gpo1;
		char setGpo2;
		char gpo2;
		char setGpo3;
		char gpo3;
		char setGpo4;
		char gpo4;
	}ActionParamSuccess;

	typedef struct {
		short keepTime;
		char setGpo1;
		char gpo1;
		char setGpo2;
		char gpo2;
		char setGpo3;
		char gpo3;
		char setGpo4;
		char gpo4;
	}ActionParamFail;

	typedef struct {
		Result rst;
		char alarmSwitch;
		char filterDataArea;
		short start;
		int contentLen;
		char content[512];
		int maskLen;
		char mask[512];
		bool setSuccessFlag;
		ActionParamSuccess success;
		char setFailFlag;
		ActionParamFail fail;
	}MsgAppSetEasAlarm;

	typedef struct {
		Result rst;
		char alarmSwitch;
		char filterData;
		short start;
		int contentLen;
		char content[512];
		int maskLen;
		char mask[512];
		bool setSuccessFlag;
		ActionParamSuccess success;
		char setFailFlag;
		ActionParamFail fail;
	}MsgAppGetEasAlarm;
#endif

#if REGION("WiFi���μ�⿪��")
	typedef struct {
		Result rst;
		char Operate;
		char Switch;
	}MsgAppWifiRoaming;
#endif

#if REGION("���Ź�ι������")
	typedef struct {
		Result rst;
		char Operate;
		char Switch;
	}MsgAppWatchDog;
#endif

#if REGION("��ѯ��д��RFID����")
	typedef struct {
		Result rst;
		unsigned char MaxPower; //���֧�ֹ���
		unsigned char MinPower; //��С֧�ֹ���
		unsigned char AntennaCount; //��������
		int FrequencyArraySize;
		unsigned char FrequencyArray[16]; //֧�ֵ�Ƶ���б�
		int ProtocolArraySize;
		unsigned char ProtocolArray[16]; //֧�ֵ�Э���б�
	}MsgBaseGetCapabilities;
#endif

#if REGION("���úͲ�ѯ��д������")
	typedef struct {
		int AntennaNo;
		unsigned char Power;
	}DictionaryPower;

	typedef struct {
		Result rst;
		DictionaryPower DicPower[128];
		int DicCount;
		bool setReadOrWriteFlag;
		char ReadOrWrite;
		bool setPowerDownSaveFlag;
		char PowerDownSave;
	}MsgBaseSetPower;

	typedef struct {
		Result rst;
		bool setReadOrWrite;
		char ReadOrWrite;
		DictionaryPower DicPower[128];
		unsigned char DicCount;
	}MsgBaseGetPower;

#endif

#if REGION("���úͲ�ѯ��д��RFƵ��")
	typedef struct {
		Result rst;
		unsigned char FreqRangeIndex;
	}MsgBaseSetFreqRange;

	typedef struct {
		Result rst;
		unsigned char FreqRangeIndex;
	}MsgBaseGetFreqRange;
#endif

#if REGION("���úͲ�ѯ��д������Ƶ��")
	typedef struct {
		Result rst;

		int Mode;

		int FrequenciesSize;
		unsigned char Frequencies[512];

		bool setPowerDownSaveFlag;
		unsigned char PowerDownSave;
	}MsgBaseSetFrequency;

	typedef struct {
		Result rst;
		int Mode;
		int FrequenciesSize;
		unsigned char Frequencycies[512];
	}MsgBaseGetFrequency;
#endif

#if REGION("���úͲ�ѯ������չ����")
	typedef struct {
		unsigned char AntennaNo;
		unsigned char ChildAntenna;
	}AntDictionary;

	typedef struct {
		Result rst;
		unsigned char DicCount;
		AntDictionary DicAntenna[256];
	}MsgBaseSetAntennaHub;

	typedef struct {
		Result rst;
		unsigned char DicCount;
		AntDictionary DicAntenna[256];
	}MsgBaseGetAntennaHub;
#endif 

#if REGION("���úͲ�ѯ��ǩ�ϴ�����")
	typedef struct {
		Result rst;

		bool setRepeatedTimeFlag;
		unsigned short RepeatedTime; //  �ظ���ǩ����ʱ�䣨��ѡ������ʾ��һ������ָ��ִ�������ڣ���ָ�����ظ�����ʱ������ͬ�ı�ǩ����ֻ�ϴ�һ�Σ�0~65535��ʱ�䵥λ��10ms����

		bool setRssiTVFlag;
		unsigned char RssiTV; //RSSI ��ֵ����ѡ������ǩRSSI ֵ������ֵʱ��ǩ���ݽ����ϴ�����������
	}MsgBaseSetTagLog;

	typedef struct {
		Result rst;

		bool setRepeatedTimeFlag;
		unsigned short RepeatedTime; //  �ظ���ǩ����ʱ�䣨��ѡ������ʾ��һ������ָ��ִ�������ڣ���ָ�����ظ�����ʱ������ͬ�ı�ǩ����ֻ�ϴ�һ�Σ�0~65535��ʱ�䵥λ��10ms����

		bool setRssiTVFlag;
		unsigned char RssiTV; //RSSI ��ֵ����ѡ������ǩRSSI ֵ������ֵʱ��ǩ���ݽ����ϴ�����������
	}MsgBaseGetTagLog;
#endif 

#if REGION("���úͲ�ѯEPC��������")
	typedef struct {
		Result rst;

		bool setBaseSpeedFlag;
		unsigned char BaseSpeed; // EPC �������ʣ���ѡ����

		bool setQValueFlag;
		unsigned char QValue; // Ĭ��Q ֵ����ѡ��(0~15)��

		bool setSessionFlag;
		unsigned char Session; // ����ѡ��(0,Session0; 1,Session1; 2,Session2; 3,Session3)��

		bool setInventoryFlag;
		unsigned char InventoryFlag; // �̴��־��������ѡ��(0,����Flag A �̴�;1,����Flag B �̴�;2,����ʹ��Flag	A ��Flag B)�� 
	}MsgBaseSetBaseband;

	typedef struct {
		Result rst;

		bool setBaseSpeedFlag;
		unsigned char BaseSpeed; // EPC �������ʣ���ѡ����

		bool setQValueFlag;
		unsigned char QValue; // Ĭ��Q ֵ����ѡ��(0~15)��

		bool setSessionFlag;
		unsigned char Session; // ����ѡ��(0,Session0; 1,Session1; 2,Session2; 3,Session3)��

		bool setInventoryFlag;
		unsigned char InventoryFlag; // �̴��־��������ѡ��(0,����Flag A �̴�;1,����Flag B �̴�;2,����ʹ��Flag	A ��Flag B)�� 
	}MsgBaseGetBaseband;

#endif

#if REGION("���úͲ�ѯ�Զ�����ģʽ")
	typedef struct {
		Result rst;
		unsigned char OnOff;

		bool setFreeTimeFlag;
		unsigned short FreeTime;
	}MsgBaseSetAutoDormancy;

	typedef struct {
		Result rst;
		unsigned char OnOff;

		bool setFreeTimeFlag;
		unsigned short FreeTime;
	}MsgBaseGetAutoDormancy;
#endif

#if REGION("���úͲ�ѯ��д��פ��ʱ�����")
	typedef struct {
		Result rst;

		bool setAntResidenceTimeFlag;
		unsigned short AntResidenceTime;

		bool setFreqResidenceTimeFlag;
		unsigned short FreqResidenceTime;
	}MsgBaseSetResidenceTime;

	typedef struct {
		Result rst;

		bool setAntResidenceTimeFlag;
		unsigned short AntResidenceTime;

		bool setFreqResidenceTimeFlag;
		unsigned short FreqResidenceTime;
	}MsgBaseGetResidenceTime;
#endif

#if REGION("���úͲ�ѯGB��������")
	typedef struct {
		char Tc;
		char Trext;
		char rlf;
		char rlc;
	}ParamBaseSpeed;

	typedef struct {
		char CIN;
		char CCN;
	}ParamAntiCollision;

	typedef struct {
		Result rst;

		bool setGbBaseSpeedFlag;
		ParamBaseSpeed GbBaseSpeed;

		bool setGbAntiCollisionFlag;
		ParamAntiCollision GbAntiCollision;

		bool setSessionFlag;
		unsigned char Session;

		bool setInventoryFlag;
		unsigned char InventoryFlag;
	}MsgBaseSetGbBaseband;

	typedef struct {
		Result rst;

		ParamBaseSpeed GbBaseSpeed;

		ParamAntiCollision GbAntiCollision;

		unsigned char Session;

		unsigned char InventoryFlag;
	}MsgBaseGetGbBaseband;
#endif

#if REGION("���úͲ�ѯGJB��������")

	typedef struct {
		Result rst;

		bool setGjbBaseSpeedFlag;
		ParamBaseSpeed GjbBaseSpeed;

		bool setGjbAntiCollisionFlag;
		ParamAntiCollision GjbAntiCollision;

		bool setInventoryFlag;
		unsigned char InventoryFlag;
	}MsgBaseSetGjbBaseband;

	typedef struct {
		Result rst;

		ParamBaseSpeed GjbBaseSpeed;

		ParamAntiCollision GjbAntiCollision;

		unsigned char InventoryFlag;
	}MsgBaseGetGjbBaseband;
#endif

#if REGION("��6c")
	typedef struct {
		Result rst;
		char Area;
		unsigned short BitStart;
		int BitLen;   //λ����
		char HexData[2048];//��Ҫƥ�����������,ʮ������
	}ParamFilter;

	typedef struct {
		Result rst;
		char Mode;
		unsigned char Len;
	}ParamReadTid;

	typedef struct {
		Result rst;
		unsigned short Start;
		unsigned char Len;
	}ParamReadUserdata;

	typedef struct {
		Result rst;
		unsigned short Start;
		unsigned char Len;
	}ParamReadReserved;

	typedef struct {
		Result rst;
		unsigned short Start;
		unsigned char Len;
	}ParamReadEpcBank;

	typedef struct {
		Result rst;
		unsigned char Fastid;
		unsigned char Foucs;
	}ImpinjMonza;

	typedef struct {
		Result rst;
		long Index;
		ParamFilter Filter;
	}MsgBaseSetMultiFilter;

	typedef struct {
		Result rst;
		long Index;
		ParamFilter Filter;
	}MsgBaseGetMultiFilter;

	typedef struct {
		Result rst;
		int AntennaEnable;
		char InventoryMode;
		ParamFilter Filter;
		ParamReadTid ReadTid;
		ParamReadUserdata ReadUserdata;
		ParamReadReserved ReadReserved;
		char StrHexPassword[9];
		int MonzaQtPeek;
		int Rfmicron;
		int EMSensorData;
		ParamReadEpcBank EpcBank;
		ImpinjMonza monza;
		int Ctesius;
		int Quanray;
		int Timeout;
		int EnableMultiFilter;
	} MsgBaseInventoryEpc;
#endif

#if REGION("дEPC")
	typedef struct {
		Result rst;
		int AntennaEnable;// ���߶˿ڣ�ʹ������ö�٣����AntennaNo,�������ʹ�û�
		char Area; //��д��ı�ǩ������(0����������1��EPC ����2��TID ����3���û�������)
		unsigned short Start; //��д���ǩ������������ʼ��ַ
		char StrHexWriteData[1024]; //��д�����������(16�����ַ���)
		ParamFilter Filter; //ѡ���ȡ�������������˵����
		char StrHexPassword[9]; //��������, 16�����ַ���
		int Block;//��д������0��ʾ����д
		int StayCarrierWave;//������ɱ����ز�����״̬
	}MsgBaseWriteEpc;
#endif

#if REGION("��EPC")
	typedef struct {
		Result rst;
		int AntennaEnable; //���߶˿ڣ�ʹ������ö�٣����AntennaNo,�������ʹ�û�
		char Area; //�������ı�ǩ������(0�������������1��������������2��EPC ����3��TID ����4���û�������)
		char Mode; //����������(0��������1��������2�����ý�����3����������)
		ParamFilter Filter; //ѡ���ȡ�������������˵����
		char StrHexPassword[9]; //��������,, 16�����ַ���
	}MsgBaseLockEpc;
#endif

#if REGION("����ǩ")
	typedef struct {
		Result rst;
		int AntennaEnable; //���߶˿ڣ�ʹ������ö�٣����AntennaNo,�������ʹ�û�
		char StrHexPassword[9]; //����,16�����ַ���
		ParamFilter Filter; //ѡ���ȡ�������������˵����
	}MsgBaseDestroyEpc;
#endif

#if REGION("MONZA QT��ǩ����")
	typedef struct {
		Result rst;
		int AntennaEnable;
		char QtOperate;

		ParamFilter Filter;

		char StrHexPassword[9];

		bool setQtParamFlag;
		unsigned char QtParam[3];

		bool setQtResultFlag;
		unsigned char QtResult[3];
	}MsgBaseMonzaQt;
#endif

#if REGION("EPC��ǩ������дָ��")
	typedef struct {
		Result rst;
		int AntennaEnable;
		char Operate;
		char Area[3];
		char SpecialCode[3];

		bool setWriteContentFlag;
		unsigned char WriteContent[3];

		bool setReadContentFlag;
		unsigned char ReadContent[3];
	}MsgBaseSuperRW;
#endif

#if REGION("��6B��ǩ")
	typedef struct {
		char Start;
		char Len;
	}Param6bReadUserdata;

	typedef struct {
		Result rst;
		int AntennaEnable;
		char InventoryMode;
		char Area;

		bool setReadUserdataFlag;
		Param6bReadUserdata ReadUserdata;

		bool setMatchTidFlag;
		char TidLenth;
		char StrHexMatchTid[1024];
	}MsgBaseInventory6B;
#endif

#if REGION("д6B��ǩ")
	typedef struct {
		Result rst;
		int AntennaEnable;
		char TidLength;
		char StrHexMatchTid[1024];
		char Start;
		char DataLength;
		char StrHexWriteData[1024];

		bool setErrorIndexFlag;
		char ErrorIndex;
	}MsgBaseWrite6B;
#endif

#if REGION("��6B��ǩ")
	typedef struct {
		Result rst;
		int AntennaEnable;
		char TidLength;
		char StrHexMatchTid[1024];
		unsigned char Address;

		bool setErrorAddressFlag;
		char ErrorAddress;
	}MsgBaseLock6B;
#endif

#if REGION("������ѯ6B��ǩ")
	typedef struct {
		Result rst;
		int AntennaEnable;
		char TidLength;
		char StrHexMatchTid[1024];
		unsigned char Address;

		bool setLockStateFlag;
		char LockState;
	}MsgBaseLockGet6B;
#endif

#if REGION("��GB��ǩ")
	typedef struct {
		Result rst;
		unsigned char ChildArea;
		unsigned short Start;
		unsigned char Len;
	}ParamGbReadUserdata;

	typedef struct {
		Result rst;
		int AntennaEnable;
		char InventoryMode;
		ParamFilter Filter;
		ParamReadTid ReadTid;
		ParamGbReadUserdata ReadUserdata;
		char StrHexPassword[9];
		bool setSafeCertificationFlag;
		char SafeCertificationFlag;
	}MsgBaseInventoryGB;
#endif

#if REGION("дGB��ǩ")
	typedef struct {
		Result rst;
		int AntennaEnable;
		char Area;
		unsigned short Start;
		char DataLength;
		char DataContent[1024];
		ParamFilter Filter;
		char StrHexPassword[9];
		bool setSafeCertificationFlag;
		char SafeCertificationFlag;
		bool setFDMicroInitFlag;
		char FDMicroInit;
		bool setStayCarrierWaveFlag;
		char StayCarrierWave;
	}MsgBaseWriteGB;
#endif

#if REGION("��GB��ǩ")
	typedef struct {
		Result rst;
		int AntennaEnable;
		char Area;
		char LockParam;
		ParamFilter Filter;
		char StrHexPassword[9];
		bool setSafeCertificationFlag;
		char SafeCertificationFlag;
	}MsgBaseLockGB;
#endif

#if REGION("���GB��ǩ")
	typedef struct {
		Result rst;
		int AntennaEnable;
		ParamFilter Filter;
		char StrHexPassword[9];
		bool setSafeCertificationFlag;
		char SafeCertificationFlag;
	}MsgBaseDestroyGB;
#endif

#if REGION("��GJB��ǩ")
	typedef struct {
		Result rst;
		int AntennaEnable;
		char InventoryMode;
		ParamFilter Filter;
		ParamReadTid ReadTid;
		ParamReadUserdata ReadUserdata;
		char StrHexPassword[9];
		bool setSafeCertificationFlag;
		char SafeCertificationFlag;
	}MsgBaseInventoryGJB;
#endif

#if REGION("дGJB��ǩ")
	typedef struct {
		Result rst;
		int AntennaEnable;
		char Area;
		unsigned short Start;
		char DataLength;
		char DataContent[1024];
		ParamFilter Filter;
		char StrHexPassword[9];
		bool setSafeCertificationFlag;
		char SafeCertificationFlag;
		bool setStayCarrierWaveFlag;
		char StayCarrierWave;
	}MsgBaseWriteGJB;
#endif

#if REGION("��GJB��ǩ")
	typedef struct {
		Result rst;
		int AntennaEnable;
		char Area;
		char LockParam;
		ParamFilter Filter;
		char StrHexPassword[9];
		bool setSafeCertificationFlag;
		char SafeCertificationFlag;
	}MsgBaseLockGJB;
#endif

#if REGION("���GJB��ǩ")
	typedef struct {
		Result rst;
		int AntennaEnable;
		ParamFilter Filter;
		char StrHexPassword[9];
		bool setSafeCertificationFlag;
		char SafeCertificationFlag;
	}MsgBaseDestroyGJB;
#endif

#if REGION("��GB/T25340��ǩ")
	typedef struct {
		Result rst;
		int tagType;
		int AntennaEnable;
		char InventoryMode;
		int Timeout;
	}MsgBaseInventoryTL;
#endif

#if REGION("��6D��ǩ")
	typedef struct {
		Result rst;
		int AntennaEnable;
		char InventoryMode;
		int Timeout;
	}MsgBaseInventory6D;
#endif

#if REGION("��ǩ��ȫ��֤���ݽ���ָ��")
	typedef struct {
		short bitLen;
		char Content[256];
	}EncryptedData;
	typedef struct {
		Result rst;
		bool setToken1Flag;
		unsigned char Token1[9];
		bool setToken2Flag;
		char Token2;
		bool setEncryptedDataFlag;
		EncryptedData Data;
		bool setKeyFlag;
		char Key[512];
	}MsgBaseSafeCertification;
#endif

#if REGION("��Э����϶�")
	typedef struct {
		Result rst;
		int AntennaEnable;
		char Read6B;
		char ReadGB;
	}MsgBaseInventoryHybrid;
#endif

#if REGION("�������͸��������Ϣ")
	typedef struct {
		Result rst;
		char Mode;
		char DataLength;
		char DataContent[1024];
	}MsgBasePassthrough;

#endif

#if REGION("ֹͣ")
	typedef struct {
		Result rst;
	}MsgBaseStop;
#endif

#if REGION("Ӧ���������")
	typedef struct {
		Result rst;
		unsigned long packetNumber;
		char packetContent[1024];
	}MsgUpgradeApp;
#endif

#if REGION("�����������")
	typedef struct {
		Result rst;
		unsigned long packetNumber;
		char packetContent[1024];
	}MsgUpgradeBaseband;
#endif

#if REGION("ģ�鹤��ģʽ��ʼ��")
	typedef struct {
		Result rst;
		int len;
		char license[1024];
		char mode;
	}MsgTestWorkingMode;
#endif

#ifdef __cplusplus
}
#endif

#endif // MESSAGE_H
