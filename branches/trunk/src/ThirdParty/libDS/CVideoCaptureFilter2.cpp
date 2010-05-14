//
// CVideoCaptureFilter2.cpp
//

#include "CVideoCaptureFilter2.h"

#include "UDsUtils.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

////////////////////////////////////////////////////////////////////////
CVideoCaptureFilter2::CVideoCaptureFilter2(IGraphBuilder * inGraph) : 
CDXFilter(inGraph, GUID_NULL, "Video Capture")
{
	mDevice = NULL;
}

CVideoCaptureFilter2::~CVideoCaptureFilter2(void)
{
	ReleaseFilter();
}

void CVideoCaptureFilter2::SetDevice(CAVDevice * inDevice)
{
	mDevice = inDevice;
}

BOOL CVideoCaptureFilter2::CreateFilter(void)
{
	if (mFilter)
	{
		return TRUE;
	}

	//ASSERT(mDevice);
	if (mGraph)
	{
		mFilter = UDsUtils::CreateCamera(*mDevice);
		if (mFilter)
		{
			if (SUCCEEDED(mGraph->AddFilter(mFilter, mName)))
			{
				// Settings on the capture filter of camera
				if (!mDevice->IsDVDevice())
				{					
					SetResolution();
					AdjustOutput(NULL);
				}

				return TRUE;
			}
		}
	}

	ReleaseFilter();
	return FALSE;
}

void CVideoCaptureFilter2::ReleaseFilter(void)
{
	CDXFilter::ReleaseFilter();
}

long CVideoCaptureFilter2::GetResolution(void)
{
	long resolution = 0;
	IAMAnalogVideoDecoder *	pDecoder = GetAnalogDecoder();
	if (pDecoder)
	{		
		pDecoder->get_TVFormat(&resolution);
	}
	return resolution;
}

IAMAnalogVideoDecoder * CVideoCaptureFilter2::GetAnalogDecoder(void)
{
	IAMAnalogVideoDecoder *	pDecoder = NULL;
	if (mFilter)
	{
		mFilter->QueryInterface(IID_IAMAnalogVideoDecoder, (void**)&pDecoder);
		if (pDecoder)
		{
			pDecoder->Release();
			return pDecoder;
		}
	}
	return NULL;
}

void CVideoCaptureFilter2::SetResolution(void)
{
	if (mDevice && mFilter)
	{
		IAMAnalogVideoDecoder *	pDecoder = GetAnalogDecoder();
		if (pDecoder)
		{
			pDecoder->put_TVFormat(mDevice->GetVideoResolution());
		}
	}
}

#define USE_MPEG4_FILTERS

// Select a media type first!
// 16 bits is preferred: 
void CVideoCaptureFilter2::AdjustOutput(IPin * pPin)
{
	HRESULT  hr         = S_OK;
	AM_MEDIA_TYPE * pmt = NULL;
	LONGLONG avgTimePerFrame = 500000;  // 20fps

#ifdef USE_MPEG4_FILTERS
	pmt = SelectMediaType(pPin, SMT_Other);
#else
	pmt = SelectMediaType(pPin, SMT_MinSampleSize);
#endif
	if (pmt)
	{
		if (pmt->formattype == FORMAT_VideoInfo) 
		{
			VIDEOINFOHEADER *pvi = (VIDEOINFOHEADER *) pmt->pbFormat;
			pvi->AvgTimePerFrame       = avgTimePerFrame;
			pvi->bmiHeader.biWidth     = Preferred_Width;
			pvi->bmiHeader.biHeight    = Preferred_Height;
			pvi->bmiHeader.biSizeImage = Preferred_Width * Preferred_Height * pvi->bmiHeader.biBitCount / 8;
			
			IAMStreamConfig * pCfg = GetStreamConfig(NULL);
			hr = pCfg->SetFormat(pmt);
		}
		DeleteMediaType(pmt);
	}
}
