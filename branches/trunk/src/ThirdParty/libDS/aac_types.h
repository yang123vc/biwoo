//-----------------------------------------------------------------------------
//
//	Monogram AAC Encoder
//
//	Base on GNU LGPL libfaac
//
//	Author : Igor Janos
//
//-----------------------------------------------------------------------------


#ifndef _aac_types_h_FILE_INCLUDED__
#define _aac_types_h_FILE_INCLUDED__

// {88F36DB6-D898-40b5-B409-466A0EECC26A}
static const GUID CLSID_MonogramAACEncoder = 
{ 0x88f36db6, 0xd898, 0x40b5, { 0xb4, 0x9, 0x46, 0x6a, 0xe, 0xec, 0xc2, 0x6a } };

// {353F6225-5DF8-4016-8C82-E435FC33C9FF}
static const GUID CLSID_MonogramAACEncoderPropertypage = 
{ 0x353f6225, 0x5df8, 0x4016, { 0x8c, 0x82, 0xe4, 0x35, 0xfc, 0x33, 0xc9, 0xff } };

// {000000FF-0000-0010-8000-00AA00389B71}
static const GUID MEDIASUBTYPE_AAC = 
{ 0x000000FF, 0x0000, 0x0010, { 0x80, 0x00, 0x00, 0xaa, 0x00, 0x38, 0x9b, 0x71 } };

// {B8CDACAE-7D39-4673-A80D-FFDD1D1B3DAF}
static const GUID IID_IMonogramAACEncoderPropertyPage = 
{ 0xb8cdacae, 0x7d39, 0x4673, { 0xa8, 0xd, 0xff, 0xdd, 0x1d, 0x1b, 0x3d, 0xaf } };

// {4289BDC9-571F-41dc-B508-86B512B9F90C}
static const GUID IID_IMonogramAACEncoder = 
{ 0x4289bdc9, 0x571f, 0x41dc, { 0xb5, 0x8, 0x86, 0xb5, 0x12, 0xb9, 0xf9, 0xc } };



#define AAC_VERSION_MPEG4		0
#define AAC_VERSION_MPEG2		1
	
#define AAC_OBJECT_MAIN			1
#define AAC_OBJECT_LOW			2
#define AAC_OBJECT_SSR			3
#define AAC_OBJECT_LTP			4

#define AAC_OUTPUT_RAW			0
#define AAC_OUTPUT_ADTS			1
#define AAC_OUTPUT_LATM			2

struct AACConfig
{
	int			version;
	int			object_type;
	int			output_type;
	int			bitrate;
};

struct AACInfo
{
	int			samplerate;
	int			channels;
	int			frame_size;
	__int64		frames_done;
};

DECLARE_INTERFACE_(IMonogramAACEncoderPropertyPage, IUnknown)
{
	STDMETHOD(Update)(int info_only) PURE;
};

DECLARE_INTERFACE_(IMonogramAACEncoder, IUnknown)
{
	STDMETHOD(GetConfig)(AACConfig *config) PURE;
	STDMETHOD(SetConfig)(AACConfig *config) PURE;
	STDMETHOD(GetInfo)(AACInfo *info) PURE;
	STDMETHOD(SetPropertyPage)(IUnknown *pUnk) PURE;
};

static const int AACFrequency[16] =
{
	96000, 88200, 64000, 48000, 44100, 32000, 24000,
	22050, 16000, 12000, 11025, 8000, 7350, 0, 0, 0
};
static const int AACChannels[16] =
{
	0, 1, 2, 3, 4, 5, 6, 8, 0, 0, 0, 0, 0, 0, 0, 0
};

static bool getAACExtraBytes(BYTE & outByte1, BYTE & outByte2, int nChannels, int nFrequency, int nAudioObjectType = 1)
{
	int chChanIndex = 2;					// 2
	int chFreqIndex = 4;					// 44100
	int AudioObjectType = nAudioObjectType;	// ?Ӧ����Ĭ�϶���
	bool nFinded = false;

	int i=0;
	for (i=0; i<16; i++)
	{
		if (AACChannels[i] == nChannels)
		{
			chChanIndex = i;
			nFinded = true;
			break;
		}
	}
	if (!nFinded) return false;

	nFinded = false;
	for (i=0; i<16; i++)
	{
		if (AACFrequency[i] == nFrequency)
		{
			chFreqIndex = i;
			nFinded = true;
			break;
		}
	}
	if (!nFinded) return false;

	outByte1 = (AudioObjectType<<3) & 0xf8;
	outByte1 |= (chFreqIndex>>1) & 0x07;
	outByte2 = (chFreqIndex<<7) & 0x80;
	outByte2 |= (chChanIndex<<3) & 0x78;

	return true;
}

#endif // _aac_types_h_FILE_INCLUDED__
