// libG729a.h file here
#ifndef __libG729a_h__
#define __libG729a_h__

class CG729a
{
public:
	void g729a_init_encoder(void);
	void g729a_encoder(const short * inBuffer, long size, unsigned char * outBuffer);
	void g729a_init_decoder(void);
	void g729a_decoder(const unsigned char * inBuffer, long size, short * outBuffer, short * v1 = 0, short * v2 = 0);

};

#ifdef WIN32
#ifdef _DEBUG
#pragma comment(lib, "libG729ad.lib")
#else
#pragma comment(lib, "libG729a.lib")
#endif // _DEBUG
#endif // WIN32

#endif // __libG729a_h__
