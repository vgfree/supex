/*
 *
 * @APPLE_LICENSE_HEADER_START@
 *
 * Copyright (c) 1999-2008 Apple Inc.  All Rights Reserved.
 *
 * This file contains Original Code and/or Modifications of Original Code
 * as defined in and that are subject to the Apple Public Source License
 * Version 2.0 (the 'License'). You may not use this file except in
 * compliance with the License. Please obtain a copy of the License at
 * http://www.opensource.apple.com/apsl/ and read it before using this
 * file.
 *
 * The Original Code and all software distributed under the License are
 * distributed on an 'AS IS' basis, WITHOUT WARRANTY OF ANY KIND, EITHER
 * EXPRESS OR IMPLIED, AND APPLE HEREBY DISCLAIMS ALL SUCH WARRANTIES,
 * INCLUDING WITHOUT LIMITATION, ANY WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, QUIET ENJOYMENT OR NON-INFRINGEMENT.
 * Please see the License for the specific language governing rights and
 * limitations under the License.
 *
 * @APPLE_LICENSE_HEADER_END@
 *
 */

/*
 *    File:       QTSSFileModule.h
 *
 *    Contains:   Content source module that uses the QTFileLib to serve Hinted QuickTime
 *                files to clients.
 *
 *
 */

#ifndef _RTPFILEMODULE_H_
#define _RTPFILEMODULE_H_

#include "QTSS.h"

extern "C"
{
EXPORT QTSS_Error QTSSFileModule_Main(void *inPrivateArgs);
}

// ygl add 2016-01-07
#include "TCPSocket.h"

class CDMSysSocket
{
public:

	CDMSysSocket();
	virtual ~CDMSysSocket() {}

	void Set(UInt32 hostAddr, UInt16 hostPort)
	{
		fHostAddr = hostAddr; fHostPort = hostPort;
	}

	OS_Error Send(char *inData, const UInt32 inLength);

	virtual OS_Error SendV(iovec *inVec, UInt32 inNumVecs) = 0;

	virtual OS_Error Read(void *inBuffer, const UInt32 inLength, UInt32 *outRcvLen) = 0;

	virtual UInt32 GetLocalAddr() = 0;

	UInt32 GetHostAddr()
	{
		return fHostAddr;
	}

	UInt32 GetEventMask()
	{
		return fEventMask;
	}

	Socket *GetSocket()
	{
		return fSocketP;
	}

	virtual void SetRcvSockBufSize(UInt32 inSize) = 0;

protected:
	OS_Error Connect(TCPSocket *inSocket);

	OS_Error Open(TCPSocket *inSocket);

	OS_Error SendSendBuffer(TCPSocket *inSocket);

	UInt32  fHostAddr;
	UInt16  fHostPort;

	Socket  *fSocketP;
	UInt32  fEventMask;

	enum
	{
		kSendBufferLen = 2048
	};

	char            fSendBuf[kSendBufferLen + 1];
	StrPtrLen       fSendBuffer;
	UInt32          fSentLength;
};

//////////////////CTCPDMSysSocket declare//////////////////
class CTCPDMSysSocket : public CDMSysSocket
{
public:

	CTCPDMSysSocket(UInt32 inSocketType);
	virtual ~CTCPDMSysSocket() {}

	virtual OS_Error SendV(iovec *inVec, UInt32 inNumVecs);

	virtual OS_Error Read(void *inBuffer, const UInt32 inLength, UInt32 *outRcvLen);

	virtual void SetRcvSockBufSize(UInt32 inSize)
	{
		fSocket.SetSocketRcvBufSize(inSize);
	}

	virtual void SetOptions(int sndBufSize = 8192, int rcvBufSize = 1024);

	virtual UInt32 GetLocalAddr()
	{
		return fSocket.GetLocalAddr();
	}

	virtual UInt16 GetLocalPort()
	{
		return fSocket.GetLocalPort();
	}

private:

	TCPSocket fSocket;
};

//////////////////CHTTPDMSysSocket declare//////////////////
class CHTTPDMSysSocket : public CDMSysSocket
{
public:

	CHTTPDMSysSocket(const StrPtrLen &inURL, UInt32 inCookie, UInt32 inSocketType);
	virtual ~CHTTPDMSysSocket();

	virtual OS_Error SendV(iovec *inVec, UInt32 inNumVecs);

	virtual OS_Error Read(void *inBuffer, const UInt32 inLength, UInt32 *outRcvLen);

	virtual UInt32 GetLocalAddr()
	{
		return fGetSocket.GetLocalAddr();
	}

	virtual void SetRcvSockBufSize(UInt32 inSize)
	{
		fGetSocket.SetSocketRcvBufSize(inSize);
	}

	void ClosePost()
	{
		delete fPostSocket; fPostSocket = NULL;
	}

private:
	void EncodeVec(iovec *inVec, UInt32 inNumVecs);

private:
	StrPtrLen       fURL;
	UInt32          fCookie;
	UInt32          fSocketType;
	UInt32          fGetReceived;

	TCPSocket       fGetSocket;
	TCPSocket       *fPostSocket;
};

//////////////////CDMSysClient declare//////////////////
//////////////////进行Send消息处理/////////////
class CDMSysClient {
public:
	CDMSysClient(CDMSysSocket * insocket);
	~CDMSysClient() {}
	void InitDMSysClient();

	void Connect2DMSys();

public:
	UInt32 GetDMSysIP()
	{
		return m_iDMSysIPCfg;
	}

	void SetDMSysIP(UInt32 ip)
	{
		m_iDMSysIPCfg = ip;
	}

	UInt16 GetDMSysPort()
	{
		return m_iDMSysPortCfg;
	}

	void SetDMSysPort(UInt16 port)
	{
		m_iDMSysPortCfg = port;
	}

	void SendData();

public:
	OSMutex *GetMutex()
	{
		return &fMutex;
	}

	OS_Error ReceiveResponse();

private:
	UInt32  m_iDMSysIPCfg;			// 主机序
	UInt16  m_iDMSysPortCfg;		// 主机序
	bool    m_bConnect;

	CDMSysSocket    *m_pcDMSysSocket;
	OSMutex         fMutex;
};

//////////////////CDMSysSource//////////////////////////
class CDMSysSource {
public:
	CDMSysSource() : fHostAddr(0), fHostPort(0), fLocalAddr(0), fDMSysClient(NULL), fDMSysTCPSocket(NULL) {}
	~CDMSysSource() {}
public:
	CDMSysClient *GetDMSysClient()
	{
		return fDMSysClient;
	}

	CTCPDMSysSocket *GetDMSysTCPSocket()
	{
		return fDMSysTCPSocket;
	}

	UInt32 GetHostAddr()
	{
		return fHostAddr;
	}									// 远端主机序地址

	UInt16 GetHostPort()
	{
		return fHostPort;
	}

	UInt32 GetLocalAddr()
	{
		return fLocalAddr;
	}

public:
	void InitDMSysClient(UInt32 inSocketType);

	void SetDMSysClientInfo(UInt32 inAddr, UInt16 inPort, char *inURL, UInt32 inLocalAddr = 0);

private:
	UInt32          fHostAddr;
	UInt16          fHostPort;
	UInt32          fLocalAddr;
	CTCPDMSysSocket *fDMSysTCPSocket;
	CDMSysClient    *fDMSysClient;
};
#endif	// _RTPFILEMODULE_H_

