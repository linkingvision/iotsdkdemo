/**********
This library is free software; you can redistribute it and/or modify it under
the terms of the GNU Lesser General Public License as published by the
Free Software Foundation; either version 2.1 of the License, or (at your
option) any later version. (See <http://www.gnu.org/copyleft/lesser.html>.)

This library is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for
more details.

You should have received a copy of the GNU Lesser General Public License
along with this library; if not, write to the Free Software Foundation, Inc.,
51 Franklin Street, Fifth Floor, Boston, MA 02110-1301  USA
**********/
// Copyright (c) 2014-2018 veyesys.com. All rights reserved.
// Live555 RTSP client wrapper

#include "live555_rtsp_client.hpp"
#include "GroupsockHelper.hh"
#include "BasicTaskSchedulerEpoll.h"


Live555Environment::Live555Environment(Live555LogInterface & pLog) : 
#if defined(OSX)
//#if 1 
BasicUsageEnvironment(*BasicTaskScheduler::createNew()), m_stop(0), 
#else //Epoll need optimize
BasicUsageEnvironment(*BasicTaskSchedulerEpoll::createNew()), m_stop(0), 
#endif
m_pLog(pLog)
{
}


UsageEnvironment& Live555Environment::operator<<(char const* str)
{
	m_pLog.Log(logLive555INFO, "%s", str);
	return *this;
}
UsageEnvironment& Live555Environment::operator<<(int i)
{
	m_pLog.Log(logLive555INFO, "%d", i);
	return *this;
}
UsageEnvironment& Live555Environment::operator<<(unsigned u)
{
	m_pLog.Log(logLive555INFO, "%u", u);
	return *this;
}
UsageEnvironment& Live555Environment::operator<<(double d)
{
	m_pLog.Log(logLive555INFO, "%f", d);
	return *this;
}
UsageEnvironment& Live555Environment::operator<<(void* p)
{
	m_pLog.Log(logLive555DEBUG, "%p", p);
	return *this;
}

Live555Environment::~Live555Environment()
{
	TaskScheduler* scheduler = &this->taskScheduler();
	this->reclaim();
	delete scheduler;	
}

Live555RTSPConnection::SessionSink::SessionSink(UsageEnvironment& env, Callback* callback, 
	Live555CodecType codec, char const* spropparametersets) 
	: MediaSink(env)
	, m_bufferSize(0)
	, m_buffer(NULL)
	, m_callback(callback) 
	, m_markerSize(0)
	, m_codecType(codec)
	, m_bFirstFrame(true)
{
	if (spropparametersets)
	{
		m_fmtp_spropparametersets = spropparametersets;
		envir() << "fmtp spropparametersets " << m_fmtp_spropparametersets.c_str() << "\n";
	}
	allocate(1024*1024);
}

Live555RTSPConnection::SessionSink::~SessionSink()
{
	delete [] m_buffer;
}

void Live555RTSPConnection::SessionSink::allocate(int bufferSize)
{
	m_bufferSize = bufferSize;
	m_buffer = new u_int8_t[m_bufferSize];
	if (m_callback)
	{
		m_markerSize = m_callback->onNewBuffer(m_buffer, m_bufferSize);
		envir() << "markerSize:" << m_markerSize << "\n";
	}
}


void Live555RTSPConnection::SessionSink::afterGettingFrame(unsigned frameSize, unsigned numTruncatedBytes, 
				struct timeval presentationTime, unsigned durationInMicroseconds)
{
	//LIVE555_LOG << "NOTIFY size:" << frameSize;
	if (numTruncatedBytes != 0)
	{
		delete [] m_buffer;
		envir()  << "buffer too small " << (int)m_bufferSize << " allocate bigger one\n";
		allocate(m_bufferSize*2);
	}
	else if (m_callback)
	{
		/* first frame set the  m_strSpropparametersets TODO update for H265*/
		if (m_bFirstFrame == true)
		{
			m_callback->onParameter("sprop-parameter-sets", m_fmtp_spropparametersets);
			m_bFirstFrame = false;
		}
		//printf("durationInMicroseconds %d\n", durationInMicroseconds);
		//gettimeofday(&presentationTime, NULL);
		if (!m_callback->onData(m_buffer, frameSize+m_markerSize, 
						presentationTime.tv_sec, presentationTime.tv_usec/1000, m_codecType))
		{
			envir()  << "NOTIFY failed\n" ;
		}
	}
	this->continuePlaying();
}

Boolean Live555RTSPConnection::SessionSink::continuePlaying()
{
	Boolean ret = False;
	if (source() != NULL)
	{
		source()->getNextFrame(m_buffer+m_markerSize, m_bufferSize-m_markerSize,
				afterGettingFrame, this,
				onSourceClosure, this);
		ret = True;
	}
	return ret;	
}


		
Live555RTSPConnection::Live555RTSPConnection(UsageEnvironment& env, Callback* callback, const std::string & rtspURL, 
	std::string strUser, std::string strPasswd, bool bTCP, int verbosityLevel, 
	double nSpeed)
				: RTSPClient(env, rtspURL.c_str(), verbosityLevel, NULL, 0
#if LIVEMEDIA_LIBRARY_VERSION_INT > 1371168000 
					,-1
#endif
					)
				, m_session(NULL)
				, m_subSession(NULL)
				, m_subSessionIter(NULL)
				, m_callback(callback)
				, m_Authenticator(strUser.c_str(), strPasswd.c_str())
				, m_bTCP(bTCP)
				, m_strUrl(rtspURL)
				, m_nSpeed(nSpeed)
				, m_bPlayed(false)
{
	// initiate connection process
	//this->sendNextCommand();
	this->sendOptions();
}

Live555RTSPConnection::~Live555RTSPConnection()
{
	delete m_subSessionIter;
	// free subsession
	if (m_session != NULL) 
	{
		MediaSubsessionIterator iter(*m_session);
		MediaSubsession* subsession;
		while ((subsession = iter.next()) != NULL) 
		{
			if (subsession->sink) 
			{
				envir() << "Close session: " << subsession->mediumName() << "/" << subsession->codecName() << "\n";
				Medium::close(subsession->sink);
				subsession->sink = NULL;
			}
		}	
		Medium::close(m_session);
	}

}

void Live555RTSPConnection::sendOptions()
{
	sendOptionsCommand(continueAfterOPTIONS, &m_Authenticator);
}

bool Live555RTSPConnection::Pause()
{
	if (m_bPlayed == false)
	{
		return false;
	}
	sendPauseCommand(*m_session, continueAfterPAUSE, &m_Authenticator);
	return true;
}
bool Live555RTSPConnection::Resume()
{
	if (m_bPlayed == false)
	{
		return false;
	}
	sendPlayCommand(*m_session, continueAfterPLAY, (double)0, (double)-1, m_nSpeed, &m_Authenticator);

	return true;
}
bool Live555RTSPConnection::Seek(long long nTime)
{
	if (m_bPlayed == false)
	{
		return false;
	}
	sendPlayCommand(*m_session, continueAfterPLAY, (double)nTime, (double)-1, m_nSpeed, &m_Authenticator);

	return true;
}
bool Live555RTSPConnection::SetSpeed(double nSpeed)
{
	if (m_bPlayed == false)
	{
		return false;
	}
	sendPlayCommand(*m_session, continueAfterPLAY, (double)0, (double)-1, nSpeed, &m_Authenticator);
	m_nSpeed = nSpeed;

	return true;
}


void Live555RTSPConnection::continueAfterOPTIONS(int resultCode, char* resultString)
{
	if (resultCode != 0) 
	{
		envir()  << "Failed to OPTIONS: " << m_strUrl.c_str() << resultString << "\n";
	}
	else
	{
		 sendNextCommand();

	}
	delete[] resultString;
}


void Live555RTSPConnection::sendNextCommand() 
{
	if (m_subSessionIter == NULL)
	{
		// no SDP, send DESCRIBE
		sendDescribeCommand(continueAfterDESCRIBE, &m_Authenticator); 
	}
	else
	{
		m_subSession = m_subSessionIter->next();
		if (m_subSession != NULL) 
		{
			// still subsession to SETUP
			if (!m_subSession->initiate()) 
			{
				envir()  << "Failed to initiate " << m_subSession->mediumName() << "/" << m_subSession->codecName() << " subsession: " 
								<< envir().getResultMsg() << "\n";
				sendNextCommand();
			} 
			else 
			{					
				envir()  << "Initiated " << m_subSession->mediumName() << "/" << m_subSession->codecName() << " subsession\n";
			}

			/* Change the multicast here */
			sendSetupCommand(*m_subSession, continueAfterSETUP, False, m_bTCP, False, &m_Authenticator);
		}
		else
		{
			// no more subsession to SETUP, send PLAY
			sendPlayCommand(*m_session, continueAfterPLAY, (double)0, (double)-1, m_nSpeed, &m_Authenticator);
		}
	}
}

void Live555RTSPConnection::continueAfterDESCRIBE(int resultCode, char* resultString)
{
	if (resultCode != 0) 
	{
		envir()  << "Failed to DESCRIBE: " << m_strUrl.c_str() << resultString << "\n";
	}
	else
	{
		envir()  << "Got SDP:" << resultString << "\n";
		m_strSDP = resultString;
		m_callback->onSDP(m_strSDP);
		m_session = MediaSession::createNew(envir(), resultString);
		
		m_subSessionIter = new MediaSubsessionIterator(*m_session);
		sendNextCommand();  
	}
	delete[] resultString;
}

void Live555RTSPConnection::continueAfterSETUP(int resultCode, char* resultString)
{
	if (resultCode != 0) 
	{
		envir()  << "Failed to SETUP:" << resultString << "\n";
	}
	else
	{	
		Live555CodecType codec = GetSessionCodecType(m_subSession->mediumName(), m_subSession->codecName());
		m_subSession->sink = SessionSink::createNew(envir(), m_callback, codec, 
			m_subSession->fmtp_spropparametersets());
		if (m_subSession->sink == NULL) 
		{
			envir()  << "Failed to create a data sink for " << m_subSession->mediumName() << "/" << m_subSession->codecName() << " subsession: " << envir().getResultMsg() << "\n";
		}
		else if (m_callback->onNewSession(codec, m_subSession->mediumName(), m_subSession->codecName()))
		{
			envir()  << "Created a data sink for the \"" << m_subSession->mediumName() << "/" << m_subSession->codecName() << "\" subsession\n" ;
			m_subSession->sink->startPlaying(*(m_subSession->readSource()), NULL, NULL);
		}
	}
	delete[] resultString;
	sendNextCommand();  
}	

void Live555RTSPConnection::continueAfterPLAY(int resultCode, char* resultString)
{
	if (resultCode != 0) 
	{
		envir()  << "Failed to PLAY: \n" << resultString << "\n";
	}
	else
	{
		envir()  << "PLAY OK\n";
		m_bPlayed = true;
	}
	delete[] resultString;
}

void Live555RTSPConnection::continueAfterPAUSE(int resultCode, char* resultString)
{
	if (resultCode != 0) 
	{
		envir()  << "Failed to PAUSE: \n" << resultString << "\n";
	}
	else
	{
		envir()  << "PAUSE OK\n";
	}
	delete[] resultString;
}


void Live555RTSPClient::Log(Live555LogLevel level, const char* format, ... )
{
	return;
}


