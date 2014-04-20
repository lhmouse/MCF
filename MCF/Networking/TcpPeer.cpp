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

namespace {

class TcpPeerDelegate : CONCRETE(TcpPeer) {
private:
	WSAInitializer xm_vWSAInitializer;

	UniqueSocket xm_sockPeer;
	PeerInfo xm_vPeerInfo;

public:
	TcpPeerDelegate(UniqueSocket &&sockPeer, const void *pSockAddr, std::size_t uSockAddrLen)
		: xm_sockPeer(std::move(sockPeer))
		, xm_vPeerInfo(pSockAddr, uSockAddrLen)
	{
	}

public:
	const PeerInfo &GetPeerInfo() const noexcept {
		return xm_vPeerInfo;
	}

	std::size_t Read(void *pData, std::size_t uSize){
		const int nBytesRead = ::recv(xm_sockPeer.Get(), (char *)pData, uSize, 0);
		if(nBytesRead < 0){
			MCF_THROW(::WSAGetLastError(), L"::recv() 失败。");
		}
		return (std::size_t)nBytesRead;
	}
	void ShutdownRead() noexcept {
		::shutdown(xm_sockPeer.Get(), SD_RECEIVE);
	}

	void SetNoDelay(bool bNoDelay){
		DWORD dwVal = bNoDelay;
		if(::setsockopt(xm_sockPeer.Get(), IPPROTO_TCP, TCP_NODELAY, (const char *)&dwVal, sizeof(dwVal))){
			MCF_THROW(::WSAGetLastError(), L"::setsockopt() 失败。");
		}
	}
	void Write(const void *pData, std::size_t uSize){
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
	void ShutdownWrite() noexcept {
		::shutdown(xm_sockPeer.Get(), SD_SEND);
	}
};

}

// 静态成员函数。
std::unique_ptr<TcpPeer> TcpPeer::xFromSocket(void *ppSocket, const void *pSockAddr, std::size_t uSockAddrLen){
	return std::unique_ptr<TcpPeer>(new TcpPeerDelegate(std::move(*(UniqueSocket *)ppSocket), pSockAddr, uSockAddrLen));
}

std::unique_ptr<TcpPeer> TcpPeer::Connect(const PeerInfo &vServerInfo){
	WSAInitializer vWSAInitializer;

	const short shFamily = vServerInfo.IsIPv4() ? AF_INET : AF_INET6;

	UniqueSocket sockServer(::socket(shFamily, SOCK_STREAM, IPPROTO_TCP));
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

	return std::unique_ptr<TcpPeer>(new TcpPeerDelegate(std::move(sockServer), &vSockAddr, uSockAddrLen));
}
std::unique_ptr<TcpPeer> TcpPeer::ConnectNoThrow(const PeerInfo &vServerInfo){
	try {
		return Connect(vServerInfo);
	} catch(Exception &e){
		SetWin32LastError(e.ulErrorCode);
		return std::unique_ptr<TcpPeer>();
	}
}

// 其他非静态成员函数。
const PeerInfo &TcpPeer::GetPeerInfo() const noexcept {
	ASSERT(dynamic_cast<const TcpPeerDelegate *>(this));

	return ((const TcpPeerDelegate *)this)->GetPeerInfo();
}

std::size_t TcpPeer::Read(void *pData, std::size_t uSize){
	ASSERT(dynamic_cast<TcpPeerDelegate *>(this));

	return ((TcpPeerDelegate *)this)->Read(pData, uSize);
}
void TcpPeer::ShutdownRead() noexcept {
	ASSERT(dynamic_cast<TcpPeerDelegate *>(this));

	((TcpPeerDelegate *)this)->ShutdownRead();
}

void TcpPeer::SetNoDelay(bool bNoDelay){
	ASSERT(dynamic_cast<TcpPeerDelegate *>(this));

	((TcpPeerDelegate *)this)->SetNoDelay(bNoDelay);
}
void TcpPeer::Write(const void *pData, std::size_t uSize){
	ASSERT(dynamic_cast<TcpPeerDelegate *>(this));

	((TcpPeerDelegate *)this)->Write(pData, uSize);
}
void TcpPeer::ShutdownWrite() noexcept {
	ASSERT(dynamic_cast<TcpPeerDelegate *>(this));

	((TcpPeerDelegate *)this)->ShutdownWrite();
}
