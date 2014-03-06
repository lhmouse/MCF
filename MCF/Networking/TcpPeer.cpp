// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2014. LH_Mouse. All wrongs reserved.

#include "../StdMCF.hpp"
#include "TcpPeer.hpp"
#include "../../MCFCRT/c/ext/assert.h"
#include "../Core/Exception.hpp"
#include "../Core/Utilities.hpp"
#include "_NetworkingUtils.hpp"
using namespace MCF;


// 嵌套类定义。
class TcpPeer::xDelegate : NO_COPY {
private:
	__MCF::UniqueSocket xm_sockPeer;
public:
	explicit xDelegate(__MCF::UniqueSocket &&sockPeer) :
		xm_sockPeer(std::move(sockPeer))
	{
	}
public:
	PeerInfo GetPeerInfo() const noexcept {
		SOCKADDR_STORAGE vSockAddr;
		int nNameLen = sizeof(vSockAddr);
		if(::getpeername(xm_sockPeer.Get(), (SOCKADDR *)&vSockAddr, &nNameLen)){
			MCF_THROW(::WSAGetLastError(), L"::getpeername() 失败。");
		}

		PeerInfo vPeerInfo;
		if(vSockAddr.ss_family == AF_INET){
			const auto &vSockAddrIn = reinterpret_cast<const SOCKADDR_IN &>(vSockAddr);
			BSet(vPeerInfo.xm_au16IPv4Zeros, false);
			BSet(vPeerInfo.xm_u16IPv4Ones, true);
			BCopy(vPeerInfo.m_au8IPv4, vSockAddrIn.sin_addr);
			BCopy(vPeerInfo.m_u16Port, vSockAddrIn.sin_port);
		} else if(vSockAddr.ss_family == AF_INET6){
			const auto &vSockAddrIn6 = reinterpret_cast<const SOCKADDR_IN6 &>(vSockAddr);
			BCopy(vPeerInfo.m_au16IPv6, vSockAddrIn6.sin6_addr);
			BCopy(vPeerInfo.m_u16Port, vSockAddrIn6.sin6_port);
		} else {
			MCF_THROW(ERROR_NOT_SUPPORTED, L"不支持该协议。");
		}
		return std::move(vPeerInfo);
	}

	std::size_t Read(void *pData, std::size_t uSize){
		auto pWrite = (char *)pData;
		const auto pEnd = pWrite + uSize;
		for(;;){
			const int nBytesToRead = pEnd - pWrite;
			if(nBytesToRead == 0){
				break;
			}
			const int nBytesRead = ::recv(xm_sockPeer.Get(), pWrite, nBytesToRead, 0);
			if(nBytesRead == 0){
				break;
			}
			if(nBytesRead == SOCKET_ERROR){
				MCF_THROW(::WSAGetLastError(), L"::recv() 失败。");
			}
			pWrite += nBytesRead;
		}
		return (std::size_t)(pWrite - (char *)pData);
	}
	void ShutdownRead() noexcept {
		::shutdown(xm_sockPeer.Get(), SD_RECEIVE);
	}

	void Write(const void *pData, std::size_t uSize){
		auto pRead = (const char *)pData;
		const auto pEnd = pRead + uSize;
		for(;;){
			const int nBytesToWrite = pEnd - pRead;
			if(nBytesToWrite == 0){
				break;
			}
			const int nBytesWritten = ::send(xm_sockPeer.Get(), pRead, nBytesToWrite, 0);
			if(nBytesWritten == 0){
				break;
			}
			if(nBytesWritten == SOCKET_ERROR){
				MCF_THROW(::WSAGetLastError(), L"::send() 失败。");
			}
			pRead += nBytesWritten;
		}
	}
	void ShutdownWrite() noexcept {
		::shutdown(xm_sockPeer.Get(), SD_SEND);
	}
};

// 构造函数和析构函数。
TcpPeer::TcpPeer(const void *pImpl)
	: xm_pDelegate(new xDelegate(std::move(*(__MCF::UniqueSocket *)pImpl)))
{
}
TcpPeer::~TcpPeer(){
}

// 其他非静态成员函数。
PeerInfo TcpPeer::GetPeerInfo() const noexcept {
	return xm_pDelegate->GetPeerInfo();
}

std::size_t TcpPeer::Read(void *pData, std::size_t uSize){
	return xm_pDelegate->Read(pData, uSize);
}
void TcpPeer::ShutdownRead() noexcept {
	xm_pDelegate->ShutdownRead();
}

void TcpPeer::Write(const void *pData, std::size_t uSize){
	xm_pDelegate->Write(pData, uSize);
}
void TcpPeer::ShutdownWrite() noexcept {
	xm_pDelegate->ShutdownWrite();
}
