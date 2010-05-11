
#define libDS_STATIC 1	// 0,1

#ifndef LIBDS_CLASS
#if libDS_STATIC
#define LIBDS_CLASS 
#else
#ifdef WIN32
#ifdef libDS_EXPORTS
#define LIBDS_CLASS __declspec(dllexport)
//#define LIB_CLASS 
#else
#define LIBDS_CLASS __declspec(dllimport)
//#define LIB_CLASS 
#endif // libDS_EXPORTS
#else
#define LIBDS_CLASS 
#endif // WIN32
#endif // libDS_STATIC
#endif // LIBDS_CLASS
