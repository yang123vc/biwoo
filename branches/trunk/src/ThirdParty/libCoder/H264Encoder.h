#ifndef __H264Encoder_h__
#define __H264Encoder_h__

extern "C"
{
	#include "stdint.h"
	#include "x264.h"
}

#define TBCOLORSPACE_YV12 0
#define TBCOLORSPACE_I420 1
#define TBCOLORSPACE_YUY2 2
#define TBCOLORSPACE_IYUV 3
#define TBCOLORSPACE_RGB24 4
#define MAX_PATH 260

class CH264Encoder
{
public:
	CH264Encoder(void);
	~CH264Encoder(void);
	bool initH264Codec(int nWidth, int nHeight, unsigned short usMaxKeyframe, int nVideoQuality,int nTBColorSpace,bool bCaptureIFrame);
	int Encode(unsigned char* pInputBuffer, int nInputSize,unsigned char* pOutputBuffer, int& nOutputSize, bool& bKeyFrame);
	void releaseCodec();
private:
	x264_t* m_Codec;
	int m_width;
	int m_height;
	int m_sizeImage;
	int framecount ;
	x264_picture_t m_pic;
};

#endif // __H264Encoder_h__
