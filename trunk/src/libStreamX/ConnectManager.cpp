/*
    StreamModule is a P2P and conference communication module base on CGCP.
    Copyright (C) 2009-2010  Akee Yang <akee.yang@gmail.com>

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "ConnectManager.h"
//#include <boost/thread/xtime.hpp>

#define PEEROPEN WM_USER+200

const short MAX_PACKET_SIZE		= 2500;
//const short MAX_PACKET_SIZE		= 1200;	// OK
const short MAX_INDEXACK_SIZE	= 20;
//const int	WAIT_NANOSECONDS	= 500000;	// 1000000000/S

//CLog ManagerLog;

ConnectManager::ConnectManager(void)
//:m_avsCgcProxy(this)
: m_VideoFlag(false)
, m_AudioFlag(false)
, m_handler(NULL)
, m_currentId(0)

{

}

ConnectManager::~ConnectManager(void)
{
	disconnectStreamServer();
}

bool ConnectManager::connectStreamServer(const CCgcAddress & ServerAddr)
{
	if (!m_avsCgcProxy.avsIsStarted())
	{
		if (!m_avsCgcProxy.avsStart(ServerAddr, (CStreamHandler*)this))
		{
			m_avsCgcProxy.avsStop();
			return false;
		}
	}

	return m_avsCgcProxy.avsIsOpenSession();
}

bool ConnectManager::isConnectedStreamServer(void) const
{
	return m_avsCgcProxy.avsIsOpenSession();
}

void ConnectManager::disconnectStreamServer(void)
{
	//m_files.clear();
	m_filestemp.clear();
	m_p2pfiles.clear();

	m_userP2PClientMap.clear();
	m_fidfiles.clear();
	stopServer();
	stopClient();
	m_avsCgcProxy.avsStop();
}

bool ConnectManager::Login(const tstring & Name)
{
	if (!m_avsCgcProxy.avsAccountRegister(Name, _T("")))
	{
		return false;
	}
	return true;
}

void ConnectManager::Logout(void)
{
	stopClient();
	m_VideoFlag = false;
	m_AudioFlag = false;

	m_avsCgcProxy.avsAccountUnRegister();
}

bool ConnectManager::IsLogined(void)
{
	return m_avsCgcProxy.getCurrentUser().size() > 0;
}

DoDSHandler::pointer ConnectManager::startServer(const CAVParameter & avp)
{
	if (m_serverHandler.get() != NULL) return m_serverHandler;

	if (m_VideoFlag || m_AudioFlag)
		m_serverHandler = m_dsproxy.startServer(avp, (OnDSHandler*)this);
	else
		m_serverHandler = m_dsproxy.startServer(avp, NULL);
	return m_serverHandler;
}

void ConnectManager::stopServer(void)
{
	if (m_serverHandler.get() != NULL)
	{
		DoDSHandler::pointer handler = m_serverHandler;
		m_serverHandler.reset();
		m_dsproxy.stopDSHandler(handler);
	}
}

DoDSHandler::pointer ConnectManager::startClient(const CAVParameter & avp)
{
	if (m_clientHandler.get() != NULL) return m_clientHandler;

	m_avpClient = avp;
	m_clientHandler = m_dsproxy.startClient(avp);
	return m_clientHandler;
}

void ConnectManager::clientVideoReversal(void)
{
	if (m_clientHandler.get() != NULL)
		m_clientHandler->RemoteVideoReversal();
}

void ConnectManager::stopClient(DoDSHandler::pointer pDoDSHandler)
{
	// ??
	if (pDoDSHandler.get() == m_clientHandler.get())
	{
		m_clientHandler.reset();
	}
	m_dsproxy.stopDSHandler(pDoDSHandler);
}

void ConnectManager::stopClient(void)
{
	// ??
	if (m_clientHandler.get() != NULL)
	{
		DoDSHandler::pointer handler = m_clientHandler;
		m_clientHandler.reset();
		m_dsproxy.stopDSHandler(handler);
	}
}

void ConnectManager::onUserInfo(const tstring & sUserId, const tstring & sUserName)
{
	if (m_handler)
		m_handler->onUserInfo(sUserId, sUserName);
}

void ConnectManager::onUserLogined(const tstring & sUserName)
{
	if (m_handler)
		m_handler->onUserLogined(sUserName);
}

void ConnectManager::onUserLogouted(const tstring & sUserName)
{
	if (m_handler)
		m_handler->onUserLogouted(sUserName);
}

void ConnectManager::onP2PRequestResult(const tstring & sFromUser, long nP2PType, long resuleValue)
{
	if (m_handler)
		m_handler->onP2PRequestResult(sFromUser, nP2PType, resuleValue);
}

CP2PHandler * ConnectManager::onP2PUserAck(CDoP2PClientHandler::pointer p2pClient, bool & disableSotpParse)
{
	BOOST_ASSERT (p2pClient.get() != NULL);

	if (m_handler)
		m_handler->onP2PUserConnected(p2pClient->getP2PUser(), p2pClient->getP2PType(), p2pClient->getP2PParam());

	if (p2pClient->getP2PType() == CDoP2PClientHandler::P2P_VIDEO)
	{
		m_VideoFlag = true;
		switch (m_avpClient.videoCoding())
		{
		case AVCoding::CT_XVID:
			p2pClient->doSetMediaType(99);
			break;
		case AVCoding::CT_H264:
			p2pClient->doSetMediaType(98);
			break;
		default:
			break;
		}
	}else if (p2pClient->getP2PType() == CDoP2PClientHandler::P2P_AUDIO)
	{
		m_AudioFlag = true;
		switch (m_avpClient.audioCoding())
		{
		case AVCoding::CT_G711A:
		case AVCoding::CT_G711U:
			p2pClient->doSetMediaType(8);
			break;
		case AVCoding::CT_G729A:
		case AVCoding::CT_AAC:
			// ?
			break;
		default:
			break;
		}
	}else if (p2pClient->getP2PType() == CDoP2PClientHandler::P2P_FILE)
	{
		CFileInfo::pointer fileInfo;
		if (m_filestemp.find(p2pClient->getP2PParam(), fileInfo, true))
		{
			long fid = p2pClient->getP2PParam();
			CP2PFiles::pointer p2pFiles;
			if (!m_p2pfiles.find(p2pClient.get(), p2pFiles))
			{
				p2pFiles = CP2PFiles::create(p2pClient);
				m_p2pfiles.insert(p2pClient.get(), p2pFiles);
			}
			p2pFiles->m_files.insert(fid, fileInfo);

			m_fidfiles.insert(fid, p2pFiles);
			fileInfo->p2pAck(true);
		}

		disableSotpParse = false;
		return (CP2PHandler*)this;
	}else
	{
		return NULL;
	}

	if (m_serverHandler.get() != NULL)
		m_serverHandler->SetOnDSHandler(this);

	m_FriendName = p2pClient->getP2PUser();

	char buffer[20];
	sprintf(buffer, ":%d", p2pClient->getP2PType());
	tstring sP2PKey(p2pClient->getP2PUser());
	sP2PKey.append(buffer);

	m_userP2PClientMap.insert(sP2PKey, p2pClient);
	//disableSotpParse = true;
	return (CP2PHandler*)this;
}

void ConnectManager::onP2PRequestTimeout(CDoP2PClientHandler::pointer p2pClient)
{
	BOOST_ASSERT (p2pClient.get() != NULL);

	if (m_handler)
		m_handler->onP2PRequestTimeout(p2pClient->getP2PUser(), p2pClient->getP2PType());
}

void ConnectManager::onP2PUserDisconnect(CDoP2PClientHandler::pointer p2pClient)
{
	BOOST_ASSERT (p2pClient.get() != NULL);

	if (m_handler)
		m_handler->onP2PUserDisconnected(p2pClient->getP2PUser(), p2pClient->getP2PType(), p2pClient->getP2PParam());

	m_FriendName = _T("");
	if (p2pClient->getP2PType() == CDoP2PClientHandler::P2P_VIDEO)
	{
		m_VideoFlag = false;
	}else if (p2pClient->getP2PType() == CDoP2PClientHandler::P2P_AUDIO)
	{
		m_AudioFlag = false;
	}/*else if (p2pClient->getP2PType() == CDoP2PClientHandler::P2P_FILE)
	{
	}*/

	if (m_serverHandler.get() != NULL)
		m_serverHandler->ClearOnDSHandler();

	char buffer[20];
	sprintf(buffer, ":%d", p2pClient->getP2PType());
	tstring sP2PKey(p2pClient->getP2PUser());
	sP2PKey.append(buffer);

	m_userP2PClientMap.remove(sP2PKey);
	m_p2pfiles.remove(p2pClient.get());
}

// CP2PHandler handler
void ConnectManager::onP2PEvent(CDoP2PClientHandler::pointer p2pClient, const cgcParserSotp & response)
{
	BOOST_ASSERT (p2pClient.get() != NULL);

	switch (response.getSign())
	{
	case const_CallSign_MsgRequestAck:
		{
			long fid = response.getRecvParameterValue(_T("MID"), 0);
			long newfid = response.getRecvParameterValue(_T("NMID"), 0);

			CP2PFiles::pointer p2pFiles;
			if (!m_p2pfiles.find(p2pClient.get(), p2pFiles))
			{
				break;
			}

			CFileInfo::pointer fileInfo;
			if (!p2pFiles->m_files.find(fid, fileInfo))
			{
				break;
			}
			fileInfo->did(newfid);

		}break;
	case const_CallSign_MsgRequest:
		{
			if (m_handler == NULL) break;

			long fid = response.getRecvParameterValue(_T("MID"), 0);
			long type = response.getRecvParameterValue(_T("Type"), 0);
			switch (type)
			{
			case 11:
				{
					// file
					const tstring & sFilename = response.getRecvParameterValue(_T("Name"));
					if (sFilename.empty()) break;
					long nFilesize = response.getRecvParameterValue(_T("Size"), 0);
					if (nFilesize <= 0) break;

					long newfid = ++m_currentId+10;

					// MsgRequestAck
					p2pClient->doBeginCallLock();
					p2pClient->doAddParameter(CGC_PARAMETER(_T("MID"), fid));
					p2pClient->doAddParameter(CGC_PARAMETER(_T("NMID"), newfid));
					p2pClient->doSendAppCall(const_CallSign_MsgRequestAck, const_Api_MsgRequestAck);

					CFileInfo::pointer fileInfo = CFileInfo::create(sFilename);
					fileInfo->did(fid);
					fileInfo->fid(newfid);
					fileInfo->filesize(nFilesize);

					CP2PFiles::pointer p2pFiles;
					if (!m_p2pfiles.find(p2pClient.get(), p2pFiles))
					{
						p2pFiles = CP2PFiles::create(p2pClient);
						m_p2pfiles.insert(p2pClient.get(), p2pFiles);
					}
					p2pFiles->m_files.insert(newfid, fileInfo);

					m_fidfiles.insert(newfid, p2pFiles);

					m_handler->onFileRequest(p2pClient->getP2PUser(), newfid, sFilename, nFilesize);
				}break;
			default:
				break;
			}
		}break;
	case const_CallSign_MsgResponse:
		{
			long fid = response.getRecvParameterValue(_T("MID"), 0);
			long nResponse = response.getRecvParameterValue(_T("Response"), 0);

			if (nResponse == 1)
			{
				// Accept
				CP2PFiles::pointer p2pFiles;
				if (!m_p2pfiles.find(p2pClient.get(), p2pFiles))
				{
					break;
				}

				CFileInfo::pointer fileInfo;
				if (!p2pFiles->m_files.find(fid, fileInfo))
				{
					break;
				}
				// Create the send thread.
				size_t indexCount = fileInfo->filesize() / MAX_PACKET_SIZE;
				if (fileInfo->filesize() % MAX_PACKET_SIZE > 0)
				{
					indexCount += 1;
				}

				for (int i=0; i<indexCount; i++)
				{
					unsigned long index = i*MAX_PACKET_SIZE;
					fileInfo->m_sendindexs.add(index);

					// for send ack.
					fileInfo->m_indexs.insert(index, true);
				}
				fileInfo->indexCount(indexCount);

				short sendThreadSize = getThreadsBysize(fileInfo->filesize());
				for (short i=1; i<=sendThreadSize; i++)
				{
					boost::thread * sendThread = new boost::thread(boost::bind(doStaProcSendData, this, p2pClient, fileInfo, i==sendThreadSize));
					fileInfo->m_sendthreads.add(sendThread);
				}
			}else if (nResponse == 2)
			{
				// Reject
				CP2PFiles::pointer p2pFiles;
				if (!m_p2pfiles.find(p2pClient.get(), p2pFiles))
				{
					break;
				}

				m_fidfiles.remove(fid);
				CFileInfo::pointer fileInfo;
				if (!p2pFiles->m_files.find(fid, fileInfo, true))
				{
					break;
				}
				fileInfo->RT(CFileInfo::RT_Rejected);

				m_handler->onFileRejected(p2pClient->getP2PUser(), fid);
				this->disconnectP2P(p2pClient);

				if (p2pFiles->m_files.empty())
				{
					m_p2pfiles.remove(p2pClient.get());
				}
			}else if (nResponse == 3)
			{
				// Cancel
				CP2PFiles::pointer p2pFiles;
				if (!m_p2pfiles.find(p2pClient.get(), p2pFiles))
				{
					break;
				}

				m_fidfiles.remove(fid);
				CFileInfo::pointer fileInfo;
				if (!p2pFiles->m_files.find(fid, fileInfo, true))
				{
					break;
				}
				fileInfo->RT(CFileInfo::RT_Canceled);

				m_handler->onFileCanceled(p2pClient->getP2PUser(), fid);
				this->disconnectP2P(p2pClient);

				fileInfo->fs().close();
				namespace fs = boost::filesystem;
				fs::path pathFile(fileInfo->filepath(), fs::native);
				if (fs::exists(pathFile))
				{
					fs::remove(pathFile);
				}

				if (p2pFiles->m_files.empty())
				{
					m_p2pfiles.remove(p2pClient.get());
				}
			}else if (nResponse == 4)
			{
				// send OK
				CP2PFiles::pointer p2pFiles;
				if (!m_p2pfiles.find(p2pClient.get(), p2pFiles))
				{
					break;
				}

				m_fidfiles.remove(fid);
				CFileInfo::pointer fileInfo;
				if (!p2pFiles->m_files.find(fid, fileInfo, true))
				{
					break;
				}
				fileInfo->RT(CFileInfo::RT_Arrived);

				if (p2pFiles->m_files.empty())
				{
					m_p2pfiles.remove(p2pClient.get());
				}
			}
		}break;
	case const_CallSign_MsgSend:
		{
			if (m_handler == NULL) break;

			long fid = response.getRecvParameterValue(_T("MID"), 0);
			cgcAttachment::pointer attach = response.getRecvAttachment();
			if (attach.get() == NULL) break;

			CP2PFiles::pointer p2pFiles;
			if (!m_p2pfiles.find(p2pClient.get(), p2pFiles))
			{
				break;
			}

			CFileInfo::pointer fileInfo;
			if (!p2pFiles->m_files.find(fid, fileInfo))
			{
				break;
			}

			// For send index ack.
			fileInfo->m_sendindexs.add(attach->getIndex());

			if (fileInfo->m_indexs.empty())
			{
				size_t indexCount = 0;
				if (attach->getIndex() + attach->getAttachSize() == attach->getTotal())
				{
					indexCount = 1;
					if (attach->getIndex() > 0)
					{
						indexCount++;
						fileInfo->m_indexs.insert(attach->getIndex(), true);
					}
					fileInfo->m_indexs.insert(attach->getTotal(), true);
				}else
				{
					indexCount = attach->getTotal() / attach->getAttachSize();
					if (attach->getTotal() % attach->getAttachSize() > 0)
						indexCount += 1;

					for (int i=1; i<= indexCount; i++)
					{
						unsigned long index = (i==indexCount) ? attach->getTotal() : i*attach->getAttachSize();
						fileInfo->m_indexs.insert(index, true);
					}
				}
				fileInfo->indexCount(indexCount);
			}

			tfstream & filestream = fileInfo->fs();
			if (filestream.is_open())
			{
				filestream.seekp(attach->getIndex(), std::ios::beg);
				filestream.write((const char*)attach->getAttachData(), attach->getAttachSize());
				filestream.flush();
			}
			fileInfo->m_indexs.remove(attach->getIndex() + attach->getAttachSize());
			if (fileInfo->m_indexs.empty())
			{
				// OK
				CP2PFiles::pointer p2pFiles;
				if (!m_p2pfiles.find(p2pClient.get(), p2pFiles))
				{
					break;
				}

				filestream.close();
				m_fidfiles.remove(fid);
				p2pFiles->m_files.remove(fid);
				fileInfo->RT(CFileInfo::RT_Arrived);

				p2pClient->doBeginCallLock();
				p2pClient->doAddParameter(CGC_PARAMETER(_T("MID"), fileInfo->did()));
				p2pClient->doAddParameter(CGC_PARAMETER(_T("Response"), 4));
				p2pClient->doSendAppCall(const_CallSign_MsgResponse, const_Api_MsgResponse);

				m_handler->onReceiveFileData(p2pClient->getP2PUser(), fid, 100.0);

				if (p2pFiles->m_files.empty())
				{
					m_p2pfiles.remove(p2pClient.get());
				}
			}else
			{
				// Already had data.
				size_t indexCount = fileInfo->indexCount();

				if (indexCount-fileInfo->m_indexs.size() == 1)
				{
					// first data event
					float percent = 0.0;
					m_handler->onReceiveFileData(p2pClient->getP2PUser(), fid, percent);
				}else
				{
					if (fileInfo->icounter() == 380)
					{
						// continue data event
						fileInfo->resetcounter();
						float percent = (indexCount-fileInfo->m_indexs.size()) * 100.0 / indexCount;
						m_handler->onReceiveFileData(p2pClient->getP2PUser(), fid, percent);
					}
				}

			}
		}break;
	case const_CallSign_MsgSendAck:
		{
			if (m_handler == NULL) break;

			long fid = response.getRecvParameterValue(_T("MID"), 0);
			long indexack = response.getRecvParameterValue(_T("Index"), 0);

			CP2PFiles::pointer p2pFiles;
			if (!m_p2pfiles.find(p2pClient.get(), p2pFiles))
			{
				break;
			}

			CFileInfo::pointer fileInfo;
			if (!p2pFiles->m_files.find(fid, fileInfo))
			//if (!m_files.find(fid, fileInfo))
			{
				break;
			}

			fileInfo->m_indexs.remove(indexack);
			for (int i=2; i<=MAX_INDEXACK_SIZE; i++)
			{
				unsigned long tosendIndex2 = 0 ;
				char buffer[10];
				sprintf(buffer, "Index%d", i);
				long indexack2 = response.getRecvParameterValue(buffer, 0);
				if (indexack2 == 0)
				{
					break;
				}

				fileInfo->m_indexs.remove(indexack2);
			}
		}break;
	default:
		break;
	}

}

void ConnectManager::onP2PEvent(CDoP2PClientHandler::pointer p2pClient, CCgcData::pointer receiveData) ///接收数据提交给Dshow
{
	BOOST_ASSERT (p2pClient.get() != NULL);
	BOOST_ASSERT (receiveData.get() != NULL);

	if (m_clientHandler.get() == NULL) return;

	if (p2pClient->getP2PType() == CDoP2PClientHandler::P2P_VIDEO)
	{
		unsigned int timestamp = (unsigned int)receiveData->attach();
		timestamp = timestamp * (10000000.0/90000.0);
		m_clientHandler->ReceiveRemoteData(receiveData->data(), receiveData->size(), timestamp, true);
	}else if (p2pClient->getP2PType() == CDoP2PClientHandler::P2P_AUDIO)
	{
		unsigned int timestamp = (unsigned int)receiveData->attach();
		timestamp = timestamp * (10000000.0/8000.0);
		m_clientHandler->ReceiveRemoteData(receiveData->data(), receiveData->size(), timestamp, false);
	}
}

void ConnectManager::receiveAVData(CAVData::pointer receiveData) ////接收dshow数据发送
{
	BOOST_ASSERT (receiveData.get() != NULL);

	long sP2PType = 0;
	if (m_VideoFlag && receiveData->isVideo())
	{
		sP2PType = CDoP2PClientHandler::P2P_VIDEO;
	}else if (m_AudioFlag && !receiveData->isVideo())
	{
		sP2PType = CDoP2PClientHandler::P2P_AUDIO;
	}else
	{
		return;
	}
	
	char buffer[20];
	sprintf(buffer, ":%d", sP2PType);
	tstring sP2PKey(m_FriendName);
	sP2PKey.append(buffer);

	CDoP2PClientHandler::pointer pP2PClientHandler;
	if (m_userP2PClientMap.find(sP2PKey, pP2PClientHandler))
	{
		double rtpfrequency = receiveData->isVideo() ? 90000.0 : 8000.0;
		unsigned int timestamp = receiveData->timestamp() * (rtpfrequency/10000000.0);
		//pP2PClientHandler->doSetMediaType(98);
		pP2PClientHandler->doSendData(receiveData->data(), receiveData->size(), timestamp);
		//pP2PClientHandler->doSendP2PData(receiveData->data(), receiveData->size(), timestamp);
	}
}

bool ConnectManager::requestAVP2P(const tstring & sFriendName, P2PAVConnectType p2pConnectType)
{
	if (p2pConnectType == P2PAVConnect::CT_Video ||
		p2pConnectType == P2PAVConnect::CT_Both)
	{
		if (m_avsCgcProxy.avsRequestP2PUser(sFriendName, CDoP2PClientHandler::P2P_VIDEO) == -1)
			return false;

		for (int i=0; i<60; i++)
		{
			if (m_VideoFlag)
				break;
			Sleep(100);
		}
		if (!m_VideoFlag) return false;
	}

	if (p2pConnectType == P2PAVConnect::CT_Audio ||
		p2pConnectType == P2PAVConnect::CT_Both)
	{
		if (m_avsCgcProxy.avsRequestP2PUser(sFriendName, CDoP2PClientHandler::P2P_AUDIO) == -1)
			return false;

		for (int i=0; i<60; i++)
		{
			if (m_AudioFlag)
				break;
			Sleep(100);
		}
		return m_AudioFlag;
	}

	return true;
}

void ConnectManager::disconnectAVP2P(const tstring & sFriendId, P2PAVConnectType p2pConnectType)
{
	if (p2pConnectType == P2PAVConnect::CT_Video ||
		p2pConnectType == P2PAVConnect::CT_Both)
	{
		int ret = m_avsCgcProxy.avsDisconnectP2PUser(sFriendId, CDoP2PClientHandler::P2P_VIDEO, 0);
		if (ret == 0)
		{
			for (int i=0; i<60; i++)
			{
				if (!m_VideoFlag)
					break;
				Sleep(100);
			}
		}else
			m_VideoFlag = false;
	}

	if (p2pConnectType == P2PAVConnect::CT_Audio ||
		p2pConnectType == P2PAVConnect::CT_Both)
	{

		int ret = m_avsCgcProxy.avsDisconnectP2PUser(sFriendId, CDoP2PClientHandler::P2P_AUDIO, 0);
		if (ret == 0)
		{
			for (int i=0; i<60; i++)
			{
				if (!m_AudioFlag)
					break;
				Sleep(100);
			}
		}else
			m_AudioFlag = false;
	}

	//return !m_VideoFlag && !m_AudioFlag;
}
void ConnectManager::doStaProcSendData(ConnectManager * pManager, CDoP2PClientHandler::pointer p2pClient, CFileInfo::pointer fileInfo, bool bLastThread)
{
	BOOST_ASSERT (pManager != NULL);
	BOOST_ASSERT (p2pClient.get() != NULL);
	BOOST_ASSERT (fileInfo.get() != NULL);
	pManager->doProcSendData(p2pClient, fileInfo, bLastThread);
}

void ConnectManager::doStaProcSendAck(ConnectManager * pManager, CDoP2PClientHandler::pointer p2pClient, CFileInfo::pointer fileInfo)
{
	BOOST_ASSERT (pManager != NULL);
	BOOST_ASSERT (p2pClient.get() != NULL);
	BOOST_ASSERT (fileInfo.get() != NULL);
	pManager->doProcSendAck(p2pClient, fileInfo);
}

void ConnectManager::doProcSendData(CDoP2PClientHandler::pointer p2pClient, CFileInfo::pointer fileInfo, bool bLastThread)
{
	BOOST_ASSERT (p2pClient.get() != NULL);
	BOOST_ASSERT (fileInfo.get() != NULL);

	if (m_handler == NULL) return;

	CDoP2PClientHandler::pointer p2pClientHandler = p2pClient;


	tfstream filestream;
	filestream.open(fileInfo->filepath().c_str(), std::ios::in|std::ios::binary);
	if (!filestream.is_open())
	{
		return;
	}

	boost::xtime xt;
	bool bDoCheckThread = false;
	char buffer[MAX_PACKET_SIZE+1];
	size_t toSendSizeTotal = fileInfo->filesize();
	cgc::cgcAttachment::pointer attach(cgcAttachment::create());
	attach->setName("file");
	attach->setTotal(toSendSizeTotal);
	while (fileInfo->RT() == CFileInfo::RT_None && p2pClientHandler->getLocalP2PStatus() && p2pClientHandler->getRemoteP2PStatus())
	{
		unsigned long tosendIndex = 0 ;
		if (!fileInfo->m_sendindexs.front(tosendIndex))
		{
			bDoCheckThread = true;
			if (bLastThread)
			{
#ifdef WIN32
				Sleep(fileInfo->m_indexs.size() > 100 ? 1500 : 100);
#else
				usleep(fileInfo->m_indexs.size() > 100 ? 1500000 : 100000);
#endif
				boost::mutex::scoped_lock lock(fileInfo->m_indexs.mutex());
				CLockMap<unsigned long, bool>::iterator iter;
				for (iter=fileInfo->m_indexs.begin(); iter!=fileInfo->m_indexs.end(); iter++)
				{
					tosendIndex = iter->first;
					fileInfo->m_sendindexs.add(tosendIndex);
				}
			}

#ifdef WIN32
			Sleep(10);
#else
			usleep(10000);
#endif
			continue;
		}

		if (bDoCheckThread && !fileInfo->m_indexs.exist(tosendIndex))
		{
			continue;
		}

		size_t tosendSize = (tosendIndex+MAX_PACKET_SIZE) > toSendSizeTotal ? toSendSizeTotal-tosendIndex : MAX_PACKET_SIZE;

		filestream.seekg(tosendIndex, std::ios::beg);
		filestream.read(buffer, tosendSize);
		attach->setAttach((const unsigned char *)buffer, tosendSize);
		attach->setIndex(tosendIndex);

		p2pClientHandler->doBeginCallLock();
		p2pClientHandler->doAddParameter(CGC_PARAMETER(_T("MID"), fileInfo->did()));
		p2pClientHandler->doSendAppCall(const_CallSign_MsgSend, const_Api_MsgSend, attach);

		if (tosendIndex == 0)
		{
			// first
			float percent = 0.0;
			m_handler->onSendFileData(p2pClientHandler->getP2PUser(), fileInfo->fid(), percent);
		}else
		{
			if (fileInfo->icounter() > 380)
			{
				fileInfo->resetcounter();
				float percent = (fileInfo->indexCount()-fileInfo->m_indexs.size()) * 100.0 / fileInfo->indexCount();
				m_handler->onSendFileData(p2pClientHandler->getP2PUser(), fileInfo->fid(), percent);
			}
		}

		//boost::xtime_get(&xt, boost::TIME_UTC);		// initialize xt with current time
		//if (xt.nsec + WAIT_NANOSECONDS < 1000000000)
		//{
		//	xt.nsec += WAIT_NANOSECONDS;							// change xt to next second
		//}else
		//{
		//	xt.sec += 1;
		//	xt.nsec -= (1000000000-WAIT_NANOSECONDS);							// change xt to next second
		//}
		//boost::thread::sleep(xt);					// do sleep

		//if (bDoCheckThread)
		{
#ifdef WIN32
			Sleep(1);
#else
			usleep(1000);
#endif
		}

	}
	filestream.close();

	if (bLastThread && (fileInfo->RT() == CFileInfo::RT_Arrived || fileInfo->m_indexs.empty()))
	{
		// OK
		float percent = 100.0;
		m_handler->onSendFileData(p2pClientHandler->getP2PUser(), fileInfo->fid(), percent);
		this->disconnectP2P(p2pClientHandler);
	}
}


void ConnectManager::doProcSendAck(CDoP2PClientHandler::pointer p2pClient, CFileInfo::pointer fileInfo)
{
	BOOST_ASSERT (p2pClient.get() != NULL);
	BOOST_ASSERT (fileInfo.get() != NULL);

	if (m_handler == NULL) return;

	while (fileInfo->RT() == CFileInfo::RT_None && p2pClient->getLocalP2PStatus() && p2pClient->getRemoteP2PStatus())
	{
		unsigned long tosendIndex = 0 ;
		if (!fileInfo->m_sendindexs.front(tosendIndex))
		{
#ifdef WIN32
			Sleep(1);
#else
			usleep(1000);
#endif
			continue;
		}

		p2pClient->doBeginCallLock();
		p2pClient->doAddParameter(CGC_PARAMETER(_T("MID"), fileInfo->did()));
		p2pClient->doAddParameter(CGC_PARAMETER(_T("Index"), (long)tosendIndex));
		for (int i=2; i<=MAX_INDEXACK_SIZE; i++)
		{
			unsigned long tosendIndex2 = 0 ;
			if (!fileInfo->m_sendindexs.front(tosendIndex2))
			{
				break;
			}
			
			char buffer[10];
			sprintf(buffer, "Index%d", i);
			p2pClient->doAddParameter(CGC_PARAMETER(buffer, (long)tosendIndex2));
		}
		p2pClient->doSendAppCall(const_CallSign_MsgSendAck, const_Api_MsgSendAck);

#ifdef WIN32
		Sleep(10);
#else
		usleep(10000);
#endif
	}
}

short ConnectManager::getThreadsBysize(unsigned long fileSize) const
{
	short sendThreadSize = fileSize < 50*1024 ? 1 : 3;
	if (fileSize > 10*1024*1024)		// > 10MB
		sendThreadSize = 13;
	else if (fileSize > 3*1024*1024)	// 3MB -> 10MB
		sendThreadSize = 10;
	else if (fileSize > 1024*1024)		// 1MB -> 3MB
		sendThreadSize = 6;

	return sendThreadSize;
}

long ConnectManager::SendFile(const tstring & sFriendID, const tstring & filepath, const tstring & filename)
{
	CFileInfo::pointer fileInfo = CFileInfo::create(filepath);
	tfstream & filestream = fileInfo->fs();
	filestream.open(filepath.c_str(), std::ios::in|std::ios::binary);
	if (!filestream.is_open())
	{
		return 0;
	}

	int fid = ++m_currentId;

	fileInfo->fid(fid);

	filestream.seekg(0, std::ios::end);
	unsigned int nFilesize = filestream.tellg();
	fileInfo->filesize(nFilesize);
	filestream.seekg(0, std::ios::beg);
	filestream.close();
	
	//m_files.insert(fid, fileInfo);
	m_filestemp.insert(fid, fileInfo);

	int ret = m_avsCgcProxy.avsRequestP2PUser(sFriendID, CDoP2PClientHandler::P2P_FILE, fid);
	if (ret == -1)
	{
		return 0;
	}else if (ret == 1)
	{
		m_avsCgcProxy.avsDisconnectP2PUser(sFriendID, CDoP2PClientHandler::P2P_FILE, fid);
		return 0;
	}

	bool result = false;
	for (int i=0; i<60; i++)
	{
#ifdef WIN32
		Sleep(100);
#else
		usleep(100000);
#endif
		if (fileInfo->p2pAck())
		{
			result = true;
			break;
		}
	}

	if (result)
	{
		CP2PFiles::pointer p2pFiles;
		if (!m_fidfiles.find(fid, p2pFiles))
		{
			return 0;
		}
		CDoP2PClientHandler::pointer p2pClient = p2pFiles->getP2PClient();

		p2pClient->doBeginCallLock();
		p2pClient->doAddParameter(CGC_PARAMETER(_T("MID"), fid));
		p2pClient->doAddParameter(CGC_PARAMETER(_T("Type"), 11));
		p2pClient->doAddParameter(CGC_PARAMETER(_T("Name"), filename));
		p2pClient->doAddParameter(CGC_PARAMETER(_T("Size"), (double)nFilesize));
		p2pClient->doSendAppCall(const_CallSign_MsgRequest, const_Api_MsgRequest);
		return fid;
	}
	return 0;
}

bool ConnectManager::AcceptFile(long fid, const tstring & savetofilepath, const tstring & filename)
{
	CP2PFiles::pointer p2pFiles;
	if (!m_fidfiles.find(fid, p2pFiles))
	{
		return false;
	}
	CDoP2PClientHandler::pointer p2pClient = p2pFiles->getP2PClient();

	CFileInfo::pointer fileInfo;
	if (!p2pFiles->m_files.find(fid, fileInfo))
	{
		return false;
	}
	fileInfo->filepath(savetofilepath);

	tfstream & filestream = fileInfo->fs();
	filestream.open(savetofilepath.c_str(), std::ios::out|std::ios::binary);
	if (!filestream.is_open())
	{
		return false;
	}

	char * buffer = new char[fileInfo->filesize()];
	memset(buffer, 0, fileInfo->filesize());
	filestream.write(buffer, fileInfo->filesize());
	filestream.flush();
	delete[] buffer;


	short sendThreadSize = fileInfo->filesize() < 50*1024 ? 1 : 3;
	for (short i=1; i<=sendThreadSize; i++)
	{
		boost::thread * ackThread = new boost::thread(boost::bind(doStaProcSendAck, this, p2pClient, fileInfo));
		fileInfo->m_sendthreads.add(ackThread);
	}

	// accept file
	p2pClient->doBeginCallLock();
	p2pClient->doAddParameter(CGC_PARAMETER(_T("MID"), fileInfo->did()));
	p2pClient->doAddParameter(CGC_PARAMETER(_T("Response"), 1));
	p2pClient->doSendAppCall(const_CallSign_MsgResponse, const_Api_MsgResponse);

	return true;
}

bool ConnectManager::RejectFile(long fid)
{
	CP2PFiles::pointer p2pFiles;
	if (!m_fidfiles.find(fid, p2pFiles))
	{
		return false;
	}
	CDoP2PClientHandler::pointer p2pClient = p2pFiles->getP2PClient();

	CFileInfo::pointer fileInfo;
	if (!p2pFiles->m_files.find(fid, fileInfo, true))
	{
		return false;
	}
	fileInfo->RT(CFileInfo::RT_Rejected);

	if (p2pClient->getLocalP2PStatus() && p2pClient->getRemoteP2PStatus())
	{
		p2pClient->doBeginCallLock();
		p2pClient->doAddParameter(CGC_PARAMETER(_T("MID"), fileInfo->did()));
		p2pClient->doAddParameter(CGC_PARAMETER(_T("Response"), 2));
		p2pClient->doSendAppCall(const_CallSign_MsgResponse, const_Api_MsgResponse);
	}

	fileInfo->fs().close();
	namespace fs = boost::filesystem;
	fs::path pathFile(fileInfo->filepath(), fs::native);
	if (fs::exists(pathFile))
	{
		fs::remove(pathFile);
	}

	if (p2pFiles->m_files.empty())
	{
		m_p2pfiles.remove(p2pClient.get());
	}
	return true;
}

bool ConnectManager::CancelFile(long fid)
{
	CP2PFiles::pointer p2pFiles;
	if (!m_fidfiles.find(fid, p2pFiles))
	{
		return false;
	}
	CDoP2PClientHandler::pointer p2pClient = p2pFiles->getP2PClient();

	CFileInfo::pointer fileInfo;
	if (!p2pFiles->m_files.find(fid, fileInfo, true))
	{
		return false;
	}
	fileInfo->RT(CFileInfo::RT_Canceled);

	if (p2pClient->getLocalP2PStatus() && p2pClient->getRemoteP2PStatus())
	{
		p2pClient->doBeginCallLock();
		p2pClient->doAddParameter(CGC_PARAMETER(_T("MID"), fileInfo->did()));
		p2pClient->doAddParameter(CGC_PARAMETER(_T("Response"), 3));
		p2pClient->doSendAppCall(const_CallSign_MsgResponse, const_Api_MsgResponse);
	}

	if (p2pFiles->m_files.empty())
	{
		m_p2pfiles.remove(p2pClient.get());
	}
	return true;
}

bool ConnectManager::requestP2P(const tstring & sFriendName,long nP2PType, long nP2PParam)
{
	return m_avsCgcProxy.avsRequestP2PUser(sFriendName, nP2PType, nP2PParam) == 0;
}

bool ConnectManager::disconnectP2P(const tstring & sFriendId, long nP2PType, long nP2PParam)
{
	return m_avsCgcProxy.avsDisconnectP2PUser(sFriendId, nP2PType, nP2PParam) == 0;
}

bool ConnectManager::disconnectP2P(CDoP2PClientHandler::pointer p2pClient)
{
	return m_avsCgcProxy.avsDisconnectP2PUser(p2pClient) == 0;
}
