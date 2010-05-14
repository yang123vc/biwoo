//
// CPreviewAdmin.h
//

#ifndef __H_CPreviewAdmin__
#define __H_CPreviewAdmin__

//#define USE_MPEG4_FILTERS_TEST	1
#define USE_CATEGORY_PREVIEW		1
#define USE_CaptureGraphBuilder2	1

#include <initguid.h>
#include <atlbase.h>

// DivX Decoder Filter
/*
DEFINE_GUID(CLSID_DivXDecoderFilter, 
0x78766964, 0x0000, 0x0010, 0x80, 0x00, 0x00, 0xaa, 0x00, 0x38, 0x9b, 0x71);

DEFINE_GUID(CLSID_XivDDecoderFilter, 
0x64697678, 0x0000, 0x0010, 0x80, 0x00, 0x00, 0xaa, 0x00, 0x38, 0x9b, 0x71);

DEFINE_GUID(CLSID_3ivxMPEG4MediaSplitterFilter, 
0x52B63861, 0xDC93, 0x11CE, 0xA0, 0x99, 0x00, 0xAA, 0x00, 0x47, 0x9A, 0x58);

DEFINE_GUID(CLSID_3ivxMPEG4MediaMuxerFilter, 
0x12C79975, 0x0C8E, 0x46BF, 0xA6, 0x80, 0x2B, 0xAB, 0x64, 0xE2, 0xF4, 0xA4);

DEFINE_GUID(CLSID_3ivxMPEG4VideoEncoderFilter, 
0x01EF6E5C, 0xC79B, 0x4E3E, 0xA3, 0x20, 0x4B, 0x57, 0xB8, 0x4F, 0x18, 0x07);

DEFINE_GUID(CLSID_DivXVideoEncoder3ivxFilter, 
0x1A8016BD, 0x7209, 0x47DB, 0xB2, 0xCC, 0x0D, 0x12, 0xCF, 0x4D, 0x59, 0x87);

DEFINE_GUID(CLSID_3ivxMPEG4VideoDecoderFilter, 
0x0E6772C0, 0xDD80, 0x11D4, 0xB5, 0x8F, 0xA8, 0x6B, 0x66, 0xD0, 0x61, 0x1C);

DEFINE_GUID(CLSID_OverlayMixer2,
0xA0025E90, 0xE45B, 0x11D1, 0xAB, 0xE9, 0x00, 0xA0, 0xC9, 0x05, 0xF3, 0x75);

*/
DEFINE_GUID(CLSID_ffdshowVideoDecoderFilter,	// ffdshow Video Decoder Filter
0x04FE9017, 0xF873, 0x410E, 0x87, 0x1E, 0xAB, 0x91, 0x66, 0x1A, 0x4E, 0xF7);

DEFINE_GUID(CLSID_ffdshowVideoEncoderFilter,	// ffdshow Video Encoder Filter
0x4DB2B5D9, 0x4556, 0x4340, 0xB1, 0x89, 0xAD, 0x20, 0x11, 0x0D, 0x95, 0x3F);

DEFINE_GUID(CLSID_3ivxMPEG4AudioEncoderFilter,		// 3ivx MPEG4 Audio Encoder Filter
0x0EC0FC36, 0x22B0, 0x4631, 0x98, 0xDB, 0x25, 0x98, 0x30, 0x9F, 0xD5, 0xF9);

DEFINE_GUID(CLSID_ffdshowAudioDecoderFilter,		// ffdshow Audio Decoder Filter
0x0F40E1E5, 0x4F79, 0x4988, 0xB1, 0xA9, 0xCC, 0x98, 0x79, 0x4E, 0x6B, 0x55);

DEFINE_GUID(CLSID_ffdshowRawVideoFilter,		// ffdshow Raw Video Filter
0x0B390488, 0xD80F, 0x4A68, 0x84, 0x08, 0x48, 0xDC, 0x19, 0x9F, 0x0E, 0x97);

DEFINE_GUID(CLSID_MONOGRAMAACEncoderFilter,		// MONOGRAM AAC Encoder Filter
0x88F36DB6, 0xD898, 0x40B5, 0xB4, 0x09, 0x46, 0x6A, 0x0E, 0xEC, 0xC2, 0x6A);

DEFINE_GUID(CLSID_MainConceptH264DecoderFilter,		// MainConcept H.264/AVC Decoder Filter
0x03AF8740, 0x1764, 0x42F0, 0xA4, 0x58, 0x68, 0x75, 0xE4, 0x92, 0xC0, 0x77);

DEFINE_GUID(CLSID_MainConceptMPEGSplitterFilter,		// MainConcept MPEG Spliter Filter
0x2BE4D120, 0x6F2E, 0x4B3A, 0xB0, 0xBD, 0xE8, 0x80, 0x91, 0x72, 0x38, 0xDC);

DEFINE_GUID(CLSID_GDCLMPEG4DemultiplexorFilter,		// GDCL MPEG4 Demultiplexor Filter
0x025BE2E4, 0x1787, 0x4DA4, 0xA5, 0x85, 0xC5, 0xB2, 0xB9, 0xEE, 0xB5, 0x7C);

// subtype
//DEFINE_GUID(MEDIASUBTYPE_AAC,
//0x000000ff, 0x0000, 0x0010, 0x80, 0x00, 0x00, 0xaa, 0x00, 0x38, 0x9b, 0x71); 

DEFINE_GUID(MEDIASUBTYPE_XVID,
0x44495658, 0x0000, 0x0010, 0x80, 0x00, 0x00, 0xAA, 0x00, 0x38, 0x9B, 0x71);

DEFINE_GUID(MEDIASUBTYPE_H264,
0x34363248, 0x0000, 0x0010, 0x80, 0x00, 0x00, 0xAA, 0x00, 0x38, 0x9B, 0x71);


class IPin;
class CDXGraph;
class CDXFilter;
class CAVDevice;
//class CVideoWindow;
class CPreviewAdmin
{
protected:
	CDXGraph *		mGraph;
	//CVideoWindow *	mVideoWindow;
	HWND 			mVideoWindow;
	CAVDevice *		mVideoDevice;
	CAVDevice *		mAudioDevice;
	
	// Filters
	CDXFilter *		mVideoFilter;
	CDXFilter *		mAudioFilter;
	CDXFilter *		mVideoRenderer;
	CDXFilter *		mAudioRenderer;
	CDXFilter *		mVideoEncoderFilter;
	CDXFilter *		mVideoDecoderFilter;
//	CDXFilter *		mRawVideoFilter;
	CDXFilter *		mAacEncoderFilter;
	CDXFilter *		mAacDecoderFilter;

#ifdef USE_CATEGORY_PREVIEW
    CComPtr<ICaptureGraphBuilder2> graphBuilder2_;
    //ICaptureGraphBuilder2 *pBuilder;
#endif // USE_CATEGORY_PREVIEW

#ifdef USE_CaptureGraphBuilder2
    CComPtr<ICaptureGraphBuilder2> m_graphBuilder2_;
#endif // USE_CaptureGraphBuilder2

public:
	CPreviewAdmin();
	virtual ~CPreviewAdmin();

	void SetAVDevice(CAVDevice * inVideo, CAVDevice * inAudio);
	//void SetVideoWindow(CVideoWindow * inWindow);
	void SetVideoWindow(HWND inWindow);
	bool ShowHideFullScreen(void);

	virtual BOOL Activate(void);
	virtual void Deactivate(void);

	BOOL PlayGraph(void);
	BOOL PauseGraph(void);
	virtual BOOL StopGraph(void);

	CDXFilter * GetVideoInputFilter(void) {return mVideoFilter;}
	CDXFilter * GetAudioInputFilter(void) {return mAudioFilter;}

	void ShowVideoCapturePropertyPage(void);
	void ShowAudioCapturePropertyPage(void);

protected:
	virtual BOOL CreateGraph(void);
	virtual void DeleteGraph(void);

	virtual BOOL CreateInputFilters(void);
	virtual void ReleaseInputFilters(void);

	virtual BOOL CreateOutputFilters(void);
	virtual void ReleaseOutputFilters(void);

	virtual BOOL ConnectFilters(void);
	virtual BOOL AfterBuildGraph(void);

	// Utilities
	BOOL RenderDVVideo(CDXGraph * inGraph, IPin * inDVPin, IPin * inRenderedPin);
};

#endif // __H_CPreviewAdmin__