//
// Copyright(c) 2019 linkingvision.com
// Distributed under Commercial license info@linkingvision.com
//
#include "demodevice.hpp"


DemoDevice::DemoDevice(std::string strUrl, std::string strUrlUser, 
			std::string strUrlPassWd, int nCh,
			std::string strDevName, std::string strDevToken, 
			std::string strIp, std::string strPort, 
			std::string strUser, std::string strPasswd, bool bSSL)
:m_strUrl(strUrl), m_strUrlUser(strUrlUser), m_strUrlPassWd(strUrlPassWd), m_nCh(nCh), 
 m_strDevName(strDevName), m_strDevToken(strDevToken), m_strIp(strIp), m_strPort(strPort), 
 m_strUser(strUser), m_strPasswd(strPasswd), m_bSSL(bSSL), 
 m_pDevice(NULL)
{
	printf("iot sdk version %s\n", IotDevice::GetVersion().c_str());
	m_strPasswd = IotDevice::MD5(strPasswd);
	Init();
	
}

DemoDevice::~DemoDevice()
{
	if (m_pDevice)
	{
		delete m_pDevice;
	}
}

bool DemoDevice::Init()
{
	for (int i = 0; i < m_nCh; i ++)
	{
		h5Src * pSrc = m_SrcList.add_src();
		pSrc->set_ntype(H5_IOTSDK);
		pSrc->set_strtoken(m_strDevToken + "-" + "token" + std::to_string(i));
		pSrc->set_strname(m_strDevToken + "-" + "name" + std::to_string(i));
		pSrc->set_stroriginaltoken("token" + std::to_string(i));
	}

	m_pDevice = new IotDevice(m_SrcList, *this, m_strDevName, m_strDevToken, 
			m_strIp, m_strPort, m_strUser, m_strPasswd, m_bSSL);
	m_pDevice->Connect();
    
    return true;
}

bool  DemoDevice::OnStartStream(std::string strToken, h5StreamType nStream, 
        std::string strPushPath, std::string strIp, std::string strPort, 
        std::string strUser, std::string strPasswd, bool bSSL)
{
	printf("iot sdk  stream %s started\n", strToken.c_str());
	/* create a video channel and push to cloud  */
	if (m_pushMap[strToken])
	{
		return true;
	}

	DemoChannel * pChannel = new DemoChannel(m_strUrl, m_strUrlUser, 
			m_strUrlPassWd, strToken, strPushPath, strIp, strPort, strUser, 
			strPasswd, bSSL);
	m_pushMap[strToken] = pChannel;
	
	return true;
}
bool  DemoDevice::OnStopStream(std::string strToken, h5StreamType nStream)
{
	printf("iot sdk  stream %s stoped\n", strToken.c_str());
	/* delete the video channel */
	if (m_pushMap[strToken])
	{
		delete m_pushMap[strToken];
		m_pushMap.erase(strToken);
	}
	
	return true;
}
bool  DemoDevice::OnPtzAction(std::string strToken, h5PtzAction action)
{
	return true;
}
bool  DemoDevice::OnSetPreset(std::string strToken, h5PtzPreset action)
{
	return true;
}
bool  DemoDevice::OnDelPreset(std::string strToken, std::string strPresetToken)
{
	return true;
}
bool  DemoDevice::OnConnected()
{
	printf("iot sdk connect %s\n", m_strDevName.c_str());	
 	return true;
}
bool  DemoDevice::OnDisConnected()
{
	printf("iot sdk disconnect %s\n", m_strDevName.c_str());	
	return true;
}
