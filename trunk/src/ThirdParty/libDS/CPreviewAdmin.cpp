//
// CPreviewAdmin.cpp
//

#include <streams.h>
#include "CPreviewAdmin.h"

#include "CDXGraph.h"
#include "CDXFilter.h"
#include "CAVDevice.h"
#include "GlobalDefs.h"
#include "UDsUtils.h"

#include "CVideoCaptureFilter2.h"
#include "CAudioCaptureFilter2.h"


/////////////////////////////////////////////////////////////////////////
CPreviewAdmin::CPreviewAdmin()
: mVideoDecoderFilter(NULL)
, mVideoEncoderFilter(NULL)
//, mRawVideoFilter(NULL)
, mAacEncoderFilter(NULL)
, mAacDecoderFilter(NULL)
{
	mGraph   	 = NULL;
	mVideoWindow = NULL;
	mVideoDevice = NULL;
	mAudioDevice = NULL;
	
	mVideoFilter = NULL;
	mAudioFilter = NULL;
	mVideoRenderer = NULL;
	mAudioRenderer = NULL;
}

CPreviewAdmin::~CPreviewAdmin()
{
	Deactivate();
}

void CPreviewAdmin::SetAVDevice(CAVDevice * inVideo, CAVDevice * inAudio)
{
	mVideoDevice = inVideo;
	mAudioDevice = inAudio;
}

//void CPreviewAdmin::SetVideoWindow(CVideoWindow * inWindow)
void CPreviewAdmin::SetVideoWindow(HWND inWindow)
{
	mVideoWindow = inWindow;
}

bool CPreviewAdmin::ShowHideFullScreen(void)
{
	if (mGraph == NULL) return false;

	BOOL bFullScreen = mGraph->GetFullScreen();
	mGraph->SetFullScreen(!bFullScreen);
	return true;
}

BOOL CPreviewAdmin::Activate(void)
{
	BOOL pass = CreateGraph();
	if (pass)
	{
		pass = CreateInputFilters();
	}
	if (pass)
	{
		pass = CreateOutputFilters();
	}
	if (pass)
	{
		pass = ConnectFilters();
	}
	if (pass)
	{
		pass = AfterBuildGraph();
	}
	return pass;
}

void CPreviewAdmin::Deactivate(void)
{
	StopGraph();

	ReleaseInputFilters();
	ReleaseOutputFilters();
	DeleteGraph();
}

BOOL CPreviewAdmin::CreateGraph(void)
{
	mGraph = new CDXGraph();
	BOOL pass = mGraph->Create();
	return pass;
}

void CPreviewAdmin::DeleteGraph(void)
{
	// Clean up the video window
	if (mVideoWindow)
	{
		//mVideoWindow->SetGraph(0);
		if (mGraph)
		{
			mGraph->SetDisplayWindow(0);
			mGraph->SetNotifyWindow(0);
		}

	}
	SAFE_DELETE(mGraph);
}

BOOL CPreviewAdmin::CreateInputFilters(void)
{
	BOOL pass = FALSE;

	mVideoFilter = new CVideoCaptureFilter2(mGraph->GetGraph());
	((CVideoCaptureFilter2 *)mVideoFilter)->SetDevice(mVideoDevice);
	pass |= mVideoFilter->CreateFilter();

	mAudioFilter = new CAudioCaptureFilter2(mGraph->GetGraph());
	((CAudioCaptureFilter2 *)mAudioFilter)->SetDevice(mAudioDevice);
	pass |= mAudioFilter->CreateFilter();

#ifdef USE_MPEG4_FILTERS_TEST
	mMpeg4VideoEncoderFilter = new CDXFilter(mGraph->GetGraph(), 
		CLSID_ffdshowMPEG4VideoEncoderFilter, "ffdshow MPEG4 Video Encoder Filter");
	pass = mMpeg4VideoEncoderFilter->CreateFilter();

#endif

	return pass;
}

void CPreviewAdmin::ReleaseInputFilters(void)
{
	SAFE_DELETE(mVideoFilter);
	SAFE_DELETE(mAudioFilter);

#ifdef USE_MPEG4_FILTERS_TEST
	SAFE_DELETE(mMpeg4VideoEncoderFilter);

#endif
}

BOOL CPreviewAdmin::CreateOutputFilters(void)
{
	BOOL pass = TRUE;
	mVideoRenderer = new CDXFilter(mGraph->GetGraph(), 
		CLSID_VideoRenderer, "Video Renderer");
	pass = mVideoRenderer->CreateFilter();
	mAudioRenderer = new CDXFilter(mGraph->GetGraph(), 
		CLSID_DSoundRender, "Audio Renderer");
	pass = mAudioRenderer->CreateFilter();

#ifdef USE_MPEG4_FILTERS_TEST
	mMpeg4VideoDecoderFilter = new CDXFilter(mGraph->GetGraph(), 
		CLSID_ffdshowMPEG4VideoDecoderFilter, "ffdsow MPEG4 Video Decoder Filter");	// OK
	pass = mMpeg4VideoDecoderFilter->CreateFilter();

#endif
#ifdef USE_CATEGORY_PREVIEW
        HRESULT hr = graphBuilder2_.CoCreateInstance( CLSID_CaptureGraphBuilder2 ); 
		pass = SUCCEEDED(hr);
		hr = graphBuilder2_->SetFiltergraph(mGraph->GetGraph());
		pass = SUCCEEDED(hr);
#endif // USE_CATEGORY_PREVIEW

	return pass;
}

void CPreviewAdmin::ReleaseOutputFilters(void)
{
	SAFE_DELETE(mVideoRenderer);
	SAFE_DELETE(mAudioRenderer);
#ifdef USE_MPEG4_FILTERS_TEST
	SAFE_DELETE(mMpeg4VideoDecoderFilter);
#endif

#ifdef USE_CATEGORY_PREVIEW
	graphBuilder2_.Release();
#endif // USE_CATEGORY_PREVIEW
}

// Connect input filters to output filters
BOOL CPreviewAdmin::ConnectFilters(void)
{
//	BOOL pass   = FALSE;
	BOOL pass   = TRUE;
	IPin * pOut = NULL;
	IPin * pIn  = NULL;

	// Connect audio capture filter to audio renderer
	if (mAudioDevice->IsDeviceAvailable())
	{
		pOut = mAudioFilter->GetPin(FALSE);
		pIn  = mAudioRenderer->GetPin(TRUE);
		pass = mGraph->ConnectFilters(pOut, pIn);
	}

	// Connect video filters
	if (pass && mVideoDevice->IsDeviceAvailable())
	{
		// DV camcorder
		if (mVideoDevice->IsDVDevice())
		{
			pOut = mVideoFilter->GetPin(FALSE, "DV Vid Out");
			pIn  = mVideoRenderer->GetPin(TRUE);
			pass = RenderDVVideo(mGraph, pOut, pIn);
		}
		else // USB camera
		{
			pOut = mVideoFilter->GetPin(FALSE, "Capture");
			if (pOut == NULL)
				pOut = mVideoFilter->GetPin(FALSE, "²¶»ñ");

			HRESULT hr = S_OK;

#ifdef USE_CATEGORY_PREVIEW
			hr = graphBuilder2_->RenderStream(&PIN_CATEGORY_PREVIEW,
				&MEDIATYPE_Interleaved, mVideoFilter->GetFilter(), NULL, NULL);
			if(hr == VFW_S_NOPREVIEWPIN)
			{
			}
			else if(hr != S_OK)
			{
				// maybe it's DV?
				hr = graphBuilder2_->RenderStream(&PIN_CATEGORY_PREVIEW,
					&MEDIATYPE_Video, mVideoFilter->GetFilter(), NULL, NULL);
				if(hr == VFW_S_NOPREVIEWPIN)
				{
					// preview was faked up for us using the (only) capture pin
				}
				else if(hr != S_OK)
				{

				}
			}
 			pass = SUCCEEDED(hr);

#else
#ifdef USE_MPEG4_FILTERS_TEST

			// video->encoder
			pIn  = mMpeg4VideoEncoderFilter->GetPin(TRUE);
			hr = mGraph->GetGraph()->Connect(pOut, pIn);
			pass = SUCCEEDED(hr);

			// encoder->decoder
			pOut  = mMpeg4VideoEncoderFilter->GetPin(FALSE);
			pIn  = mMpeg4VideoDecoderFilter->GetPin(TRUE);
			hr = mGraph->GetGraph()->Connect(pOut, pIn);
			pass = SUCCEEDED(hr);

			// decoder->render
			pOut  = mMpeg4VideoDecoderFilter->GetPin(FALSE);

#endif // USE_MPEG4_FILTERS_TEST
			pIn  = mVideoRenderer->GetPin(TRUE);
			// Try intelligent connect
			hr = mGraph->GetGraph()->Connect(pOut, pIn);
			pass = SUCCEEDED(hr);

#endif // USE_CATEGORY_PREVIEW
		}
	}

	return pass;
}

BOOL CPreviewAdmin::AfterBuildGraph(void)
{
	if (mVideoWindow)
	{
		if (mGraph)
		{
			mGraph->SetDisplayWindow(mVideoWindow);
			mGraph->SetNotifyWindow(mVideoWindow);
		}

		// Auto-play
		PlayGraph();
		return TRUE;
	}
	return FALSE;
}

BOOL CPreviewAdmin::PlayGraph(void)
{
	if (mGraph)
	{
		mGraph->Run();
		return TRUE;
	}
	return FALSE;
}

BOOL CPreviewAdmin::PauseGraph(void)
{
	if (mGraph)
	{
		mGraph->Pause();
		return TRUE;
	}
	return FALSE;
}

BOOL CPreviewAdmin::StopGraph(void)
{
	if (mGraph)
	{
		mGraph->Stop();
		return TRUE;
	}
	return FALSE;
}

void CPreviewAdmin::ShowVideoCapturePropertyPage(void)
{
	if (!mVideoDevice->IsDVDevice())
	{
		UDsUtils::ShowFilterPropertyPage(mVideoFilter->GetFilter());

		// Update new resolution?
		long format = ((CVideoCaptureFilter2 *)mVideoFilter)->GetResolution();
		mVideoDevice->SetVideoResolution(format);
	}
}

void CPreviewAdmin::ShowAudioCapturePropertyPage(void)
{
	UDsUtils::ShowFilterPropertyPage(mAudioFilter->GetFilter());

	// Update new connector?
	long index = ((CAudioCaptureFilter2 *)mAudioFilter)->GetConnector();
	mAudioDevice->SetAudioConnector(index);
}

// Utilities
// Render DV to renderer, inserting a DV decoder filter
BOOL CPreviewAdmin::RenderDVVideo(CDXGraph * inGraph, IPin * inDVPin, IPin * inRenderedPin)
{
	BOOL pass   = FALSE;
	IPin * pOut = NULL;
	IPin * pIn  = NULL;

	if (inGraph)
	{
		// Add DV video decoder
		CDXFilter * pDecoder = NULL;
		pDecoder = new CDXFilter(inGraph->GetGraph(), CLSID_DVVideoCodec,
			"DV Video Decoder");
		pDecoder->CreateFilter();
		// Settings on DV Video Decoder
		IIPDVDec *  pIPDVDec = NULL; 
		pDecoder->QueryInterface(IID_IIPDVDec, (void**)&pIPDVDec);
		if (pIPDVDec)
		{
			pIPDVDec->Release();
			// DVRESOLUTION_HALF? DVRESOLUTION_QUARTER?
			pIPDVDec->put_IPDisplay(DVRESOLUTION_HALF);
		}

		pIn  = pDecoder->GetPin(TRUE);
		pass = inGraph->ConnectFilters(inDVPin, pIn);

		pOut = pDecoder->GetPin(FALSE);
		pass = inGraph->ConnectFilters(pOut, inRenderedPin);

		pDecoder->Detach();
		delete pDecoder;
	}

	return pass;
}
