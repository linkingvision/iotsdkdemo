//
// Copyright(c) 2019 linkingvision.com
// Distributed under Commercial license info@linkingvision.com
//
#ifndef __IOT_VIDEO_H__
#define __IOT_VIDEO_H__
#include "iottype.hpp"
#include "iotvideotype.hpp"
#include "h5scloud.pb.h"

#include "iotdelegate.hpp"
class IotVideoPushPri;
class IOT_LIBRARY_API IotVideoPush
{
public:
 	IotVideoPush(IIotVideoPushDelegate & pDelegate, 
					std::string strToken, std::string strPath, 
					std::string strIp, std::string strPort, 
					std::string strUser, std::string strPasswd, bool bSSL);
	~IotVideoPush();
public:
	/* Push frame buf by this interface */
	void PushFrameBuf(unsigned char* buffer, int size, unsigned long long secs, 
			unsigned long long msecs,  
			VideoStreamType streamType, int ptsdiff,  InfoFrame &pInfo);
	/* Below interface is used by internal */
	void PushFrame(VideoFrame& frame);

private:
	IotVideoPushPri * m_pri;
};

#endif /* __IOT_VIDEO_H__ */