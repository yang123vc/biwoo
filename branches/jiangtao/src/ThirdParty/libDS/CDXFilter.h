//
// CDXFilter.h
//

#ifndef __H_CDXFilter__
#define __H_CDXFilter__

#include <streams.h>

class CDXFilter  
{
protected:
	IGraphBuilder *		mGraph;
	IBaseFilter *		mFilter;
	GUID				mClsid;
	WCHAR				mName[256];

public:
	CDXFilter(IGraphBuilder * inGraph, GUID inClsid = GUID_NULL, 
		const char * inName = NULL);
	virtual ~CDXFilter(void);

	virtual BOOL CreateFilter(void);
	virtual void ReleaseFilter(void);

	BOOL Attach(IBaseFilter * inFilter, const char * inName = NULL);
	// Simply detach, NOT remove filter from the graph
	void Detach(void);
	BOOL QueryInterface(REFIID inIID, LPVOID * outObj);

	IBaseFilter * GetFilter(void);  // NOT outstanding reference
	
	// Several method to retrieve specified pin when filter connecting
	// NOT outstanding reference
	IPin * GetPin(BOOL inInput, const char * inPartialName = NULL);
	BOOL GetPinCount(int & outInputPin, int & outOutputPin);

	IAMStreamConfig * GetStreamConfig(IPin * pPin);

	enum SelectMT
	{
		SMT_MinSampleSize	= 1,
		SMT_MP4V			= 2,
		SMT_Other			= 0xff
	};
	AM_MEDIA_TYPE * SelectMediaType(IPin * pPin, SelectMT smt);

};

#endif // __H_CDXFilter__
