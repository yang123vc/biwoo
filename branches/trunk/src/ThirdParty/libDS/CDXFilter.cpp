//
// CDXFilter.cpp
//

/*-----------------------------------------------------*\
			HQ Tech, Make Technology Easy!       
 More information, please go to http://hqtech.nease.net.
/*-----------------------------------------------------*/

#include "CDXFilter.h"
#include "UFilterUtils.h"

////////////////////////////////////////////////////////////////////////////////
CDXFilter::CDXFilter(IGraphBuilder * inGraph, GUID inClsid, const char * inName)
{
	mGraph  = inGraph;	
	mClsid  = inClsid;
	mFilter = NULL;

	if (inName)
	{
		MultiByteToWideChar(CP_ACP, 0, inName, -1, mName, 256);
	}
	else
	{
		wcscpy(mName, L"");
	}
}

CDXFilter::~CDXFilter()
{
	ReleaseFilter();
}

BOOL CDXFilter::CreateFilter(void)
{
	if (!mFilter && mGraph)
	{
		if (SUCCEEDED(CoCreateInstance(mClsid, NULL, CLSCTX_INPROC_SERVER,
			IID_IBaseFilter, (void **)&mFilter)))
		{
			if (SUCCEEDED(mGraph->AddFilter(mFilter, mName)))
			{
				return TRUE;
			}
		}
	}
	ReleaseFilter();
	return FALSE;
}

BOOL CDXFilter::Attach(IBaseFilter * inFilter, const char * inName)
{
	ReleaseFilter();
	
	if (inName)
	{
		MultiByteToWideChar(CP_ACP, 0, inName, -1, mName, 256);
	}

	if (mGraph && inFilter)
	{
		if (SUCCEEDED(mGraph->AddFilter(inFilter, mName)))
		{
			mFilter = inFilter;
			mFilter->AddRef();  // Add extra refercence count
			return TRUE;
		}
	}
	return FALSE;
}

void CDXFilter::Detach(void)
{
	if (mFilter)
	{
		mFilter->Release();
		mFilter = NULL;
	}
}

BOOL CDXFilter::QueryInterface(REFIID inIID, LPVOID * outObj)
{
	if (mFilter)
	{
		HRESULT hr = mFilter->QueryInterface(inIID, outObj);
		return SUCCEEDED(hr);
	}
	return FALSE;
}

void CDXFilter::ReleaseFilter(void)
{
	if (mFilter)
	{
		if (mGraph)
		{
			mGraph->RemoveFilter(mFilter);
		}
		mFilter->Release();
		mFilter = NULL;
	}
}

IBaseFilter * CDXFilter::GetFilter(void)
{
	return mFilter;
}

IPin * CDXFilter::GetPin(BOOL inInput, const char * inPartialName)
{
	return UFilterUtils::GetPin(mFilter, inInput, inPartialName);
}

BOOL CDXFilter::GetPinCount(int & outInputPin, int & outOutputPin)
{
	return UFilterUtils::GetPinCount(mFilter, outInputPin, outOutputPin);
}

IAMStreamConfig * CDXFilter::GetStreamConfig(IPin * pPin)
{
	IAMStreamConfig * pConfig = NULL;
	if (mFilter)
	{
		// Get the capture output pin first
		IPin * pCapture = pPin;
		if (pCapture  == NULL)
			pCapture = GetPin(FALSE, "Capture");
		if (pCapture == NULL)
			pCapture = GetPin(FALSE, "捕获");
		if (pCapture)
		{
			pCapture->QueryInterface(IID_IAMStreamConfig, (void **)&pConfig);
		}

		if (pConfig)
		{
			pConfig->Release();
		}
	}
	return pConfig;
}

AM_MEDIA_TYPE * CDXFilter::SelectMediaType(IPin * pPin, SelectMT smt)
{
	// Preferred sequence: UYVY, YUY2, RGB565, RGB555, RGB24, RGB32  
	VIDEO_STREAM_CONFIG_CAPS  pSCC;
	AM_MEDIA_TYPE * pmt = NULL;
	HRESULT hr = S_OK;
	int nCounts, nSize;
	int preferredIndex = -1;
	enum {
		UYVY = 0, YUY2, RGB565, RGB555, RGB24, RGB32, Unknown
	} currentPreferred, temp;
	currentPreferred = Unknown;
	long currentSampleSize = 0xffffff;

	IAMStreamConfig * pCfg = GetStreamConfig(pPin);
	pCfg->GetNumberOfCapabilities(&nCounts, &nSize);
	for (int i = 0; i < nCounts; i++)
	{
		if (pCfg->GetStreamCaps(i, &pmt, (BYTE *)&pSCC) == S_OK)
		{
			if (pmt->subtype == MEDIASUBTYPE_RGB32)
			{
				temp = RGB32;
			}
			else if (pmt->subtype == MEDIASUBTYPE_RGB24)
			{
				temp = RGB24;
			}
			else if (pmt->subtype == MEDIASUBTYPE_RGB565)
			{
				temp = RGB565;
			}
			else if (pmt->subtype == MEDIASUBTYPE_RGB555)
			{
				temp = RGB555;
			}
			else if (pmt->subtype == MEDIASUBTYPE_YUY2)
			{
				temp = YUY2;
			}
			else if (pmt->subtype == MEDIASUBTYPE_UYVY)
			{
				temp = UYVY;
			}
			else
			{
				temp = Unknown;
			}

			// 选择一个帧流数据较少的
			if (smt == SMT_MinSampleSize)
			{
				if (pmt->lSampleSize < currentSampleSize)
				{
					currentSampleSize = pmt->lSampleSize;
					preferredIndex   = i;
				}
			}else if (smt == SMT_MP4V)
			{
				GUID subtype = pmt->subtype;

			}else
			{
				// 默认
				if (temp < currentPreferred)
				{
					currentPreferred = temp;
					preferredIndex   = i;
				}
			}

			DeleteMediaType(pmt);
		}
	}

	// Get the preferred media type
	if (preferredIndex != -1)
	{
		hr = pCfg->GetStreamCaps(preferredIndex, &pmt, (BYTE *)&pSCC);
		if (FAILED(hr))
		{
			hr = pCfg->GetStreamCaps(++preferredIndex, &pmt, (BYTE *)&pSCC);
		}
	}
	else
	{
		hr = pCfg->GetFormat(&pmt);
	}

	return pmt;
}
