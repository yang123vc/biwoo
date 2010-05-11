// AVParameter.h file here
#ifndef __AVParameter_h__
#define __AVParameter_h__

#include "AVCoding.h"

class CAVParameter
{
public:
	enum TimestampType
	{
		TT_Frequency = 1
		, TT_Timestamp = 2
		, TT_Null = 3
	};

	CAVParameter(const CAVParameter & avp)
	{
		equal(avp);
	}
	const CAVParameter & operator = (const CAVParameter & avp)
	{
		equal(avp);
		return *this;
	}

	void preview(void * newv) {m_wndPreivew = newv;}
	void * preview(void) const {return m_wndPreivew;}
	void audio(bool newv) {m_bHasAudio = newv;}
	bool audio(void) const {return m_bHasAudio;}
	void audio(int channel, long frequency=44100) {m_nChannels=channel; m_nAudioFrequency=frequency;}
	int audioChannel(void) const {return m_nChannels;}
	long audioFrequency(void) const {return m_nAudioFrequency;}
	void video(bool newv) {m_bHasVideo = newv;}
	bool video(void) const {return m_bHasVideo;}
	void video(int fps, int width=320, int height=240) {m_nVideoFPS=fps, m_nWidth=width; m_nHeight=height;}
	void reversal(bool newv) {m_bReversal = newv;}
	bool reversal(void) const {return m_bReversal;}
	int videoFPS(void) const {return m_nVideoFPS;}
	long videoFrequency(void) const {return m_nVideoFPS==0 ? 0 : 10000000/m_nVideoFPS;}
	int videoWidth(void) const {return m_nWidth;}
	int videoHeight(void) const {return m_nHeight;}

	void audioCoding(AVCodingType newv) {m_audioCoding = newv;}
	AVCodingType audioCoding(void) const {return m_audioCoding;}
	void videoCoding(AVCodingType newv) {m_videoCoding = newv;}
	AVCodingType videoCoding(void) const {return m_videoCoding;}

	void audiott(TimestampType newv) {m_audiott = newv;}
	TimestampType audiott(void) const {return m_audiott;}
	void videott(TimestampType newv) {m_videott = newv;}
	TimestampType videott(void) const {return m_videott;}

	/*void handler(void * newv) {m_handler = newv;}
	void * handler(void) const {return m_handler;}*/

protected:
	void equal(const CAVParameter & avp)
	{
		this->m_wndPreivew = avp.preview();
		this->m_bHasAudio = avp.audio();
		this->m_bHasVideo = avp.video();
		this->m_bReversal = avp.reversal();
		this->m_nChannels = avp.audioChannel();
		this->m_nAudioFrequency = avp.audioFrequency();
		this->m_nVideoFPS = avp.videoFPS();
		this->m_nWidth = avp.videoWidth();
		this->m_nHeight = avp.videoHeight();
		this->m_audioCoding = avp.audioCoding();
		this->m_videoCoding = avp.videoCoding();
		this->m_audiott = avp.audiott();
		this->m_videott = avp.videott();
	}

public:
	CAVParameter(void)
		: m_wndPreivew(0)
		, m_bHasAudio(true)
		, m_bHasVideo(false), m_bReversal(false)
		, m_nChannels(2), m_nAudioFrequency(44100)
		, m_nVideoFPS(25), m_nWidth(320), m_nHeight(240)
		, m_audioCoding(AVCoding::CT_Other), m_videoCoding(AVCoding::CT_Other)
		, m_audiott(TT_Null), m_videott(TT_Null)
		//, m_handler(0)
	{}
	~CAVParameter(void)
	{}
private:
	void * m_wndPreivew;	// default NULL
	bool m_bHasAudio;	// default true
	bool m_bHasVideo;	// default false
	bool m_bReversal;	// Í¼Ïñ·­×ª default false
	int m_nChannels;		// default 2
	long m_nAudioFrequency;	// default 44100
	// Vido parameters
	int m_nVideoFPS;		// default 25;
	int m_nWidth;		// default 320
	int m_nHeight;		// default 240
	// coding
	AVCodingType m_audioCoding;
	AVCodingType m_videoCoding;

	TimestampType m_audiott;
	TimestampType m_videott;
	//void * m_handler;

};

#endif // __AVParameter_h__
