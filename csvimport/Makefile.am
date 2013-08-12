bin_PROGRAMS = csvimport

csvimport_SOURCES = \
    include.h \
    csvimport.cpp \
    Process.cpp \
    Process.h \
    Util.cpp \
    Util.h

csvimport_CXXFLAGS = -I$(top_srcdir)/datamanager

csvimport_LDADD = \
    ../datamanager/libdatamanager.a \
    -ldaework \
    -lpthread
