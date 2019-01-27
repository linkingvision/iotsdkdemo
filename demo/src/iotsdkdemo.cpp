//
// Copyright(c) 2019 linkingvision.com
// Distributed under Commercial license info@linkingvision.com
//
#include <thread>
#include <mutex>
#include <chrono>
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#ifdef _WIN32
#include <windows.h>
#include <winsock2.h> /* DTL add for SO_EXCLUSIVE */
#include <ws2tcpip.h>
#endif
#include "demodevice.hpp"

/*
 This device demo will connect to url as a channel, the chnum is the channel of this device, 
 this device will connect chnum times to the url.
*/

void Usage()
{
	printf("iotsdkdemo url(rtsp://192.168.1.1/stream1) user(admin) password(12345) chnum(2)\n"
            "devname(test1) cloudip(10.0.0.1) cloudport(8080) clouduser(admin) cloudpassword(12345) ssl(0/1)\n");
	return;
}

int main(int argc, char *argv[])
{
#ifdef _WIN32
	WSADATA data;
	WSAStartup(MAKEWORD(2, 2), &data);
#endif

	if (argc < 11)
	{
		Usage();
		return 0;
	}

	std::string strUrl = argv[1];
	std::string strUser = argv[2];
	std::string strPassWd = argv[3];

	std::string strChnum = argv[4];

	int nChnum = std::stoi(strChnum);

	std::string strDevName = argv[5];
	std::string strCloudIp = argv[6];
	std::string strCloudPort = argv[7];
	std::string strCloudUser = argv[8];
	std::string strCloudPassWd = argv[9];

	std::string strBSSL = argv[10];

	bool bSSL = false;
	if (strBSSL == "1")
	{
	    bSSL = true;
	}

	DemoDevice *pDev = new DemoDevice(strUrl, strUser, strPassWd, nChnum, strDevName, strDevName, strCloudIp, strCloudPort,
	                strCloudUser, strCloudPassWd, bSSL);

	int j = 0;
	while (1)
	{
		std::this_thread::sleep_for(std::chrono::seconds(1));
		j ++;

	}
		

}
