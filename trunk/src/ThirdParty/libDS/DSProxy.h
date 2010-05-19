// DSProxy.h file here
#ifndef __DSProxy_h__
#define __DSProxy_h__

#include "dlldefine.h"
#include "libdshandler.h"
#include "AVParameter.h"

class LIBDS_CLASS CDSProxy
{
public:
	// Server
	bool isServerStarted(void) const;
	DoDSHandler::pointer startServer(const CAVParameter & parameter, OnDSHandler * pDSHandler = 0);
	DoDSHandler::pointer startClient(const CAVParameter & parameter);
	void stopDSHandler(DoDSHandler::pointer pDoHandler);
	void stopAllClient(bool bBothStopServer = true);

public:
	CDSProxy(void);
	~CDSProxy(void);

};

#endif // __DSProxy_h__
