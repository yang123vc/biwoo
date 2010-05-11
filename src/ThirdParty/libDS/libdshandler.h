// libdshandler.h file here
#ifndef __libdshandler_h__
#define __libdshandler_h__

#include "AVData.h"

class OnDSHandler
{
public:
	virtual void receiveAVData(CAVData::pointer receiveData) = 0;
	//virtual 
};

class DoDSHandler
{
public:
	////////////////////////////////
	// handler
	virtual void SetOnDSHandler(OnDSHandler * handler) = 0;
	virtual void ClearOnDSHandler(void) = 0;

	////////////////////////////////
	// control
	virtual void PlayLocal(void) = 0;
	virtual void PauseLocal(void) = 0;
	virtual void StopLocal(void) = 0;
	virtual void PlayRemote(void) = 0;
	virtual void PauseRemote(void) = 0;
	virtual void StopRemote(void) = 0;
	virtual void StopAll(void) = 0;
	virtual void RemoteVideoReversal(void) = 0;

	///////////////////////////////
	// AUDIO
	// VOLUME: range from -10000 to 0, and 0 is FULL_VOLUME.
	virtual bool SetRemoteAudioVolume(long volume) = 0;
	virtual long GetRemoteAudioVolume(void) const = 0;
	// BALANCE: range from -10000(left) to 10000(right), and 0 is both.
	virtual bool SetRemoteAudioBalance(long balance) = 0;
	virtual long GetRemoteAudioBalance(void) const = 0;

	/////////////////////////////////////////////
	// Full Screen
	virtual bool GetRemoteFullScreen(void) const = 0;
	virtual bool SetRemoteFullScreen(bool enable) = 0;
	virtual bool GetLocalFullScreen(void) const = 0;
	virtual bool SetLocalFullScreen(bool enable) = 0;
	virtual void MoveWindow(void) = 0;

	virtual void ReceiveRemoteData(const unsigned char * data, long size, unsigned int timestamp, bool is_video) = 0;

};

#endif // __libdshandler_h__
