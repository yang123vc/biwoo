// AVCoding.h file here
#ifndef __AVCoding_h__
#define __AVCoding_h__

namespace AVCoding
{
	enum AVCodingType_
	{
		// audio
		CT_G711A = 0x001
		, CT_G711U = 0x002
		, CT_G729A = 0x003
		, CT_AAC = 0x011		// Decoder ONLY

		// video
		, CT_XVID = 0x101		// MPEG4
		//, CT_H263 = 0x102
		, CT_H264 = 0x103


		, CT_Other = 0xFFF

	};
};

typedef AVCoding::AVCodingType_ AVCodingType;


#endif // __AVCoding_h__
