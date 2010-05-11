//
// CNetOutPin.h
//

#ifndef __H_CNetOutPin__
#define __H_CNetOutPin__
#include "../AVConfigData.h"
#include "../AVCoding.h"

class CFilterNetReceiver;
class CNetOutPin : public CBaseOutputPin
{
private:
	CFilterNetReceiver *	mFilter;
	CMediaType				mPreferredMt;

public:
	CNetOutPin(CFilterNetReceiver *inFilter, HRESULT *phr, LPCWSTR pName);
	~CNetOutPin();

	void SetupMediaType(CAVConfigData::pointer configData);
	//void SetCoding(AVCodingType avc) {m_avcoding = avc;}

	// check if the pin can support this specific proposed type and format
	virtual HRESULT CheckMediaType(const CMediaType *);
	virtual HRESULT DecideBufferSize(IMemAllocator * pAlloc, ALLOCATOR_PROPERTIES * pprop);
	// returns the preferred formats for a pin
	HRESULT GetMediaType(int iPosition,CMediaType *pMediaType);

    //================================================================================
    // IQualityControl methods
    //================================================================================

	// 解决ffdshow弹出对话框
    STDMETHODIMP Notify(IBaseFilter * pSender, Quality q) {return E_NOTIMPL;}
	//AVCodingType m_avcoding;

};

#endif // __H_CNetOutPin__