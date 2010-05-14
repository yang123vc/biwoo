lib_LTLIBRARIES = libbiwoo.la
THESOURCES = AVSProxy.cpp AccountMgr.cpp biwoo.cpp DialogMgr.cpp \
	MessageMgr.cpp sendresponse.cpp

libbiwoo_la_SOURCES = $(THESOURCES)
libbiwoo_la_LIBADD = $(libdir)/libCGCBase.a $(libdir)/libCGCClass.a \
	$(libdir)/libbodb.a $(libdir)/libsqlparser.a 

AM_CFLAGS=-fPIC -shared -Wall
AM_CXXFLAGS=-fPIC -shared -Wall
