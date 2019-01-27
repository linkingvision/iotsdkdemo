//
// Copyright(c) 2019 linkingvision.com
// Distributed under Commercial license info@linkingvision.com
//
#ifndef __IOT_DEVICE_H__
#define __IOT_DEVICE_H__
#include "iottype.hpp"
#include "iotdelegate.hpp"
class IotDevicePri;
class IOT_LIBRARY_API IotDevice
{
public:
    IotDevice(h5SrcList &pSrcList, IIotDelegate &pDelegate, 
			std::string strDevName, std::string strDevToken, 
			std::string strIp, std::string strPort, 
			std::string strUser, std::string strPasswd, bool bSSL);
    ~IotDevice();
public:
	bool Connect();
	bool AddSrc(h5Src &pSrc);
	bool DelSrc(std::string strToken);
public:
	static std::string GetVersion();
	static std::string MD5(std::string str);
	static bool SetLogger(IIotLogInterface *pLogger);
private:
	IotDevicePri * m_pri;
};

#endif /* __IOT_DEVICE_H__ */