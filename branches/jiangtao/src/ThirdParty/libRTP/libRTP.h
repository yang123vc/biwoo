// libRTP.h file here
#ifndef __libRTP_h__
#define __libRTP_h__

#include "Rtp.h"

#ifdef WIN32
#ifdef _DEBUG
#pragma comment(lib, "libRTPd.lib")
#else
#pragma comment(lib, "libRTP.lib")
#endif // _DEBUG
#endif // WIN32

#ifdef WIN32
#ifdef _DEBUG
//#pragma comment(linker,"/NODEFAULTLIB:uafxcwd.lib") 
//#pragma comment(lib, "uafxcwd.lib")
#pragma comment(lib, "Ws2_32.lib")
#pragma comment(lib, "jthreadd.lib")
#pragma comment(lib, "jrtplibd.lib")
#else
#pragma comment(linker,"/NODEFAULTLIB:libcmt.lib") 
#pragma comment(lib, "Ws2_32.lib")
#pragma comment(lib, "jthread.lib")
#pragma comment(lib, "jrtplib.lib")
#endif // _DEBUG
#endif // WIN32

#endif // __libRTP_h__
