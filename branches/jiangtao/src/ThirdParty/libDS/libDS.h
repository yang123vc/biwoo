// libDS.h file here
#ifndef __libDS_h__
#define __libDS_h__

#include "DSProxy.h"

#ifdef WIN32
//#pragma  comment(lib, "ws2_32.lib")
// 应该连接
#pragma comment(lib,"strmiids.lib")
#pragma comment(lib,"quartz.lib")
#pragma comment(lib,"uuid.lib")
#pragma comment(lib,"winmm.lib")
// link directshow baseclasses lib
#ifdef UNICODE
#ifdef _DEBUG
#pragma comment(lib,"ustrmbasd.lib")
#else
#pragma comment(lib,"uSTRMBASE.lib")
#endif 
#else
#ifdef _DEBUG
#pragma comment(lib,"strmbasd.lib")
#else
#pragma comment(lib,"STRMBASE.lib")
#endif
#endif
#ifdef _DEBUG
//#pragma comment(lib,"msvcrtd.lib")
#pragma comment(linker,"/NODEFAULTLIB:libcmtd.lib") 
#else
//#pragma comment(lib,"msvcrt.lib")	// ??Release编译时，不同切换注释可以播放视频
#pragma comment(linker,"/NODEFAULTLIB:libcmt.lib") 
#endif
#endif

#ifdef WIN32
#ifdef _DEBUG
#pragma comment(lib, "libG729ad.lib")
#pragma comment(lib, "libDSd.lib")
#pragma comment(lib, "libCoderd.lib")
#else
#pragma comment(lib, "libG729a.lib")
#pragma comment(lib, "libDS.lib")
#pragma comment(lib, "libCoder.lib")
#endif // _DEBUG
#endif // WIN32

#endif // __libDS_h__
