// libCoder.h file here
#ifndef __libCoder_h__
#define __libCoder_h__

// G711A & G711U
#include "G711Coder.h"

// Xvid
//#include "XvidProxy.h"

// ffmpeg
//#include "AVCodecProxy.h"


#ifdef WIN32
#ifdef _DEBUG
#pragma comment(lib, "libCoderd.lib")
#else
#pragma comment(lib, "libCoder.lib")
#endif // _DEBUG
#endif // WIN32

#endif // __libCoder_h__
