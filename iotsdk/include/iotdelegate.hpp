//
// Copyright(c) 2019 linkingvision.com
// Distributed under Commercial license info@linkingvision.com
//
#ifndef __IOT_DELEGATE_H__
#define __IOT_DELEGATE_H__
#include "iottype.hpp"
#include "h5scloud.pb.h"

//TODO add connect failed delegate
class IOT_LIBRARY_API IIotDelegate
{
public:
	virtual bool  OnStartStream(std::string strToken, h5StreamType nStream, 
			std::string strPushPath, std::string strIp, std::string strPort, 
			std::string strUser, std::string strPasswd, bool bSSL) = 0;
	virtual bool  OnStopStream(std::string strToken, h5StreamType nStream) = 0;
	virtual bool  OnPtzAction(std::string strToken, h5PtzAction action) = 0;
	virtual bool  OnSetPreset(std::string strToken, h5PtzPreset action) = 0;
	virtual bool  OnDelPreset(std::string strToken, std::string strPresetToken) = 0;
	virtual bool  OnConnected() = 0;//TODO add token parameter
	virtual bool  OnDisConnected() = 0;//TODO add token parameter
};

class IOT_LIBRARY_API IIotVideoPushDelegate
{
public:
	virtual bool  OnVideoPushConnected() = 0;//TODO add token and stream parameter
	virtual bool  OnVideoPushDisConnected() = 0; //TODO add token and stream parameter
};

#endif /* __IOT_DELEGATE_H__ */

