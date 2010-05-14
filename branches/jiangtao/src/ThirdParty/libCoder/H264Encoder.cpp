#include "H264Encoder.h"
//#include "common/common.h"
#include <memory>
#ifdef WIN32
#ifdef _DEBUG
#pragma comment(lib, "libx264d.lib") 
#else
#pragma comment(lib, "libx264.lib") 
#endif
#endif

CH264Encoder::CH264Encoder(void)
{
	m_Codec = NULL;
	m_width = 0;
	m_height = 0;
	m_sizeImage= 0;

	memset( &m_pic, 0, sizeof( x264_picture_t ) );
}

CH264Encoder::~CH264Encoder(void)
{
	releaseCodec();
}

bool CH264Encoder::initH264Codec(int nWidth, int nHeight, unsigned short usMaxKeyframe, int nVideoQuality,int nTBColorSpace,bool bCaptureIFrame)
{
	x264_param_t param;

	m_width = nWidth;
	m_height = nHeight;
	m_sizeImage = nWidth*nHeight;
	/* Destroy previous handle */
	if (m_Codec != NULL)
	{
		x264_encoder_close(m_Codec);
		m_Codec = NULL;
	}

	/* Get default param */
	x264_param_default( &param );

	param.i_csp = X264_CSP_I420;
	param.rc.psz_stat_out = (char*)malloc (MAX_PATH);
	param.rc.psz_stat_in = (char*)malloc (MAX_PATH);
	param.i_threads = 1;
	param.analyse.i_noise_reduction = 0;

	param.i_log_level = X264_LOG_NONE;
	//param.pf_log = x264_log_vfw;
	param.p_log_private = NULL;

	param.analyse.b_psnr = 0;

	/* Set params: TODO to complete */
	param.i_width = nWidth;
	param.i_height= nHeight;

	param.i_fps_num = usMaxKeyframe;
	param.i_fps_den = 1;
	param.i_frame_total = 0;

	param.i_frame_reference = 1;
	param.i_keyint_min = 25;
	param.i_keyint_max = 250;
	param.i_scenecut_threshold = 40;
	param.rc.i_qp_min = 20;//20;
	param.rc.i_qp_max = 32;
	param.rc.i_qp_step = 1;
	param.b_deblocking_filter = 1;
	param.b_cabac = 1;
	param.analyse.i_trellis = 0;

	param.analyse.b_chroma_me = 1;
	param.rc.f_ip_factor = 1.4;
	param.rc.f_pb_factor = 1.3;
	param.rc.f_qcompress = 1.0;
	param.vui.i_sar_width = 0;
	param.vui.i_sar_height = 0;

	param.i_bframe = 2;
	param.analyse.b_weighted_bipred = 0;
	param.b_bframe_pyramid = 0;
	//param.b_bframe_adaptive = 0;
	//param.analyse.b_bidir_me = 0;
	param.i_bframe_bias = 0;
	param.analyse.i_subpel_refine = 6; /* 0..5 -> 1..6 */
	param.analyse.i_me_method = X264_ME_DIA;//X264_ME_HEX?X264_ME_DIA
	param.analyse.i_me_range = 16;

	param.analyse.i_direct_mv_pred = X264_DIRECT_PRED_AUTO;
	param.i_deblocking_filter_alphac0 = 0;
	param.i_deblocking_filter_beta = 0;
	param.analyse.intra = X264_ANALYSE_I4x4;// 帧内预测模式
	param.analyse.inter = X264_ANALYSE_I4x4 | X264_ANALYSE_PSUB16x16 | X264_ANALYSE_BSUB16x16; // 帧间预测模式
	param.analyse.b_transform_8x8 = 1;

	switch (1)
	{
	case 0: /* 1 PASS ABR */
		param.rc.i_rc_method = X264_RC_ABR;
		param.rc.i_bitrate = nVideoQuality; // max = 5000
		break;
	case 1: /* 1 PASS CQ */
		param.rc.i_rc_method = X264_RC_CQP;
		param.rc.i_qp_constant = 26;//10 - 51
		break;
	}

	/* Open the encoder */
	m_Codec = x264_encoder_open( &param );

	free( param.rc.psz_stat_out );
	free( param.rc.psz_stat_in );

	if (m_Codec == NULL)
		return false;
	if (nTBColorSpace == TBCOLORSPACE_YV12)
		m_pic.img.i_csp = X264_CSP_YV12;
	else if (nTBColorSpace == TBCOLORSPACE_I420)
		m_pic.img.i_csp = X264_CSP_I420;
	else if (nTBColorSpace == TBCOLORSPACE_YUY2)
		m_pic.img.i_csp = X264_CSP_YUYV;
	else if (nTBColorSpace == TBCOLORSPACE_IYUV)
		m_pic.img.i_csp = X264_CSP_I422;
	else if (nTBColorSpace == TBCOLORSPACE_RGB24)
		m_pic.img.i_csp = X264_CSP_BGR;

	if (bCaptureIFrame)
		m_pic.i_qpplus1 = 1;

	return true; 
	
}
int CH264Encoder::Encode(unsigned char* pInputBuffer, int nInputSize,unsigned char* pOutputBuffer, int& nOutputSize, bool& bKeyFrame)
{
	if (m_Codec == NULL)
		return 0;
	int nLen = 0;
	//x264_picture_t pic;
	int i_nal = 0;
	x264_nal_t* nal;
	x264_picture_t pic_out;
	int i = 0;

	/* Init the picture */
	/*
		memset( &pic, 0, sizeof( x264_picture_t ) );
			if (nTBColorSpace == TBCOLORSPACE_YV12)
				pic.img.i_csp = X264_CSP_YV12;
			else if (nTBColorSpace == TBCOLORSPACE_I420)
				pic.img.i_csp = X264_CSP_I420;
			else if (nTBColorSpace == TBCOLORSPACE_YUY2)
				pic.img.i_csp = X264_CSP_YUYV;
			else if (nTBColorSpace == TBCOLORSPACE_IYUV)
				pic.img.i_csp = X264_CSP_I422;
			else if (nTBColorSpace == TBCOLORSPACE_RGB24)
				pic.img.i_csp = X264_CSP_RGB;
		
			if (bCaptureIFrame)
				pic.i_qpplus1 = 1;*/
		
	switch( m_pic.img.i_csp & X264_CSP_MASK )
	{
	case X264_CSP_I420:
	case X264_CSP_YV12://?
		m_pic.img.i_plane = 3;
		m_pic.img.i_stride[0] = m_width;
		m_pic.img.i_stride[1] =
			m_pic.img.i_stride[2] = m_width / 2;

		m_pic.img.plane[0]    = (uint8_t*)pInputBuffer;
		m_pic.img.plane[1]    = m_pic.img.plane[0] + m_width * m_height;
		m_pic.img.plane[2]    = m_pic.img.plane[1] + m_width * m_height / 4;
		break;

	case X264_CSP_YUYV:
		m_pic.img.i_plane = 1;
		m_pic.img.i_stride[0] = 2 * m_width;
		m_pic.img.plane[0]    = (uint8_t*)pInputBuffer;
		break;
	case X264_CSP_RGB:
		m_pic.img.i_plane = 1;
		m_pic.img.i_stride[0] = 3 * m_width;
		m_pic.img.plane[0]    = (uint8_t*)pInputBuffer;
		break;
	case X264_CSP_BGR:
		m_pic.img.i_plane = 1;
		m_pic.img.i_stride[0] = 3 * m_width;
		m_pic.img.plane[0]    = (uint8_t*)pInputBuffer;
		break;

	case X264_CSP_BGRA://?
		m_pic.img.i_plane = 1;
		m_pic.img.i_stride[0] = 4 * m_width;
		m_pic.img.plane[0]    = (uint8_t*)pInputBuffer;
		break;

	default:
		return 0;
	}

	/* encode it */
	m_sizeImage++;
	m_pic.i_pts = (int64_t)m_sizeImage ;
	m_pic.i_type = X264_TYPE_AUTO;
	m_pic.i_qpplus1 = 0;
	x264_encoder_encode( m_Codec, &nal, &i_nal, &m_pic, &pic_out);

	/* create bitstream, unless we're dropping it in 1st pass */
	nOutputSize = 0;

	for( i = 0; i < i_nal; i++ ) {
		int i_size = m_sizeImage - nOutputSize;
		x264_nal_encode( (uint8_t*)pOutputBuffer + nOutputSize, &i_size, 1, &nal[i] );
		nOutputSize += i_size;
	}

	/* Set key frame only for IDR, as they are real synch point, I frame
	aren't always synch point (ex: with multi refs, ref marking) */
	if( pic_out.i_type == X264_TYPE_IDR )
		bKeyFrame = true;
	else
		bKeyFrame = false;
	return nOutputSize; 
	

}
void CH264Encoder::releaseCodec()
{	
	//x264_picture_clean( &m_pic );
	if (m_Codec!=NULL)
	{
		x264_encoder_close(m_Codec);
		m_Codec = NULL;
	}
}