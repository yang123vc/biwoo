#pragma once
#ifndef _CAVCodecProxy_h_FILE_INCLUDED__
#define _CAVCodecProxy_h_FILE_INCLUDED__

#define nptr NULL

extern "C"
{
//#include "C:/Documents and Settings/Administrator/桌面/CLient/drffmpeg/export/drffmpeg/avcodec.h"
#include "libavformat/avformat.h"
#include "libavdevice/avdevice.h"
#include "libavcodec/avcodec.h"
#include "libswscale/swscale.h"
}


class FVInfo
{
public:
	FVInfo(const FVInfo & v)
	{
		equal(v);
	}
	const FVInfo & operator = (const FVInfo & v)
	{
		equal(v);
		return *this;
	}
	void csp(PixelFormat newv) {m_csp = newv;}
	PixelFormat csp(void) const {return m_csp;}
	void bitcount(int newv) {m_bitcount = newv;}
	int bitcount(void) const {return m_bitcount;}

	void width(int newv) {m_width = newv;}
	int width(void) const {return m_width;}
	void height(int newv) {m_height = newv;}
	int height(void) const {return m_height;}	

	int stride(void) const {return m_width * m_bitcount / 8;}
	int framesize(void) const {return stride() * m_height;}

private:
	void equal(const FVInfo & v)
	{
		this->m_csp = v.csp();
		this->m_bitcount = v.bitcount();
		this->m_width = v.width();
		this->m_height = v.height();
	}
public:
	FVInfo(PixelFormat csp, int bitcount=32, int width=320, int height=240)
		: m_csp(csp), m_bitcount(bitcount)
		, m_width(width), m_height(height)
	{}
	FVInfo(void)
		: m_csp(PIX_FMT_RGB32), m_bitcount(32)
		, m_width(320), m_height(240)
	{
	}
private:
	PixelFormat m_csp;
	int m_width;
	int m_height;
	int m_bitcount;
};


//定义目标格式
//#define DEST_FORMAT PIX_FMT_BGR24
//#define DEST_FORMAT PIX_FMT_RGBA

//#define DEST_FORMAT PIX_FMT_RGB32
//#define DEFAULT_BPP	4

////////////////
class _AVReadHandler_
{
public:
	virtual void onReadAudio(const int16_t * audioFrame) = 0;
	virtual void onReadVideo(const AVFrame & videoFrame) = 0;

};

class CAVCodecProxy
{
private:
	AVFormatContext *pFormatCtx;   //
	AVCodecContext  *pVideoCodecCtx, *pAudioCodecCtx;    
	AVCodec         *pVideoCodec, *pAudioCodec;      //编解码器
	int             videoStream, audioStream;
	uint8_t         *m_pBufferDec;
	uint8_t         *m_pBufferEnc;     

	AVFrame         *m_pFrameRGB;			//YUV帧
	int16_t			*pAutionSample;
	
	AVPacket		avPacket;                    //AV包。
	unsigned char * m_samples;
	unsigned char * m_bit_buffer;

	// decode
	bool			m_bFindKeyFrame;
	FVInfo			m_decFvinfo;
	AVFrame *		m_pFrameDecode;
	SwsContext *	m_swsContextDecode;

	// encode
	AVCodecContext *m_pVideoCodecCtxEnc;
	AVFrame *		m_pFrameEncode;
	SwsContext *	m_swsContextEncode;

	// handler
	_AVReadHandler_ * m_handler;

public:
	long openfile(char *filename,long *out_width , long *out_height,long *out_framenum, long *out_bufsize);
	bool startread(void);	// 有时间试一下
	void closeav(void);
	long GetNextFrame(unsigned char *lpdata);

	void setdec_info(const FVInfo & fvinfo) {m_decFvinfo = fvinfo;}
	bool openav(enum CodecID codecID, const unsigned char * extradata=0, int extrasize=0);
	bool isopenav(void) const;
	bool decode(const unsigned char * inBuffer, int inLength, unsigned char ** outBuffer, int & outSize, bool reversal = false);
	bool encode(const unsigned char * inBuffer, int inLength, unsigned char ** outBuffer, int & outSize);

protected:

public:
	static bool GetNextFrame(AVFormatContext *pFormatCtx, AVCodecContext *pCodecCtx, int videoStream, AVFrame *pFrame);
	CAVCodecProxy(void);
	virtual ~CAVCodecProxy(void);
};

#endif // _CAVCodecProxy_h_FILE_INCLUDED__
