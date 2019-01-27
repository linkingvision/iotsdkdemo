//
// Copyright(c) 2019 linkingvision.com
// Distributed under Commercial license info@linkingvision.com
//
#include "demochannel.hpp"
#include "demodevice.hpp"
#include "iotvideo.hpp"

DemoChannel::DemoChannel(std::string strUrl, std::string strUrlUser, 
		std::string strUrlPassWd, 
		std::string strToken, std::string strPath,  
		std::string strIp, std::string strPort, 
		std::string strUser, std::string strPasswd, bool bSSL)
:m_push(NULL),m_buf(NULL), m_bufLen(0),
 m_rtspClient(NULL)
{
	m_rtspClient = new IotRTSPClient(strUrl, strUrlUser, strUrlPassWd);
	m_push = new IotVideoPush((IIotVideoPushDelegate&)(*this), strToken, 
				strPath, strIp, strPort, strUser, strPasswd, bSSL);
}
DemoChannel::~DemoChannel()
{
	if (m_rtspClient)
	{
		delete m_rtspClient;
	}
	if (m_push)
	{
		delete m_push; 
	}
	if (m_buf)
	{
		free(m_buf);
		m_buf = NULL;
	}
	
}

bool  DemoChannel::OnVideoPushConnected()
{
	m_rtspClient->RegCallback((void *)this, (IotRtspCallback *)this);
	m_rtspClient->Start();
	return true;
}
bool  DemoChannel::OnVideoPushDisConnected()
{
	m_rtspClient->UnRegCallback((void *)this);
	m_rtspClient->Stop();
	return true;
}

bool   DemoChannel::OnData(unsigned char* buffer, int size, unsigned long long secs, 
				unsigned long long msecs, CodecType codec, 
					int ptsdiff, InfoFrame &pInfo)
{
	VideoStreamType streamType = VIDEO_STREAM_VIDEO;
	switch(codec)
	{
		case LIVE555_CODEC_PCMU:
		case LIVE555_CODEC_PCMA:
		case LIVE555_CODEC_G711A:
		case LIVE555_CODEC_G711U:
		case LIVE555_CODEC_AAC:
			streamType = VIDEO_STREAM_AUDIO;
			return true;
			break;
		default:
			break;
			
	}
	if (m_bufLen < (size + 4))
	{
		if (m_buf)
		{
			free(m_buf);
			m_buf = NULL;
		}
		m_bufLen = (size + 4) * 2;
		m_buf = (char *)malloc(m_bufLen);
		m_buf[0] = 0;
		m_buf[1] = 0;
		m_buf[2] = 0;
		m_buf[3] = 0x01;
	}

	memcpy(m_buf + 4, buffer, size);

	m_push->PushFrameBuf((unsigned char *)m_buf, size + 4, secs, msecs, streamType, 0, pInfo);
	
	return true;
}