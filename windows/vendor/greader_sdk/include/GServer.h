#pragma once
#ifndef __GSERVER_H__
#define __GSERVER_H__

#include <Windows.h>
#include "GClient.h"
#include "message.h"

#ifdef __cplusplus
extern "C" {
#endif

	typedef void(*delegateGClientConnected)(char* readerName, GClient* client);

	typedef struct {
		HANDLE handle;
		delegateGClientConnected call_GClientConnected;
	}GServer;

	void RegGServerCallBack(GServer* s, void* call);

	BOOL __stdcall OpenTcpServer(short port, GServer* gserver);
	BOOL __stdcall CloseGServer(GServer* gserver);
#ifdef __cplusplus
}
#endif


#endif
