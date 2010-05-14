//
// CFilterNetReceiver.cpp
//

#include "GlobalDefs.h"
#include <streams.h>
#include "CFilterNetReceiver.h"
//#include "stldef.h"
#include <algorithm>
#include <initguid.h>
//#include "../G711Coder.h"

// {58E6FA74-AEEA-4129-A1C3-79107B535C6F}
DEFINE_GUID(CLSID_NetReceiver, 
0x58e6fa74, 0xaeea, 0x4129, 0xa1, 0xc3, 0x79, 0x10, 0x7b, 0x53, 0x5c, 0x6f);

////////////////////////////////////////////////////////////////////////////
CFilterNetReceiver::CFilterNetReceiver(LPUNKNOWN punk, HRESULT *phr) : 
CBaseFilter(NAME("Reciever"), punk, &mFilterLock, CLSID_NetReceiver)
, m_bIsStoped(false)
, m_hDoSampleProc(NULL)
, m_hOnDoSampleBuffer(NULL)
, m_hToSetMediaType(NULL)
, m_timeIncrementAudio(0)
//, m_avcoding(AVCoding::CT_Other)

{
	mOutPin  = new CNetOutPin(this, phr, L"Out");
//	mIsVideo = TRUE;
	mSample  = NULL;

	mAudioBytesPerSecond = 0;
	mLastSampleTimeV      = 0;
#if (USES_H264CODEC)
	//mLastSampleTimeA      = 15000000;
	mLastSampleTimeA      = 0;
#else
	mLastSampleTimeA      = 0;
#endif	// (USES_H264CODEC)

//	mReceiver.SetOwnerFilter(this);
	m_hToSetMediaType = CreateEvent(NULL, FALSE, FALSE, NULL);
	m_hKilledOwner = CreateEvent(NULL, FALSE, FALSE, NULL);
}

CFilterNetReceiver::~CFilterNetReceiver()
{
//	mReceiver.StopReceiving();
	if (m_hToSetMediaType)
	{
		CloseHandle(m_hToSetMediaType);
		m_hToSetMediaType = NULL;
	}
	if (m_hKilledOwner)
	{
		CloseHandle(m_hKilledOwner);
		m_hKilledOwner = NULL;
	}

	if (mSample)
	{
		mSample->Release();
		mSample = NULL;
	}

	delete mOutPin;
	//for_each(m_listReceived.begin(), m_listReceived.end(), DeletePtr());
	m_listReceived.clear();
}

int CFilterNetReceiver::GetPinCount()
{
	return 1;
}

CBasePin * CFilterNetReceiver::GetPin(int n)
{
	if (n == 0) 
	{
		return mOutPin;
	}
	else 
	{
		return NULL;
	}
}

STDMETHODIMP CFilterNetReceiver::Stop()
{
	CAutoLock lck(&mFilterLock);
	stopDoSampleProc();
	// Stop receiving to exit blocking
	//mReceiver.StopReceiving();
	if (mSample)
	{
		mSample->Release();
		mSample = NULL;
	}

	return CBaseFilter::Stop();
}

void CFilterNetReceiver::SetAudioBytesPerSecond(long inBytes)
{
	mAudioBytesPerSecond = inBytes;
}

BOOL CFilterNetReceiver::GetSampleBuffer(PBYTE * outBuffer)
{
	if (mSample)
	{
		mSample->Release();
		mSample = NULL;
	}

	// -2147220975=0x80040211=VFW_E_NOT_COMMITTED
	long nSize = 0;
	HRESULT hr = mOutPin->GetDeliveryBuffer(&mSample,NULL,NULL,0);
	if (mSample)
	{
		mSample->GetPointer(outBuffer);
		//nSize = mSample->GetSize();
	}
	return SUCCEEDED(hr);
}

IMediaSample * CFilterNetReceiver::GetMediaSample(void)
{
	IMediaSample * result = NULL;
	HRESULT hr = mOutPin->GetDeliveryBuffer(&result,NULL,NULL,0);
	return result;
}

bool CFilterNetReceiver::DeliverBuffer(const unsigned char * pData, long nLen, unsigned int timestamp)
{
	if (pData == NULL || nLen <= 0) return false;

	PBYTE  pSampleBuffer = NULL;
	IMediaSample * pSample = NULL;
	// Receive media data
	BOOL pass = FALSE;
	do
	{
		pSample = GetMediaSample();
		if (pSample != NULL)
		{
			pSample->GetPointer(&pSampleBuffer);
			if (pSampleBuffer != NULL)
				pass = TRUE;				
		}
		Sleep(10);
		if (m_bIsStoped)
			break;
	} while (!pass);

	if (m_bIsStoped) return false;

	//MessageBox(NULL, L"CFilterNetReceiver::doSampleBuffer memcpy()", L"test", MB_OK);
	memcpy(pSampleBuffer, pData, nLen);

	// Deliver this sample
	BOOL ret = DeliverHoldingSample(pSample, nLen, timestamp);

	return ret ? true : false;
}
//
//BOOL CFilterNetReceiver::DeliverHoldingSample(long inSampleSize)
//{
//
//	if (mSample && m_configData.get() != 0)
//	{
//		mSample->SetActualDataLength(inSampleSize);
//		mSample->SetSyncPoint(TRUE);
//		if (m_configData->getIsVideo())
//		{
//			// Do not use time stamp for video
//			mSample->SetTime(NULL, NULL);
//		}
//		else
//		{
//			REFERENCE_TIME rtStart = mLastSampleTimeA;
//			mLastSampleTimeA += (UNITS * inSampleSize / mAudioBytesPerSecond);
//			REFERENCE_TIME rtEnd   = mLastSampleTimeA;
//			mSample->SetTime(&rtStart, &rtEnd);
//		}
//
//		HRESULT hr = mOutPin->Deliver(mSample);
//		mSample->Release();
//		mSample = NULL;
//		return SUCCEEDED(hr);
//	}
//	return TRUE;
//}

BOOL CFilterNetReceiver::DeliverHoldingSample(IMediaSample * pSample, long inSampleSize, unsigned int timestamp)
{
	if (pSample && m_configData.get() != 0)
	{
		HRESULT hr = S_OK;
		pSample->SetActualDataLength(inSampleSize);
		if (m_configData->getIsVideo())
		{
			//  TIMECODE_SAMPLE
			// 要加上时间错才可以正常写文件；
			if (m_configData->timestampType() == CAVParameter::TT_Frequency && m_configData->getFrequency() > 0)
			{
				REFERENCE_TIME rtStart = mLastSampleTimeV;
				//mLastSampleTimeV += 500000;	// SameGraph OK, no decoder
				mLastSampleTimeV += m_configData->getFrequency();
				REFERENCE_TIME rtEnd   = mLastSampleTimeV;
				hr = pSample->SetTime(&rtStart, &rtEnd);
			}else if (m_configData->timestampType() == CAVParameter::TT_Timestamp && timestamp > 0)
			{
				REFERENCE_TIME rtEnd   = mLastSampleTimeV + timestamp;
				hr = pSample->SetTime(&mLastSampleTimeV, &rtEnd);
				mLastSampleTimeV = rtEnd;
			}else
				hr = pSample->SetTime(NULL, NULL);

			// Do not use time stamp for video
			//hr = pSample->SetTime(NULL, NULL);
		}
		else
		{
			if (m_configData->timestampType() == CAVParameter::TT_Frequency && m_timeIncrementAudio > 0)
			{
				REFERENCE_TIME rtStart = mLastSampleTimeA;
				//mLastSampleTimeA += 231973;
				mLastSampleTimeA += m_timeIncrementAudio;
				REFERENCE_TIME rtEnd   = mLastSampleTimeA;
				hr = pSample->SetTime(&rtStart, &rtEnd);
			//}else if (m_configData->timestampType() == CAVParameter::TT_Timestamp && timestamp > 0)
			}else if (m_configData->timestampType() == CAVParameter::TT_Timestamp)
			{
				REFERENCE_TIME rtEnd   = mLastSampleTimeA + timestamp;
				hr = pSample->SetTime(&mLastSampleTimeA, &rtEnd);
				mLastSampleTimeA = rtEnd;
			}else
				hr = pSample->SetTime(NULL, NULL);

			
/*			if (mLastSampleTime == 0)
				mLastSampleTime = presentationTime.tv_usec;
			REFERENCE_TIME rtStart = mLastSampleTime;
			mLastSampleTime = presentationTime.tv_usec;
			REFERENCE_TIME rtEnd   = mLastSampleTime;
			pSample->SetTime(&rtStart, &rtEnd);*/

		}
		pSample->SetSyncPoint(TRUE);
		hr = mOutPin->Deliver(pSample);
		pSample->Release();
		pSample = NULL;
		return SUCCEEDED(hr);
	}
	return TRUE;
}

// --- public methods ---
void CFilterNetReceiver::SetupMediaType(CAVConfigData::pointer configData)
{
	BOOST_ASSERT(configData.get() != 0);

	//m_avcoding = avc;
	//if (mOutPin)
	//	mOutPin->SetCoding(avc);

	m_configData = configData;
	if (!m_configData->getIsVideo())
	{
		m_timeIncrementAudio = m_configData->getFrequency() == 0 ? 0 :
			(DOUBLE)UNITS/((DOUBLE)(m_configData->getFrequency()*16*8)/(128*1024))-(DOUBLE)UNITS/m_configData->getFrequency();
	}
}

void CFilterNetReceiver::BuildMediaType(void)
{
	SetEvent(m_hToSetMediaType);
}

void CFilterNetReceiver::receiveData(CAVData::pointer receiveData)
{
	m_listReceived.add(receiveData);
}

void CFilterNetReceiver::startDoSampleProc(void)
{
	if (m_hDoSampleProc == NULL)
	{
		DWORD dwThreadId = 0;
		m_hDoSampleProc = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)CFilterNetReceiver::doSampleBufferProc, this, 0, &dwThreadId);
	}
}

void CFilterNetReceiver::stopDoSampleProc(void)
{
	m_bIsStoped = true;
	SetEvent(m_hKilledOwner);
	if (m_hDoSampleProc)
	{
		WaitForSingleObject(m_hDoSampleProc, INFINITE);
		CloseHandle(m_hDoSampleProc);
		m_hDoSampleProc = NULL;
	}
}

DWORD WINAPI CFilterNetReceiver::doSampleBufferProc(void * pParam)
{
	CoInitialize(NULL);
	CFilterNetReceiver * pAdmin = (CFilterNetReceiver *) pParam;
	if (pAdmin != NULL)
	{
		Sleep(100);
		pAdmin->doSampleBuffer(true);
	}
	CoUninitialize();
	return 1;
}

void CFilterNetReceiver::doSampleBuffer(bool bisVideo)
{
	int nPauseSize = 0;
	m_bIsStoped = false;

	//DWORD ret = WaitForSingleObject(this->m_hToSetMediaType, INFINITE);
	HANDLE arrWaitForEvents[2];
	arrWaitForEvents[0] = m_hToSetMediaType;
	arrWaitForEvents[1] = m_hKilledOwner;
	DWORD ret = WaitForMultipleObjects(2, arrWaitForEvents, FALSE, INFINITE);
	if (ret == WAIT_OBJECT_0)
	{
		if (mOutPin && m_configData.get() != 0)
		{
			mOutPin->SetupMediaType(m_configData);			
		}

		if (m_hOnDoSampleBuffer && m_configData.get() != 0)
			m_hOnDoSampleBuffer->onAfterSetMediaType(m_configData->getIsVideo());
	}else if (ret == WAIT_OBJECT_0+1)
	{
		return;
	}

	int retOnLastSampleTime = 0;
	while(!m_bIsStoped)
	{
		CAVData::pointer pData;
		if (!m_listReceived.front(pData))
		{
			Sleep(10);
			continue;
		}
		long nReceovedSize = m_listReceived.size();

		//DeliverBuffer(pData->data(), pData->len(), pData->presentationTime());
		// {0x00,0x00,0x01,0xb6,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF};
		if ((pData->size() == 10)
			&&(pData->data()[0] == 0x00 )&&(pData->data()[1] == 0x00 )&&(pData->data()[2] == 0x01 )
			&&(pData->data()[3] == 0xb6 )&&(pData->data()[4] == 0xFF )&&(pData->data()[5] == 0xFF )
			&&(pData->data()[6] == 0xFF )&&(pData->data()[7] == 0xFF )&&(pData->data()[8] == 0xFF )
			&&(pData->data()[9] == 0xFF ))
		{
			mLastSampleTimeV += m_configData->getFrequency();
		}else{
			DeliverBuffer(pData->data(), pData->size(), pData->timestamp());
		}

		if (m_bIsStoped) break;


		if (m_hOnDoSampleBuffer)
		{
			///////////////////////////////////////////
			// == 0.0 实时流
			if (m_configData->getDuration() == 0.0)
			{
				m_hOnDoSampleBuffer->onIsOverflow(m_configData->getIsVideo(), nReceovedSize);
			//}
			//{
				/////////////////////////////////////
				// 检查视频跟音频的最后时间差
				// 如果 == -1：视频延后，加上一帧的时间
				// 如果 == 1：视频超前，减少一帧的时间
				retOnLastSampleTime = m_hOnDoSampleBuffer->onLastSampleTime(m_configData->getIsVideo(),
					m_configData->getIsVideo() ? mLastSampleTimeV : mLastSampleTimeA);

				if (m_configData->getIsVideo())
				{
					if (retOnLastSampleTime == 1)
						mLastSampleTimeV -= m_configData->getFrequency();
					else if (retOnLastSampleTime == -1)
						mLastSampleTimeV += m_configData->getFrequency();
				}
			}
		}

	}
}
