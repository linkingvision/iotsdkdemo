//
// Copyright(c) 2019 linkingvision.com
// Distributed under Commercial license info@linkingvision.com
//
#ifndef __IOT_TYPE_H__
#define __IOT_TYPE_H__
#ifdef __cplusplus

#include <string.h>
#include <vector>
#include <iostream>
#include <thread>
#include <mutex>

#if defined(_WIN32)
#if defined(IOT_EXPORTS)
	#define IOT_LIBRARY_API __declspec(dllexport)
#else
	#define IOT_LIBRARY_API __declspec(dllimport)
#endif
#else
	#define IOT_LIBRARY_API
#endif

#ifndef  V_TYPE
#define V_TYPE
typedef  unsigned char u8;
typedef  char s8;
typedef  unsigned short u16;
typedef  short s16;
typedef  unsigned int  u32;
typedef  int s32;
#if defined(__GNUC__)
#if defined(__LP64__)
typedef  long  s64;
typedef  unsigned  long u64;
#else
typedef  long long s64;
typedef  unsigned  long long u64;
#endif
#else
typedef  long long s64;
typedef  unsigned  long long u64;
#endif

typedef unsigned long       DWORD;
typedef int                 BOOL;
typedef unsigned char       BYTE;
typedef unsigned short      WORD;
typedef float               FLOAT;

typedef char CHAR;
typedef short SHORT;
typedef long LONG;
typedef int INT;
typedef double DATE;

typedef signed char         INT8, *PINT8;
typedef signed short        INT16, *PINT16;
typedef signed int          INT32, *PINT32;
typedef signed long long    INT64, *PINT64;
typedef unsigned char       UINT8, *PUINT8;
typedef unsigned short      UINT16, *PUINT16;
typedef unsigned int        UINT32, *PUINT32;
typedef unsigned long long  UINT64, *PUINT64;

#ifndef TRUE 
#define TRUE 1
#endif

#ifndef FALSE
#define FALSE 0
#endif
#endif /* V_TYPE */


enum IotLogLevel
{
	IotLogERROR, 
	IotLogWARNING, 
	IotLogINFO, 
	IotLogDEBUG
};

class IIotLogInterface
{
public:
	virtual void Log(IotLogLevel level, char* str) = 0;
};

inline void IotSleep(int ms)
{
	std::this_thread::sleep_for(std::chrono::milliseconds(ms));
}

#endif /* __cplusplus */

#endif /* __IOT_TYPE_H__ */
