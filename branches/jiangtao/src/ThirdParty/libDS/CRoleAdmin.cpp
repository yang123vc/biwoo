//
// CRoleAdmin.cpp
//

/*-----------------------------------------------------*\
			HQ Tech, Make Technology Easy!       
 More information, please go to http://hqtech.nease.net.
/*-----------------------------------------------------*/

#include <streams.h>
#include "CRoleAdmin.h"
#include "GlobalDefs.h"
#include "CDXGraph.h"
#include "CAVDevice.h"
#include "CVideoCaptureFilter2.h"
#include "CAudioCaptureFilter2.h"
#include "CFilterNetReceiver.h"
#include "CFilterNetSender.h"
//#include "aac_types.h"
#include <libG729a/libG729a.h>
#include "aac_types.h"

/////////////////////////////////////////////////////////////////////////
CRoleAdmin::CRoleAdmin()
: mDeviceConfig(0)
, m_handlerSender(NULL)
, m_bKilledOwner(false)
, mIsVideoMediaTypeSent(false)
, mIsAudioMediaTypeSent(false)
, m_nVideoOverflowData(0)
, m_nAudioOverflowData(0)
, m_wndRemoteVideoWindow(NULL)
, m_bIsRemotePaused(false)
, m_filterAVIMux(NULL)
, m_filterFileWriter(NULL)
, m_hCompletedVideoGraph(NULL)
, m_filterInfTee1(NULL)
, m_filterInfTee2(NULL)
, m_filterAVISplitter(NULL)
, m_filterAsyncFileReader(NULL)
, m_filterMainConpectMPEGSplitter(NULL)
/*, m_nChannels(2)
, m_nFrequency(44100)
, m_nFrameRate(25)
, m_nWidth(320)
, m_nHeight(240)*/
, m_nVideoCount(0)
, m_lastSampleAudioTime(0)
, m_filterMP3Enc(NULL)
, m_filterMP3Dec(NULL)
, m_procAVData(NULL)
, m_MediaSubtype(1)
{
	mRemoteVideoGraph = NULL;
	mRemoteAudioGraph = NULL;
	mLocalVideoGraph  = NULL;
	mLocalAudioGraph  = NULL;
	mLocalPreviewGraph = NULL;

	mVideoReceiver = NULL;
	mVideoPreviewReceiver = NULL;
	mAudioReceiver = NULL;
	mVideoSender   = NULL;
	mAudioSender   = NULL;
	mVideoPreviewRenderer = NULL;
	mVideoPreviewDecoderFilter = NULL;
	m_hCompletedVideoGraph = CreateEvent(NULL, FALSE, FALSE, NULL);

	m_g729a.g729a_init_encoder();
	m_g729a.g729a_init_decoder();

	/*
m_AviFile.CreateFormatContext("D:\\temp\\test2.avi");
	stuVideoParam videoParam;
	videoParam.bit_rate = 20;
	strcpy(videoParam.codec_id,"H264");
	videoParam.width = 320;
	videoParam.height = 240;
	stuAudioParam audioParam;
	audioParam.sample_bits = 16;
	audioParam.sample_mp3rate = 128;
	audioParam.channels = 1;
	audioParam.sample_rate = 16000;
	audioParam.sample_format = 7;
	m_AviFile.CreateStream(audioParam,videoParam);
	m_AviFile.StartWriteFile();
*/



}

CRoleAdmin::~CRoleAdmin()
{
	if (m_hCompletedVideoGraph)
	{
		CloseHandle(m_hCompletedVideoGraph);
		m_hCompletedVideoGraph = NULL;
	}
	Deactivate();
}

/*
void CRoleAdmin::setAudioParams(int nChannels, int nFrequency)
{
	m_nChannels = nChannels;
	m_nFrequency = nFrequency;
}

void CRoleAdmin::setVideoParams(int nFrameRate, int nWidth, int nHeight)
{
	m_nFrameRate = nFrameRate;
	m_nWidth = nWidth;
	m_nHeight = nHeight;
}
*/

void CRoleAdmin::receiveConfig(CAVConfigData::pointer configData)
{
	BOOST_ASSERT(configData.get() != 0);

	if (configData->getIsVideo() && mVideoReceiver != NULL)
	{
		mVideoReceiver->SetupMediaType(configData);
	}else if (!configData->getIsVideo() && mAudioReceiver != NULL)
	{
		//m_avcoding
		mAudioReceiver->SetupMediaType(configData);
	}

}

void CRoleAdmin::receiveData(CAVData::pointer recvdata)
{
	if (recvdata.get() == NULL) return;
	if (m_bKilledOwner || m_bIsRemotePaused)
	{
		return;
	}

	if (recvdata->isVideo() && mVideoReceiver != NULL)
	{
		// 每5个数据抛弃掉一个；
		//if (m_nVideoOverflowData > 0 && ((m_nVideoOverflowData-- % 5) == 0))
		if (m_nVideoOverflowData > 0 && m_nVideoOverflowData-- > 0)
		{
			//const BYTE * data = recvdata->data();
			//// 查找I帧
			//char cFrame='u';
			//unsigned   int   head   =   *((unsigned   int   *)data);   
			//if(head == 0xb3010000 || head == 0xb0010000)   
			//{
			//	cFrame = 'i';   
			//}else if(head   ==   0xb6010000)   
			//{   
			//	unsigned   char   vt_byte   =   data[4];   
			//	vt_byte   &=   0xC0;   
			//	vt_byte   =   vt_byte   >>   6;   
			//	switch(vt_byte)   
			//	{   
			//	case   0:   
			//		cFrame = 'i';   
			//		break;
			//	default:
			//		break;
			//	}
			//}

			//// 如果不是I帧，丢掉该帧数据；
			//if (cFrame != 'i')
			//{
			//	return;
			//}
		}

		{
			if (!mIsVideoMediaTypeSent)
			{
				mVideoReceiver->BuildMediaType();
				mIsVideoMediaTypeSent = true;

				if (mVideoReceiver->GetVideoDuration() == 0.0)
				{
					/////////////////////////////////////////////////////////////
					// 增加三帧，避免写AVI文件时，去掉第一帧；
					mVideoReceiver->receiveData(recvdata);	
					mVideoReceiver->receiveData(recvdata);
					mVideoReceiver->receiveData(recvdata);
				}
			}
			mVideoReceiver->receiveData(recvdata);
			return;
		}
	}else if (!recvdata->isVideo() && mAudioReceiver != NULL)
	{
		// MPEG4要查找到I帧才开始接收数据，所以音频要等开始接收到视频数据后，才能开始处理；否则音视频将不同步；
		/*if (this->video() && !mIsVideoMediaTypeSent)
		{
			return;
		}*/

		// 每3个数据抛弃掉一个；
		//if (m_nAudioOverflowData > 0 && ((m_nAudioOverflowData-- % 3) == 0))
		if (m_nAudioOverflowData > 0 && (m_nAudioOverflowData-- > 0))
		{
			return;
		}

		{
			if (!mIsAudioMediaTypeSent)
			{
				mAudioReceiver->BuildMediaType();
				mIsAudioMediaTypeSent = true;
			}

			mAudioReceiver->receiveData(recvdata);
			return;
		}
	}
}

void CRoleAdmin::setSenderHandler(OnDSHandler * handler)
{
	this->m_handlerSender = handler;
}

void CRoleAdmin::SetRemoteVideoWindow(HWND inWindow)
{
	m_wndRemoteVideoWindow = inWindow;
}

void CRoleAdmin::SetRecordFilename(const WCHAR * recordFilename)
{
	if (recordFilename != NULL)
		m_recordFilename = recordFilename;
}

void CRoleAdmin::SetReadFilename(const WCHAR * readFilename)
{
	if (readFilename != NULL)
		m_readFilename = readFilename;
}

void CRoleAdmin::SetDeviceConfig(long inConfig)
{
	mDeviceConfig |= inConfig;
	if ((mDeviceConfig & Record_Remote_AV) == Record_Remote_AV)
		mDeviceConfig |= Remote_AV_SameGraph;
}

void CRoleAdmin::Disconnect(BOOL inNotifyPeer)
{
	m_bKilledOwner = true;
	setSenderHandler(NULL);
	if (m_procAVData)
	{
		m_procAVData->join();
		delete m_procAVData;
		m_procAVData = 0;
	}
	
	if (mVideoReceiver)
		mVideoReceiver->stopDoSampleProc();
	if (mAudioReceiver)
		mAudioReceiver->stopDoSampleProc();
	if (mVideoPreviewReceiver)
	{
		mVideoPreviewReceiver->stopDoSampleProc();
	}

	StopAll();

	Sleep(500);

	// Notify the remote to disconnect
	if (inNotifyPeer)
	{
	//	SendSimpleCommand(cmd_DisconnectRequest);
	}
	// Release all the local resources
	Deactivate();
	//Uninit();
}

bool CRoleAdmin::findVideoDev(void)
{
	static HRESULT CoInitializeHR = CoInitialize(NULL);
	ICreateDevEnum *pDevEnum = NULL;
	IEnumMoniker *pEnum = NULL;

	// Create the System Device Enumerator.
	HRESULT hr = CoCreateInstance(CLSID_SystemDeviceEnum, NULL,
		CLSCTX_INPROC_SERVER, IID_ICreateDevEnum, 
		reinterpret_cast<void**>(&pDevEnum));
	if (FAILED(hr))
	{
		return false;
	}
	// Create an enumerator for the video capture category.
	hr = pDevEnum->CreateClassEnumerator(
		CLSID_VideoInputDeviceCategory,  //CLSID_AudioInputDeviceCategory
		&pEnum, 0);
	if (FAILED(hr))
	{
		pDevEnum->Release();
		return false;
	}
	if (pEnum)
	{
		IMoniker *pMoniker = NULL;
		if (pEnum->Next(1, &pMoniker, NULL) == S_OK)
		{
			pMoniker->Release();
			pDevEnum->Release();
			return true;
		}else
		{	
			pDevEnum->Release();
			return false;

		}
	}
	return false;
}
bool CRoleAdmin::findAudioDev(void)
{
	static HRESULT CoInitializeHR = CoInitialize(NULL);
	ICreateDevEnum *pDevEnum = NULL;
	IEnumMoniker *pEnum = NULL;

	// Create the System Device Enumerator.
	HRESULT hr = CoCreateInstance(CLSID_SystemDeviceEnum, NULL,
		CLSCTX_INPROC_SERVER, IID_ICreateDevEnum, 
		reinterpret_cast<void**>(&pDevEnum));
	if (FAILED(hr))
	{
		return false;
	}
	// Create an enumerator for the video capture category.
	hr = pDevEnum->CreateClassEnumerator(
		CLSID_AudioInputDeviceCategory,  //
		&pEnum, 0);
	if (FAILED(hr))
	{
		pDevEnum->Release();
		return false;
	}
	if (pEnum)
	{
		IMoniker *pMoniker = NULL;
		if (pEnum->Next(1, &pMoniker, NULL) == S_OK)
		{
			pMoniker->Release();
			pDevEnum->Release();
			return true;
		}else
		{	
			pDevEnum->Release();
			return false;

		}
	}
	return false;

}

void CRoleAdmin::PlayLocal(void)
{
	if (mLocalVideoGraph)
	{
		mLocalVideoGraph->Run();
	}
	if (mLocalPreviewGraph)
	{
		mLocalPreviewGraph->Run();
	}
	if (mLocalAudioGraph)
	{
		mLocalAudioGraph->Run();
	}
}

void CRoleAdmin::PauseLocal(void)
{
	if (mLocalVideoGraph)
	{
		mLocalVideoGraph->Pause();
	}
	if (mLocalPreviewGraph)
	{
		mLocalPreviewGraph->Pause();
	}
	if (mLocalAudioGraph)
	{
		mLocalAudioGraph->Pause();
	}
}

void CRoleAdmin::StopLocal(void)
{
	//m_AviFile.CloseWriteFile();
	if (mLocalVideoGraph)
	{
		mLocalVideoGraph->Stop();
	}
	if (mLocalPreviewGraph)
	{
		mLocalPreviewGraph->Stop();
	}
	if (mLocalAudioGraph)
	{
		mLocalAudioGraph->Stop();
	}
}

void CRoleAdmin::PlayRemote(void)
{
	m_bIsRemotePaused = false;
	if (mRemoteVideoGraph)
	{
		mRemoteVideoGraph->Run();
	}

	if (mRemoteAudioGraph)
	{
		mRemoteAudioGraph->Run();
	}
}

void CRoleAdmin::PauseRemote(void)
{
	m_bIsRemotePaused = true;
	if (mRemoteVideoGraph)
	{
		mRemoteVideoGraph->Pause();
	}
	if (mRemoteAudioGraph)
	{
		mRemoteAudioGraph->Pause();
	}
}

void CRoleAdmin::StopRemote(void)
{
	//m_bIsRemotePaused = true;
	if (mRemoteVideoGraph)
	{
		mRemoteVideoGraph->Stop();
	}
	if (mRemoteAudioGraph)
	{
		mRemoteAudioGraph->Stop();
	}
}

void CRoleAdmin::StopAll(void)
{
	if (mVideoReceiver)
		mVideoReceiver->setOnDoSampleBuffer(NULL);
	if (mAudioReceiver)
		mAudioReceiver->setOnDoSampleBuffer(NULL);

	StopRemote();
	StopLocal();
}

void CRoleAdmin::CompleteVideoReceivingGraph(void)
{
	HRESULT hr = S_OK;


	if (mRemoteVideoGraph && mVideoReceiver && mVideoRenderer)
	{
		IPin * pOut = NULL;
		IPin * pIn = NULL;
		if (mVideoReceiver)
			pOut = mVideoReceiver->GetPin(0);

		if ((mDeviceConfig & Record_Remote_AV) == Record_Remote_AV)
		{
			// video receiver->Inftee1
			pIn  = m_filterInfTee1->GetPin(TRUE);
			hr = mRemoteVideoGraph->GetGraph()->Connect(pOut, pIn);

			// Inftee1 output1->AVI mux pin01
			pOut = m_filterInfTee1->GetPin(FALSE, "Output1");
			pIn  = m_filterAVIMux->GetPin(TRUE, "Input 01");
			hr = mRemoteVideoGraph->GetGraph()->Connect(pOut, pIn);

			// Inftee1 output2->xx
			pOut = m_filterInfTee1->GetPin(FALSE, "Output2");
		}

		if ((mDeviceConfig & Remote_Video_MPEG4Dec) == Remote_Video_MPEG4Dec)
		{
//#if (USES_H264CODEC)
			// xx->splitter
	/*		pIn  = m_filterMainConpectMPEGSplitter->GetPin(TRUE);
			hr = mRemoteVideoGraph->GetGraph()->Connect(pOut, pIn);

			// splitter->xx
			//pOut = m_filterMainConpectMPEGSplitter->GetPin(FALSE, "H264");
			pOut = m_filterMainConpectMPEGSplitter->GetPin(FALSE, "Video 1");
*/
//#endif // USES_H264CODEC
			// xx->decoder
			pIn  = mVideoDecoderFilter->GetPin(TRUE);
			hr = mRemoteVideoGraph->GetGraph()->Connect(pOut, pIn);
			/*
			// decoder->raw video filter
			pOut = mVideoDecoderFilter->GetPin(FALSE);
			pIn = mRawVideoFilter->GetPin(TRUE);
			hr = mRemoteVideoGraph->GetGraph()->Connect(pOut, pIn);

			// raw video filter->render
			pOut = mRawVideoFilter->GetPin(FALSE);
			pIn  = mVideoRenderer->GetPin(TRUE);
			*/
			// decoder->render
			pIn  = mVideoRenderer->GetPin(TRUE);
			pOut = mVideoDecoderFilter->GetPin(FALSE);
		}else
		{
			pIn  = mVideoRenderer->GetPin(TRUE);
		}

		// Try intelligent connect
		hr = mRemoteVideoGraph->GetGraph()->Connect(pOut, pIn);
		if (m_wndRemoteVideoWindow)
		{
#ifdef USE_VMRMode_Windowless
			mRemoteVideoGraph->InitWindowlessVMR(m_wndRemoteVideoWindow);
#else
			mRemoteVideoGraph->SetDisplayWindow(m_wndRemoteVideoWindow);
			mRemoteVideoGraph->SetNotifyWindow(m_wndRemoteVideoWindow);
#endif
		}

		if ((mDeviceConfig & (Remote_Has_Audio|Remote_AV_SameGraph)) == (Remote_Has_Audio|Remote_AV_SameGraph))
			SetEvent(m_hCompletedVideoGraph);
		else
			mRemoteVideoGraph->Run();
	}
}

void CRoleAdmin::CompleteAudioReceivingGraph(void)
{
	bool bIsRemoteAVAampleGraph = ((mDeviceConfig & Remote_AV_SameGraph) == Remote_AV_SameGraph);
	if (bIsRemoteAVAampleGraph && mRemoteVideoGraph == NULL) return;
	if (!bIsRemoteAVAampleGraph && mRemoteAudioGraph == NULL) return;

	if ((mDeviceConfig & Remote_AV_SameGraph) == Remote_AV_SameGraph)
		DWORD ret = WaitForSingleObject(this->m_hCompletedVideoGraph, INFINITE);

/*	if ((mDeviceConfig & Record_Remote_AV) == Record_Remote_AV)
	{
		IPin * pOut = mAudioReceiver->GetPin(0);
		IPin * pIn = NULL;
		HRESULT hr = S_OK;

		// receiver->aac decoder
		pIn  = mAacDecoderFilter->GetPin(TRUE);
		hr = mRemoteVideoGraph->GetGraph()->Connect(pOut, pIn);

		// aac decoder->AVI mux pin02
		pOut = mAacDecoderFilter->GetPin(FALSE);
		pIn  = m_filterAVIMux->GetPin(TRUE, "Input 02");	// ???
		hr = mRemoteVideoGraph->GetGraph()->Connect(pOut, pIn);

		IFileSinkFilter *pSink = NULL; 
		m_filterFileWriter->QueryInterface(IID_IFileSinkFilter, (void**)&pSink); 
		if (m_recordFilename.length() > 0)
			hr = pSink->SetFileName(m_recordFilename.c_str(), NULL);
		else
			hr = pSink->SetFileName(L"d:\\myvideo2.avi", NULL);

		// AVI mux->File writer
		pOut = m_filterAVIMux->GetPin(FALSE);
		pIn  = m_filterFileWriter->GetPin(TRUE);
		hr = mRemoteVideoGraph->GetGraph()->Connect(pOut, pIn);

		mRemoteVideoGraph->Run();
		return;
	}
	*/

	if (mAudioReceiver && mAudioRenderer)
	{
		IPin * pOut = mAudioReceiver->GetPin(0);
		IPin * pIn = NULL;
		HRESULT hr = S_OK;
		if ((mDeviceConfig & Remote_Audio_AACDec) == Remote_Audio_AACDec)
		{
			// audio receiver->aac decoder
			pIn  = mAacDecoderFilter->GetPin(TRUE);
			if (bIsRemoteAVAampleGraph)
				hr = mRemoteVideoGraph->GetGraph()->Connect(pOut, pIn);
			else
				hr = mRemoteAudioGraph->GetGraph()->Connect(pOut, pIn);

			// aac decoder->xx
			pOut = mAacDecoderFilter->GetPin(FALSE);
		}

		if ((mDeviceConfig & Record_Remote_AV) == Record_Remote_AV)
		{
			// xx(aac decoder)->Inftee2
			pIn  = m_filterInfTee2->GetPin(TRUE);
			hr = mRemoteVideoGraph->GetGraph()->Connect(pOut, pIn);

			// Inftee2 output1->AVI mux pin02
			pOut = m_filterInfTee2->GetPin(FALSE, "Output1");
			pIn  = m_filterAVIMux->GetPin(TRUE, "Input 02");
			hr = mRemoteVideoGraph->GetGraph()->Connect(pOut, pIn);

			IFileSinkFilter *pSink = NULL; 
			m_filterFileWriter->QueryInterface(IID_IFileSinkFilter, (void**)&pSink); 
			if (m_recordFilename.length() > 0)
				hr = pSink->SetFileName(m_recordFilename.c_str(), NULL);
			else
				hr = pSink->SetFileName(L"d:\\myvideo2.avi", NULL);

			// AVI mux->File writer
			pOut = m_filterAVIMux->GetPin(FALSE);
			pIn  = m_filterFileWriter->GetPin(TRUE);
			hr = mRemoteVideoGraph->GetGraph()->Connect(pOut, pIn);

			// Inftee2 output2->xx
			pOut = m_filterInfTee2->GetPin(FALSE, "Output2");
		}

		// Graph不设置任何参考时钟。
		//mRemoteVideoGraph->SetSyncSource(NULL);

		pIn  = mAudioRenderer->GetPin(TRUE);
		// Try intelligent connect
		if (bIsRemoteAVAampleGraph)
		{
			hr = mRemoteVideoGraph->GetGraph()->Connect(pOut, pIn);
			mRemoteVideoGraph->Run();
		}else
		{
			hr = mRemoteAudioGraph->GetGraph()->Connect(pOut, pIn);
			mRemoteAudioGraph->Run();
		}
	}
}

BOOL CRoleAdmin::CreateGraph(void)
{
	BOOL pass = TRUE;
	if ((mDeviceConfig & Remote_AV_SameGraph) == Remote_AV_SameGraph)
	{
		mRemoteVideoGraph = new CDXGraph();
		pass = mRemoteVideoGraph->Create();
	}else
	{

		// Prepare to receive remote video
		if ((mDeviceConfig & Remote_Has_Video) == Remote_Has_Video)
		{
			mRemoteVideoGraph = new CDXGraph();
			pass = mRemoteVideoGraph->Create();
		}
		// Prepare to receive remote audio
		if ((mDeviceConfig & Remote_Has_Audio) == Remote_Has_Audio)
		{
			mRemoteAudioGraph = new CDXGraph();
			pass = mRemoteAudioGraph->Create();
		}
	}
	bool bIsSplitLocalAV = (mDeviceConfig & Split_Local_AV) == Split_Local_AV;
	// Prepare to send local video
	if ((mDeviceConfig & Local_Has_Video) == Local_Has_Video || bIsSplitLocalAV)
	{
		mLocalVideoGraph = new CDXGraph();
		pass = mLocalVideoGraph->Create();

		mLocalPreviewGraph = new CDXGraph();
		pass = mLocalPreviewGraph->Create();
	}
	// Prepare to send local audio
	if ((mDeviceConfig & Local_Has_Audio) == Local_Has_Audio)
	{
		mLocalAudioGraph = new CDXGraph();
		pass = mLocalAudioGraph->Create();
	}
	
	return pass;
}

void CRoleAdmin::DeleteGraph(void)
{
	// Clean up the video window
	if (m_wndRemoteVideoWindow)
	{
		if (mRemoteVideoGraph)
		{
			mRemoteVideoGraph->SetDisplayWindow(0);
			mRemoteVideoGraph->SetNotifyWindow(0);
		}
		m_wndRemoteVideoWindow = NULL;
	}

	SAFE_DELETE(mRemoteVideoGraph);
	SAFE_DELETE(mRemoteAudioGraph);
	SAFE_DELETE(mLocalVideoGraph);
	SAFE_DELETE(mLocalPreviewGraph);
	SAFE_DELETE(mLocalAudioGraph);
}

BOOL CRoleAdmin::CreateInputFilters(void)
{
	BOOL  pass = FALSE;
	HRESULT hr = NOERROR;

	// Create net receiver filters
	if ((mDeviceConfig & Remote_Has_Video) == Remote_Has_Video)
	{
		mVideoReceiver = new CFilterNetReceiver(NULL, &hr);
		IBaseFilter * pFilter = NULL;
		mVideoReceiver->QueryInterface(IID_IBaseFilter, (void**)&pFilter);
		pass |= mRemoteVideoGraph->AddFilter(pFilter, L"Video Receiver");
			// start proc to doSample
		mVideoReceiver->startDoSampleProc();
		mVideoReceiver->setOnDoSampleBuffer(this);
	}
	if ((mDeviceConfig & Remote_Has_Audio) == Remote_Has_Audio)
	{
		mAudioReceiver = new CFilterNetReceiver(NULL, &hr);
		IBaseFilter * pFilter = NULL;
		mAudioReceiver->QueryInterface(IID_IBaseFilter, (void**)&pFilter);
		if ((mDeviceConfig & Remote_AV_SameGraph) == Remote_AV_SameGraph)
			pass |= mRemoteVideoGraph->AddFilter(pFilter, L"Audio Receiver");
		else
			pass |= mRemoteAudioGraph->AddFilter(pFilter, L"Audio Receiver");

		//if ((mDeviceConfig & Remote_AV_SameGraph) != Remote_AV_SameGraph)
		{
			// Listen to the message frome video receiver

			mAudioReceiver->startDoSampleProc();
			mAudioReceiver->setOnDoSampleBuffer(this);
		}
	}
	bool bIsSplitLocalAV = (mDeviceConfig & Split_Local_AV) == Split_Local_AV;
	if (bIsSplitLocalAV)
	{
#if (USE_AVIREADERFILTER)
		m_filterAsyncFileReader = new CDXFilter(mLocalVideoGraph->GetGraph(), 
			CLSID_AVIDoc, "AVI file reader filter");
		pass |= m_filterAsyncFileReader->CreateFilter();

#else
		// create AVI splitter filter
		m_filterAVISplitter = new CDXFilter(mLocalVideoGraph->GetGraph(), 
			CLSID_AviSplitter, "AVI splitter filter");
		pass |= m_filterAVISplitter->CreateFilter();

		// create Async file reader filter
		m_filterAsyncFileReader = new CDXFilter(mLocalVideoGraph->GetGraph(), 
			CLSID_AsyncReader, "Async file reader filter");
		pass |= m_filterAsyncFileReader->CreateFilter();
#endif // (USE_AVIREADERFILTER)
		if (m_readFilename.length() > 0)
		{
			IFileSourceFilter *pSink = NULL; 
			m_filterAsyncFileReader->QueryInterface(IID_IFileSourceFilter, (void**)&pSink); 
			if (pSink)
			{
				hr = pSink->Load(m_readFilename.c_str(), NULL);
				pSink->Release();
			}

			//////////////////////////////////////////////////////
			// Add AudioRenderer for clock.
/*
			mAudioRenderer = new CDXFilter(mLocalVideoGraph->GetGraph(), 
				CLSID_DSoundRender, "Audio Renderer");
			pass |= mAudioRenderer->CreateFilter();
			*/
		}
	}

	// Create local video and audio devices
	if ((mDeviceConfig & Local_Has_Video) == Local_Has_Video)
	{
		mVideoFilter = new CVideoCaptureFilter2(mLocalVideoGraph->GetGraph());
		((CVideoCaptureFilter2 *)mVideoFilter)->SetDevice(mVideoDevice);
		pass |= mVideoFilter->CreateFilter();
		if (!pass) return pass;

		mVideoPreviewReceiver = new CFilterNetReceiver(NULL, &hr);
		IBaseFilter * pFilter = NULL;
		mVideoPreviewReceiver->QueryInterface(IID_IBaseFilter, (void**)&pFilter);
		pass |= mLocalPreviewGraph->AddFilter(pFilter, L"Video Receiver");

	
		mVideoPreviewReceiver->startDoSampleProc();
		mVideoPreviewReceiver->setOnDoSampleBuffer(NULL);
		
		mVideoPreviewRenderer = new CDXFilter(mLocalPreviewGraph->GetGraph(), 
			//CLSID_VideoRenderer, "Video Renderer");
			CLSID_VideoMixingRenderer9, "Video Mixing Renderer9");
		pass |= mVideoPreviewRenderer->CreateFilter();

		//mVideoPreviewDecoderFilter = new CDXFilter(mLocalPreviewGraph->GetGraph(), 
		//	CLSID_ffdshowVideoDecoderFilter, "ffdsow Video Decoder Filter");	// OK

		//pass = mVideoPreviewDecoderFilter->CreateFilter();
//


#ifdef USE_CaptureGraphBuilder2
        hr = m_graphBuilder2_.CoCreateInstance( CLSID_CaptureGraphBuilder2 ); 
		pass |= SUCCEEDED(hr);
		hr = m_graphBuilder2_->SetFiltergraph(mLocalVideoGraph->GetGraph());
		pass |= SUCCEEDED(hr);
#endif // USE_CaptureGraphBuilder2

		if ((mDeviceConfig & Local_Video_MPEG4Enc) == Local_Video_MPEG4Enc)
		{
			mVideoEncoderFilter = new CDXFilter(mLocalVideoGraph->GetGraph(), 
				CLSID_ffdshowVideoEncoderFilter, "ffdshow Video Encoder Filter");
			pass = mVideoEncoderFilter->CreateFilter();
		}
	}
	if ((mDeviceConfig & Local_Has_Audio) == Local_Has_Audio)
	{
		mAudioFilter = new CAudioCaptureFilter2(bIsSplitLocalAV ? mLocalVideoGraph->GetGraph() : mLocalAudioGraph->GetGraph());
		((CAudioCaptureFilter2 *)mAudioFilter)->SetDevice(mAudioDevice);
		pass |= mAudioFilter->CreateFilter();
		if (!pass) return pass;
	}

	// For AVI splitter or audio capture.
	if ((mDeviceConfig & Local_Audio_AACEnc) == Local_Audio_AACEnc)
	{
		mAacEncoderFilter = new CDXFilter(bIsSplitLocalAV ? mLocalVideoGraph->GetGraph() : mLocalAudioGraph->GetGraph(), 
			CLSID_MONOGRAMAACEncoderFilter, "MONOGRAM AAC Encoder Filter");
		pass = mAacEncoderFilter->CreateFilter();
	}

	return pass;
}


void CRoleAdmin::ReleaseInputFilters(void)
{
	if (mVideoReceiver)
		mVideoReceiver->setOnDoSampleBuffer(NULL);
	if (mAudioReceiver)
		mAudioReceiver->setOnDoSampleBuffer(NULL);

	SAFE_RELEASE(mVideoReceiver);
	SAFE_RELEASE(mAudioReceiver);
	SAFE_DELETE(mVideoFilter);
	SAFE_DELETE(mAudioFilter);

	SAFE_RELEASE(mVideoPreviewReceiver);


#ifdef USE_CaptureGraphBuilder2
    m_graphBuilder2_.Release();
#endif // USE_CaptureGraphBuilder2

	SAFE_DELETE(mVideoEncoderFilter);
//	SAFE_DELETE(m3ivxMuxerFilter);
	SAFE_DELETE(mAacEncoderFilter);

	SAFE_DELETE(m_filterAVISplitter);
	SAFE_DELETE(m_filterAsyncFileReader);

	SAFE_DELETE(m_filterMP3Dec);

	SAFE_DELETE(mVideoPreviewDecoderFilter);
	SAFE_DELETE(mVideoPreviewRenderer);

}

BOOL CRoleAdmin::CreateOutputFilters(void)
{
	BOOL  pass = FALSE;
	HRESULT hr = NOERROR;

	bool bIsRemoteAVAampleGraph = ((mDeviceConfig & Remote_AV_SameGraph) == Remote_AV_SameGraph);
	bool bIsRecordRemoteAV = ((mDeviceConfig & Record_Remote_AV) == Record_Remote_AV);

	// Create video renderer and audio renderer for playback
	if ((mDeviceConfig & Remote_Has_Video) == Remote_Has_Video)
	{
		//if (!bIsRecordRemoteAV)
		{
			mVideoRenderer = new CDXFilter(mRemoteVideoGraph->GetGraph(), 
			//CLSID_VideoRenderer, "Video Renderer");
			CLSID_VideoMixingRenderer9, "Video Mixing Renderer9");
			pass |= mVideoRenderer->CreateFilter();
		}

		//if ((mDeviceConfig & Remote_Video_MPEG4Dec) == Remote_Video_MPEG4Dec && !bIsRecordRemoteAV)
		if ((mDeviceConfig & Remote_Video_MPEG4Dec) == Remote_Video_MPEG4Dec)
		{
			mVideoDecoderFilter = new CDXFilter(mRemoteVideoGraph->GetGraph(), 
//				CLSID_ffdshowVideoDecoderFilter, "ffdsow Video Decoder Filter");	// OK
#if (USES_H264CODEC)
				CLSID_MainConceptH264DecoderFilter, "MainConcept H.264/AVC1 Decoder Filter");
#else
				CLSID_ffdshowVideoDecoderFilter, "ffdsow Video Decoder Filter");	// OK
#endif
			pass = mVideoDecoderFilter->CreateFilter();

			// ffdshow raw video filter
			//		mRawVideoFilter = new CDXFilter(mRemoteVideoGraph->GetGraph(), 
			//			CLSID_ffdshowRawVideoFilter, "ffdshow Raw Video Filter");
			//		pass = mRawVideoFilter->CreateFilter();
/*
#if (USES_H264CODEC)
			m_filterMainConpectMPEGSplitter = new CDXFilter(mRemoteVideoGraph->GetGraph(), 
				//CLSID_MainConceptMPEGSplitterFilter, "MainConcept MPEG Splitter Filter"); // .h264可以，.3gp不行
				CLSID_GDCLMPEG4DemultiplexorFilter, "GDCL MPEG4 Demultiplexor Filter");
			pass = m_filterMainConpectMPEGSplitter->CreateFilter();
			
#endif // USES_H264CODEC
*/
		}

	}

	if ((mDeviceConfig & Record_Remote_AV) == Record_Remote_AV)
	{
		// create AVI mux filter
		m_filterAVIMux = new CDXFilter(mRemoteVideoGraph->GetGraph(), 
			CLSID_AviDest, "AVI mux filter");
		pass |= m_filterAVIMux->CreateFilter();

		// create file writer filter
		m_filterFileWriter = new CDXFilter(mRemoteVideoGraph->GetGraph(), 
			CLSID_FileWriter, "File writer filter");
		pass |= m_filterFileWriter->CreateFilter();

		// create Inftee1
		m_filterInfTee1 = new CDXFilter(mRemoteVideoGraph->GetGraph(), 
			CLSID_InfTee, "Inftee001 filter");
		pass |= m_filterInfTee1->CreateFilter();

		// create Inftee2
		m_filterInfTee2 = new CDXFilter(mRemoteVideoGraph->GetGraph(), 
			CLSID_InfTee, "Inftee002 filter");
		pass |= m_filterInfTee2->CreateFilter();

		// create MPEP-Layer-3 encoder
		// {6A08CF80-0E18-11CF-A24D-0020AFD79767}
//		m_filterMP3Enc = new CDXFilter(mRemoteVideoGraph->GetGraph(), 
//			CLSID_ACMWrapper, "MPEG-Layer-3 filter");
//		pass |= m_filterMP3Enc->CreateFilter();
	}


	if ((mDeviceConfig & Remote_Has_Audio) == Remote_Has_Audio)
	{
		//if (!bIsRecordRemoteAV)
		{
			mAudioRenderer = new CDXFilter(bIsRemoteAVAampleGraph ? mRemoteVideoGraph->GetGraph() : mRemoteAudioGraph->GetGraph(), 
				CLSID_DSoundRender, "Audio Renderer");
			pass |= mAudioRenderer->CreateFilter();
/*
			IPin * pIn = mAudioRenderer->GetPin(TRUE);

			// IAMStreamConfig test.
			IAMStreamConfig * pCfg = NULL;
			HRESULT hr = pIn->QueryInterface(IID_IAMStreamConfig, (void **)&pCfg);
			// Read current media type/format
			AM_MEDIA_TYPE *pmt={0};
			hr = pCfg->GetFormat(&pmt);
			if (SUCCEEDED(hr))
			{

			}
*/
		}
		if ((mDeviceConfig & Remote_Audio_AACDec) == Remote_Audio_AACDec)
		{
			mAacDecoderFilter = new CDXFilter(bIsRemoteAVAampleGraph ? mRemoteVideoGraph->GetGraph() : mRemoteAudioGraph->GetGraph(), 
				CLSID_ffdshowAudioDecoderFilter, "ffdsow Audio Decoder Filter");
			pass = mAacDecoderFilter->CreateFilter();
		}

	}

	bool bIsSplitLocalAV = (mDeviceConfig & Split_Local_AV) == Split_Local_AV;
	// Create net sender filters
	if ((mDeviceConfig & Local_Has_Video) == Local_Has_Video || bIsSplitLocalAV)
	{
		mVideoSender = new CFilterNetSender(NULL, &hr);
		IBaseFilter * pFilter = NULL;
		mVideoSender->QueryInterface(IID_IBaseFilter, (void**)&pFilter);
		pass |= mLocalVideoGraph->AddFilter(pFilter, L"Video Sender");
	}
	if ((mDeviceConfig & Local_Has_Audio) == Local_Has_Audio || bIsSplitLocalAV)
	{
		mAudioSender = new CFilterNetSender(NULL, &hr);
		IBaseFilter * pFilter = NULL;
		mAudioSender->QueryInterface(IID_IBaseFilter, (void**)&pFilter);

		// Use the same VideoGraph when split local AV file.
		if (bIsSplitLocalAV)
			pass |= mLocalVideoGraph->AddFilter(pFilter, L"Audio Sender");
		else
			pass |= mLocalAudioGraph->AddFilter(pFilter, L"Audio Sender");
	}
	
	return pass;
}

void CRoleAdmin::ReleaseOutputFilters(void)
{
	SAFE_DELETE(mVideoRenderer);
	SAFE_DELETE(mAudioRenderer);
	SAFE_RELEASE(mVideoSender);
	SAFE_RELEASE(mAudioSender);

//	SAFE_DELETE(m3ivxSpltterFilter);
//	SAFE_DELETE(mOverlayMixer2Filter);
	SAFE_DELETE(mVideoDecoderFilter);
#if (USES_H264CODEC)
	SAFE_DELETE(m_filterMainConpectMPEGSplitter);
#endif // USES_H264CODEC
	SAFE_DELETE(mAacDecoderFilter);
//	SAFE_DELETE(mRawVideoFilter);

	SAFE_DELETE(m_filterAVIMux);
	SAFE_DELETE(m_filterFileWriter);
	SAFE_DELETE(m_filterInfTee1);
	SAFE_DELETE(m_filterInfTee2);

	SAFE_DELETE(m_filterMP3Enc);

}

#include <atlstr.h>
BOOL CRoleAdmin::ConnectFilters(void)
{
	BOOL   pass = TRUE;
	IPin * pOut = NULL;
	IPin * pIn  = NULL;
	bool bHasLocalMPEG4Enc = (mDeviceConfig & Local_Video_MPEG4Enc) == Local_Video_MPEG4Enc;
	bool bIsSplitLocalAV = (mDeviceConfig & Split_Local_AV) == Split_Local_AV;

	if (mVideoSender)
		mVideoSender->SetIsLiveMedia(!bIsSplitLocalAV);

	if (bIsSplitLocalAV)
	{
#if (USE_AVIREADERFILTER)
		USES_CONVERSION;
		CStringA sPinname = "";
		CStringA sReadFilename = W2A(m_readFilename.c_str());

		// avi reader(video)->video sender
		//sPinname.Format("%s 视频 #1", sReadFilename);
		sPinname = "视频";

		pOut = m_filterAsyncFileReader->GetPin(FALSE, sPinname);
		pIn  = mVideoSender->GetPin(0);
		pass = mLocalVideoGraph->ConnectFilters(pOut, pIn);

		// avi reader(audio)->aac decoder
		//sPinname.Format("%s 音频 #1", sReadFilename);
		sPinname = "音频";
		pOut = m_filterAsyncFileReader->GetPin(FALSE, sPinname);
		pIn  = mAacEncoderFilter->GetPin(TRUE);
		pass = mLocalVideoGraph->ConnectFilters(pOut, pIn);

#else
		// async file reader->avi splitter
		pOut = m_filterAsyncFileReader->GetPin(FALSE);
		pIn  = m_filterAVISplitter->GetPin(TRUE);
		pass = mLocalVideoGraph->ConnectFilters(pOut, pIn);

		// avi aplitter stream 00(video)->video sender
		pOut = m_filterAVISplitter->GetPin(FALSE, "Stream 00");
		pIn  = mVideoSender->GetPin(0);
		pass = mLocalVideoGraph->ConnectFilters(pOut, pIn);

		// avi aplitter stream 01(audio)->aac decoder
		pOut = m_filterAVISplitter->GetPin(FALSE, "Stream 01");
		pIn  = mAacEncoderFilter->GetPin(TRUE);
		pass = mLocalVideoGraph->ConnectFilters(pOut, pIn);

#endif	// (USE_AVIREADERFILTER)
		// aac encoder->audio sender
		pOut = mAacEncoderFilter->GetPin(FALSE);
		pIn  = mAudioSender->GetPin(0);
		pass = mLocalVideoGraph->ConnectFilters(pOut, pIn);

		return pass;
	}

	// Only connect sending filter graphs
	if (mLocalVideoGraph && mVideoFilter && mVideoSender)
	{		
		if (mVideoDevice->IsDVDevice()) // DV camcorder
		{
			pOut = mVideoFilter->GetPin(FALSE, "DV Vid Out");
			pIn  = mVideoSender->GetPin(0);
			pass = RenderDVVideo(mLocalVideoGraph, pOut, pIn);
		}
		else // USB camera
		{
			// IAMStreamConfig test.b
			pOut = mVideoFilter->GetPin(FALSE, "Capture");
			if (pOut == NULL)
				pOut = mVideoFilter->GetPin(FALSE, "捕获");

			IAMStreamConfig * pCfg = NULL;
			HRESULT hr = pOut->QueryInterface(IID_IAMStreamConfig, (void **)&pCfg);
			// Read current media type/format
			int nCounts,nSize;
			AM_MEDIA_TYPE *pmt={0};
			bool isSetMediaSubtype = false;
			VIDEO_STREAM_CONFIG_CAPS  pSCC;
			int H264CS=1;
			XInfo xinfo;
			pCfg->GetNumberOfCapabilities(&nCounts, &nSize);
			for (int i = 0; i < nCounts; i++)
			{
				if (pCfg->GetStreamCaps(i, &pmt, (BYTE *)&pSCC) == S_OK)
				{
					if (pmt->subtype == MEDIASUBTYPE_RGB32)
					{
						
						VIDEOINFOHEADER *pvi = (VIDEOINFOHEADER *)pmt->pbFormat;
						if((pvi->bmiHeader.biWidth == this->videoWidth())&&(pvi->bmiHeader.biHeight == this->videoHeight()))
						{
							pCfg->SetFormat(pmt);
							xinfo.csp(XVID_CSP_BGRA);
							m_MediaSubtype = MST_RGB32;
							isSetMediaSubtype = true;
							break;
						}
					}
					else if (pmt->subtype == MEDIASUBTYPE_RGB24)
					{
						VIDEOINFOHEADER *pvi = (VIDEOINFOHEADER *)pmt->pbFormat;
						if((pvi->bmiHeader.biWidth == this->videoWidth())&&(pvi->bmiHeader.biHeight == this->videoHeight()))
						{
							pCfg->SetFormat(pmt);
							xinfo.csp(XVID_CSP_BGR);
							H264CS = TBCOLORSPACE_RGB24;
							m_MediaSubtype = MST_RGB24;
							isSetMediaSubtype = true;
							break;
						}
					}
					else if (pmt->subtype == MEDIASUBTYPE_YUY2)
					{
						VIDEOINFOHEADER *pvi = (VIDEOINFOHEADER *)pmt->pbFormat;
						if((pvi->bmiHeader.biWidth == this->videoWidth())&&(pvi->bmiHeader.biHeight == this->videoHeight()))
						{
							pCfg->SetFormat(pmt);
							xinfo.csp(XVID_CSP_YUY2);
							H264CS = TBCOLORSPACE_YUY2;
							m_MediaSubtype = MST_YUY2;
							isSetMediaSubtype = true;
							break;
						}
					}
					else if (pmt->subtype == MEDIASUBTYPE_H264)
					{
						VIDEOINFOHEADER *pvi = (VIDEOINFOHEADER *)pmt->pbFormat;
						if((pvi->bmiHeader.biWidth == this->videoWidth())&&(pvi->bmiHeader.biHeight == this->videoHeight()))
						{
							pCfg->SetFormat(pmt);
							m_MediaSubtype = MST_H264;
							isSetMediaSubtype = true;
							break;
						}
					}
				}
			}
			if (!isSetMediaSubtype)
			{
				//return false;
				 DeleteMediaType(pmt);
				 pCfg->Release();
				 return false;
			}

		/*
					hr = pCfg->GetFormat(&pmt);
							if (SUCCEEDED(hr))
							{
								int H264CS;
								XInfo xinfo;
								if (pmt->subtype == MEDIASUBTYPE_RGB24)
								{
									xinfo.csp(XVID_CSP_BGR);
									H264CS = TBCOLORSPACE_RGB24;
									m_MediaSubtype = MST_RGB24;
								}else if (pmt->subtype == MEDIASUBTYPE_RGB32)
								{
									xinfo.csp(XVID_CSP_BGRA);
									m_MediaSubtype = MST_RGB32;
								}else if (pmt->subtype == MEDIASUBTYPE_YUY2)
								{
									xinfo.csp(XVID_CSP_YUY2);
									H264CS = TBCOLORSPACE_YUY2;
									m_MediaSubtype = MST_YUY2;
								}else if (pmt->subtype == MEDIASUBTYPE_UYVY)
								{
									xinfo.csp(XVID_CSP_UYVY);
								}else if (pmt->subtype == MEDIASUBTYPE_YVYU)
								{
									xinfo.csp(XVID_CSP_YVYU);
								}else if (pmt->subtype == MEDIASUBTYPE_MJPG)
								{
									// ?
									//xinfo.csp(XVID_CSP_BGRA);
									m_MediaSubtype = MST_MJPG;
								}else if (pmt->subtype == MEDIASUBTYPE_H264)
								{	
									m_MediaSubtype = MST_H264;
								}else {
									H264CS = TBCOLORSPACE_YUY2;
								}*/
				



                if(pmt->formattype == FORMAT_VideoInfo)
                {
                    VIDEOINFOHEADER *pvi = (VIDEOINFOHEADER *)pmt->pbFormat;
                    //pvi->AvgTimePerFrame =(LONGLONG)(10000000 / m_nFrameRate);
					pvi->AvgTimePerFrame = this->videoFrequency();
					pvi->bmiHeader.biWidth = this->videoWidth();
					pvi->bmiHeader.biHeight = this->videoHeight();
					pvi->bmiHeader.biSizeImage = GetBitmapSize(&pvi->bmiHeader);

					/*if (pmt->subtype == MEDIASUBTYPE_MJPG)
						xinfo.bitcount(32);
					else*/
						xinfo.bitcount(pvi->bmiHeader.biBitCount);
					xinfo.width(this->videoWidth());
					xinfo.height(this->videoHeight());

					m_H264Codec.initH264Codec(this->videoWidth(),this->videoHeight(),this->videoFrequency(),4000,H264CS,true);

					m_xvid.setenc_info(xinfo);

                    hr = pCfg->SetFormat(pmt);
                }
                DeleteMediaType(pmt);
			//}

#ifdef USE_CaptureGraphBuilder2
			hr = m_graphBuilder2_->RenderStream(&PIN_CATEGORY_CAPTURE,
				&MEDIATYPE_Interleaved,
				mVideoFilter->GetFilter(), NULL, bHasLocalMPEG4Enc ? mVideoEncoderFilter->GetFilter() : mVideoSender);
			if(hr != NOERROR)
			{
				hr = m_graphBuilder2_->RenderStream(&PIN_CATEGORY_CAPTURE,
					&MEDIATYPE_Video,
					mVideoFilter->GetFilter(), NULL, bHasLocalMPEG4Enc ? mVideoEncoderFilter->GetFilter() : mVideoSender);
				if(hr != NOERROR)
				{
					pass = FALSE;
				}
			}

			//if (mVideoWindow)
			//{
			//	hr = m_graphBuilder2_->RenderStream(&PIN_CATEGORY_PREVIEW,
			//		&MEDIATYPE_Interleaved, mVideoFilter->GetFilter(), NULL, NULL);
			//	if(hr == VFW_S_NOPREVIEWPIN)
			//	{
			//	}
			//	else if(hr != S_OK)
			//	{
			//		// maybe it's DV?
			//		hr = m_graphBuilder2_->RenderStream(&PIN_CATEGORY_PREVIEW,
			//			&MEDIATYPE_Video, mVideoFilter->GetFilter(), NULL, NULL);
			//		if(hr == VFW_S_NOPREVIEWPIN)
			//		{
			//			// preview was faked up for us using the (only) capture pin
			//		}
			//		else if(hr != S_OK)
			//		{

			//		}
			//	}
			//	pass = SUCCEEDED(hr);
			//}
			if (mVideoWindow) ///有预览
			{
				IPin * pPreviewOut = NULL;
				IPin * pPreviewIn = NULL;
				if (mVideoPreviewReceiver)
				{
					//pConfigData->set
					CAVConfigData::pointer pConfigData = CAVConfigData::create();
					pConfigData->setIsVideo(true);
					pConfigData->setMediaSubtype(m_MediaSubtype);
					pConfigData->setWidth(this->videoWidth());
					pConfigData->setHeight(this->videoHeight());
					pConfigData->setFrequency(this->videoFrequency());
					mVideoPreviewReceiver->SetupMediaType(pConfigData);
					mVideoPreviewReceiver->BuildMediaType();
					//mVideoPreviewReceiver->ActiveSetType();
					pPreviewOut = mVideoPreviewReceiver->GetPin(0);
					if (m_MediaSubtype == MST_H264) ///H264添加解码Filter
					{
						mVideoPreviewDecoderFilter = new CDXFilter(mLocalPreviewGraph->GetGraph(), 
							CLSID_ffdshowVideoDecoderFilter, "ffdsow Video Decoder Filter");	
						BOOL isSuccess = mVideoPreviewDecoderFilter->CreateFilter();
						if(isSuccess)
						{
							pPreviewIn  = mVideoPreviewDecoderFilter->GetPin(TRUE);
							hr = mLocalPreviewGraph->GetGraph()->Connect(pPreviewOut, pPreviewIn);
							pPreviewOut = mVideoPreviewDecoderFilter->GetPin(FALSE);
						}
					}
					pPreviewIn  = mVideoPreviewRenderer->GetPin(TRUE);
					hr = mLocalPreviewGraph->GetGraph()->Connect(pPreviewOut, pPreviewIn);
				}



			}

			if ((mDeviceConfig & Local_Video_MPEG4Enc) == Local_Video_MPEG4Enc)
			{
				// encoder->sender
				pOut = mVideoEncoderFilter->GetPin(FALSE);
				pIn  = mVideoSender->GetPin(0);
				pass = mLocalVideoGraph->ConnectFilters(pOut, pIn);

				/*
//				IffdshowDec;
				//if ((mDeviceConfig & Local_Video_H264Enc) == Local_Video_H264Enc)
				{
				}

				// For test.
				CMediaType mediaType;
				mediaType.InitMediaType();
				mediaType.SetType(&MEDIATYPE_Video);
				mediaType.SetSubtype(&MEDIASUBTYPE_H264);
				pass = mLocalVideoGraph->ConnectFilters(pOut, pIn, &mediaType);
				*/
			}else
			{
				// source->sender
				pIn  = mVideoSender->GetPin(0);
				pass = mLocalVideoGraph->ConnectFilters(pOut, pIn);
			}

#else
			pOut = mVideoFilter->GetPin(FALSE, "Capture");
			if (pOut == NULL)
				pOut = mVideoFilter->GetPin(FALSE, "捕获");

			if ((mDeviceConfig & Local_Video_MPEG4Enc) == Local_Video_MPEG4Enc)
			{
				// video->encoder
				pIn  = mVideoEncoderFilter->GetPin(TRUE);
				pass = mLocalVideoGraph->ConnectFilters(pOut, pIn);

				// encoder->sender
				pOut = mVideoEncoderFilter->GetPin(FALSE);
				pIn  = mVideoSender->GetPin(0);
				pass = mLocalVideoGraph->ConnectFilters(pOut, pIn);

				// encoder->muxer
				/*			pOut = mVideoEncoderFilter->GetPin(FALSE);
				pIn  = m3ivxMuxerFilter->GetPin(TRUE);
				pass = mLocalVideoGraph->ConnectFilters(pOut, pIn);

				// muxer->sender
				pOut = m3ivxMuxerFilter->GetPin(FALSE);
				pIn  = mVideoSender->GetPin(0);
				pass = mLocalVideoGraph->ConnectFilters(pOut, pIn);
				*/
			}else
			{
				pIn  = mVideoSender->GetPin(0);
				//			AM_MEDIA_TYPE * mt = ((CVideoCaptureFilter2*)mVideoFilter)->SelectMediaType(pOut);
				pass = mLocalVideoGraph->ConnectFilters(pOut, pIn);
			}
#endif // USE_CaptureGraphBuilder2

		}
	}

	if (mLocalAudioGraph && mAudioFilter && mAudioSender)
	{
		pOut = mAudioFilter->GetPin(FALSE);

		// IAMStreamConfig test.
		IAMStreamConfig * pCfg = NULL;
		HRESULT hr = pOut->QueryInterface(IID_IAMStreamConfig, (void **)&pCfg);
		// Read current media type/format
		AM_MEDIA_TYPE *pmt={0};
		hr = pCfg->GetFormat(&pmt);
		if (SUCCEEDED(hr))
		{
			// Fill in values for the new format		
			WAVEFORMATEX *pWF = (WAVEFORMATEX *) pmt->pbFormat;
			 /*pWF->nChannels = (Word) nChannels;						// 2
			pWF->nSamplesPerSec = nFrequency;							// 44100
			pWF->nAvgBytesPerSec = lBytesPerSecond;						// 176400=44100*4
			pWF->wBitsPerSample = (WORD) (nBytesPerSample * 8);			// 16;	nBytesPerSample=2
			pWF->nBlockAlign = (WORD) (nBytesPerSample * nChannels);	// 4
			// Set the new formattype for the output pin
			hr = pCfg->SetFormat(pmt);*/
			int nBytesPerSample = 2;
			pWF->nChannels = (WORD) this->audioChannel();						// 2
			pWF->nSamplesPerSec = this->audioFrequency();					// 44100
			pWF->nBlockAlign = (WORD) (nBytesPerSample * this->audioChannel());
			pWF->nAvgBytesPerSec = this->audioFrequency()*pWF->nBlockAlign;		// 176400=44100*4
			pWF->wBitsPerSample = (WORD) (nBytesPerSample * 8);			// 16;	nBytesPerSample=2
			// Set the new formattype for the output pin
			hr = pCfg->SetFormat(pmt);
			DeleteMediaType(pmt);
		}
		// Release interfaces
		pCfg->Release();
		((CAudioCaptureFilter2 *)mAudioFilter)->SetCaptureBufferSize();

		//设置aac编码器的bitrate
		// ???
		//if (this->getAudioFrequency() == 8000)
		//{
		//	IMonogramAACEncoder * pProperty=NULL;
		//	AACConfig pAacConfig ;
		//	mAacEncoderFilter->QueryInterface(IID_IMonogramAACEncoder,(void**) &pProperty);
		//	if (pProperty)
		//	{
		//		pProperty->GetConfig(&pAacConfig);
		//		pAacConfig.bitrate = 48;         //视频采集率8000时为48或者32 采集率32000或者44100时为128
		//		pProperty->SetConfig(&pAacConfig);
		//		pProperty->Release();
		//	}
		//}

		// IAMBufferNegotiation test.
		IAMBufferNegotiation * pNeg = NULL;
		pOut->QueryInterface(IID_IAMBufferNegotiation, (void **)&pNeg);
		// Set the buffer size based on selected settings
		ALLOCATOR_PROPERTIES prop={0};
		hr = pNeg->GetAllocatorProperties(&prop);
		if (SUCCEEDED(hr))
		{		
			//prop.cbBuffer = lBufferSize;
			//prop.cBuffers = 6;
			//prop.cbAlign = nBytesPerSample * nChannels;
			//hr = pNeg->SuggestAllocatorProperties(&prop);
		}
		pNeg->Release();


		if ((mDeviceConfig & Local_Audio_AACEnc) == Local_Audio_AACEnc)
		{
			// source->aac encoder
			pIn  = mAacEncoderFilter->GetPin(TRUE);
			pass = mLocalAudioGraph->ConnectFilters(pOut, pIn);

			// aac encoder->x
			pOut = mAacEncoderFilter->GetPin(FALSE);
		}

		pIn  = mAudioSender->GetPin(0);
		pass = mLocalAudioGraph->ConnectFilters(pOut, pIn);
	}

	return pass;
}

BOOL CRoleAdmin::AfterBuildGraph(void)
{
	if ((mDeviceConfig & Remote_AV_SameGraph) == Remote_AV_SameGraph)
	{
		//startDoSampleProc();
	}
	// Start sending filter graphs
	if (mLocalVideoGraph)
	{
		// for local preview
		if (mVideoWindow)
		{
			mLocalPreviewGraph->SetDisplayWindow(mVideoWindow);
			mLocalPreviewGraph->SetNotifyWindow(mVideoWindow);
			mLocalPreviewGraph->Run();
			/*mLocalVideoGraph->SetDisplayWindow(mVideoWindow);
			mLocalVideoGraph->SetNotifyWindow(mVideoWindow);*/
		}
		if ((mDeviceConfig & Split_Local_AV) != Split_Local_AV)
			mLocalVideoGraph->Run();
	}
	if (mLocalAudioGraph)
	{
		mLocalAudioGraph->Run();
	}

	if (mAudioSender)
	{
		mAudioSender->SetHandler(this);
		//mAudioSender->SetCoding(this->audioCoding());
	}

	if (mVideoSender)
	{
		mVideoSender->SetHandler(this);
		//mVideoSender->SetCoding(this->videoCoding());
	}

	m_bKilledOwner = false;
	if (m_procAVData == 0)
	{
		m_procAVData = new boost::thread(boost::bind(&CRoleAdmin::do_procAVData, this));
	}
	
	return TRUE;
}

void CRoleAdmin::onAfterSetMediaType(bool bIsVideo)
{
	if (bIsVideo)
	{
		//m_xvid.setdec_info(XInfo(1));
		CompleteVideoReceivingGraph();
	}else
	{
		CompleteAudioReceivingGraph();
	}
}

/*
void CRoleAdmin::onPause(BOOL bIsVideo)
{
	if (bIsVideo || ((mDeviceConfig & Remote_AV_SameGraph) == Remote_AV_SameGraph))
	{
		if (mIsVideoMediaTypeSent && mRemoteVideoGraph)
			mRemoteVideoGraph->Pause();
	}else
	{
		if (mIsAudioMediaTypeSent && mRemoteAudioGraph)
			mRemoteAudioGraph->Pause();
	}
}

void CRoleAdmin::onRun(BOOL bIsVideo)
{
	if (bIsVideo || ((mDeviceConfig & Remote_AV_SameGraph) == Remote_AV_SameGraph))
	{
		if (mIsVideoMediaTypeSent && mRemoteVideoGraph)
			mRemoteVideoGraph->Run();
	}else
	{
		if (mIsAudioMediaTypeSent && mRemoteAudioGraph)
			mRemoteAudioGraph->Run();
	}
}
*/

void CRoleAdmin::onIsOverflow(bool bIsVideo, long nDataSize)
{
	if (!this->audio() || !this->video()) return;

	// （可调整）
	if (bIsVideo)
	{
		if (m_nVideoOverflowData == 0 && nDataSize > 50)
		{
			m_nVideoOverflowData = 20;						// 抛弃掉后面（N）个数据；
			//m_nAudioOverflowData = m_nVideoOverflowData*2;	// ??
		}
	}else
	{
		if (m_nAudioOverflowData == 0 && nDataSize > 50)
			m_nAudioOverflowData = 20;
	}
}

int CRoleAdmin::onLastSampleTime(bool bIsVideo, REFERENCE_TIME lastSampleTime)
{
	if (bIsVideo)
	{
		if (++m_nVideoCount > 30)
		{
			// ??? 为什么视频落后这么多
			REFERENCE_TIME result = lastSampleTime - m_lastSampleAudioTime;
			if (result > 6000000)
				return 1;		// 视频超前音频
			else if (result < -6000000)
				return -1;		// 视频落后音频
			m_nVideoCount = 0;
		}
	}else
	{
		m_lastSampleAudioTime = lastSampleTime;
	}

	return 0;
}

void CRoleAdmin::doProcAVData(void)
{
	while (!m_bKilledOwner)
	{
		CAVData::pointer avdata;
		if (!m_avdatas.front(avdata))
		{
			Sleep(100);
			continue;
		}

		if (m_handlerSender)
			m_handlerSender->receiveAVData(avdata);
	}

	m_avdatas.clear();
}

void CRoleAdmin::do_procAVData(CRoleAdmin * pOwner)
{
	BOOST_ASSERT (pOwner != NULL);

	pOwner->doProcAVData();
}

void CRoleAdmin::onReceive(const unsigned char * data, long size, unsigned int timestamp, bool is_video)
{
	BOOST_ASSERT (data != NULL && size > 0);
	if (mVideoWindow) ///有预览
	{
		if (is_video)
		{
			mVideoPreviewReceiver->receiveData(CAVData::create(data, size, timestamp, is_video));
		}
	}

	if (m_handlerSender == NULL) return;

	if (is_video)
	{
		switch (this->videoCoding())
		{
		/*case AVCoding::CT_H263:
			{

				if (!m_videoCodec.isopenav())
				{
					if (m_videoCodec.openav(CODEC_ID_H263))
						return;
				}

				m_videoCodec.encode(data, size);
			}break;*/
		case AVCoding::CT_XVID:
			{
				unsigned char * encode_buffer;

				int encode_size = 0;

				if (m_xvid.encode(data, &encode_buffer, encode_size))
				{
					//unsigned char* copy_buffer = new unsigned char[encode_size];
					//memcpy(copy_buffer,encode_buffer,encode_size);
					//mVideoPreviewReceiver->receiveData(CAVData::create(copy_buffer, encode_size, timestamp, is_video));
					m_avdatas.add(CAVData::create(encode_buffer, encode_size, timestamp, is_video));
					//if (m_handlerSender)
					//	m_handlerSender->receiveAVData(CAVData::create(encode_buffer, encode_size, timestamp, is_video));
				}
				return;
			}break;
		case AVCoding::CT_H264:
			{
				unsigned char* encode_buffer = new unsigned char[100*1024];
				int encode_size = 0;
				bool bKeyframe;
				//long nowTime = GetTickCount();
				if (m_H264Codec.Encode((unsigned char*)data,size,encode_buffer, encode_size,bKeyframe))
				{
					//long dealTime = GetTickCount() - nowTime;
					//FILE *fp = fopen("D:\\Temp\\count.txt", "a"); //以添加的方式输出到文件
					//if (fp)
					//{
					//	fprintf(fp, "%d,Time:%d\n", encode_size,dealTime);  
					//	fclose(fp);  
					//} 

					
					m_avdatas.add(CAVData::create(encode_buffer, encode_size, timestamp, is_video));
					//if (m_handlerSender)
					//	m_handlerSender->receiveAVData(CAVData::create(encode_buffer, encode_size, timestamp, is_video));
				//	m_AviFile.insertDataPack(encode_buffer,encode_size,timestamp, is_video);
	



	
					
					/*
					if (!m_videoCodec.isopenav())
										{
											FVInfo fvinfo;
											fvinfo.width(this->videoWidth());
											fvinfo.height(this->videoHeight());
											m_videoCodec.setdec_info(fvinfo);
					
											// CODEC_ID_MPEG4
											// CODEC_ID_XVID
											if (!m_videoCodec.openav(CODEC_ID_H264))
												return;
										}
										unsigned char * decode_buffer;
										int decode_size = 0;
										if (m_videoCodec.decode(encode_buffer, encode_size, &decode_buffer, decode_size, this->reversal()))
										{
											//receiveData(CAVData::create(decode_buffer, decode_size, timestamp, is_video));
										}*/
					
				}
				return;
			}break;
		//case AVCoding::CT_MPEG4:
		//	{
		//		// For TEST
		//		if (!m_videoCodec.isopenav())
		//		{
		//			return;
		//			// CODEC_ID_MPEG4
		//			// CODEC_ID_H263
		//			if (!m_videoCodec.openav(CODEC_ID_MPEG4))
		//				return;
		//		}

		//		unsigned char * encode_buffer;
		//		int encode_size = 0;
		//		if (m_videoCodec.encode(data, size, &encode_buffer, encode_size))
		//		{
		//			if (m_handlerSender)
		//				m_handlerSender->receiveAVData(CAVData::create(encode_buffer, encode_size, timestamp, is_video));
		//		}
		//		return;
		//	}break;
		default:
			break;
		}


	}else
	{
		switch (this->audioCoding())
		{
		case AVCoding::CT_G711A:
			{
				unsigned char * tempBuffer = new unsigned char[size/2];
				G711Coder::EncodeA(tempBuffer,(const short*)data, size/2);
				m_avdatas.add(CAVData::create(tempBuffer, size/2, timestamp, is_video));
				//m_handlerSender->receiveAVData(CAVData::create(tempBuffer, size/2, timestamp, is_video));
				return;
			}break;
		case AVCoding::CT_G711U:
			{
				unsigned char * tempBuffer = new unsigned char[size/2];
				G711Coder::EncodeU(tempBuffer,(const short*)data, size/2);
				m_avdatas.add(CAVData::create(tempBuffer, size/2, timestamp, is_video));
				//if (m_handlerSender)
				//	m_handlerSender->receiveAVData(CAVData::create(tempBuffer, size/2, timestamp, is_video));
				return;
			}break;
		case AVCoding::CT_G729A:
			{
				unsigned char * tempBuffer = new unsigned char[size*(10.0/80.0)];
				m_g729a.g729a_encoder((const short*)data, size/2, tempBuffer);
				m_avdatas.add(CAVData::create(tempBuffer, size*(10.0/80.0)/2, timestamp, is_video));
				//if (m_handlerSender)
				//	m_handlerSender->receiveAVData(CAVData::create(tempBuffer, size*(10.0/80.0)/2, timestamp, is_video));
				return;
			}break;
		case AVCoding::CT_AAC:
			{
				/*if (!m_audioCodec.isopenav())
				{
					if (!m_audioCodec.openav(CODEC_ID_AAC))
						return;
				}

				unsigned char * encode_buffer;
				int encode_size = 0;

				if (m_audioCodec.encode(data, size, &encode_buffer, encode_size))
				{
					m_handlerSender->receiveAVData(CAVData::create(encode_buffer, encode_size, timestamp, is_video));
				}
				return;*/
			}break;
		default:
			break;
		}
	}
	//FILE *fp = fopen("D:\\DataLen.txt", "a"); //以添加的方式输出到文件
	//if (fp)
	//{
	//	fprintf(fp, "%d\n", size);  
	//	fclose(fp);  
	//} 
	m_avdatas.add(CAVData::create(data, size, timestamp, is_video));
	//m_handlerSender->receiveAVData(CAVData::create(data, size, timestamp, is_video));
}

void CRoleAdmin::ReceiveRemoteData(const unsigned char * data, long size, unsigned int timestamp, bool is_video)
{
	BOOST_ASSERT (data != NULL && size > 0);

	if (is_video)
	{
		switch (this->videoCoding())
		{
		/*case AVCoding::CT_H263:
			{
				if (!m_videoCodec.isopenav())
				{
					if (m_videoCodec.openav(CODEC_ID_H263))
						return;
				}

				m_videoCodec.decode(data, size);

			}break;*/
		case AVCoding::CT_XVID:
			{
				/*unsigned char * decode_buffer;
				int decode_size = 0;
				if (m_xvid.decode(data, size, &decode_buffer, decode_size))
				{
					receiveData(CAVData::create(decode_buffer, decode_size, timestamp, is_video));
				}

				return;*/

				if (!m_videoCodec.isopenav())
				{
					FVInfo fvinfo;
					fvinfo.width(this->videoWidth());
					fvinfo.height(this->videoHeight());
					m_videoCodec.setdec_info(fvinfo);

					// CODEC_ID_MPEG4
					// CODEC_ID_XVID
					if (!m_videoCodec.openav(CODEC_ID_MPEG4))
						return;
				}
				unsigned char * decode_buffer;
				int decode_size = 0;
				if (m_videoCodec.decode(data, size, &decode_buffer, decode_size, this->reversal()))
				{
					receiveData(CAVData::create(decode_buffer, decode_size, timestamp, is_video));
				}
				return;
			}break;
		case AVCoding::CT_H264:
			{
				if (!m_videoCodec.isopenav())
				{
					FVInfo fvinfo;
					fvinfo.width(this->videoWidth());
					fvinfo.height(this->videoHeight());
					m_videoCodec.setdec_info(fvinfo);

					// CODEC_ID_MPEG4
					// CODEC_ID_XVID
					if (!m_videoCodec.openav(CODEC_ID_H264))
						return;
				}
				unsigned char * decode_buffer;
				int decode_size = 0;
				try
				{

					if (m_videoCodec.decode(data, size, &decode_buffer, decode_size, this->reversal()))
					{
						receiveData(CAVData::create(decode_buffer, decode_size, timestamp, is_video));
					}
				}
				catch (...)
				{
					
				}
				return;
			}break;
		default:
			break;
		}

	}else
	{
		switch (this->audioCoding())
		{
		case AVCoding::CT_G711A:
			{
				/*if (!m_audioCodec.isopenav())
				{
					if (!m_audioCodec.openav(CODEC_ID_PCM_ALAW))
						return;
				}
				m_audioCodec.decode(data, size);*/


				unsigned char * tempBuffer = new unsigned char[size * 2];
				G711Coder::DecodeA((short*)tempBuffer, data, size);
				receiveData(CAVData::create(tempBuffer, 2*size, timestamp, is_video));
				return;
			}break;
		case AVCoding::CT_G711U:
			{
				unsigned char * tempBuffer = new unsigned char[size * 2];
				G711Coder::DecodeU((short*)tempBuffer, data, size);
				receiveData(CAVData::create(tempBuffer, 2*size, timestamp, is_video));
				return;
			}break;
		case AVCoding::CT_G729A:
			{
				unsigned char * tempBuffer = new unsigned char[size * (80.0/10.0)*2];
				m_g729a.g729a_decoder(data, size, (short*)tempBuffer);
				receiveData(CAVData::create(tempBuffer, size*(80.0/10.0)*2, timestamp, is_video));
				return;
			}break;
		case AVCoding::CT_AAC:
			{
				if (!m_audioCodec.isopenav())
				{
					// AAC 
					BYTE config[2];
					getAACExtraBytes(config[0], config[1], this->audioChannel(), this->audioFrequency());

					if (!m_audioCodec.openav(CODEC_ID_AAC, config, 2))
						return;
				}
				unsigned char * decode_buffer;
				int decode_size = 0;
				if (m_audioCodec.decode(data, size, &decode_buffer, decode_size))
				{
					receiveData(CAVData::create(decode_buffer, decode_size, timestamp, is_video));
				}
				return;
			}break;
		default:
			break;
		}
	}

	receiveData(CAVData::create(data, size, timestamp, is_video));
}
