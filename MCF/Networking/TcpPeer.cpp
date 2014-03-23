// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2014. LH_Mouse. All wrongs reserved.

#include "../StdMCF.hpp"
#include "TcpPeer.hpp"
#include "../../MCFCRT/c/ext/assert.h"
#include "../Core/Exception.hpp"
#include "../Core/Utilities.hpp"
#include "_SocketUtils.hpp"
using namespace MCF;

// 嵌套类定义。
class TcpPeer::xDelegate : NO_COPY {
private:
	enum : std::uintptr_t {
		FLAG_READABLE	= 0x0001,
		FLAG_WRITABLE	= 0x0002
	};

private:
	__MCF::WSAInitializer xm_vWSAInitializer;

	__MCF::UniqueSocket xm_sockPeer;
	PeerInfo xm_vPeerInfo;
	bool xm_bNoDelay;
	std::uintptr_t xm_uFlags;

public:
	xDelegate(__MCF::UniqueSocket &&sockPeer, const void *pSockAddr, std::size_t uSockAddrLen)
		: xm_sockPeer(std::move(sockPeer))
		, xm_vPeerInfo(pSockAddr, uSockAddrLen)
		, xm_bNoDelay(false)
		, xm_uFlags(FLAG_READABLE | FLAG_WRITABLE)
	{
	}

public:
	const PeerInfo &GetPeerInfo() const noexcept {
		return xm_vPeerInfo;
	}

	std::size_t Read(void *pData, std::size_t uSize){
		auto pCur = (char *)pData;
		const auto pEnd = pCur + uSize;
		for(;;){
			const int nBytesToRead = pEnd - pCur;
			if(nBytesToRead == 0){
				break;
			}
			const int nBytesRead = ::recv(xm_sockPeer.Get(), pCur, nBytesToRead, 0);
			if(nBytesRead == 0){
				break;
			}
			if(nBytesRead < 0){
				MCF_THROW(::WSAGetLastError(), L"::recv() 失败。");
			}
			pCur += nBytesRead;
		}
		return (std::size_t)(pCur - (char *)pData);
	}
	bool ShutdownRead() noexcept {
		if(xm_uFlags & FLAG_READABLE){
			::shutdown(xm_sockPeer.Get(), SD_RECEIVE);
			xm_uFlags &= ~FLAG_READABLE;
		}
		return xm_uFlags;
	}

	void Write(const void *pData, std::size_t uSize, bool bNoDelay){
		if(xm_bNoDelay != bNoDelay){
			static const DWORD TRUE_VALUE	= 1;
			static const DWORD FALSE_VALUE	= 0;
			if(::setsockopt(xm_sockPeer.Get(), IPPROTO_TCP, TCP_NODELAY, (const char *)&(bNoDelay ? TRUE_VALUE : FALSE_VALUE), sizeof(TRUE_VALUE))){
				MCF_THROW(::WSAGetLastError(), L"::setsockopt() 失败。");
			}

			xm_bNoDelay = bNoDelay;
		}
		auto pCur = (const char *)pData;
		const auto pEnd = pCur + uSize;
		for(;;){
			const int nBytesToWrite = pEnd - pCur;
			if(nBytesToWrite == 0){
				break;
			}
			const int nBytesWritten = ::send(xm_sockPeer.Get(), pCur, nBytesToWrite, 0);
			if(nBytesWritten == 0){
				break;
			}
			if(nBytesWritten < 0){
				MCF_THROW(::WSAGetLastError(), L"::send() 失败。");
			}
			pCur += nBytesWritten;
		}
	}
	bool ShutdownWrite() noexcept {
		if(xm_uFlags & FLAG_WRITABLE){
			::shutdown(xm_sockPeer.Get(), SD_SEND);
			xm_uFlags &= ~FLAG_WRITABLE;
		}
		return xm_uFlags;
	}
};

// 构造函数和析构函数。
TcpPeer::TcpPeer(void *ppSocket, const void *pSockAddr, std::size_t uSockAddrLen)
	: xm_pDelegate(new xDelegate(std::move(*(__MCF::UniqueSocket *)ppSocket), pSockAddr, uSockAddrLen))
{
}

TcpPeer::TcpPeer() noexcept {
}
TcpPeer::TcpPeer(const PeerInfo &vServerInfo){
	Connect(vServerInfo);
}
TcpPeer::TcpPeer(TcpPeer &&rhs) noexcept
	: xm_pDelegate(std::move(rhs.xm_pDelegate))
{
}
TcpPeer &TcpPeer::operator=(TcpPeer &&rhs) noexcept {
	if(&rhs != this){
		xm_pDelegate = std::move(rhs.xm_pDelegate);
	}
	return *this;
}
TcpPeer::~TcpPeer(){
}

// 其他非静态成员函数。
bool TcpPeer::IsConnected() const noexcept {
	return (bool)xm_pDelegate;
}
const PeerInfo &TcpPeer::GetPeerInfo() const {
	if(!xm_pDelegate){
		MCF_THROW(WSAENOTCONN, L"TcpPeer 未连接。");
	}
	return xm_pDelegate->GetPeerInfo();
}
unsigned long TcpPeer::ConnectNoThrow(const PeerInfo &vServerInfo){
	try {
		Connect(vServerInfo);
		return ERROR_SUCCESS;
	} catch(Exception &e){
		return e.ulErrorCode;
	}
}
void TcpPeer::Connect(const PeerInfo &vServerInfo){
	Disconnect();

	__MCF::WSAInitializer vWSAInitializer;

	const short shFamily = vServerInfo.IsIPv4() ? AF_INET : AF_INET6;

	__MCF::UniqueSocket sockServer(::socket(shFamily, SOCK_STREAM, IPPROTO_TCP));
	if(!sockServer){
		MCF_THROW(::WSAGetLastError(), L"::socket() 失败。");
	}

	SOCKADDR_STORAGE vSockAddr;
	std::size_t uSockAddrLen;
	BZero(vSockAddr);
	vSockAddr.ss_family = shFamily;
	if(shFamily == AF_INET){
		auto &vSockAddrIn = reinterpret_cast<SOCKADDR_IN &>(vSockAddr);
		BCopy(vSockAddrIn.sin_addr, vServerInfo.m_au8IPv4);
		BCopy(vSockAddrIn.sin_port, vServerInfo.m_u16Port);
		uSockAddrLen = sizeof(SOCKADDR_IN);
	} else {
		auto &vSockAddrIn6 = reinterpret_cast<SOCKADDR_IN6 &>(vSockAddr);
		BCopy(vSockAddrIn6.sin6_addr, vServerInfo.m_au16IPv6);
		BCopy(vSockAddrIn6.sin6_port, vServerInfo.m_u16Port);
		uSockAddrLen = sizeof(SOCKADDR_IN6);
	}
	if(::connect(sockServer.Get(), (const SOCKADDR *)&vSockAddr, uSockAddrLen)){
		MCF_THROW(::WSAGetLastError(), L"::connect() 失败。");
	}

	*this = TcpPeer(&sockServer, &vSockAddr, uSockAddrLen);
}
void TcpPeer::Disconnect() noexcept {
	xm_pDelegate.reset();
}

std::size_t TcpPeer::Read(void *pData, std::size_t uSize){
	if(!xm_pDelegate){
		MCF_THROW(WSAENOTCONN, L"TcpPeer 未连接。");
	}
	return xm_pDelegate->Read(pData, uSize);
}
void TcpPeer::ShutdownRead() noexcept {
	if(xm_pDelegate && !xm_pDelegate->ShutdownRead()){
		xm_pDelegate.reset();
	}
}

void TcpPeer::Write(const void *pData, std::size_t uSize, bool bNoDelay){
	if(!xm_pDelegate){
		MCF_THROW(WSAENOTCONN, L"TcpPeer 未连接。");
	}
	xm_pDelegate->Write(pData, uSize, bNoDelay);
}
void TcpPeer::ShutdownWrite() noexcept {
	if(xm_pDelegate && !xm_pDelegate->ShutdownWrite()){
		xm_pDelegate.reset();
	}
}
