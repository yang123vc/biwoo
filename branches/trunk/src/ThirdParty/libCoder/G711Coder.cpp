#include "coder/g711.h"
#include "G711Coder.h"

void G711Coder::EncodeA(unsigned char* destBuf, const short* srcBuf,int nLen)
{
	for (int i= 0;i<nLen;i++)
	{
		destBuf[i] = linear2alaw(srcBuf[i]);
	}
}

void G711Coder::DecodeA(short* destBuf, const unsigned char* srcBuf,int nLen, short * v1, short * v2)
{
	for (int i= 0;i<nLen;i++)
	{
		destBuf[i] = alaw2linear(srcBuf[i]);
		if (v1 != 0 && destBuf[i] > *v1)
		{
			*v1 = destBuf[i];
		}else if (v2 != 0 && destBuf[i] < *v2)
		{
			*v2 = destBuf[i];
		}
	}
}

void G711Coder::EncodeU(unsigned char* destBuf, const short* srcBuf,int nLen)
{
	for (int i= 0;i<nLen;i++)
	{
		destBuf[i] = linear2ulaw(srcBuf[i]);
	}
}

void G711Coder::DecodeU(short* destBuf, const unsigned char* srcBuf,int nLen, short * v1, short * v2)
{
	for (int i= 0;i<nLen;i++)
	{
		destBuf[i] = ulaw2linear(srcBuf[i]);
		if (v1 != 0 && destBuf[i] > *v1)
		{
			*v1 = destBuf[i];
		}else if (v2 != 0 && destBuf[i] < *v2)
		{
			*v2 = destBuf[i];
		}
	}
}
