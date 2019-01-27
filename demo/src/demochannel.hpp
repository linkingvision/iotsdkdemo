//
// Copyright(c) 2019 linkingvision.com
// Distributed under Commercial license info@linkingvision.com
//
#ifndef __DEMO_CHANNEL_H__
#define __DEMO_CHANNEL_H__
#include "iotdevice.hpp"
#include "iotvideo.hpp"
#include "iotdelegate.hpp"
#include "iotrtspclient.hpp"


class DemoChannel : public IotRtspCallback, public IIotVideoPushDelegate
{
public:
    DemoChannel(std::string strUrl, std::string strUrlUser, 
			std::string strUrlPassWd, 
			std::string strToken, std::string strPath,  
			std::string strIp, std::string strPort, 
			std::string strUser, std::string strPasswd, bool bSSL);
    ~DemoChannel();

	virtual bool   OnData(unsigned char* buffer, int size, unsigned long long secs, 
				unsigned long long msecs, CodecType codec, 
					int ptsdiff, InfoFrame &pInfo);
	virtual bool  OnVideoPushConnected();
	virtual bool  OnVideoPushDisConnected();
public:

private:
	std::string m_strUrl;
	std::string m_strUrlUser;
	std::string m_strUrlPassWd;
	IotRTSPClient *m_rtspClient;
	IotVideoPush *m_push;
	char * m_buf;
	int m_bufLen;
	
};

#endif /* __DEMO_CHANNEL_H__ */