//
// Copyright(c) 2019 linkingvision.com
// Distributed under Commercial license info@linkingvision.com
//
#ifndef __DEMO_DEVICE_H__
#define __DEMO_DEVICE_H__
#include "iotdevice.hpp"
#include "iotvideo.hpp"
#include "iotdelegate.hpp"
#include "demochannel.hpp"


typedef std::map<std::string, DemoChannel *> ChannelMap;
class DemoDevice : public IIotDelegate
{
public:
    DemoDevice(std::string strUrl, std::string strUrlUser, 
			std::string strUrlPassWd, int nCh,
			std::string strDevName, std::string strDevToken, 
			std::string strIp, std::string strPort, 
			std::string strUser, std::string strPasswd, bool bSSL);
    ~DemoDevice();
public:
	bool Init();
	
public:
	virtual bool  OnStartStream(std::string strToken, h5StreamType nStream, 
			std::string strPushPath, std::string strIp, std::string strPort, 
			std::string strUser, std::string strPasswd, bool bSSL);
	virtual bool  OnStopStream(std::string strToken, h5StreamType nStream);
	virtual bool  OnPtzAction(std::string strToken, h5PtzAction action);
	virtual bool  OnSetPreset(std::string strToken, h5PtzPreset action);
	virtual bool  OnDelPreset(std::string strToken, std::string strPresetToken);

	
	virtual bool  OnConnected();
	virtual bool  OnDisConnected();

private:
	h5SrcList m_SrcList;
	std::string m_strUrl;
	std::string m_strUrlUser;
	std::string m_strUrlPassWd;
	int m_nCh;
	std::string m_strDevName;
	std::string m_strDevToken;
	std::string m_strIp; 
	std::string m_strPort;
	std::string m_strUser;
	std::string m_strPasswd;
	bool m_bSSL;

	IotDevice *m_pDevice;

	ChannelMap m_pushMap;
};

#endif /* __DEMO_DEVICE_H__ */