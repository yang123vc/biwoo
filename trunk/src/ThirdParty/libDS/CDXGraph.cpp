//
// CDXGraph.cpp
//

#include "CDXGraph.h"
#include "GlobalDefs.h"


////////////////////////////////////////////////////////////////////////////////
CDXGraph::CDXGraph()
: m_pWc(NULL)

{
	mGraph        = NULL;
	mMediaControl = NULL;
	mEvent        = NULL;
	mBasicVideo   = NULL;
	mBasicAudio   = NULL;
	mVideoWindow  = NULL;
	mSeeking      = NULL;

	mObjectTableEntry = 0;

	mGraphState = GS_Stopped;
}

CDXGraph::~CDXGraph()
{
	Release();
}

BOOL CDXGraph::Create(void)
{
	if (!mGraph)
	{
		if (SUCCEEDED(CoCreateInstance(CLSID_FilterGraph, NULL, CLSCTX_INPROC_SERVER,
			IID_IGraphBuilder, (void **)&mGraph)))
		{
			AddToObjectTable();

			return QueryInterfaces();
		}
		mGraph = NULL;
	}
	return FALSE;
}

BOOL CDXGraph::QueryInterfaces(void)
{
	if (mGraph)
	{
		HRESULT hr = NOERROR;
		hr |= mGraph->QueryInterface(IID_IMediaControl, (void **)&mMediaControl);
		hr |= mGraph->QueryInterface(IID_IMediaEventEx, (void **)&mEvent);
		hr |= mGraph->QueryInterface(IID_IBasicVideo, (void **)&mBasicVideo);
		hr |= mGraph->QueryInterface(IID_IBasicAudio, (void **)&mBasicAudio);
		hr |= mGraph->QueryInterface(IID_IVideoWindow, (void **)&mVideoWindow);
		hr |= mGraph->QueryInterface(IID_IMediaSeeking, (void **)&mSeeking);
		if (mSeeking)
		{
			mSeeking->SetTimeFormat(&TIME_FORMAT_MEDIA_TIME);
		}
		return SUCCEEDED(hr);
	}
	return FALSE;
}

void CDXGraph::Release(void)
{
	if (mSeeking)
	{
		mSeeking->Release();
		mSeeking = NULL;
	}
	if (mMediaControl)
	{
		mMediaControl->Release();
		mMediaControl = NULL;
	}
	if (mEvent)
	{
		mEvent->Release();
		mEvent = NULL;
	}
	if (mBasicVideo)
	{
		mBasicVideo->Release();
		mBasicVideo = NULL;
	}
	if (mBasicAudio)
	{
		mBasicAudio->Release();
		mBasicAudio = NULL;
	}
	if (mVideoWindow)
	{
		mVideoWindow->put_Visible(OAFALSE);
		mVideoWindow->put_MessageDrain((OAHWND)NULL);
		mVideoWindow->put_Owner(OAHWND(0));
		mVideoWindow->Release();
		mVideoWindow = NULL;
	}
	RemoveFromObjectTable();
	if (mGraph) 
	{
		mGraph->Release(); 
		mGraph = 0;
	}

	if (m_pWc)
	{
		m_pWc->Release();
		m_pWc = 0;
	}
}

BOOL CDXGraph::Attach(IGraphBuilder * inGraphBuilder)
{
	Release();

	if (inGraphBuilder)
	{
		mGraph = inGraphBuilder;
		mGraph->AddRef();
		
		AddToObjectTable();
		return QueryInterfaces();
	}
	return TRUE;
}

IGraphBuilder * CDXGraph::GetGraph(void)
{
	return mGraph;
}

BOOL CDXGraph::AddFilter(IBaseFilter * inFilter, LPCWSTR inName)
{
	if (mGraph)
	{
		HRESULT hr = mGraph->AddFilter(inFilter, inName);
		return SUCCEEDED(hr);
	}
	return FALSE;
}

// Connect filter from the upstream output pin to the downstream input pin
BOOL CDXGraph::ConnectFilters(IPin * inOutputPin, IPin * inInputPin, 
							  const AM_MEDIA_TYPE * inMediaType)
{
	if (mGraph && inOutputPin && inInputPin)
	{
		HRESULT hr = mGraph->ConnectDirect(inOutputPin, inInputPin, inMediaType);
		return SUCCEEDED(hr) ? TRUE : FALSE;
	}
	return FALSE;
}

void CDXGraph::DisconnectFilters(IPin * inOutputPin)
{
	if (mGraph && inOutputPin)
	{
		HRESULT hr = mGraph->Disconnect(inOutputPin);
	}
}

#ifdef USE_VMRMode_Windowless

HRESULT InitWindowlessVMR(HWND hwndApp,						// 视频窗体 
						  IGraphBuilder* pGraph,			// 过滤器图表指针 
						  IVMRWindowlessControl** ppWc		// 接收VMR指针
						  ) 
{ 
	if (!pGraph || !ppWc) return E_POINTER;
	IBaseFilter* pVmr = NULL; 
	IVMRWindowlessControl* pWc = NULL; 
	// 创建VMR 
	HRESULT hr = CoCreateInstance(CLSID_VideoMixingRenderer, NULL, 
		CLSCTX_INPROC, IID_IBaseFilter, (void**)&pVmr); 
	if (FAILED(hr))
	{
		return hr;
	}

	// 把VMR添加到过滤器图表中
	hr = pGraph->AddFilter(pVmr, L"Video Mixing Renderer"); 
	if (FAILED(hr)) 
	{
		pVmr->Release();
		return hr;
	}
	// 设置显示模式 
	IVMRFilterConfig* pConfig; 
	hr = pVmr->QueryInterface(IID_IVMRFilterConfig, (void**)&pConfig); 
	if (SUCCEEDED(hr)) 
	{ 
		hr = pConfig->SetRenderingMode(VMRMode_Windowless); 
		pConfig->Release(); 
	}
	if (SUCCEEDED(hr))
	{
		// 设置窗体 
		hr = pVmr->QueryInterface(IID_IVMRWindowlessControl, (void**)&pWc);
		if( SUCCEEDED(hr)) 
		{ 
			hr = pWc->SetVideoClippingWindow(hwndApp); 
			if (SUCCEEDED(hr))
			{
				*ppWc = pWc; //返回AddRef指针 
			}
			else
			{
				pWc->Release();
			}
		} 
	} 
	pVmr->Release(); 
	return hr; 
} 

HRESULT CDXGraph::InitWindowlessVMR(HWND hwndApp)
{
	HRESULT hr = ::InitWindowlessVMR(hwndApp, mGraph, &m_pWc);
	return hr;
}

IVMRWindowlessControl * CDXGraph::GetVMRWc(void)
{
	return m_pWc;
}
#endif

BOOL CDXGraph::SetDisplayWindow(HWND inWindow)
{	
	if (mVideoWindow)
	{
	//	long lVisible;
	//	mVideoWindow->get_Visible(&lVisible);
		// Hide the video window first
		mVideoWindow->put_Visible(OAFALSE);
		mVideoWindow->put_Owner((OAHWND)inWindow);

		RECT windowRect;
		::GetClientRect(inWindow, &windowRect);
		mVideoWindow->put_Left(0);
		mVideoWindow->put_Top(0);
		mVideoWindow->put_Width(windowRect.right - windowRect.left);
		mVideoWindow->put_Height(windowRect.bottom - windowRect.top);
		mVideoWindow->put_WindowStyle(WS_CHILD|WS_CLIPCHILDREN|WS_CLIPSIBLINGS);
		//mVideoWindow->put_WindowStyle(WS_CHILD);

		HWND wndParent = GetParent(inWindow);
		HWND wndP = NULL;
		while ((wndP = GetParent(wndParent)) != NULL)
		{
			wndParent = wndP;
		}

		if (wndParent)
			mVideoWindow->put_MessageDrain((OAHWND) wndParent);
		else
			mVideoWindow->put_MessageDrain((OAHWND) inWindow);

		// Restore the video window
		if (inWindow != NULL)
		{
		//	mVideoWindow->put_Visible(lVisible);
			mVideoWindow->put_Visible(OATRUE);
		}
		else
		{
			mVideoWindow->put_Visible(OAFALSE);
		}
		return TRUE;
	}
	return FALSE;
}

BOOL CDXGraph::ResizeVideoWindow(long inLeft, long inTop, long inWidth, long inHeight)
{
	if (mVideoWindow)
	{
		long lVisible = OATRUE;
		mVideoWindow->get_Visible(&lVisible);
		// Hide the video window first
		mVideoWindow->put_Visible(OAFALSE);

		mVideoWindow->put_Left(inLeft);
		mVideoWindow->put_Top(inTop);
		mVideoWindow->put_Width(inWidth);
		mVideoWindow->put_Height(inHeight);
	
		// Restore the video window
		mVideoWindow->put_Visible(lVisible);
		return TRUE;
	}
	return FALSE;
}

void CDXGraph::moveWindow(void)
{
	if (mVideoWindow)
	{
		mVideoWindow->put_Visible(OAFALSE);
		OAHWND pWnd = NULL;
		mVideoWindow->get_Owner(&pWnd);

		RECT windowRect;
		::GetClientRect((HWND)pWnd, &windowRect);
		mVideoWindow->put_Left(0);
		mVideoWindow->put_Top(0);
		mVideoWindow->put_Width(windowRect.right - windowRect.left);
		mVideoWindow->put_Height(windowRect.bottom - windowRect.top);

		mVideoWindow->put_Visible(OATRUE);
	}
}

void CDXGraph::NotifyOwnerMessage(HWND hWnd, long uMsg,LONG_PTR wParam,LONG_PTR lParam)
{
	if (mVideoWindow)
	{
		mVideoWindow->NotifyOwnerMessage((OAHWND)hWnd, uMsg, wParam, lParam);
	}
}

BOOL CDXGraph::SetNotifyWindow(HWND inWindow)
{
	if (mEvent)
	{
		mEvent->SetNotifyWindow((OAHWND)inWindow, WM_GRAPHNOTIFY, 0);
		return TRUE;
	}
	return FALSE;
}

void CDXGraph::HandleEvent(WPARAM inWParam, LPARAM inLParam)
{
	if (mEvent)
	{
		LONG eventCode = 0, eventParam1 = 0, eventParam2 = 0;
		while (SUCCEEDED(mEvent->GetEvent(&eventCode, &eventParam1, &eventParam2, 0)))
		{
			mEvent->FreeEventParams(eventCode, eventParam1, eventParam2);
			switch (eventCode)
			{
			case EC_COMPLETE:	
			case EC_USERABORT:
			case EC_ERRORABORT:
				//Broadcast(msg_FilterGraphError);
				break;

			case EC_DEVICE_LOST:
				// As a result, windows will process device lost event first.
				// We needn't process this event in DirectShow any more.
				if (eventParam2 == 0)
				{
				//	Broadcast(msg_DShowDeviceLost, &eventParam1);
				}
				break;
			default:
				break;
			}
		}
	}
}

BOOL CDXGraph::Run(void)
{
	if (mGraph && mMediaControl)
	{
		if (!IsRunning())
		{
			if (SUCCEEDED(mMediaControl->Run()))
			{
				mGraphState = GS_Running;
				return TRUE;
			}
		}
		else
		{
			return TRUE;
		}
	}
	return FALSE;
}

BOOL CDXGraph::Stop(void)
{
	if (mGraph && mMediaControl)
	{
		if (!IsStopped())
		{	
			if (SUCCEEDED(mMediaControl->Stop()))
			{
				mGraphState = GS_Stopped;
				return TRUE;
			}
		}
		else
		{
			return TRUE;
		}
	}
	return FALSE;
}

BOOL CDXGraph::Pause(void)
{
	if (mGraph && mMediaControl)
	{
		if (!IsPaused())
		{	
			if (SUCCEEDED(mMediaControl->Pause()))
			{
				mGraphState = GS_Paused;
				return TRUE;
			}
		}
		else
		{
			return TRUE;
		}
	}
	return FALSE;
}

BOOL CDXGraph::IsRunning(void)
{
	if (mGraph && mMediaControl)
	{
		OAFilterState state = State_Stopped;
		if (SUCCEEDED(mMediaControl->GetState(10, &state)))
		{
			return state == State_Running;
		}
	}
	return FALSE;
	return (mGraphState == GS_Running);
}

BOOL CDXGraph::IsStopped(void)
{
/*	if (mGraph && mMediaControl)
	{
		OAFilterState state = State_Stopped;
		if (SUCCEEDED(mMediaControl->GetState(10, &state)))
		{
			return state == State_Stopped;
		}
	}
	return FALSE;*/
	return (mGraphState == GS_Stopped);
}

BOOL CDXGraph::IsPaused(void)
{
/*	if (mGraph && mMediaControl)
	{
		OAFilterState state = State_Stopped;
		if (SUCCEEDED(mMediaControl->GetState(10, &state)))
		{
			return state == State_Paused;
		}
	}
	return FALSE;*/
	return (mGraphState == GS_Paused);
}

bool CDXGraph::SetFullScreen(bool inEnabled)
{
	if (mVideoWindow)
	{
		HRESULT hr = mVideoWindow->put_FullScreenMode(inEnabled ? OATRUE : OAFALSE);
		return SUCCEEDED(hr);
	}
	return false;
}

bool CDXGraph::GetFullScreen(void) const
{
	if (mVideoWindow)
	{
		long  fullScreenMode = OAFALSE;
		mVideoWindow->get_FullScreenMode(&fullScreenMode);
		return (fullScreenMode == OATRUE);
	}
	return false;
}

BOOL CDXGraph::SetSyncSource(IBaseFilter * inFilter)
{
	if (inFilter)
	{
		IReferenceClock *pClock = 0;
		inFilter->QueryInterface(IID_IReferenceClock, (void**)&pClock);
		if (pClock)
		{
			// Set the graph clock.
			IMediaFilter *pMediaFilter = 0;
			mGraph->QueryInterface(IID_IMediaFilter, (void**)&pMediaFilter);
			pMediaFilter->SetSyncSource(pClock);
			pClock->Release();
			pMediaFilter->Release();
			return TRUE;
		}
		return FALSE;
	}
	else
	{
		// Do not use any reference clock
		IMediaFilter *pMediaFilter = 0;
		mGraph->QueryInterface(IID_IMediaFilter, (void**)&pMediaFilter);
		pMediaFilter->SetSyncSource(NULL);
		pMediaFilter->Release();
		return TRUE;
	}
}

BOOL CDXGraph::SetDefaultSyncSource(void)
{
	if (mGraph)
	{
		HRESULT hr = mGraph->SetDefaultSyncSource();
		return SUCCEEDED(hr) ?  TRUE : FALSE;
	}
	return FALSE;
}

// IMediaSeeking features
BOOL CDXGraph::GetCurrentPosition(double * outPosition)
{
	if (mSeeking)
	{
		__int64 position = 0;
		if (SUCCEEDED(mSeeking->GetCurrentPosition(&position)))
		{
			*outPosition = ((double)position) / 10000000.;
			return TRUE;
		}
	}
	return FALSE;
}

BOOL CDXGraph::GetStopPosition(double * outPosition)
{
	if (mSeeking)
	{
		__int64 position = 0;
		if (SUCCEEDED(mSeeking->GetStopPosition(&position)))
		{
			*outPosition = ((double)position) / 10000000.;
			return TRUE;
		}
	}
	return FALSE;
}

BOOL CDXGraph::SetCurrentPosition(double inPosition)
{
	if (mSeeking)
	{
		__int64 one = 10000000;
		__int64 position = (__int64)(one * inPosition);
		HRESULT hr = mSeeking->SetPositions(&position, AM_SEEKING_AbsolutePositioning | AM_SEEKING_SeekToKeyFrame, 
			0, AM_SEEKING_NoPositioning);
		return SUCCEEDED(hr);
	}
	return FALSE;
}

BOOL CDXGraph::SetStartStopPosition(double inStart, double inStop)
{
	if (mSeeking)
	{
		__int64 one = 10000000;
		__int64 startPos = (__int64)(one * inStart);
		__int64 stopPos  = (__int64)(one * inStop);
		HRESULT hr = mSeeking->SetPositions(&startPos, AM_SEEKING_AbsolutePositioning | AM_SEEKING_SeekToKeyFrame, 
			&stopPos, AM_SEEKING_AbsolutePositioning | AM_SEEKING_SeekToKeyFrame);
		return SUCCEEDED(hr);
	}
	return FALSE;
}

BOOL CDXGraph::GetDuration(double * outDuration)
{
	if (mSeeking)
	{
		__int64 length = 0;
		if (SUCCEEDED(mSeeking->GetDuration(&length)))
		{
			*outDuration = ((double)length) / 10000000.;
			return TRUE;
		}
	}
	return FALSE;
}

BOOL CDXGraph::SetPlaybackRate(double inRate)
{
	if (mSeeking)
	{
		if (SUCCEEDED(mSeeking->SetRate(inRate)))
		{
			return TRUE;
		}
	}
	return FALSE;
}

// Attention: range from -10000 to 0, and 0 is FULL_VOLUME.
bool CDXGraph::SetAudioVolume(long inVolume)
{
	if (mBasicAudio)
	{
		HRESULT hr = mBasicAudio->put_Volume(inVolume);
		return SUCCEEDED(hr);
	}
	return false;
}

long CDXGraph::GetAudioVolume(void) const
{
	long volume = 0;
	if (mBasicAudio)
	{
		mBasicAudio->get_Volume(&volume);
	}
	return volume;
}

// Attention: range from -10000(left) to 10000(right), and 0 is both.
bool CDXGraph::SetAudioBalance(long inBalance)
{
	if (mBasicAudio)
	{
		HRESULT hr = mBasicAudio->put_Balance(inBalance);
		return SUCCEEDED(hr);
	}
	return false;
}

long CDXGraph::GetAudioBalance(void) const
{
	long balance = 0;
	if (mBasicAudio)
	{
		mBasicAudio->get_Balance(&balance);
	}
	return balance;
}

BOOL CDXGraph::GetCurrentImage(const char * inFile)
{
	if (mBasicVideo)
	{
		long bitmapSize = 0;
		if (SUCCEEDED(mBasicVideo->GetCurrentImage(&bitmapSize, 0)))
		{
			/*
			BOOL pass = FALSE;
			BYTE * buffer = new BYTE[bitmapSize];
			if (SUCCEEDED(mBasicVideo->GetCurrentImage(&bitmapSize, (long *)buffer)))
			{
				 BITMAPFILEHEADER	hdr;
	             LPBITMAPINFOHEADER	lpbi;
	            
	             lpbi = (LPBITMAPINFOHEADER)buffer;

	             int nColors = 1 << lpbi->biBitCount;
	             if (nColors > 256)
                 nColors = 0;

	             hdr.bfType		= ((WORD) ('M' << 8) | 'B'); // always is "BM"
	             hdr.bfSize		= bitmapSize + sizeof( hdr );
	             hdr.bfReserved1 = 0;
	             hdr.bfReserved2 = 0;
	             hdr.bfOffBits	 = (DWORD) (sizeof(BITMAPFILEHEADER) + lpbi->biSize +
						nColors * sizeof(RGBQUAD));
#ifdef UNICODE
				 USES_CONVERSION;
				 CFile bmpFile(A2T(inFile), CFile::modeReadWrite | CFile::modeCreate | CFile::typeBinary);
#else
				 CFile bmpFile(inFile, CFile::modeReadWrite | CFile::modeCreate | CFile::typeBinary);
#endif
                bmpFile.Write(&hdr, sizeof(BITMAPFILEHEADER));
				bmpFile.Write(buffer, bitmapSize);
				bmpFile.Close();
				pass = TRUE;
			}
			delete [] buffer;
			return pass;
			*/
		}
	}
	return FALSE;
}



//////////////////////// For GraphEdit Dubug purpose /////////////////////////////
void CDXGraph::AddToObjectTable(void)
{
	IMoniker * pMoniker = 0;
    IRunningObjectTable * objectTable = 0;
    if (SUCCEEDED(GetRunningObjectTable(0, &objectTable))) 
	{
		WCHAR wsz[256];
		wsprintfW(wsz, L"FilterGraph %08p pid %08x", (DWORD_PTR)mGraph, GetCurrentProcessId());
		HRESULT hr = CreateItemMoniker(L"!", wsz, &pMoniker);
		if (SUCCEEDED(hr)) 
		{
			hr = objectTable->Register(0, mGraph, pMoniker, &mObjectTableEntry);
			pMoniker->Release();
		}
		objectTable->Release();
	}
}

void CDXGraph::RemoveFromObjectTable(void)
{
	IRunningObjectTable * objectTable = 0;
    if (SUCCEEDED(GetRunningObjectTable(0, &objectTable))) 
	{
        objectTable->Revoke(mObjectTableEntry);
        objectTable->Release();
		mObjectTableEntry = 0;
    }
}