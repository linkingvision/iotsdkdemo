//
// Copyright(c) 2019 linkingvision.com
// Distributed under Commercial license info@linkingvision.com
//
#ifndef __IOT_VIDEO_TYPE_H__
#define __IOT_VIDEO_TYPE_H__
#include "iottype.hpp"

typedef enum __CodecType
{
	CODEC_PCMU = 0, 
	CODEC_PCMA = 8,
	CODEC_G711A = 19,
	CODEC_G711U = 20, 
	CODEC_H264 = 96,
	CODEC_H265 = 98,
	CODEC_MPEG4 = 97,
	CODEC_MJPEG = 26,
	CODEC_AAC = 100,
	CODEC_NONE = 254,
	CODEC_LAST = 1000
}CodecType;

typedef enum __VideoStreamType
{
	VIDEO_STREAM_VIDEO = 1,
	VIDEO_STREAM_AUDIO,
	VIDEO_STREAM_INFO,
	VIDEO_STREAM_TEXT,
	VIDEO_STREAM_SDP,
	VIDEO_STREAM_END,
	VIDEO_STREAM_LAST
}VideoStreamType;

typedef enum __VideoFrameType
{
	VIDEO_FRM_NONE = 0,
	VIDEO_FRM_I = 1,
	VIDEO_FRM_P = 2,
	VIDEO_FRM_B = 3,
	VIDEO_FRM_INFO = 4,
	VIDEO_FRM_AUDIO = 5,
	VIDEO_FRM_LAST
} VideoFrameType;

typedef struct __VideoFrame
{
	VideoStreamType streamType;
 	VideoFrameType frameType;
 	u64 secs;       /* timestamp in seconds */
	u32 msecs;      /* timestamp in mseconds */
	u32 dataLen; /* the length of the payload data */
	u8   *dataBuf;
	u32 bufLen;/* the length of the buffer */
	u32 ptsdiff;/* video Presentation timestamp - Decompression timestamp */
}VideoFrame;

typedef struct __VideoStreamInfo
{
	CodecType codec;
	int width;
	int height;
	int fps;
}VideoStreamInfo;

typedef struct __VideoFrameHeader
{
	u32 streamType; /* VideoStreamType */
 	u32 frameType; /* VideoFrameType */
	u32 seq;/* seq for frame lost */
 	u64 secs;       /* timestamp in seconds */
	u32 msecs;      /* timestamp in mseconds */
	u32 dataLen;
	u32 ptsdiff;/* video Presentation timestamp - Decompression timestamp */
}VideoFrameHeader;


typedef struct __InfoFrameP /* All P frame use this */
{
	u8 video;/* CodecType */
	u8 audio;/* CodecType */
	u16 padding1;
}InfoFrameP;

typedef struct __InfoFrameI /* All I frame use thie */
{
	u8 video;/* CodecType */
	u8 audio;/* CodecType */
	u16 padding0;
	u16 vFps;
	u32 vWidth;
	u32 vHeight;
	u16 aSampleRate;
	u16 aSampleBit;
	u16 aChannels;
	u32 padding1;
	u32 padding2;
	u32 padding3;
	u32 padding4;
	u16 padding5;
}InfoFrameI;

/* Must same with RAPID_MAX_EXTRA_SIZE don't change that !!!*/
#define EXTRA_MAX_SIZE 1024 * 4
typedef struct __InfoFrame
{
	CodecType video;
	u32 vWidth;
	u32 vHeight;
	u32 vFps;
	u8 vExtra[EXTRA_MAX_SIZE];
	u32 vExtraLen;
	CodecType audio;
	u32 aSampleRate;
	u32 aSampleBit;
	u32 aChannels;
	u8 aExtra[EXTRA_MAX_SIZE];
	u32 aExtraLen;
	u32 padding1;
	u32 padding2;
	u32 padding3;
	u32 padding4;
	u32 padding5;
	u32 padding6;
	u32 padding7;
	u32 padding8;
}InfoFrame;

typedef enum __VideoRawType
{
	VIDEO_RAW_VIDEO = 1,
	VIDEO_RAW_AUDIO,
	VIDEO_RAW_TEXT,
	VIDEO_RAW_LAST
}VideoRawType;

typedef struct __RawFrame {
	VideoRawType type;
	CodecType codec;
	u32 secs;       /* timestamp in seconds */
	u32 msecs;      /* timestamp in mseconds */
#define VE_NUM_POINTERS 8
	char *data[VE_NUM_POINTERS];
	int linesize[VE_NUM_POINTERS];
	int width, height;
	int format;
	u16 aSampleRate;
	u16 aSampleBit;
	u16 aChannels;
} RawFrame;

#endif /* __IOT_VIDEO_TYPE_H__ */
