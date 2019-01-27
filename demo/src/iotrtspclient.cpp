//
// Copyright(c) 2019 linkingvision.com
// Distributed under Commercial license info@linkingvision.com
//
#include "iotrtspclient.hpp"
#include <algorithm>

IotRTSPClient::IotRTSPClient(const std::string uri, 
	std::string strUser, std::string strPasswd, bool bTCP) 
: Live555RTSPClient(uri, strUser, strPasswd, bTCP, 1.0), m_bStarted(false), 
m_pThread(NULL), m_VideoCodec(CODEC_NONE), 
m_uri(uri), m_spsSize(0), m_ppsSize(0), m_vpsSize(0), m_bFirstFrame(true)
{
	memset(&m_pInfo, 0, sizeof(m_pInfo));
	m_pInfo.video =  CODEC_NONE;
	m_pInfo.audio =  CODEC_NONE;
}

IotRTSPClient::~IotRTSPClient() 
{
	Stop();
}

bool IotRTSPClient::Start()
{
	if (m_bStarted == true)
	{
		return true;
	}
	m_pThread = new std::thread(IotRTSPClient::Thread, this);
	m_bStarted = true;

	return true;
}


bool IotRTSPClient::Stop()
{
	if (m_bStarted == false)
	{
		return true;
	}
	RTSPStop();
	m_pThread->join();
	delete m_pThread;
	m_bStarted = NULL;

	return true;
}

bool IotRTSPClient::onNewSession(Live555CodecType nCodec, const char* media, const char* codec)
{
	std::string strCodec = codec;
	std::transform(strCodec.begin(), strCodec.end(), strCodec.begin(), ::toupper);

	bool success = false;
	if ( (strcasecmp(media, "video") == 0) && (strstr(strCodec.c_str(), "H264") != 0) )
	{
		m_pInfo.video = CODEC_H264;
		success = true;
	}
	else if ((strcasecmp(media, "video") == 0) && (strstr(strCodec.c_str(), "JPEG") != 0))
	{
		m_pInfo.video = CODEC_MJPEG;
		success = true;
	}
	else if ((strcasecmp(media, "video") == 0) && (strstr(strCodec.c_str(), "H265") != 0))
	{
		m_pInfo.video = CODEC_H265;
		success = true;
	}else if ((strcasecmp(media, "audio") == 0) && (strstr(strCodec.c_str(), "MPEG4-GENERIC") != 0))
	{
		m_pInfo.video = CODEC_AAC;
		success = true;
	}else if ((strcasecmp(media, "audio") == 0) && (strstr(strCodec.c_str(), "PCMA") != 0))
	{
		m_pInfo.video = CODEC_PCMA;
		success = true;
	}else if ((strcasecmp(media, "audio") == 0) && (strstr(strCodec.c_str(), "PCMU") != 0))
	{
		m_pInfo.video = CODEC_PCMU;
		success = true;
	}else if ((strcasecmp(media, "audio") == 0) && (strstr(strCodec.c_str(), "G711A") != 0))
	{	
		m_pInfo.video = CODEC_G711A;
		success = true;
	}else if ((strcasecmp(media, "audio") == 0) && (strstr(strCodec.c_str(), "G711U") != 0))
	{
		m_pInfo.video = CODEC_G711U;
		success = true;
	}
	Log(logLive555INFO, "URL %s Media %s  codec %s\n", m_uri.c_str(), media, codec);
	return success;			
}

bool IotRTSPClient::onParameter(std::string strKey, std::string strValue)
{
	if (strKey.size() <= 0 || strValue.size() <= 0)
	{
		return false;
	}
	Log(logLive555INFO, "%s key %s value %s\n", m_uri.c_str(), strKey.c_str(), strValue.c_str());

	m_pParamMap[strKey] = strValue;
	return true;
}

int IotRTSPClient::onSDP(std::string &strSDP)
{
	if (strSDP.length() <= 0)
	{
		return false;
	}
	Log(logLive555INFO, "onSDP %s \n", strSDP.c_str());
	
	return true;
}

bool IotRTSPClient::onData(unsigned char* buffer, int size, unsigned long long secs, 
									unsigned long long msecs, Live555CodecType codec)
{
	OnIotData(buffer, size, secs, msecs, (CodecType)codec);
	return true;
}


void IotRTSPClient::Log(Live555LogLevel level, const char* format, ... )
{
	char str[IOT_MAX_LOG_LEN]="";
	int len;
	va_list args;
	va_start(args, format);
	len = vsnprintf(str, IOT_MAX_LOG_LEN-1, format, args);
	if (len >0 & len < IOT_MAX_LOG_LEN)
	{
		str[len] = '\0';
	}else
	{
		return;
	}
	printf("[iotrtspclient] %s\n", str);
	va_end(args);
	
	return;
}

bool IotRTSPClient::UpdateCodecSink(CodecType codec)
{

	switch (codec)
	{
		case CODEC_H264:
		{
			SPropRecord* sPropRecords = NULL;
			u_int8_t* sps = NULL; unsigned spsSize = 0;
			u_int8_t* pps = NULL; unsigned ppsSize = 0;
			/* Add parseSPropParameterSets to get pps & sps here  */
			IotParameterMap::iterator it = m_pParamMap.begin(); 
			for(; it!=m_pParamMap.end(); ++it)
			{
				if ((*it).first == "sprop-parameter-sets")
				{
					unsigned numSPropRecords;
					sPropRecords = parseSPropParameterSets((*it).second.c_str(), 
									numSPropRecords);
					for (unsigned i = 0; i < numSPropRecords; ++i) {
						if (sPropRecords[i].sPropLength == 0) continue; // bad data
						u_int8_t nal_unit_type = (sPropRecords[i].sPropBytes[0])&0x1F;
						if (nal_unit_type == 7/*SPS*/) {
						  sps = sPropRecords[i].sPropBytes;
						  spsSize = sPropRecords[i].sPropLength;
						} else if (nal_unit_type == 8/*PPS*/) {
						  pps = sPropRecords[i].sPropBytes;
						  ppsSize = sPropRecords[i].sPropLength;
						}
					}
				}
			}
			Log(logLive555INFO, "%s sps size %d pps %d\n", m_uri.c_str(), spsSize, ppsSize);
			if (spsSize < IOT_SPS_PPS_MAX)
			{
				memcpy(m_sps, sps, spsSize);
				m_spsSize = spsSize;
			}
			if (ppsSize < IOT_SPS_PPS_MAX)
			{
				memcpy(m_pps, pps, ppsSize);
				m_ppsSize = ppsSize;
			}

			if (sPropRecords)
			{
				delete[] sPropRecords;
			}
			break;
		}
		case CODEC_H265:
		{
			SPropRecord* sPropRecords = NULL;
			u_int8_t* sps = NULL; unsigned spsSize = 0;
			u_int8_t* pps = NULL; unsigned ppsSize = 0;
			u_int8_t* vps = NULL; unsigned vpsSize = 0;
			/* Add parseSPropParameterSets to get pps & sps here  */
			IotParameterMap::iterator it = m_pParamMap.begin(); 
			for(; it!=m_pParamMap.end(); ++it)
			{
				if ((*it).first == "sprop-parameter-sets")
				{
					unsigned numSPropRecords;
					sPropRecords = parseSPropParameterSets((*it).second.c_str(), 
									numSPropRecords);
					for (unsigned i = 0; i < numSPropRecords; ++i) {
						if (sPropRecords[i].sPropLength == 0) continue; // bad data
						u_int8_t nal_unit_type = (sPropRecords[i].sPropBytes[0])&0x1F;
						if (nal_unit_type == 33/*SPS*/) {
						  sps = sPropRecords[i].sPropBytes;
						  spsSize = sPropRecords[i].sPropLength;
						} else if (nal_unit_type == 34/*PPS*/) {
						  pps = sPropRecords[i].sPropBytes;
						  ppsSize = sPropRecords[i].sPropLength;
						} else if (nal_unit_type == 32/*VPS*/) {
						  vps = sPropRecords[i].sPropBytes;
						  vpsSize = sPropRecords[i].sPropLength;
						}
					}
				}
			}
			Log(logLive555INFO, "%s vps size %d sps size %d pps %d\n", m_uri.c_str(), 
								vpsSize, spsSize, ppsSize);
			if (spsSize < IOT_SPS_PPS_MAX)
			{
				memcpy(m_sps, sps, spsSize);
				m_spsSize = spsSize;
			}
			if (ppsSize < IOT_SPS_PPS_MAX)
			{
				memcpy(m_pps, pps, ppsSize);
				m_ppsSize = ppsSize;
			}
			if (vpsSize < IOT_SPS_PPS_MAX)
			{
				memcpy(m_vps, pps, vpsSize);
				m_vpsSize = vpsSize;
			}
			
			if (sPropRecords)
			{
				delete[] sPropRecords;
			}
			break;
		}
		default:
		   	return false;
	};
	m_VideoCodec = codec;
	return true;
}

bool   IotRTSPClient::OnIotData(unsigned char* buffer, int size, unsigned long long secs, 
			unsigned long long msecs, CodecType codec)
{
	//Live555DumpHex(buffer, 100);
	//printf("Function %s (%lld, %lld) type %d len %d\n", __FUNCTION__, secs, msecs, frm, size);

	std::lock_guard<std::mutex> guard(m_pCBLock);

	IotCallbackMap::iterator it = m_pCBMap.begin(); 
	for(; it!=m_pCBMap.end(); ++it)
	{
		if ((*it).second)
		{
			(*it).second->OnData(buffer, size, secs, msecs, codec, 0, m_pInfo);
		}
	}
	return true;
}

bool IotRTSPClient::RegCallback(void * pIndex, IotRtspCallback * pCallback)
{
	std::lock_guard<std::mutex> guard(m_pCBLock);

	m_pCBMap[pIndex] = pCallback;

	return true;
}

bool IotRTSPClient::UnRegCallback(void * pIndex)
{
	std::lock_guard<std::mutex> guard(m_pCBLock);

	m_pCBMap.erase(pIndex);

	return true;
}


bool IotRTSPClient::Thread(void* pData)
{
	IotRTSPClient * pThread = (IotRTSPClient *)pData;

	if (pThread)
	{
		return pThread->ThreadRun();
	}
	return false;
}

bool IotRTSPClient::ThreadRun()
{
	Run();

	return true;
}
