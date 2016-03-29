#Environment
MKDIR = @mkdir
CP = cp
GREP = grep
CC = gcc
CXX = g++

#Object Directory
OBJECTDIR = .

#Object files
OBJECTFILES = ${OBJECTDIR}/_ext/main.o \
	      ${OBJECTDIR}/_ext/Live.o \
	      ${OBJECTDIR}/_ext/LiveServer.o \
	      ${OBJECTDIR}/_ext/LiveModule.o \
	      ${OBJECTDIR}/_ext/LiveRelayModule.o

# C Compiler Flags
CFLAGS = -g -c -Wall

# C++ Compiler Flags
CXXFLAGS = -g -c -Wall

${OBJECTDIR}/main : ${OBJECTFILES}
	$(CXX) -g -Wall -o ${OBJECTDIR}/main ${OBJECTFILES}

${OBJECTDIR}/_ext/main.o : ${OBJECTDIR}/main.cpp
	${MKDIR} -p ${OBJECTDIR}/_ext/
	$(CXX) $(CXXFLAGS) -o ${OBJECTDIR}/_ext/main.o ${OBJECTDIR}/main.cpp

${OBJECTDIR}/_ext/Live.o : ${OBJECTDIR}/Live.cpp
	${MKDIR} -p ${OBJECTDIR}/_ext/
	$(CXX) $(CXXFLAGS) -o ${OBJECTDIR}/_ext/Live.o ${OBJECTDIR}/Live.cpp

${OBJECTDIR}/_ext/LiveServer.o : ${OBJECTDIR}/LiveServer.cpp
	${MKDIR} -p ${OBJECTDIR}/_ext/
	$(CXX) $(CXXFLAGS) -o ${OBJECTDIR}/_ext/LiveServer.o ${OBJECTDIR}/LiveServer.cpp

${OBJECTDIR}/_ext/LiveModule.o : ${OBJECTDIR}/LiveModule.cpp
	${MKDIR} -p ${OBJECTDIR}/_ext/
	$(CXX) $(CXXFLAGS) -o ${OBJECTDIR}/_ext/LiveModule.o ${OBJECTDIR}/LiveModule.cpp

${OBJECTDIR}/_ext/LiveRelayModule.o : ${OBJECTDIR}/LiveRelayModule.cpp
	${MKDIR} -p ${OBJECTDIR}/_ext/
	$(CXX) $(CXXFLAGS) -o ${OBJECTDIR}/_ext/LiveRelayModule.o ${OBJECTDIR}/LiveRelayModule.cpp

PHONY : clean
clean :
	-rm -rf ${OBJECTDIR}/_ext ${OBJECTDIR}/main
