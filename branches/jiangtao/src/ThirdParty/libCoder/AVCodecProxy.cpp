#include "AVCodecProxy.h"

#ifdef WIN32
#pragma comment(lib, "avcodec.lib") 
//#pragma comment(lib, "avdevice.lib") 
#pragma comment(lib, "avformat.lib") 
#pragma comment(lib, "avutil.lib") 
#pragma comment(lib, "swscale.lib") 

#include <windows.h>
#endif


static int bit_buffer_size= 1024*256;

CAVCodecProxy::CAVCodecProxy(void)
: pFormatCtx(NULL)
, m_pVideoCodecCtxEnc(NULL)
, pVideoCodecCtx(NULL)
, pAudioCodecCtx(NULL)
, pVideoCodec(NULL)
, pAudioCodec(NULL)
, videoStream(-1)
, audioStream(-1)
, m_handler(NULL)
, m_pBufferDec(NULL)
, m_pBufferEnc(NULL)
, m_pFrameRGB(NULL)
, m_pFrameDecode(NULL)
, m_pFrameEncode(NULL)
, m_swsContextDecode(NULL)
, m_swsContextEncode(NULL)
, m_bFindKeyFrame(false)

{
	m_samples = (unsigned char*)av_malloc(AVCODEC_MAX_AUDIO_FRAME_SIZE * 2);
	m_bit_buffer = (unsigned char*)av_malloc(bit_buffer_size);
	av_init_packet(&avPacket);

}

CAVCodecProxy::~CAVCodecProxy(void)
{
	closeav();
	av_free(m_samples);
	av_free(m_bit_buffer);
}

//获取下一帧
bool CAVCodecProxy::GetNextFrame(AVFormatContext *pFormatCtx, 
										AVCodecContext *pCodecCtx,
										int videoStream, 
										AVFrame *pFrame)
{
	static AVPacket packet;                    //AV包。静态变量。
	static int      bytesRemaining=0;          //字节剩余。静态变量。
	static bool     fFirstTime=true;           //标志，第一次；。静态变量。
	int             bytesDecoded;              //解码后获得的字节；
	int             frameFinished;             //帧解码完毕标志；

	// First time we're called, set packet.data to NULL to indicate it
	// doesn't have to be freed 当第一次被调用的时候，把packet.data设置为NULL,以表示
	//它没有必要被释放；
	if (fFirstTime){
		fFirstTime = false;
		packet.data = NULL;
	}

	// Decode packets until we have decoded a complete frame
	while (true)
	{
		//Work on the current packet until we have decoded all of it
		while (bytesRemaining > 0)
		{
			// Decode the next chunk of data
			bytesDecoded = avcodec_decode_video(pCodecCtx, pFrame, &frameFinished, packet.data, packet.size);
			//bytesDecoded = avcodec_decode_video2(pCodecCtx, pFrame, &frameFinished, &packet);
			

			// Was there an error?
			if (bytesDecoded < 0){
				fprintf(stderr, "Error while decoding frame\n");
				return false;
			}

			bytesRemaining -= bytesDecoded;
		//	rawData += bytesDecoded;

			// Did we finish the current frame? Then we can return
			if (frameFinished)
				return true;
		}

		// Read the next packet, skipping all packets that aren't for this
		// stream
		do{
			// Free old packet  释放旧包
			if(packet.data != NULL)
				av_free_packet(&packet);

			// Read new packet  读取新包
			if(av_read_frame(pFormatCtx, &packet) < 0)
				goto loop_exit;
		} while(packet.stream_index != videoStream);  //当不是要找的视频流的时候，继续循环，就是重新读了；
		//直到找到要找的视频流，退出循环；

		bytesRemaining = packet.size; //纪录包的字节数；
	}

loop_exit:

	// Decode the rest of the last frame
	//bytesDecoded = avcodec_decode_video(pCodecCtx, pFrame, &frameFinished, rawData, bytesRemaining);
	//bytesDecoded = avcodec_decode_video2(pCodecCtx, pFrame, &frameFinished, &packet);
	bytesDecoded = avcodec_decode_video(pCodecCtx, pFrame, &frameFinished, packet.data, packet.size);

	// Free last packet
	if(packet.data != NULL)
		av_free_packet(&packet);

	return frameFinished != 0;
}

long  CAVCodecProxy::openfile(char *filename,long *out_width ,
							  long *out_height,long *out_framenum,
							  long *out_bufsize)
{
	// Register all formats and codecs
	av_register_all();

	// Open video file
	if (av_open_input_file(&pFormatCtx, filename, NULL, 0, NULL) != 0)  
		return -1; // Couldn't open file

	// Retrieve stream information           
	if (av_find_stream_info(pFormatCtx) < 0)
		return -1; // Couldn't find stream information

	// Dump information about file onto standard error
	dump_format(pFormatCtx, 0, filename, false);

//	t = clock();

	// Find the first video stream  
	videoStream = -1;
	audioStream = -1;
	int i=0;
	for (i=0; i<pFormatCtx->nb_streams; i++)
	{
		if (pFormatCtx->streams[i]->codec->codec_type == CODEC_TYPE_VIDEO)
		{
			videoStream=i;
		}else if (pFormatCtx->streams[i]->codec->codec_type == CODEC_TYPE_AUDIO)
		{
			audioStream=i;
		}
	}
	if (audioStream != -1)
	{
		// Get a pointer to the codec context for the audio stream  
		pAudioCodecCtx = pFormatCtx->streams[audioStream]->codec;
		avcodec_alloc_context();

		// Find the decoder for the video stream
		pAudioCodec = avcodec_find_decoder(pAudioCodecCtx->codec_id); 

		if (pAudioCodec == NULL)
			return -1; // Codec not found

		// ??
		// Inform the codec that we can handle truncated bitstreams -- i.e.,    
		// bitstreams where frame boundaries can fall in the middle of packets 
		if ( pAudioCodec->capabilities & CODEC_CAP_TRUNCATED )
			pAudioCodecCtx->flags|=CODEC_FLAG_TRUNCATED;


		// Open codec //打开解码器
		if ( avcodec_open(pAudioCodecCtx, pAudioCodec) < 0 )
			return -1; // Could not open codec 不能打开解码器，返回-1；

	}
	if (videoStream != -1)
	{

		//获取该视频流的一个编解码器上下文的指针；
		// Get a pointer to the codec context for the video stream  
		pVideoCodecCtx = pFormatCtx->streams[videoStream]->codec;

		// Find the decoder for the video stream 获取解码器
		pVideoCodec = avcodec_find_decoder(pVideoCodecCtx->codec_id); 

		if (pVideoCodec == NULL)
			return -1; // Codec not found解码器没有找到；

		//告知解码器，我们能处理被删节的位流
		// 也就是说，帧的分界处的位流可以落到包的中间；
		// Inform the codec that we can handle truncated bitstreams -- i.e.,    
		// bitstreams where frame boundaries can fall in the middle of packets 
		if ( pVideoCodec->capabilities & CODEC_CAP_TRUNCATED )
			pVideoCodecCtx->flags|=CODEC_FLAG_TRUNCATED;

		// Open codec //打开解码器
		if ( avcodec_open(pVideoCodecCtx, pVideoCodec) < 0 )
			return -1; // Could not open codec 不能打开解码器，返回-1；

		// Allocate video frame   分配视频帧
		m_pFrameDecode = avcodec_alloc_frame();

		// Allocate an AVFrame structure     分配一个AVFrame结构
		m_pFrameRGB=avcodec_alloc_frame();     //解码后的帧
		if(m_pFrameRGB == NULL)
			return -1;        

		//决定需要多大的缓冲空间，并且分配空间；
		// Determine required buffer size and allocate buffer   
		//int numBytes=avpicture_get_size(DEST_FORMAT, pVideoCodecCtx->width, pVideoCodecCtx->height);
		int numBytes = m_decFvinfo.framesize();
		m_pBufferDec = (uint8_t*)malloc(numBytes);

		//向外界输出宽高、帧数；
		*out_width = pVideoCodecCtx->width;
		*out_height = pVideoCodecCtx->height;
		*out_framenum = pVideoCodecCtx->frame_number;
		*out_bufsize = numBytes;

		// Assign appropriate parts of buffer to image planes in pFrameRGB  
		//把缓冲区中合适的部分指派到pFrameRGB中的图像面板
		avpicture_fill((AVPicture *)m_pFrameRGB, m_pBufferDec, m_decFvinfo.csp(), m_decFvinfo.width(), m_decFvinfo.height());
	}

	return 0;
}

bool CAVCodecProxy::startread(void)
{
	static AVPacket packet;                    //AV包。静态变量。
	static int      bytesRemaining=0;          //字节剩余。静态变量。
	static uint8_t  *rawData;                  //原始数据字节数。静态变量。
	int             bytesDecoded;              //解码后获得的字节；
	int             frameFinished;             //帧解码完毕标志；

	bool result = false;
	while (av_read_frame(pFormatCtx, &packet) >= 0)
	{
		bytesRemaining = packet.size; //纪录包的字节数；
		rawData = packet.data;        //

		AVCodecContext * pCodecCtx = packet.stream_index == videoStream ? pVideoCodecCtx : (packet.stream_index == audioStream ? pAudioCodecCtx : NULL);
		//Work on the current packet until we have decoded all of it
		while (bytesRemaining > 0)
		{
			// Decode the next chunk of data 解码出下一个数据块
			if (packet.stream_index == videoStream)
				bytesDecoded = avcodec_decode_video(pVideoCodecCtx, m_pFrameDecode, &frameFinished, rawData, bytesRemaining);
			else if (packet.stream_index == audioStream)
				bytesDecoded = avcodec_decode_audio2(pAudioCodecCtx, pAutionSample, &frameFinished, rawData, bytesRemaining);
			else
				break;

			// Was there an error?
			if (bytesDecoded < 0){
				fprintf(stderr, "Error while decoding frame\n");
				return false;
			}

			bytesRemaining -= bytesDecoded;
			rawData += bytesDecoded;

			// Did we finish the current frame? Then we can return
			if (frameFinished)
			{
				if (m_handler)
				{
					if (packet.stream_index == videoStream)
					{
						m_handler->onReadVideo(*m_pFrameDecode);
					}else if (packet.stream_index == audioStream)
					{
						m_handler->onReadAudio(pAutionSample);
					}
				}
			}
		}

		result = true;
	}

	return result;
}

void CAVCodecProxy::closeav(void)
{
	//calculate decode rate 计算解码速率
//	t = clock() - t;
//	fps = (double)(t) / CLOCKS_PER_SEC;
//	fps = i_frame / fps;
//	printf("\n==>Decode rate %.4f fps!\n", fps);

	// Free the YUV image  释放yuv图像
	m_bFindKeyFrame = false;

	if (m_pBufferDec)
	{
		delete[] m_pBufferDec;
		m_pBufferDec = NULL;
	}
	if (m_pBufferEnc)
	{
		delete[] m_pBufferEnc;
		m_pBufferEnc = 0;
	}

	if (m_pFrameRGB)
	{
		av_free(m_pFrameRGB);
		m_pFrameRGB = NULL;
	}

	// Free the YUV frame  释放yuv帧
	if (m_pFrameDecode)
	{
		av_free(m_pFrameDecode);
		m_pFrameDecode = NULL;
	}
	if (m_pFrameEncode)
	{
		av_free(m_pFrameEncode);
		m_pFrameEncode = NULL;
	}
	// Close the codec
	if (pAudioCodecCtx)
	{
		avcodec_close(pAudioCodecCtx);
		pAudioCodecCtx = NULL;
	}
	if (pVideoCodecCtx)
	{
		avcodec_close(pVideoCodecCtx);
		pVideoCodecCtx = NULL;
	}
	if (m_pVideoCodecCtxEnc)
	{
		avcodec_close(m_pVideoCodecCtxEnc);
		m_pVideoCodecCtxEnc = NULL;
	}
	// Close the video file
	if (pFormatCtx)
	{
		av_close_input_file(pFormatCtx);
		pFormatCtx = NULL;
	}

	if (m_swsContextDecode)
	{
		sws_freeContext(m_swsContextDecode);
		m_swsContextDecode = NULL;
	}
	if (m_swsContextEncode)
	{
		sws_freeContext(m_swsContextEncode);
		m_swsContextEncode = NULL;
	}
}

//创建一个bmp文件。用于调试
static int av_create_bmp(char* filename,uint8_t *pRGBBuffer,
						 int width,int height,int bpp)
{
	BITMAPFILEHEADER bmpheader;
	BITMAPINFO bmpinfo;
	FILE *fp;

	fp = fopen(filename,"wb");
	if(!fp)return -1;

	bmpheader.bfType = ('M'<<8)|'B';
	bmpheader.bfReserved1 = 0;
	bmpheader.bfReserved2 = 0;
	bmpheader.bfOffBits = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);
	bmpheader.bfSize = bmpheader.bfOffBits + width*height*bpp/8;

	bmpinfo.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	bmpinfo.bmiHeader.biWidth = width;
	bmpinfo.bmiHeader.biHeight = height;
	bmpinfo.bmiHeader.biPlanes = 1;
	bmpinfo.bmiHeader.biBitCount = bpp;
	bmpinfo.bmiHeader.biCompression = BI_RGB;
	bmpinfo.bmiHeader.biSizeImage = 0;
	bmpinfo.bmiHeader.biXPelsPerMeter = 100;
	bmpinfo.bmiHeader.biYPelsPerMeter = 100;
	bmpinfo.bmiHeader.biClrUsed = 0;
	bmpinfo.bmiHeader.biClrImportant = 0;

	fwrite(&bmpheader,sizeof(BITMAPFILEHEADER),1,fp);
	fwrite(&bmpinfo.bmiHeader,sizeof(BITMAPINFOHEADER),1,fp);
	fwrite(pRGBBuffer,width*height*bpp/8,1,fp);
	fclose(fp);

	return 0;
}

long CAVCodecProxy::GetNextFrame(unsigned char *lpdata)
{

	// Read frames  读取个个帧
	if (GetNextFrame(pFormatCtx, pVideoCodecCtx, videoStream, m_pFrameDecode))
	{
		//		img_convert((AVPicture *)m_pFrameRGB, DEST_FORMAT, (AVPicture*)pFrame, 
		//			pCodecCtx->pix_fmt, pCodecCtx->width, pCodecCtx->height);
		SwsContext *ctx = sws_getContext(
			pVideoCodecCtx->width,
			pVideoCodecCtx->height,
			pVideoCodecCtx->pix_fmt,
			pVideoCodecCtx->width,
			pVideoCodecCtx->height,
			PIX_FMT_BGR24,
			SWS_BICUBIC, NULL, NULL, NULL);
		sws_scale(ctx,m_pFrameDecode->data,m_pFrameDecode->linesize,0,pVideoCodecCtx->height,m_pFrameRGB->data,m_pFrameRGB->linesize);

		//调试用，向C盘写入一个bmp文件；
		av_create_bmp("D:/WorkRecord/Project/1.bmp",(unsigned char *)m_pFrameRGB->data[0],pVideoCodecCtx->width,pVideoCodecCtx->height,24);

/*		i_frame++;
		y_size = pCodecCtx->width * pCodecCtx->height;                          

		//写入文件
		/*fwrite(m_pFrameRGB->data[0], 1, y_size, fp);
		fwrite(m_pFrameRGB->data[1], 1, (y_size/4), fp);
		fwrite(m_pFrameRGB->data[2], 1, (y_size/4), fp);*/
		//	memcpy(lpdata,m_pFrameRGB->data[0],y_size*3);
		//	UpendBmp(lpdata,pCodecCtx->width,pCodecCtx->height);
		return 0;
	}
	else
	{return -1;}
}

bool CAVCodecProxy::openav(enum CodecID codecID, const unsigned char * extradata, int extrasize)
{
	if (isopenav()) return true;

	// Register all formats and codecs
	avcodec_register_all();
	//avdevice_register_all();
	av_register_all();

	// Find the decoder for the video stream
	AVCodec * pCodec = avcodec_find_decoder(codecID); 
	if (pCodec == NULL)
		return false; // Codec not found

	AVCodecContext * pCodecCtx = avcodec_alloc_context();
	if (pCodecCtx == NULL)
		return false;

	if (extrasize > 0)
	{
		pCodecCtx->extradata_size = extrasize;
		pCodecCtx->extradata = new unsigned char[extrasize+FF_INPUT_BUFFER_PADDING_SIZE];
		memset(pCodecCtx->extradata, 0, extrasize+FF_INPUT_BUFFER_PADDING_SIZE);
		memcpy(pCodecCtx->extradata, extradata, extrasize);
	}

	if (pCodec->type == CODEC_TYPE_AUDIO)
	{
		pAudioCodecCtx = pCodecCtx;
		pAudioCodec = pCodec;
	}else if (pCodec->type == CODEC_TYPE_VIDEO)
	{
		// some codecs need / can use extradata like Huffman tables.
		pVideoCodecCtx = pCodecCtx;
		pVideoCodec = pCodec;

		// ??
		pVideoCodecCtx->width = m_decFvinfo.width();
		pVideoCodecCtx->height = m_decFvinfo.height();

		//// ??
		//m_pVideoCodecCtxEnc = avcodec_alloc_context();
		//m_pVideoCodecCtxEnc->bit_rate = 400000;
		//m_pVideoCodecCtxEnc->width = 320;
		//m_pVideoCodecCtxEnc->height = 240;
		//m_pVideoCodecCtxEnc->sample_aspect_ratio.num = 25;
		//m_pVideoCodecCtxEnc->sample_aspect_ratio.den = 1;
		//m_pVideoCodecCtxEnc->time_base.num = 25;
		//m_pVideoCodecCtxEnc->time_base.den = 1;
		//m_pVideoCodecCtxEnc->pix_fmt = PIX_FMT_YUV420P;
		//m_pVideoCodecCtxEnc->gop_size = 240;
		//// OK
		//m_pVideoCodecCtxEnc->qmin = 10;
		//m_pVideoCodecCtxEnc->qmax = 51;
		//m_pVideoCodecCtxEnc->max_qdiff = 4;
		//m_pVideoCodecCtxEnc->me_range = 16;
		//m_pVideoCodecCtxEnc->keyint_min = 25;
		//m_pVideoCodecCtxEnc->qcompress = 0.6;
		//m_pVideoCodecCtxEnc->me_method = ME_FULL;

		////m_pVideoCodecCtxEnc->max_b_frames = 2;
		////if(!strcmp(pFormatCtxEnc->oformat->name, "mp4") || !strcmp(pFormatCtxEnc->oformat->name, "mov") || !strcmp(pFormatCtxEnc->oformat->name, "3gp"))
		//	//pVideoCodecCtx->flags |= CODEC_FLAG_GLOBAL_HEADER;

		//if ( pCodec->capabilities & CODEC_CAP_TRUNCATED )
		//	m_pVideoCodecCtxEnc->flags|=CODEC_FLAG_TRUNCATED;

		//// Open codec //打开解码器
		//if ( avcodec_open(m_pVideoCodecCtxEnc, pCodec) < 0 )
		//	return false; // Could not open codec 不能打开解码器，返回-1；

	}

	// ??
	// Inform the codec that we can handle truncated bitstreams -- i.e.,    
	// bitstreams where frame boundaries can fall in the middle of packets 
	if ( pCodec->capabilities & CODEC_CAP_TRUNCATED )
		pCodecCtx->flags|=CODEC_FLAG_TRUNCATED;


	// Open codec //打开解码器
	if ( avcodec_open(pCodecCtx, pCodec) < 0 )
		return false; // Could not open codec 不能打开解码器，返回-1；

	if (pCodec == pVideoCodec)
	{
		// Allocate video frame   分配视频帧
		m_pFrameDecode = avcodec_alloc_frame();
		if(m_pFrameDecode == NULL)
			return false;        

		m_pFrameEncode = avcodec_alloc_frame();
		if(m_pFrameEncode == NULL)
			return false;        

		// Allocate an AVFrame structure     分配一个AVFrame结构
		m_pFrameRGB=avcodec_alloc_frame();     //解码后的帧
		if(m_pFrameRGB == NULL)
			return false;        
	}

	return true;
}

bool CAVCodecProxy::isopenav(void) const
{
	return (pVideoCodecCtx != NULL || pAudioCodecCtx != NULL);
}

bool CAVCodecProxy::decode(const unsigned char * inBuffer, int inLength, unsigned char ** outBuffer, int & outSize, bool reversal)
{
	int      bytesRemaining = inLength;		//字节剩余。静态变量。
	int      bytesDecoded;					//解码后获得的字节；
	int      frameFinished=0;				//帧解码完毕标志；

//	bytesRemaining += (4-bytesRemaining % 4);
//	inLength = bytesRemaining;

	av_init_packet(&avPacket);
	/*unsigned char * pBufferTemp = new unsigned char[inLength+FF_INPUT_BUFFER_PADDING_SIZE];
	memset(pBufferTemp, 0, inLength+FF_INPUT_BUFFER_PADDING_SIZE);
	memcpy(pBufferTemp, inBuffer, inLength);
	avPacket.data = pBufferTemp;*/
	avPacket.data = const_cast<unsigned char*>(inBuffer);
	avPacket.size = inLength;

	//Work on the current packet until we have decoded all of it
	while (bytesRemaining >= 0)
	{
		//FF_INPUT_BUFFER_PADDING_SIZE
		
		// Decode the next chunk of data
		if (pVideoCodecCtx)
		{
			//frameFinished = 1;
			bytesDecoded = avcodec_decode_video2(pVideoCodecCtx, m_pFrameDecode, &frameFinished, &avPacket);
			//bytesDecoded = avcodec_decode_video(pVideoCodecCtx, m_pFrameDecode, &frameFinished, inBuffer, inLength);
		}else if (pAudioCodecCtx)
		{
			frameFinished = AVCODEC_MAX_AUDIO_FRAME_SIZE * 2;
			bytesDecoded = avcodec_decode_audio3(pAudioCodecCtx, (int16_t *)m_samples, &frameFinished, &avPacket);
			//bytesDecoded = avcodec_decode_audio2(pAudioCodecCtx, (int16_t *)m_samples, &frameFinished, inBuffer, inLength);
		}else
		{
			//delete[] pBufferTemp;
			return false;
		}

		// Was there an error?
		if (bytesDecoded < 0){
			fprintf(stderr, "Error while decoding frame\n");

			avPacket.data = NULL;
			avPacket.size = 0;
			//delete[] pBufferTemp;
			return false;
		}

		bytesRemaining -= bytesDecoded;

		// Did we finish the current frame? Then we can return
		if (frameFinished)
		{
			avPacket.data = NULL;
			avPacket.size = 0;

			//delete[] pBufferTemp;

			if (pAudioCodecCtx)
			{
				outSize = frameFinished;
				*outBuffer = new unsigned char[outSize];
				memcpy(*outBuffer, m_samples, outSize);
			}else if (pVideoCodecCtx)
			{
				if (!m_bFindKeyFrame)
				{
					m_bFindKeyFrame = m_pFrameDecode->key_frame == 1;
					if (!m_bFindKeyFrame)
						return false;
				}
				//m_pFrameEncode->data[0] = m_pFrameDecode->data[0];
				//m_pFrameEncode->data[1] = m_pFrameDecode->data[1];
				//m_pFrameEncode->data[2] = m_pFrameDecode->data[2];
				//m_pFrameEncode->linesize[0] = m_pFrameDecode->linesize[0];
				//m_pFrameEncode->linesize[1] = m_pFrameDecode->linesize[1];
				//m_pFrameEncode->linesize[2] = m_pFrameDecode->linesize[2];
				//
				//// For test
				//int bytesEncoded = avcodec_encode_video(m_pVideoCodecCtxEnc, (uint8_t *)m_bit_buffer, bit_buffer_size/2, m_pFrameEncode);

				if (m_pBufferDec == NULL)
				{
					// Determine required buffer size and allocate buffer   
					int numBytes = m_decFvinfo.framesize();
					m_pBufferDec = new uint8_t [m_decFvinfo.framesize()];

					avpicture_fill((AVPicture *)m_pFrameRGB, m_pBufferDec, m_decFvinfo.csp(), m_decFvinfo.width(), m_decFvinfo.height());
				}

				// For test.
				if (m_swsContextDecode == NULL)
				{
					m_swsContextDecode = sws_getContext(
						pVideoCodecCtx->width,
						pVideoCodecCtx->height,
						pVideoCodecCtx->pix_fmt,
						m_decFvinfo.width(),
						m_decFvinfo.height(),
						m_decFvinfo.csp(),
						SWS_BICUBIC,
						//SWS_POINT,
						NULL, NULL, NULL);
				}

				///////////////////////////////////////////////////
				unsigned char *data[4];
				int linesize[4];
				int i;

				// 移动数组指针，实现图像翻转；
				if (reversal)
				{
					// 先记录数组指针
					for(i = 0; i < 4; i++)
					{
						data[ i ] = m_pFrameDecode->data[ i ];
						linesize[ i ] = m_pFrameDecode->linesize[ i ];
					}

					int height = pVideoCodecCtx->height;
					m_pFrameDecode->data[0] += m_pFrameDecode->linesize[0] * (height - 1);
					m_pFrameDecode->linesize[0] *= -1;                      
					m_pFrameDecode->data[1] += m_pFrameDecode->linesize[1] * (height / 2 - 1);
					m_pFrameDecode->linesize[1] *= -1;
					m_pFrameDecode->data[2] += m_pFrameDecode->linesize[2] * (height / 2 - 1);
					m_pFrameDecode->linesize[2] *= -1;
				}

				// x->RGB32 test
				sws_scale(m_swsContextDecode,m_pFrameDecode->data,m_pFrameDecode->linesize,0,pVideoCodecCtx->height,m_pFrameRGB->data,m_pFrameRGB->linesize);

				// 恢复数组指针，避免发生内存泄漏；
				if (reversal)
				{
					for(i = 0; i < 4; i++)
					{
						m_pFrameDecode->data[ i ] = data[ i ];
						m_pFrameDecode->linesize[ i ] = linesize[ i ]; 
					}
				}

				//outSize = pVideoCodecCtx->width*pVideoCodecCtx->height*DEFAULT_BPP;
				outSize = m_decFvinfo.framesize();
				*outBuffer = new unsigned char[outSize];
				memcpy(*outBuffer, m_pFrameRGB->data[0], outSize);
				//调试用，向硬盘写入一个bmp文件；
				//av_create_bmp("D:/WorkRecord/Project/2.bmp",(unsigned char *)m_pFrameRGB->data[0],pVideoCodecCtx->width,pVideoCodecCtx->height,24);
			}

			return true;
		}
	}
	avPacket.data = NULL;
	avPacket.size = 0;
	//delete[] pBufferTemp;
	return false;
}


bool CAVCodecProxy::encode(const unsigned char * inBuffer, int inLength, unsigned char ** outBuffer, int & outSize)
{
	int      bytesEncoded=0;					// 编码后获得的字节；

	if (pVideoCodecCtx)
	{
		if (m_pBufferEnc == NULL)
		{
			int numBytes=avpicture_get_size(m_pVideoCodecCtxEnc->pix_fmt, m_pVideoCodecCtxEnc->width, m_pVideoCodecCtxEnc->height);
			m_pBufferEnc = new uint8_t[numBytes];

			avpicture_fill((AVPicture *)m_pFrameEncode, m_pBufferEnc, m_pVideoCodecCtxEnc->pix_fmt, m_pVideoCodecCtxEnc->width, m_pVideoCodecCtxEnc->height);
		}

		if (m_swsContextEncode == NULL)
		{
			m_swsContextEncode = sws_getContext(
				m_pVideoCodecCtxEnc->width,
				m_pVideoCodecCtxEnc->height,
				PIX_FMT_YUYV422,
				m_pVideoCodecCtxEnc->width,
				m_pVideoCodecCtxEnc->height,
				m_pVideoCodecCtxEnc->pix_fmt,
				SWS_BICUBIC, NULL, NULL, NULL);
		}

		AVFrame * big_picture = avcodec_alloc_frame();
		big_picture->data[0] = const_cast<unsigned char*>(inBuffer);
		big_picture->linesize[0] = 320;

		sws_scale(m_swsContextEncode,big_picture->data,big_picture->linesize,0,m_pVideoCodecCtxEnc->height,m_pFrameEncode->data,m_pFrameEncode->linesize);

		//pVideoCodecCtx->pix_fmt = PIX_FMT_YUYV422;
		////m_pFrameEncode->data[0] = const_cast<unsigned char*>(inBuffer);
		//if (m_pFrameEncode->data[0] == NULL)
		//	m_pFrameEncode->data[0] = (unsigned char*)malloc(320*240*4);
		//memcpy(m_pFrameEncode->data[0], inBuffer, inLength);
		//m_pFrameEncode->linesize[0] = 320;

		// CODEC_FLAG_TRUNCATED
		//bytesEncoded = avcodec_encode_video(m_pVideoCodecCtxEnc, m_bit_buffer, bit_buffer_size, m_pFrameEncode);
		av_free(big_picture);

	}else if (pAudioCodecCtx)
	{
		/*pAudioCodecCtx->frame_size = pAudioCodecCtx->bit_rate;
		uint8_t * samples = (uint8_t*)av_malloc(AVCODEC_MAX_AUDIO_FRAME_SIZE*2);
		memcpy(samples, inBuffer, inLength); 
		bytesEncoded = avcodec_encode_audio(pAudioCodecCtx, m_bit_buffer, inLength, (const short*)samples);
		av_free(samples);*/

	}else
		return false;

	// Was there an error?
	if (bytesEncoded <= 0){
		fprintf(stderr, "Error while decoding frame\n");
		return false;
	}

	outSize = bytesEncoded;
	*outBuffer = new unsigned char[outSize];
	memcpy(*outBuffer, m_bit_buffer, outSize);
	memset(m_bit_buffer, 0, bit_buffer_size);

	return true;
}
