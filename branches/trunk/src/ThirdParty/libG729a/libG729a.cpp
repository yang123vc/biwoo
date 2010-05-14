#include "libG729a.h"
#include "va_g729.h"

void CG729a::g729a_init_encoder(void)
{
	va_g729a_init_encoder();
}

void CG729a::g729a_encoder(const short * inBuffer, long size, unsigned char * outBuffer)
{
	int i=0;
	for (i=0; i<size/L_FRAME; i++)
	{
		va_g729a_encoder(inBuffer+(i*L_FRAME), outBuffer+(i*L_FRAME_COMPRESSED));
	}
}

void CG729a::g729a_init_decoder(void)
{
	va_g729a_init_decoder();
}

void CG729a::g729a_decoder(const unsigned char * inBuffer, long size, short * outBuffer, short * v1, short * v2)
{
	int i=0;
	for (i=0; i<size/L_FRAME_COMPRESSED; i++)
	{
		va_g729a_decoder(inBuffer+(i*L_FRAME_COMPRESSED), outBuffer+(i*L_FRAME), 0, v1, v2);
	}
}

