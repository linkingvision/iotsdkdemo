//
// Copyright(c) 2019 linkingvision.com
// Distributed under Commercial license info@linkingvision.com
//
#ifndef __IOT_RTSP_CLIENT_LIB__
#define __IOT_RTSP_CLIENT_LIB__

#include <string.h>
#include <vector>
#include <iostream>
#include <thread>
#include <mutex>
#include <chrono>
#include <map>
#include "live555_rtsp_client.hpp"
#include "iotvideotype.hpp"

#define IOT_MAX_LOG_LEN 4096
#define IOT_SPS_PPS_MAX 1024
class  IotRtspCallback
{
public:
	virtual bool   OnData(unsigned char* buffer, int size, unsigned long long secs, 
				unsigned long long msecs, CodecType codec, int ptsdiff, InfoFrame &pInfo)
				{return true;}
};
typedef std::map<void *, IotRtspCallback * > IotCallbackMap;
typedef std::map<std::string, std::string> IotParameterMap;

//std::this_thread::sleep_for(std::chrono::seconds(2));
/* RTSP Client */
class IotRTSPClient : public Live555RTSPClient
{
public:
	IotRTSPClient(const std::string uri, 
		std::string strUser, std::string strPasswd, bool bTCP = true);
  
	virtual ~IotRTSPClient();
	
	virtual bool onNewSession(Live555CodecType nCodec, const char* media, const char* codec);
	/* parseSPropParameterSets to get pps & sps */
	virtual bool onData(unsigned char* buffer, int size, unsigned long long secs, 
										unsigned long long msecs, Live555CodecType codec);
	virtual int onSDP(std::string &strSDP);
	virtual bool onParameter(std::string strKey, std::string strValue);
	virtual void Log(Live555LogLevel level, const char* format, ... );

public:
	bool RegCallback(void * pIndex, IotRtspCallback * pCallback);
	bool UnRegCallback(void * pIndex);
	bool   OnIotData(unsigned char* buffer, int size, unsigned long long secs,
		unsigned long long msecs, CodecType codec);
	bool UpdateCodecSink(CodecType codec);
	
	static bool Thread(void* pData);
	bool ThreadRun();

	bool Start();
	bool Stop();

private:
	bool m_bStarted;
	std::thread * m_pThread;
	CodecType m_VideoCodec;
	std::mutex m_pCBLock;
	std::string m_uri;
	IotCallbackMap m_pCBMap;
	IotParameterMap m_pParamMap;
	char m_sps[IOT_SPS_PPS_MAX];
	char m_pps[IOT_SPS_PPS_MAX];
	char m_vps[IOT_SPS_PPS_MAX];
	int m_spsSize;
	int m_ppsSize;
	int m_vpsSize;
	bool m_bFirstFrame;
	InfoFrame m_pInfo;
		
};

#endif 

