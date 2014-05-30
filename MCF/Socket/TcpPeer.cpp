// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2014. LH_Mouse. All wrongs reserved.

#include "../StdMCF.hpp"
#include "TcpPeer.hpp"
#include "_SocketUtils.hpp"
#include "../Core/Exception.hpp"
#include "../Core/Utilities.hpp"
using namespace MCF;

namespace {

class TcpPeerDelegate : CONCRETE(TcpPeer) {
private:
	Impl::WSAInitializer xm_vWSAInitializer;

	Impl::UniqueSocket xm_sockPeer;
	PeerInfo xm_vPeerInfo;

public:
	TcpPeerDelegate(Impl::UniqueSocket &&sockPeer, const void *pSockAddr, std::size_t uSockAddrSize)
		: xm_sockPeer	(std::move(sockPeer))
		, xm_vPeerInfo	(pSockAddr, uSockAddrSize)
	{
	}

public:
	const PeerInfo &GetPeerInfo() const noexcept {
		return xm_vPeerInfo;
	}

	std::size_t Read(void *pData, std::size_t uSize){
		std::size_t uTotalRead = 0;
		auto pCur = (char *)pData;
		const auto pEnd = pCur + uSize;
		for(;;){
			const int nBytesToRead = std::min<std::ptrdiff_t>(pEnd - pCur, 0x10000);
			if(nBytesToRead == 0){
				break;
			}
			const int nBytesRead = ::recv(xm_sockPeer.Get(), (char *)pData, (int)uSize, 0);
			if(nBytesRead == 0){
				break;
			}
			if(nBytesRead < 0){
				MCF_THROW(::GetLastError(), L"::send() 失败。");
			}
			pCur += nBytesRead;

			uTotalRead += (std::size_t)nBytesRead;
			if(nBytesRead < nBytesToRead){
				break;
			}
		}
		return uTotalRead;
	}
	void ShutdownRead() noexcept {
		::shutdown(xm_sockPeer.Get(), SD_RECEIVE);
	}

	void SetNoDelay(bool bNoDelay){
		const DWORD dwVal = bNoDelay;
		if(::setsockopt(xm_sockPeer.Get(), IPPROTO_TCP, TCP_NODELAY, (const char *)&dwVal, sizeof(dwVal))){
			MCF_THROW(::GetLastError(), L"::setsockopt() 失败。");
		}
	}
	void Write(const void *pData, std::size_t uSize){
		auto pCur = (const char *)pData;
		const auto pEnd = pCur + uSize;
		for(;;){
			const int nBytesToWrite = std::min<std::ptrdiff_t>(pEnd - pCur, 0x10000);
			if(nBytesToWrite == 0){
				break;
			}
			const int nBytesWritten = ::send(xm_sockPeer.Get(), pCur, nBytesToWrite, 0);
			if(nBytesWritten == 0){
				break;
			}
			if(nBytesWritten < 0){
				MCF_THROW(::GetLastError(), L"::send() 失败。");
			}
			pCur += nBytesWritten;
		}
	}
	void ShutdownWrite() noexcept {
		::shutdown(xm_sockPeer.Get(), SD_SEND);
	}
};

}

namespace MCF {

namespace Impl {
	std::unique_ptr<TcpPeer> TcpPeerFromSocket(
		UniqueSocket vSocket,
		const void *pSockAddr,
		std::size_t uSockAddrSize
	){
		return std::make_unique<TcpPeerDelegate>(std::move(vSocket), pSockAddr, uSockAddrSize);
	}
}

}

// 静态成员函数。
std::unique_ptr<TcpPeer> TcpPeer::Connect(const PeerInfo &piServerInfo){
	Impl::WSAInitializer vWSAInitializer;

	const short shFamily = piServerInfo.IsIPv4() ? AF_INET : AF_INET6;

	Impl::UniqueSocket sockServer(::socket(shFamily, SOCK_STREAM, IPPROTO_TCP));
	if(!sockServer){
		MCF_THROW(::GetLastError(), L"::socket() 失败。");
	}

	SOCKADDR_STORAGE vSockAddr;
	const int nSockAddrSize = piServerInfo.ToSockAddr(&vSockAddr, sizeof(vSockAddr));
	if(::connect(sockServer.Get(), (const SOCKADDR *)&vSockAddr, nSockAddrSize)){
		MCF_THROW(::GetLastError(), L"::connect() 失败。");
	}
	return std::make_unique<TcpPeerDelegate>(std::move(sockServer), &vSockAddr, (std::size_t)nSockAddrSize);
}
std::unique_ptr<TcpPeer> TcpPeer::Connect(const PeerInfo &piServerInfo, const PeerInfo &piLocalInfo){
	Impl::WSAInitializer vWSAInitializer;

	const short shFamily = piServerInfo.IsIPv4() ? AF_INET : AF_INET6;

	Impl::UniqueSocket sockServer(::socket(shFamily, SOCK_STREAM, IPPROTO_TCP));
	if(!sockServer){
		MCF_THROW(::GetLastError(), L"::socket() 失败。");
	}

	SOCKADDR_STORAGE vLocalAddr;
	const int nLocalAddrSize = piLocalInfo.ToSockAddr(&vLocalAddr, sizeof(vLocalAddr));
	if(::bind(sockServer.Get(), (const SOCKADDR *)&vLocalAddr, nLocalAddrSize)){
		MCF_THROW(::GetLastError(), L"::bind() 失败。");
	}

	SOCKADDR_STORAGE vSockAddr;
	const int nSockAddrSize = piServerInfo.ToSockAddr(&vSockAddr, sizeof(vSockAddr));
	if(::connect(sockServer.Get(), (const SOCKADDR *)&vSockAddr, nSockAddrSize)){
		MCF_THROW(::GetLastError(), L"::connect() 失败。");
	}
	return std::make_unique<TcpPeerDelegate>(std::move(sockServer), &vSockAddr, (std::size_t)nSockAddrSize);
}

std::unique_ptr<TcpPeer> TcpPeer::ConnectNoThrow(const PeerInfo &piServerInfo){
	try {
		return Connect(piServerInfo);
	} catch(Exception &e){
		SetWin32LastError(e.m_ulErrorCode);
		return std::unique_ptr<TcpPeer>();
	}
}
std::unique_ptr<TcpPeer> TcpPeer::ConnectNoThrow(const PeerInfo &piServerInfo, const PeerInfo &piLocalInfo){
	try {
		return Connect(piServerInfo, piLocalInfo);
	} catch(Exception &e){
		SetWin32LastError(e.m_ulErrorCode);
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

	return static_cast<TcpPeerDelegate *>(this)->Read(pData, uSize);
}
void TcpPeer::ShutdownRead() noexcept {
	ASSERT(dynamic_cast<TcpPeerDelegate *>(this));

	static_cast<TcpPeerDelegate *>(this)->ShutdownRead();
}

void TcpPeer::SetNoDelay(bool bNoDelay){
	ASSERT(dynamic_cast<TcpPeerDelegate *>(this));

	static_cast<TcpPeerDelegate *>(this)->SetNoDelay(bNoDelay);
}
void TcpPeer::Write(const void *pData, std::size_t uSize){
	ASSERT(dynamic_cast<TcpPeerDelegate *>(this));

	static_cast<TcpPeerDelegate *>(this)->Write(pData, uSize);
}
void TcpPeer::ShutdownWrite() noexcept {
	ASSERT(dynamic_cast<TcpPeerDelegate *>(this));

	static_cast<TcpPeerDelegate *>(this)->ShutdownWrite();
}
