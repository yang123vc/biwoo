#include "XvidProxy.h"
#include <memory.h>
#include <stdlib.h>
#if 0

#ifdef WIN32
#ifdef _DEBUG
#pragma comment(lib, "libxvidcored.lib") 
#else
#pragma comment(lib, "libxvidcore.lib") 
#endif
#endif

#endif

#ifndef NULL
#define NULL 0
#endif


#define   DEF_FRAMERATE   15 
#define   DEF_BITRATE_K   700 
#define   DEF_QUANT   5 
#define   DEF_BQRATIO   150 
#define   DEF_BQOFFSET   100 
#define   DEF_MAXBFRAMES   0   
#define   DEF_MAXKEYINTERVAL   300 
#define   DEF_REACTION_DELAY_FACTOR   16 
#define   DEF_AVERAGING_PERIOD   100 
#define   DEF_ZONEBASE   100 
#define   DEF_FRAMERATE_INCR   1001 

static   const   int   motion_presets[]   =   { 
/*   quality   0   */ 
0, 
XVID_ME_ADVANCEDDIAMOND16,/*   quality   1   */ 
XVID_ME_ADVANCEDDIAMOND16   |   XVID_ME_HALFPELREFINE16,/*   quality   2   */ 
/*   quality   3   */ 
XVID_ME_ADVANCEDDIAMOND16   |   XVID_ME_HALFPELREFINE16   | 
XVID_ME_ADVANCEDDIAMOND8   |   XVID_ME_HALFPELREFINE8, 
/*   quality   4   */ 
XVID_ME_ADVANCEDDIAMOND16   |   XVID_ME_HALFPELREFINE16   | 
XVID_ME_ADVANCEDDIAMOND8   |   XVID_ME_HALFPELREFINE8   | 
XVID_ME_CHROMA_PVOP   |   XVID_ME_CHROMA_BVOP, 
/*   quality   5   */ 
XVID_ME_ADVANCEDDIAMOND16   |   XVID_ME_HALFPELREFINE16   | 
XVID_ME_ADVANCEDDIAMOND8   |   XVID_ME_HALFPELREFINE8   | 
XVID_ME_CHROMA_PVOP   |   XVID_ME_CHROMA_BVOP, 
/*   quality   6   */ 
XVID_ME_ADVANCEDDIAMOND16   |   XVID_ME_HALFPELREFINE16   |   XVID_ME_EXTSEARCH16   | 
XVID_ME_ADVANCEDDIAMOND8   |   XVID_ME_HALFPELREFINE8   |   XVID_ME_EXTSEARCH8   | 
XVID_ME_CHROMA_PVOP   |   XVID_ME_CHROMA_BVOP, 

}; 
#define   ME_ELEMENTS   (sizeof(motion_presets)/sizeof(motion_presets[0])) 

static   const   int   vop_presets[]   =   { 
0,/*   quality   0   */ 
0,/*   quality   1   */ 
XVID_VOP_HALFPEL,/*   quality   2   */ 
XVID_VOP_HALFPEL   |   XVID_VOP_INTER4V,/*   quality   3   */ 
/*   quality   4   */ 
XVID_VOP_HALFPEL   |   XVID_VOP_INTER4V, 
/*   quality   5   */ 
XVID_VOP_HALFPEL   |   XVID_VOP_INTER4V   |XVID_VOP_TRELLISQUANT, 
/*   quality   6   */ 
XVID_VOP_HALFPEL   |   XVID_VOP_INTER4V   |XVID_VOP_TRELLISQUANT   |   XVID_VOP_HQACPRED, 

}; 
#define   VOP_ELEMENTS   (sizeof(vop_presets)/sizeof(vop_presets[0])) 

static int bit_buffer_size= 1024*256*4;

CXvidProxy::CXvidProxy(void)
: m_encHandle(NULL)
, m_decHandle(NULL)

{
	m_encBuffer = new unsigned char[bit_buffer_size];
	m_decBuffer = new unsigned char[bit_buffer_size];
}

CXvidProxy::~CXvidProxy(void)
{
	if (m_encHandle)
		xvid_encore(m_encHandle, XVID_ENC_DESTROY, 0, 0);
	if (m_decHandle)
		xvid_decore(m_decHandle, XVID_DEC_DESTROY, 0, 0);

	delete[] m_encBuffer;
	delete[] m_decBuffer;
}

bool CXvidProxy::encode(const unsigned char * inFrame, unsigned char ** outBuffer, int & outSize)
{
	if (!init_enc())
		return false;

	m_encFrame.input.plane[0]   =   const_cast<unsigned char*>(inFrame); 

	memset(&m_encStats, 0, sizeof(m_encStats));
	m_encStats.version = XVID_VERSION;

	/* Encode */
	int bytesEncoded = xvid_encore(m_encHandle, XVID_ENC_ENCODE, &m_encFrame, &m_encStats);
	if (bytesEncoded > 0)
	{
		//bytesEncoded = m_encStats.length;
		outSize = bytesEncoded;
		*outBuffer = new unsigned char[outSize];
		memcpy(*outBuffer, m_encBuffer, outSize);
	}

	return bytesEncoded > 0;
}

bool CXvidProxy::decode(const unsigned char * inFrame, int inSize, unsigned char ** outBuffer, int & outSize)
{
	if (!init_dec())
		return false;

	memset(&m_decFrame, 0, sizeof(m_decFrame));
	m_decFrame.version = XVID_VERSION;

	m_decFrame.bitstream = (void*)const_cast<unsigned char*>(inFrame);
	m_decFrame.length = inSize;

	memset(&m_decStats, 0, sizeof(m_decStats));
	m_decStats.version = XVID_VERSION;

	// Decode
	int bytesDecoded = 0;
	//do
	//{
		bytesDecoded = xvid_decore(m_decHandle, XVID_DEC_DECODE, &m_decFrame, &m_decStats);
		if(m_decStats.type == XVID_TYPE_VOL)
		{
			//return false;

		}else
		{

		}
	//}while(m_decStats.type <= 0);

	if (bytesDecoded > 0)
	{
		outSize = m_decxInfo.framesize();
		*outBuffer = new unsigned char[outSize];
		memcpy(*outBuffer, m_decBuffer, outSize);
	}

	return bytesDecoded > 0;
}

bool CXvidProxy::init_enc(void)
{
	if (m_encHandle == 0)
	{
		xvid_gbl_init_t   xvid_gbl_init; 
		memset(&xvid_gbl_init,0,sizeof(xvid_gbl_init_t)); 

		xvid_gbl_init.cpu_flags   =   0;   //�Զ����cpu���� 
		xvid_gbl_init.debug   =   XVID_DEBUG_ERROR; 
		xvid_gbl_init.version   =   XVID_VERSION; 

		int ret = xvid_global(0, XVID_GBL_INIT, &xvid_gbl_init, 0); 

		//����������ʵ�� 
		xvid_enc_create_t   enc_create; 
		xvid_enc_zone_t   xvid_enc_zone; 
		//xvid_plugin_single_t   single; 
		//xvid_enc_plugin_t   plugin; 

		memset(&enc_create, 0, sizeof(xvid_enc_create_t));
		memset(&xvid_enc_zone,0,sizeof(xvid_enc_zone_t)); 
		//memset(&single,0,sizeof(xvid_plugin_single_t)); 
		//memset(&plugin,0,sizeof(xvid_enc_plugin_t)); 

		enc_create.version   =   XVID_VERSION; 

		// ??
		int m_nFrameRate = 25;
		//   zone 
		xvid_enc_zone.mode   =   XVID_ZONE_QUANT; 
		xvid_enc_zone.frame   =   DEF_FRAMERATE; 
		xvid_enc_zone.base   =   DEF_ZONEBASE; 
		xvid_enc_zone.increment   =   DEF_ZONEBASE*m_nFrameRate; 


		enc_create.num_zones   =   1; 
		enc_create.zones   =   &xvid_enc_zone; 

		//   ��װ��� 
		/*single.version   =   XVID_VERSION; 
		single.bitrate   =   DEF_BITRATE_K*1000;   
		single.reaction_delay_factor   =   DEF_REACTION_DELAY_FACTOR; 
		single.averaging_period   =   DEF_AVERAGING_PERIOD; 

		plugin.func   =   xvid_plugin_single_func; 
		plugin.param   =   &single; 

		enc_create.num_plugins   =   1; 
		enc_create.plugins   =   &plugin; */

		enc_create.profile   =   XVID_PROFILE_AS_L4;	// XVID_PROFILE_AS_L4 ���ѹ������
		enc_create.width   =   m_encxInfo.width(); 
		enc_create.height   =   m_encxInfo.height(); 

		enc_create.num_threads   =   0; 

		//enc_create.fincr   =   DEF_FRAMERATE_INCR; 
		//enc_create.fbase   =   DEF_FRAMERATE_INCR   *   700; 
		enc_create.fincr   =   1; 
		enc_create.fbase   =   3; 

		enc_create.max_key_interval   =   DEF_FRAMERATE*10; 

		//enc_create.min_quant[0]   =   1; 
		//enc_create.min_quant[1]   =   1; 
		//enc_create.min_quant[2]   =   1; 
		//enc_create.max_quant[0]   =   5; 
		//enc_create.max_quant[1]   =   5; 
		//enc_create.max_quant[2]   =   5; 

		/*   ����B֡   */ 
		//enc_create.max_bframes   =   DEF_MAXBFRAMES;   
		//enc_create.bquant_ratio   =   DEF_BQRATIO; 
		//enc_create.bquant_offset   =   DEF_BQOFFSET;

		enc_create.frame_drop_ratio   =   0; 
		enc_create.global   =   XVID_GLOBAL_PACKED; 

		ret = xvid_encore(NULL,   XVID_ENC_CREATE,   &enc_create,   NULL); 
		if (ret != 0)
			return false;
		
		m_encHandle =   enc_create.handle; 

		//if (ff_xvid_encode_init(m_pVideoCodecCtxEnc, &m_enc) != 0)
		//	return false;
		/////////////////////////////////
		//xvid_enc_frame_t xvid_enc_frame;
		//xvid_enc_stats_t xvid_enc_stats;

		/* Start setting up the frame */
		memset(&m_encFrame, 0, sizeof(m_encFrame));
		m_encFrame.version = XVID_VERSION;

		m_encFrame.vol_flags   =   0; 
		m_encFrame.vol_flags   =   XVID_VOL_MPEGQUANT; 
		m_encFrame.par   =   XVID_PAR_11_VGA; 
		m_encFrame.vop_flags   =   vop_presets[6]; 
		m_encFrame.motion   =   motion_presets[6]; 
		m_encFrame.type   =   XVID_TYPE_AUTO; 
		m_encFrame.quant   = 6;		// ��������=����������0~31����ֵԽС����Խ�ߺ����ʳɷ���

		m_encFrame.bitstream = m_encBuffer;
		m_encFrame.length = bit_buffer_size;

		m_encFrame.input.csp = m_encxInfo.csp(); 
		m_encFrame.input.stride[0]  = m_encxInfo.stride();
	}

	return m_encHandle != 0;
}


bool CXvidProxy::init_dec(void)
{
	if (m_decHandle == 0)
	{
		xvid_gbl_init_t   xvid_gbl_init; 
		memset(&xvid_gbl_init,0,sizeof(xvid_gbl_init_t)); 

		xvid_gbl_init.cpu_flags   =   0;   //�Զ����cpu���� 
		xvid_gbl_init.debug   =   XVID_DEBUG_ERROR; 
		xvid_gbl_init.version   =   XVID_VERSION; 

		int ret = xvid_global(0, XVID_GBL_INIT, &xvid_gbl_init, 0); 

		xvid_dec_create_t dec_create;
		memset(&dec_create, 0, sizeof(xvid_dec_create_t));

		dec_create.version   =   XVID_VERSION; 
		dec_create.width   =   0; 
		dec_create.height   =   0; 

		ret = xvid_decore(NULL,   XVID_DEC_CREATE,   &dec_create,   NULL); 
		if (ret != 0)
			return false;

		m_decHandle = dec_create.handle;

		/* Start setting up the frame */
		memset(&m_decFrame, 0, sizeof(m_decFrame));
		m_decFrame.version = XVID_VERSION;

		//m_decFrame.general   =   XVID_LOWDELAY; 
		m_decFrame.output.csp   =   m_decxInfo.csp(); 
		m_decFrame.output.plane[0] = m_decBuffer;
		m_decFrame.output.stride[0] = m_decxInfo.stride();
	}

	return m_decHandle != 0;
}

