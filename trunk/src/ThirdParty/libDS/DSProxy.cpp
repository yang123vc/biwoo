
#define libDS_EXPORTS

#include <winsock2.h>
#include "DSProxy.h"
#include <streams.h>
#include "CRoleAdmin.h"
#include "CAVDevice.h"
#include "boost/shared_ptr.hpp"
#include "stl/LockMap.h"
#include "aac_types.h"

/////////////////////////////////////////////////////////
//
class CDSAdmin
	: public CRoleAdmin
{
public:
	typedef boost::shared_ptr<CDSAdmin> pointer;

	static CDSAdmin::pointer create(void)
	{
		return CDSAdmin::pointer(new CDSAdmin());
	}

private:
	CAVDevice		mVideoDeviceServer;
	CAVDevice		mAudioDeviceServer;

public:
	CDSAdmin(void)
	{
		SetAVDevice(&mVideoDeviceServer, &mAudioDeviceServer);
	}
	~CDSAdmin()
	{}
};

CDSAdmin::pointer m_serverAdmin;
CLockMap<DoDSHandler*, CDSAdmin::pointer> m_mapClientAdmin;


CDSProxy::CDSProxy(void)
{

}

CDSProxy::~CDSProxy(void)
{
	//stopServer();
	//stopAllClient();
}

bool CDSProxy::isServerStarted(void) const
{
	return (m_serverAdmin.get() != NULL);
}

DoDSHandler::pointer CDSProxy::startServer(const CAVParameter & parameter, OnDSHandler * pDSHandler)
{
	if (m_serverAdmin.get() != NULL)
	{
		return m_serverAdmin;
		//return (DoDSHandler*)m_serverAdmin.get();
	}
	CoInitialize(NULL);
	m_serverAdmin = CDSAdmin::create();

	m_serverAdmin->SetVideoWindow((HWND)parameter.preview());
	m_serverAdmin->setSenderHandler(pDSHandler);
	((CAVParameter*)m_serverAdmin.get())->operator =(parameter);

	if (m_serverAdmin->audio() && !CRoleAdmin::findAudioDev())
	{
		m_serverAdmin->audio(false);
	}
	if (m_serverAdmin->video() && !CRoleAdmin::findVideoDev())
	{
		m_serverAdmin->video(false);
	}

	UINT deviceConfig = 0;
	if (m_serverAdmin->audio())
	{
		deviceConfig |= Local_Has_Audio;

		switch (parameter.audioCoding())
		{
		case AVCoding::CT_AAC:
			deviceConfig |= Local_Audio_AACEnc;
			break;
		default:
			break;
		}
	}
	if (m_serverAdmin->video())
	{
		deviceConfig |= Local_Has_Video;

		//switch (parameter.videoCoding())
		//{
		//case AVCoding::CT_MPEG4:
		//	deviceConfig |= Local_Video_MPEG4Enc;
		//	break;
		//default:
		//	break;
		//}
	}

	m_serverAdmin->SetDeviceConfig(deviceConfig);

//#if (USES_H264CODEC)
//	m_serverAdmin->SetDeviceConfig(Local_Has_Video|Local_Has_Audio|Local_Audio_AACEnc);
//#else
//	m_serverAdmin->SetDeviceConfig(Local_Has_Video|Local_Has_Audio|Local_Video_MPEG4Enc|Local_Audio_AACEnc);
//#endif

	//m_serverAdmin->audio(parameter.audioChannel(), parameter.audioFrequency());
	//m_serverAdmin->video(parameter.videoFPS(), parameter.videoWidth(), parameter.videoHeight());
	if (!m_serverAdmin->Activate())
	{
		m_serverAdmin->Disconnect();
		m_serverAdmin.reset();
		return m_serverAdmin;
		//return NULL;
	}

	return m_serverAdmin;
	//DoDSHandler * pDoHandler = (DoDSHandler*)m_serverAdmin.get();
	//return pDoHandler;
}

DoDSHandler::pointer CDSProxy::startClient(const CAVParameter & parameter)
{
	CoInitialize(NULL);
	CDSAdmin::pointer clientAdmin = CDSAdmin::create();

	((CAVParameter*)clientAdmin.get())->operator =(parameter);
	//clientAdmin->audioCoding(parameter.audioCoding());
	//clientAdmin->videoCoding(parameter.videoCoding());

	UINT deviceConfig = 0;
	if (parameter.audio())
	{
		deviceConfig |= Remote_Has_Audio;

		/*switch (parameter.audioCoding())
		{
		case AVCoding::CT_AAC:
			deviceConfig |= Remote_Audio_AACDec;
			break;
		default:
			break;
		}*/
	}
	if (parameter.video())
	{
		deviceConfig |= Remote_Has_Video;

		//switch (parameter.videoCoding())
		//{
		//case AVCoding::CT_XVID:
		//	deviceConfig |= Remote_Video_MPEG4Dec;
		//	break;
		//default:
		//	break;
		//}
	}

	clientAdmin->SetDeviceConfig(deviceConfig);
	clientAdmin->SetRemoteVideoWindow((HWND)parameter.preview());

	//clientAdmin->audio(parameter.audioChannel(), parameter.audioFrequency());
	//clientAdmin->video(parameter.videoFPS(), parameter.videoWidth(), parameter.videoHeight());

	if (!clientAdmin->Activate())
	{
		clientAdmin.reset();
		return clientAdmin;
		//return NULL;
	}

	if (parameter.audio())
	{
		CAVConfigData::pointer configData = CAVConfigData::create();
		configData->setIsVideo(false);
		configData->setFrequency(parameter.audioFrequency());
		configData->setNumChannels(parameter.audioChannel());
		configData->timestampType(parameter.audiott());

		if (parameter.audioCoding() == AVCoding::CT_AAC)
		{
			// AAC 
			BYTE config[2];
			getAACExtraBytes(config[0], config[1], configData->getNumChannels(), configData->getFrequency());
			configData->setConfigData(config, 2);
		}

		//unsigned char temp[2] = {0x15,0x88};
		///*configData->setFrequency(44100);
		//configData->setNumChannels(2);
		//unsigned char temp[2] = {0x0a,0x10};*/
		//configData->setConfigData((unsigned char*)temp,2);
		clientAdmin->receiveConfig(configData);
	}

	if (parameter.video())
	{
		CAVConfigData::pointer configData = CAVConfigData::create();
		configData->setIsVideo(true);
		configData->setFrequency(parameter.videoFrequency());
		configData->setWidth(parameter.videoWidth());
		configData->setHeight(parameter.videoHeight());
		configData->timestampType(parameter.videott());
		clientAdmin->receiveConfig(configData);
	}

	DoDSHandler * clientHandler = (DoDSHandler*)clientAdmin.get();
	m_mapClientAdmin.insert(clientHandler, clientAdmin);
	return clientAdmin;
	//return clientHandler;
}

void CDSProxy::stopDSHandler(DoDSHandler::pointer pDoHandler)
{
	if (pDoHandler.get() != NULL && pDoHandler.get() == m_serverAdmin.get())
	{
		m_serverAdmin->Disconnect();
		pDoHandler->ClearOnDSHandler();
		m_serverAdmin.reset();
		return;
	}

	CDSAdmin::pointer clientAdmin;
	if (m_mapClientAdmin.find(pDoHandler.get(), clientAdmin, true))
	{
		clientAdmin->Disconnect();
		pDoHandler->ClearOnDSHandler();
	}
}

void CDSProxy::stopAllClient(bool bBothStopServer)
{
	if (bBothStopServer && m_serverAdmin.get() != NULL)
	{
		m_serverAdmin->Disconnect();
		m_serverAdmin.reset();
	}

	boost::mutex::scoped_lock lock(m_mapClientAdmin.mutex());
	//for_each(m_mapClientAdmin.begin(), m_mapClientAdmin.end(),
		//boost::bind(&CRoleAdmin::Disconnect, boost::bind(&std::map<unsigned long, CDSAdmin::pointer>::value_type::second,_1)));
	std::map<DoDSHandler*, CDSAdmin::pointer>::iterator iter;
	for (iter=m_mapClientAdmin.begin(); iter!=m_mapClientAdmin.end(); iter++)
	{
		iter->second->Disconnect();
	}
	m_mapClientAdmin.clear(false);
}
