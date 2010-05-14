// libCoder.h file here
#ifndef __libCoder_h__
#define __libCoder_h__

#define USES_XVID		1		// [0,1]
#define USES_FFMPEG		1		// [0,1]

// G711A & G711U
#include "G711Coder.h"

// Xvid
#if (USES_XVID)
#include "XvidProxy.h"
#endif

// ffmpeg
#if (USES_FFMPEG)
#include "AVCodecProxy.h"
#endif

#ifdef WIN32
#ifdef _DEBUG
#pragma comment(lib, "libCoderd.lib")
#else
#pragma comment(lib, "libCoder.lib")
#endif // _DEBUG
#endif // WIN32

#endif // __libCoder_h__
