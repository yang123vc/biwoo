//
// CNetOutPin.cpp
//

#include "GlobalDefs.h"
#include <streams.h>
#include "CFilterNetReceiver.h"
#include "CNetOutPin.h"

///////////////////////////////////////////////////////////////////////////////
CNetOutPin::CNetOutPin(CFilterNetReceiver *inFilter, HRESULT *phr, LPCWSTR pName) :
CBaseOutputPin(NAME("Net_Stream"), inFilter, &inFilter->mFilterLock, phr, pName)
//, m_avcoding(AVCoding::CT_Other)

{
	mFilter = inFilter;
	mPreferredMt.InitMediaType();

	// Test
/*	mPreferredMt.SetType(&MEDIATYPE_Video);
	mPreferredMt.SetSubtype(&MEDIASUBTYPE_RGB24);
	mPreferredMt.SetFormatType(&FORMAT_VideoInfo);
	mPreferredMt.SetTemporalCompression(FALSE);
	VIDEOINFOHEADER     info;
	ZeroMemory(&info, sizeof(VIDEOINFOHEADER));
	info.AvgTimePerFrame = 400000;
	info.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	info.bmiHeader.biWidth  = 360;
	info.bmiHeader.biHeight = 288;
	info.bmiHeader.biBitCount  = 24;
	info.bmiHeader.biSizeImage = 360 * 288 * 3;
	info.bmiHeader.biCompression = 0;
	info.bmiHeader.biPlanes = 1;
	mPreferredMt.SetFormat((BYTE*)&info, sizeof(VIDEOINFOHEADER));
	mPreferredMt.SetSampleSize(info.bmiHeader.biSizeImage);*/
}

CNetOutPin::~CNetOutPin()
{
}

// for test
#include <initguid.h>
DEFINE_GUID(MEDIASUBTYPE_XVID,
0x44495658, 0x0000, 0x0010, 0x80, 0x00, 0x00, 0xAA, 0x00, 0x38, 0x9B, 0x71);

DEFINE_GUID(MEDIASUBTYPE_H264,
0x34363248, 0x0000, 0x0010, 0x80, 0x00, 0x00, 0xAA, 0x00, 0x38, 0x9B, 0x71);

DEFINE_GUID(MEDIASUBTYPE_X264,
0x34363258, 0x0000, 0x0010, 0x80, 0x00, 0x00, 0xAA, 0x00, 0x38, 0x9B, 0x71);

DEFINE_GUID(MEDIASUBTYPE_AVC1,
0x31435641, 0x0000, 0x0010, 0x80, 0x00, 0x00, 0xAA, 0x00, 0x38, 0x9B, 0x71);

void CNetOutPin::SetupMediaType(CAVConfigData::pointer configData)
{
	if (configData.get() == 0) return;

	if (configData->getIsVideo())
	{
		mPreferredMt.InitMediaType () ;
		mPreferredMt.SetType(& MEDIATYPE_Video) ;
		switch(configData->getMediaSubtype())
		{
		case MST_RGB32:
			{
				mPreferredMt.SetSubtype(& MEDIASUBTYPE_RGB32); 
				break;
			}
		case MST_RGB24:
			{
				mPreferredMt.SetSubtype(& MEDIASUBTYPE_RGB24); 
				break;
			}
		case MST_YUY2:
			{
				mPreferredMt.SetSubtype(& MEDIASUBTYPE_YUY2);
				break;
			}
		case MST_MJPG:
			{
				mPreferredMt.SetSubtype(& MEDIASUBTYPE_MJPG);
				break;
			}
		case MST_H264:
			{
				mPreferredMt.SetSubtype(& MEDIASUBTYPE_H264) ;
				break;
			}
		default:
			{
				mPreferredMt.SetSubtype(& MEDIASUBTYPE_RGB32);
			}
		}
		
		//mPreferredMt.SetSubtype    (& MEDIASUBTYPE_YUYV) ; 
		//mPreferredMt.SetSubtype    (& MEDIASUBTYPE_RGB32) ;	// ffmpeg OK

		////////设置outputpin的Format
		VIDEOINFO *pvi = (VIDEOINFO *) mPreferredMt.AllocFormatBuffer(sizeof(VIDEOINFO));
		ZeroMemory(pvi, sizeof(VIDEOINFO));
		switch(configData->getMediaSubtype())
		{
		case MST_RGB32:
			{
				
				break;
			}
		case MST_RGB24:
			{

				break;
			}
		case MST_YUY2:
			{
				pvi->bmiHeader.biCompression = mmioFOURCC('Y','U','Y','2');
				break;
			}
		case MST_MJPG:
			{
				pvi->bmiHeader.biCompression = mmioFOURCC('M','J','P','G');
				break;
			}
		case MST_H264:
			{
				pvi->bmiHeader.biCompression = mmioFOURCC('H','2','6','4');
				break;
			}
		default:
			{
				
			}
		}

		if (configData->getNumChannels()==3)
		{
			pvi->bmiHeader.biCompression = mmioFOURCC('Y','U','Y','2');
		}else if (configData->getNumChannels()==4)
		{
			pvi->bmiHeader.biCompression = mmioFOURCC('H','2','6','4');
		}


		//switch (m_avcoding)
		//{
		////case AVCoding::CT_H264:
		//	//mPreferredMt.SetSubtype    (& MEDIASUBTYPE_H264) ;	// MC ok
		////	pvi->bmiHeader.biCompression = mmioFOURCC('H','2','6','4');	// MC ok
		////	break;
		//case AVCoding::CT_MPEG4:
		//	mPreferredMt.SetSubtype    (& TIME_FORMAT_NONE) ; 
		//	pvi->bmiHeader.biCompression = mmioFOURCC('X','V','I','D');
		//	break;
		//default:
		//	break;
		//}

		pvi->AvgTimePerFrame = configData->getFrequency();

		//int nWidth = 352;
		//int nHeight = 288;
		//pvi->bmiHeader.biBitCount     = 24;
		pvi->bmiHeader.biBitCount     = 32;
		pvi->bmiHeader.biSize        = sizeof(BITMAPINFOHEADER);
		pvi->bmiHeader.biWidth       = configData->getWidth();
		pvi->bmiHeader.biHeight      = configData->getHeight();
		pvi->bmiHeader.biPlanes      = 1;
		pvi->bmiHeader.biSizeImage   = GetBitmapSize(&pvi->bmiHeader);
		SetRectEmpty(&(pvi->rcSource));
		SetRectEmpty(&(pvi->rcTarget));
		pvi->rcTarget.right = configData->getWidth();
		pvi->rcTarget.bottom = configData->getHeight();
		memcpy(&pvi->rcSource, &pvi->rcTarget, sizeof(pvi->rcTarget));
		mPreferredMt.SetFormatType(&FORMAT_VideoInfo);
		mPreferredMt.SetSampleSize(pvi->bmiHeader.biSizeImage);
	}else
	{
		mPreferredMt.InitMediaType () ;
		mPreferredMt.SetType(&MEDIATYPE_Audio);
		mPreferredMt.SetSubtype(&TIME_FORMAT_NONE);
		//mPreferredMt.SetSubtype(&MEDIASUBTYPE_PCM);
		mPreferredMt.SetFormatType(&FORMAT_WaveFormatEx);
		
		// Verify sample size (one second)
		WAVEFORMATEX * pWaveFormat = (WAVEFORMATEX *) mPreferredMt.AllocFormatBuffer(sizeof(WAVEFORMATEX) +2);
		ZeroMemory(pWaveFormat , sizeof(WAVEFORMATEX)+2); 
		pWaveFormat->nChannels  = configData->getNumChannels(); 
		pWaveFormat->nSamplesPerSec  = configData->getFrequency(); 
		pWaveFormat->wBitsPerSample = 16; 
		pWaveFormat->nBlockAlign  = pWaveFormat->nChannels * pWaveFormat->wBitsPerSample / 8; 
		pWaveFormat->nAvgBytesPerSec = pWaveFormat->nSamplesPerSec * pWaveFormat->nBlockAlign; 
		pWaveFormat->wFormatTag = 0x1;//0xFF==AAC;//85==MP3;//1==WAVE_FORMAT_PCM 

		//switch (m_avcoding)
		//{
		//case AVCoding::CT_AAC:
		//	{

		//		pWaveFormat->wFormatTag = 0xFF;
		//		pWaveFormat->cbSize = 2;

		//		if (configData->getConfigStr())
		//		{
		//			memcpy(((BYTE *)pWaveFormat)+sizeof(WAVEFORMATEX), configData->getConfigStr(), 2);
		//		}else
		//		{
		//			/*
		//			const int AACFrequency[16] =
		//			{
		//			96000, 88200, 64000, 48000, 44100, 32000, 24000,
		//			22050, 16000, 12000, 11025, 8000, 7350, 0, 0, 0
		//			};
		//			const int AACChannels[16] =
		//			{
		//			0, 1, 2, 3, 4, 5, 6, 8, 0, 0, 0, 0, 0, 0, 0, 0
		//			};
		//			*/
		//			int chChanIndex = 2;		// 2
		//			int chFreqIndex = 4;		// 44100
		//			int AudioObjectType = 1;	// ?应该是默认对象
		//			((BYTE *)pWaveFormat)[sizeof(WAVEFORMATEX)+ 0] = (AudioObjectType<<3) & 0xf8;
		//			((BYTE *)pWaveFormat)[sizeof(WAVEFORMATEX)+ 0] |= (chFreqIndex>>1) & 0x07;
		//			((BYTE *)pWaveFormat)[sizeof(WAVEFORMATEX)+ 1] = (chFreqIndex<<7) & 0x80;
		//			((BYTE *)pWaveFormat)[sizeof(WAVEFORMATEX)+ 1] |= (chChanIndex<<3) & 0x78;
		//		}
		//	}break;
		//default:
		//		pWaveFormat->cbSize = 0;
		//		pWaveFormat->wFormatTag = 0x1;//0xFF==AAC;//85==MP3;//1==WAVE_FORMAT_PCM 
		//	break;
		//}

		mFilter->SetAudioBytesPerSecond(pWaveFormat->nAvgBytesPerSec);
	}
	
	mPreferredMt.SetTemporalCompression(FALSE);
}

HRESULT CNetOutPin::CheckMediaType(const CMediaType * inMediaType)
{
	if (*inMediaType == mPreferredMt)
	{
		return NOERROR;
	}
	return E_FAIL;
}

HRESULT CNetOutPin::DecideBufferSize(IMemAllocator * pAlloc, ALLOCATOR_PROPERTIES * pprop)
{
	//ASSERT(pAlloc);
	//ASSERT(pprop);
	HRESULT hr = NOERROR;

	// Determine sample size
	if (mPreferredMt.formattype == FORMAT_VideoInfo)
	{
		VIDEOINFOHEADER * info = (VIDEOINFOHEADER *) mPreferredMt.pbFormat;
		if (info->bmiHeader.biSizeImage > 0)
			pprop->cbBuffer = info->bmiHeader.biSizeImage;
		else
			pprop->cbBuffer = 320*240*4;
	}else
	{
		WAVEFORMATEX * info = (WAVEFORMATEX *) mPreferredMt.pbFormat;
		pprop->cbBuffer = info->nAvgBytesPerSec;
		if (pprop->cbBuffer == 0)
			pprop->cbBuffer = info->nSamplesPerSec;
	}
	pprop->cBuffers  = 1;
	pprop->cbAlign   = 1;

	//ASSERT(pprop->cbBuffer);

	ALLOCATOR_PROPERTIES Actual;
	hr = pAlloc->SetProperties(pprop, &Actual);
	if (FAILED(hr)) 
	{
		return hr;
	}

	//ASSERT( Actual.cBuffers == 1 );

	if (pprop->cBuffers > Actual.cBuffers ||
		pprop->cbBuffer > Actual.cbBuffer) 
	{
		return E_FAIL;
	}
	return NOERROR;
}

HRESULT CNetOutPin::GetMediaType(int iPosition,CMediaType *pMediaType)
{
	if (iPosition == 0)
	{
		*pMediaType = mPreferredMt;
		return NOERROR;
	}
	return E_INVALIDARG;
}
