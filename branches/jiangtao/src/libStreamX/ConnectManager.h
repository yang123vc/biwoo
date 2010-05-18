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

// ConnectManager.h file here
#ifndef __ConnectManager_h__
#define __ConnectManager_h__

#include "../libStreamClient/libStreamClient.h"
#include "../ThirdParty/libDS/libDS.h"
#include "libP2PAVDef.h"
#include "libstreamxhandler.h"

const tstring const_Api_MsgSend					= _T("6101");
const ULONG const_CallSign_MsgSend				= 0x6101;
const tstring const_Api_MsgSendAck				= _T("6102");
const ULONG const_CallSign_MsgSendAck			= 0x6102;
const tstring const_Api_MsgRequest				= _T("6111");
const ULONG const_CallSign_MsgRequest			= 0x6111;
const tstring const_Api_MsgRequestAck			= _T("6112");
const ULONG const_CallSign_MsgRequestAck		= 0x6112;
const tstring const_Api_MsgResponse				= _T("6113");
const ULONG const_CallSign_MsgResponse			= 0x6113;

class CFileInfo
{
public:
	enum ResultType
	{
		RT_None	= 0x1
		, RT_Rejected
		, RT_Canceled
		, RT_Arrived
	};

	typedef boost::shared_ptr<CFileInfo> pointer;
	static CFileInfo::pointer create(const std::string & filepath)
	{
		return CFileInfo::pointer(new CFileInfo(filepath));
	}

	tfstream & fs(void) {return m_filestream;}

	void fid(long newv) {m_fid = newv;}
	long fid(void) const {return m_fid;}
	void did(long newv) {m_did = newv;}
	long did(void) const {return m_did;}

	void filepath(const std::string & newv) {m_filepath = newv;}
	const std::string & filepath(void) const {return m_filepath;}

	void filesize(size_t newv) {m_filesize = newv;}
	size_t filesize(void) const {return m_filesize;}

	void p2pAck(bool newv) {m_p2pAck = newv;}
	bool p2pAck(void) const {return m_p2pAck;}

	void RT(ResultType newv) {m_resultType = newv;}
	ResultType RT(void) const {return m_resultType;}

	int icounter(void) {return ++m_counter;}
	void resetcounter(void) {m_counter = 0;}

	void indexCount(size_t newv) {m_indexCount = newv;}
	size_t indexCount(void) const {return m_indexCount;}

	CLockMap<unsigned long, bool>	m_indexs;
	CLockList<unsigned long>	m_sendindexs;	// for send hreads

	CLockListPtr<boost::thread*> m_sendthreads;

public:
	CFileInfo(const std::string & filepath)
		: m_filepath(filepath), m_filesize(0)
		, m_p2pAck(false), m_fid(0), m_did(0)
		, m_resultType(RT_None)
		, m_indexCount(0), m_counter(0)
	{
	}
	~CFileInfo(void)
	{
		m_filestream.close();
		m_indexs.clear();
		m_sendindexs.clear();

		CLockListPtr<boost::thread*>::iterator pIterThread;
		for (pIterThread=m_sendthreads.begin(); pIterThread!=m_sendthreads.end(); pIterThread++)
		{
			boost::thread * sendThread = *pIterThread;
			sendThread->join();
		}
		m_sendthreads.clear();
	}
private:
	bool m_p2pAck;
	long m_fid;
	long m_did;
	std::string m_filepath;
	size_t m_filesize;
	tfstream m_filestream;

	ResultType m_resultType;

	size_t	m_indexCount;
	int		m_counter;

};

class CP2PFiles
{
public:
	typedef boost::shared_ptr<CP2PFiles> pointer;
	static CP2PFiles::pointer create(CDoP2PClientHandler::pointer p2pClient)
	{
		return CP2PFiles::pointer(new CP2PFiles(p2pClient));
	}

	CDoP2PClientHandler::pointer getP2PClient(void) const {return m_p2pClient;}

public:
	CP2PFiles(CDoP2PClientHandler::pointer p2pClient)
		: m_p2pClient(p2pClient)
	{
		BOOST_ASSERT (m_p2pClient.get() != NULL);
	}
	~CP2PFiles(void)
	{
		m_files.clear();
	}

	CLockMap<long, CFileInfo::pointer> m_files;							// fid ->

private:
	CDoP2PClientHandler::pointer m_p2pClient;

};

class ConnectManager
	: public CStreamHandler
	, public CP2PHandler
	, public OnDSHandler
{
public:
	ConnectManager(void);
	~ConnectManager(void);

	void setP2PAVHandler(OnP2PHandler * handler) {m_handler = handler;}
	bool connectStreamServer(const CCgcAddress & ServerAddr,const CCgcAddress & RtpAddr,const CCgcAddress & UdpAddr);
	void disconnectStreamServer(void);
	bool isConnectedStreamServer(void) const;

	bool Login(const tstring & Name);
	void Logout(void);
	bool IsLogined(void);

	DoDSHandler * startServer(const CAVParameter & avp);
	void stopServer(void);
	DoDSHandler * startClient(const CAVParameter & avp);
	void clientVideoReversal(void);
	void stopClient(DoDSHandler * pDoDSHandler);
	void stopClient(void);

	bool requestAVP2P(const tstring & sFriendName, P2PAVConnectType p2pConnectType);  
	void disconnectAVP2P(const tstring & sFriendId, P2PAVConnectType p2pConnectType);

	long SendFile(const tstring & sFriendID, const tstring & filepath, const tstring & filename);
	bool AcceptFile(long fid, const tstring & savetofilepath, const tstring & filename);
	bool RejectFile(long fid);
	bool CancelFile(long fid);

	bool requestP2P(const tstring & sFriendName,long nP2PType, long nP2PParam);  
	bool disconnectP2P(const tstring & sFriendId, long nP2PType, long nP2PParam);
	bool disconnectP2P(CDoP2PClientHandler::pointer p2pClient);

	void doProcSendData(CDoP2PClientHandler::pointer p2pClient, CFileInfo::pointer fileInfo, bool bLastThread);
	void doProcSendAck(CDoP2PClientHandler::pointer p2pClient, CFileInfo::pointer fileInfo);

protected:
	// CStreamHandler handler
	virtual void onUserInfo(const tstring & sUserId, const tstring & sUserName);
	virtual void onUserLogined(const tstring & sUserName);
	virtual void onUserLogouted(const tstring & sUserName);
	virtual void onP2PRequestResult(const tstring & sFromUser, long nP2PType, long resuleValue);
	virtual CP2PHandler * onP2PUserAck(CDoP2PClientHandler::pointer p2pClient, bool & disableSotpParse);
	virtual void onP2PRequestTimeout(CDoP2PClientHandler::pointer p2pClient);
	virtual void onP2PUserDisconnect(CDoP2PClientHandler::pointer p2pClient);

	// P2PHandler handler
	virtual void onP2PEvent(CDoP2PClientHandler::pointer p2pClient, const cgcParser & response);
	virtual void onP2PEvent(CDoP2PClientHandler::pointer p2pClient, CCgcData::pointer receiveData);
	// OnDSHandler handler
	virtual void receiveAVData(CAVData::pointer receiveData);

	static void doStaProcSendData(ConnectManager * pManager, CDoP2PClientHandler::pointer p2pClient, CFileInfo::pointer fileInfo, bool bLastThread);
	static void doStaProcSendAck(ConnectManager * pManager, CDoP2PClientHandler::pointer p2pClient, CFileInfo::pointer fileInfo);

	short getThreadsBysize(unsigned long fileSize) const;

private:
	unsigned long m_currentId;
	tstring m_FriendName;
	//CAvsCgcProxy m_avsCgcProxy;
	CStreamClient m_avsCgcProxy;
	CLockMap<tstring, CDoP2PClientHandler::pointer> m_userP2PClientMap;
	CLockMap<long, CP2PFiles::pointer> m_fidfiles;			// fid ->

	CLockMap<long, CFileInfo::pointer> m_filestemp;							// fid ->
	CLockMap<void*, CP2PFiles::pointer> m_p2pfiles;						// CDoP2PClientHandler* ->

	CDSProxy m_dsproxy;
	DoDSHandler * m_serverHandler;
	DoDSHandler * m_clientHandler;
	CAVParameter m_avpClient;

	bool m_VideoFlag;  ////视频通道打开标志
	bool m_AudioFlag;  ////音频通道打开标志
	OnP2PHandler * m_handler;
};

#endif // __ConnectManager_h__
