//
// CDXGraph.h
//

#ifndef __H_CDXGraph__
#define __H_CDXGraph__

// Filter graph notification to the specified window
#define WM_GRAPHNOTIFY  (WM_USER+20)

// ???for test
//#define USE_VMRMode_Windowless 1

#include <streams.h>
//#include "CMsgStation.h"

class CDXGraph// : public CMsgStation
{
private:
	IGraphBuilder *     mGraph;
	IMediaControl *		mMediaControl;
	IMediaEventEx *		mEvent;
	IBasicVideo *		mBasicVideo;
	IBasicAudio *		mBasicAudio;
	IVideoWindow  *		mVideoWindow;
	IMediaSeeking *		mSeeking;
	IVMRWindowlessControl * m_pWc;

	DWORD				mObjectTableEntry;

	// For fast reference!
	enum {GS_Stopped, GS_Paused, GS_Running} mGraphState;                

public:
	CDXGraph();
	virtual ~CDXGraph();

#ifdef USE_VMRMode_Windowless
	HRESULT InitWindowlessVMR(HWND hwndApp);
	IVMRWindowlessControl * GetVMRWc(void);
#endif

public:
	virtual BOOL Create(void);
	virtual void Release(void);
	virtual BOOL Attach(IGraphBuilder * inGraphBuilder);

	IGraphBuilder * GetGraph(void); // Not outstanding reference count
	BOOL AddFilter(IBaseFilter * inFilter, LPCWSTR inName);

	BOOL ConnectFilters(IPin * inOutputPin, IPin * inInputPin, const AM_MEDIA_TYPE * inMediaType = 0);
	void DisconnectFilters(IPin * inOutputPin);

	BOOL SetDisplayWindow(HWND inWindow);
	BOOL SetNotifyWindow(HWND inWindow);
	BOOL ResizeVideoWindow(long inLeft, long inTop, long inWidth, long inHeight);
	void moveWindow(void);
	void HandleEvent(WPARAM inWParam, LPARAM inLParam);
	void NotifyOwnerMessage(HWND hWnd, long uMsg,LONG_PTR wParam,LONG_PTR lParam);

	BOOL Run(void);        // Control filter graph
	BOOL Stop(void);
	BOOL Pause(void);
	BOOL IsRunning(void);  // Filter graph status
	BOOL IsStopped(void);
	BOOL IsPaused(void);

	bool SetFullScreen(bool inEnabled);
	bool GetFullScreen(void) const;

	BOOL SetSyncSource(IBaseFilter * inFilter);
	BOOL SetDefaultSyncSource(void);

	// IMediaSeeking
	BOOL GetCurrentPosition(double * outPosition);
	BOOL GetStopPosition(double * outPosition);
	BOOL SetCurrentPosition(double inPosition);
	BOOL SetStartStopPosition(double inStart, double inStop);
	BOOL GetDuration(double * outDuration);
	BOOL SetPlaybackRate(double inRate);

	// Attention: range from -10000 to 0, and 0 is FULL_VOLUME.
	bool SetAudioVolume(long inVolume);
	long GetAudioVolume(void) const;
	// Attention: range from -10000(left) to 10000(right), and 0 is both.
	bool SetAudioBalance(long inBalance);
	long GetAudioBalance(void) const;

	// When using VMR, it's reliable!
	BOOL GetCurrentImage(const char * inFile);

private:
	void AddToObjectTable(void) ;
	void RemoveFromObjectTable(void);
	
	BOOL QueryInterfaces(void);
};

#endif // __H_CDXGraph__