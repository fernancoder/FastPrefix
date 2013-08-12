bin_PROGRAMS = fastprefix

fastprefix_SOURCES = \
    fastprefix.cpp \
    FastPrefixDispatcher.cpp \
    FastPrefixDispatcher.h \
    FastPrefixServer.cpp \
    FastPrefixServer.h \
    FastPrefixThread.cpp \
    FastPrefixThread.h \
    QueryAction.cpp \
    QueryAction.h \
    ReloadAction.cpp \
    ReloadAction.h

fastprefix_CXXFLAGS = -I$(top_srcdir)/datamanager

fastprefix_LDADD = \
    ../datamanager/libdatamanager.a \
    -ldaework \
    -lpthread

AM_LDFLAGS = -lpthread

