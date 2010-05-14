//
// CXInputPin.cpp
//

#include <streams.h>
#include <dvdmedia.h>
#include "CFilterNetSender.h"
#include "CXInputPin.h"
#include "stdio.h"
#include "sys/timeb.h"
#include "time.h"
//#include "../G711Coder.h"
//#include "Base64.h"

/////////////////////////////////////////////////////////////////////////////////////////
CXInputPin::CXInputPin(CFilterNetSender *pFilter, HRESULT *phr) :
CRenderedInputPin(NAME("Input Pin"), pFilter, &pFilter->mFilterLock, phr, L"Input")
, m_handler(NULL)
, preEndTime(0)
, m_bIsLiveMedia(true)
, m_lastEndTime(0)

{
	mFilter      = pFilter;
	mIsMediaTypeSent = FALSE;
}

CXInputPin::~CXInputPin()
{
}
#define USES_FFMPEGTEST		0	// [0,1]

#if (USES_FFMPEGTEST)
#include "AVCodecProxy.h"
static CAVCodecProxy avcodecProxy;
#endif // USES_FFMPEGTEST


STDMETHODIMP CXInputPin::Receive(IMediaSample *pSample)
{
	if (m_handler == NULL) return S_OK;

	int stepLen = 0 ; ///增加变量
	CAutoLock  lck(&mReceiveLock);
	HRESULT hr = CRenderedInputPin::Receive(pSample);

	if (SUCCEEDED(hr))
	{
		if (m_SampleProps.dwStreamId == AM_STREAM_MEDIA) 
		{
			PBYTE pData;
			pSample->GetPointer(&pData);
			long length = pSample->GetActualDataLength();
			int  bytes  = 0;

			//MessageBox(NULL, L"CXInputPin::Receive data", L"test", MB_OK);
			struct timeval presentationTime;
			struct timeb tb;
			ftime(&tb);
			presentationTime.tv_sec = tb.time;
			presentationTime.tv_usec = 1000*tb.millitm;

			REFERENCE_TIME rtStart = 0;
			REFERENCE_TIME rtEnd = 0;
			pSample->GetTime(&rtStart, &rtEnd);

			unsigned int timestamp = (m_lastEndTime == 0) ? (rtEnd - rtStart) : rtEnd - m_lastEndTime;
			m_lastEndTime = rtEnd;

			bool bIsVideo = (m_mt.majortype == MEDIATYPE_Video);
			if (bIsVideo)
			{
				/*BYTE b0 = pData[0];
				BYTE b1 = pData[1];
				BYTE b2 = pData[2];
				BYTE b3 = pData[3];*/

				if (!m_bIsLiveMedia)
				{
					long len = rtEnd-rtStart;
					long mediaLen = rtStart - preEndTime;
					if (preEndTime == 0)
					{
						stepLen = 1;
					}else
					{
						stepLen = mediaLen/len;
						preEndTime = mediaLen % len;
					}
					preEndTime += rtStart;

					if (stepLen > 1)
					{
						for (int i=1;i< stepLen;i++)
						{
							BYTE t[10]={0x00,0x00,0x01,0xb6,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF};
							m_handler->onReceive(t, 10, timestamp, bIsVideo);
						}
					}

					if (len != 400000)
						int i=0;
				}
			}

			m_handler->onReceive(pData, length, timestamp, bIsVideo);
		}
	}
	
	return hr;
}

STDMETHODIMP CXInputPin::EndOfStream(void)
{
	return CRenderedInputPin::EndOfStream();
}

STDMETHODIMP CXInputPin::BeginFlush(void)
{
	CAutoLock lck(&mFilter->mFilterLock);

	return CRenderedInputPin::BeginFlush();
}

STDMETHODIMP CXInputPin::EndFlush(void)
{
	CAutoLock lck(&mFilter->mFilterLock);

	return CRenderedInputPin::EndFlush();
}

STDMETHODIMP CXInputPin::ReceiveCanBlock(void)
{
	return S_OK;
}

#include <initguid.h>

DEFINE_GUID(MEDIASUBTYPE_XVID,
0x44495658, 0x0000, 0x0010, 0x80, 0x00, 0x00, 0xAA, 0x00, 0x38, 0x9B, 0x71);

DEFINE_GUID(MEDIASUBTYPE_H264,
0x34363248, 0x0000, 0x0010, 0x80, 0x00, 0x00, 0xAA, 0x00, 0x38, 0x9B, 0x71);

DEFINE_GUID(MEDIASUBTYPE_X264,
0x34363258, 0x0000, 0x0010, 0x80, 0x00, 0x00, 0xAA, 0x00, 0x38, 0x9B, 0x71);

DEFINE_GUID(MEDIASUBTYPE_AVC1,
0x31435641, 0x0000, 0x0010, 0x80, 0x00, 0x00, 0xAA, 0x00, 0x38, 0x9B, 0x71);

/*
#if (USES_H264CODEC)
HRESULT CXInputPin::GetMediaType(int iPosition,CMediaType *pMediaType)
{
	ASSERT (pMediaType) ;
	if (iPosition == 0)
	{
		pMediaType -> InitMediaType () ;
		////////修改了DSnetwork原有的MPEG2的MEDIATYPE和MEDIASUBTYPE
		pMediaType -> SetType (& MEDIATYPE_Video) ;
		pMediaType -> SetSubtype (& TIME_FORMAT_NONE) ;//适应任何形式
		//pMediaType -> SetSubtype (& MEDIASUBTYPE_AVC1) ;
		pMediaType->SetFormatType(&FORMAT_VideoInfo);
/*
		VIDEOINFOHEADER *pvi = (VIDEOINFOHEADER *) pMediaType->AllocFormatBuffer(sizeof(VIDEOINFOHEADER));
		if(NULL == pvi)
			return(E_OUTOFMEMORY);

		ZeroMemory(pvi, sizeof(VIDEOINFOHEADER));
		pvi->bmiHeader.biCompression = mmioFOURCC('A','V','C','1');
		pvi->bmiHeader.biBitCount = 24;
		pvi->bmiHeader.biSize       = sizeof(BITMAPINFOHEADER);
		pvi->bmiHeader.biWidth      = 320;
		pvi->bmiHeader.biHeight     = 240;

		pvi->bmiHeader.biPlanes     = 1;
		pvi->bmiHeader.biSizeImage  = 320*240*3/2;
		pvi->bmiHeader.biClrImportant = 0;
		pvi->dwBitRate                = 0;//m_iBitRate;
		pvi->AvgTimePerFrame        = 10000000/20;//400400000;

		pMediaType->cbFormat = sizeof(VIDEOINFOHEADER);
		SetRectEmpty(&(pvi->rcSource)); // we want the whole image area rendered.
		SetRectEmpty(&(pvi->rcTarget)); // no particular destination rectangle

		pvi->rcSource.right = 320;
		pvi->rcSource.bottom  = 240;
		pvi->rcTarget.right = 320;
		pvi->rcTarget.bottom  = 240;

		pMediaType->SetType(&MEDIATYPE_Video);
		pMediaType->SetFormatType(&FORMAT_VideoInfo);
		pMediaType->SetTemporalCompression(TRUE);
		pMediaType->SetSubtype(&MEDIASUBTYPE_AVC1);
		pMediaType->SetSampleSize(pvi->bmiHeader.biSizeImage);
*//*
		return S_OK ;
	}

	return VFW_S_NO_MORE_ITEMS;
}
#endif // USES_H264CODEC
*/

HRESULT CXInputPin::CheckMediaType(const CMediaType * inMediaType)
{
	if (inMediaType->formattype == FORMAT_WaveFormatEx)
		return S_OK;

	if((*inMediaType->Type()) != MEDIATYPE_Video)
	{
		return VFW_E_INVALIDMEDIATYPE;
	} 
#if (USES_H264CODEC)
	if((*inMediaType->Subtype()) != MEDIASUBTYPE_H264 &&
		(*inMediaType->Subtype()) != MEDIASUBTYPE_X264 &&
		(*inMediaType->Subtype()) != MEDIASUBTYPE_AVC1)
	{
		return VFW_E_INVALIDMEDIATYPE;
	}
#endif // (USES_H264CODEC)
	return S_OK;
}

/*
void CXInputPin::CancelPendingSend(void)
{

}
*/