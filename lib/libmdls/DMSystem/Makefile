INCLUDES = -I.
PREFIX = /usr/local
LIBDIR = $(PREFIX)/lib
##### Change the following for your environment:
COMPILE_OPTS =		 $(COMPILE.cc) -O2 -D__USE_MAX_PRINTF__  -D__PTHREADS__ -D__PTHREADS_MUTEXES__ -DCOMMON_UTILITIES_LIB -DDSS_USE_API_CALLBACKS -D_REENTRANT -D__USE_POSIX -D__linux__ -I. -include PlatformHeader.h

C =			c
C_COMPILER =		cc
C_FLAGS =		$(COMPILE_OPTS) -pipe -Wall -Wno-format-y2k $(CPPFLAGS) $(CFLAGS)
CPP =			cpp
CPLUSPLUS_COMPILER =	c++
CPLUSPLUS_FLAGS =	$(COMPILE_OPTS) -pipe -Wall -Wno-format-y2k $(CPPFLAGS) $(CXXFLAGS)
OBJ =			o
LINK =			c++ -o
LINK_OPTS =		-L. $(LDFLAGS)
CONSOLE_LINK_OPTS =	$(LINK_OPTS)
LIBRARY_LINK =		ar cr 
LIBRARY_LINK_OPTS =	
LIB_SUFFIX =			a
LIBS_FOR_CONSOLE_APPLICATION =
LIBS_FOR_GUI_APPLICATION =
EXE =
##### End of variables to change


QTFILE_OBJS = StringParser.o \
			StrPtrLen.o \
			MyAssert.o \
			ev.o \
			OS.o \
			OSRef.o \
			OSQueue.o \
			OSHeap.o \
			OSThread.o \
			OSMutex.o \
			OSMutexRW.o \
			OSCond.o \
			OSFileSource.o \
			InternalStdLib.o \
			QTSS_Private.o \
			RTPMetaInfoPacket.o \
			DateTranslator.o \
			QTAtom.o \
			QTAtom_dref.o \
			QTAtom_elst.o \
			QTAtom_hinf.o \
			QTAtom_mdhd.o \
			QTAtom_mvhd.o \
			QTAtom_stco.o \
			QTAtom_stsc.o \
			QTAtom_stsd.o \
			QTAtom_stss.o \
			QTAtom_stsz.o \
			QTAtom_stts.o \
			QTAtom_tkhd.o \
			QTAtom_tref.o \
			QTFile.o \
			QTFile_FileControlBlock.o \
			QTHintTrack.o \
			QTRTPFile.o \
			QTTrack.o

main:main.o $(QTFILE_OBJS)
	g++ -o main main.o $(QTFILE_OBJS) -lpthread -ldl -lstdc++ -lm
#mytest.o:$(QTFILE_OBJS)
#	g++ -o mytest  $(QTFILE_OBJS)
main.o:main.cpp
	$(COMPILE_OPTS) main.cpp

MyAssert.o:
	$(COMPILE_OPTS) MyAssert.cpp
StringParser.o:
	$(COMPILE_OPTS) StringParser.cpp
StrPtrLen.o:
	$(COMPILE_OPTS) StrPtrLen.cpp
ev.o:
	$(COMPILE_OPTS) ev.cpp
OS.o:
	$(COMPILE_OPTS) OS.cpp
OSRef.o:
	$(COMPILE_OPTS) OSRef.cpp
OSQueue.o:
	$(COMPILE_OPTS) OSQueue.cpp
OSHeap.o:
	$(COMPILE_OPTS) OSHeap.cpp
OSThread.o:
	$(COMPILE_OPTS) OSThread.cpp	
OSMutex.o:
	$(COMPILE_OPTS) OSMutex.cpp
OSMutexRW.o:
	$(COMPILE_OPTS) OSMutexRW.cpp
OSCond.o:
	$(COMPILE_OPTS) OSCond.cpp
OSFileSource.o:
	$(COMPILE_OPTS) OSFileSource.cpp
		
InternalStdLib.o:
	$(COMPILE.cc) -O2 -D__USE_POSIX -D__linux__ -I. -include PlatformHeader.h InternalStdLib.cpp	
	
QTSS_Private.o:
	$(COMPILE_OPTS) QTSS_Private.cpp
RTPMetaInfoPacket.o:
	$(COMPILE_OPTS) RTPMetaInfoPacket.cpp
DateTranslator.o:
	$(COMPILE_OPTS) DateTranslator.cpp
		
QTAtom.o:
	$(COMPILE_OPTS) QTAtom.cpp	
		
QTAtom_dref.o:	
	$(COMPILE_OPTS) QTAtom_dref.cpp	
	
QTAtom_elst.o:
	$(COMPILE_OPTS) QTAtom_elst.cpp	
	
QTAtom_hinf.o:
	$(COMPILE_OPTS) QTAtom_hinf.cpp	
	
QTAtom_mdhd.o:
	$(COMPILE_OPTS) QTAtom_mdhd.cpp
	
QTAtom_mvhd.o:
	$(COMPILE_OPTS) QTAtom_mvhd.cpp
	
QTAtom_stco.o:
	$(COMPILE_OPTS) QTAtom_stco.cpp
	
QTAtom_stsc.o:
	$(COMPILE_OPTS) QTAtom_stsc.cpp
QTAtom_stsd.o:
	$(COMPILE_OPTS) QTAtom_stsd.cpp
QTAtom_stss.o:
	$(COMPILE_OPTS) QTAtom_stss.cpp
QTAtom_stsz.o:
	$(COMPILE_OPTS) QTAtom_stsz.cpp
QTAtom_stts.o:
	$(COMPILE_OPTS) QTAtom_stts.cpp
QTAtom_tkhd.o:
	$(COMPILE_OPTS) QTAtom_tkhd.cpp
QTAtom_tref.o:
	$(COMPILE_OPTS) QTAtom_tref.cpp
	
QTFile.o:
	$(COMPILE_OPTS) QTFile.cpp
QTFile_FileControlBlock.o:
	$(COMPILE_OPTS) QTFile_FileControlBlock.cpp
QTHintTrack.o:
	$(COMPILE_OPTS) QTHintTrack.cpp
QTRTPFile.o:
	$(COMPILE_OPTS) QTRTPFile.cpp
QTTrack.o:
	$(COMPILE_OPTS) QTTrack.cpp

clean:
	-rm -rf *.$(OBJ) *.o.d $(ALL) core *.core *~ 

