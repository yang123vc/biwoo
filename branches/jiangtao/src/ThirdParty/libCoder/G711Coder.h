// G711Coder.h file here
#ifndef __G711Coder_h__
#define __G711Coder_h__

class G711Coder
{
public:
	static void EncodeA(unsigned char* destBuf, const short* srcBuf, int nLen);
	static void DecodeA(short* destBuf, const unsigned char* srcBuf, int nLen, short * v1 = 0, short * v2 = 0);
	static void EncodeU(unsigned char* destBuf, const short* srcBuf, int nLen);
	static void DecodeU(short* destBuf, const unsigned char* srcBuf, int nLen, short * v1 = 0, short * v2 = 0);
};

#endif // __G711Coder_h__
