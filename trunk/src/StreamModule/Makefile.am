lib_LTLIBRARIES = libStreamModule.la
THESOURCES = avconfer/AVConference.cpp avconfer/ConferenceInfo.cpp avconfer/MemberData.cpp \
		AVSProxy.cpp AccountMgr.cpp P2PMgr.cpp SAVConference.cpp StreamModule.cpp

libStreamModule_la_SOURCES = $(THESOURCES)
libStreamModule_la_LIBADD = $(libdir)/libCGCBase.a $(libdir)/libCGCClass.a \
	../ThirdParty/libCoder/libCoder.a \
	../ThirdParty/libG729a/libG729a.a \
	../ThirdParty/libSIP/libSIP.a $(libdir)/libeXosip2.a \
	$(libdir)/libosip2.a $(libdir)/libosipparser2.a

AM_CFLAGS=-fPIC -shared -Wall -ldns
AM_CXXFLAGS=-fPIC -shared -Wall -ldns
