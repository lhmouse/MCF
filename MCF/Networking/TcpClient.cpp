// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2014. LH_Mouse. All wrongs reserved.

#include "../StdMCF.hpp"
#include "TcpClient.hpp"
#include "../Core/Exception.hpp"
#include "../Core/Utilities.hpp"
#include "_NetworkingUtils.hpp"
using namespace MCF;

// 构造函数和析构函数。
TcpClient::TcpClient(){
	WSADATA vWsaData;
	if(::WSAStartup(MAKEWORD(2, 2), &vWsaData)){
		MCF_THROW(::WSAGetLastError(), L"::WSAStartup() 失败。");
	}
}
TcpClient::~TcpClient(){
	::WSACleanup();
}

// 其他非静态成员函数。
bool TcpClient::IsConnected() const noexcept {
	return (bool)xm_pPeer;
}
unsigned long TcpClient::Connect(const PeerInfo &vServerInfo){
	xm_pPeer.reset();

	const short shFamily = vServerInfo.IsIPv4() ? AF_INET : AF_INET6;

	__MCF::UniqueSocket sockServer(::socket(shFamily, SOCK_STREAM, IPPROTO_TCP));
	if(!sockServer){
		return ::WSAGetLastError();
	}

	SOCKADDR_STORAGE vSockAddr;
	BZero(vSockAddr);
	vSockAddr.ss_family = shFamily;
	if(shFamily == AF_INET){
		auto &vSockAddrIn = reinterpret_cast<SOCKADDR_IN &>(vSockAddr);
		BCopy(vSockAddrIn.sin_addr, vServerInfo.m_au8IPv4);
		BCopy(vSockAddrIn.sin_port, vServerInfo.m_u16Port);
	} else {
		auto &vSockAddrIn6 = reinterpret_cast<SOCKADDR_IN6 &>(vSockAddr);
		BCopy(vSockAddrIn6.sin6_addr, vServerInfo.m_au16IPv6);
		BCopy(vSockAddrIn6.sin6_port, vServerInfo.m_u16Port);
	}
	if(::connect(sockServer.Get(), (const SOCKADDR *)&vSockAddr, sizeof(vSockAddr))){
		return ::WSAGetLastError();
	}

	xm_pPeer.reset(new TcpPeer(&sockServer));
	return ERROR_SUCCESS;
}
void TcpClient::Shutdown() noexcept {
	xm_pPeer.reset();
}

PeerInfo TcpClient::GetPeerInfo() const noexcept {
	if(!xm_pPeer){
		MCF_THROW(WSA_INVALID_HANDLE, L"TcpClient 未连接到服务器。");
	}
	return xm_pPeer->GetPeerInfo();
}

std::size_t TcpClient::Read(void *pData, std::size_t uSize){
	if(!xm_pPeer){
		MCF_THROW(WSA_INVALID_HANDLE, L"TcpClient 未连接到服务器。");
	}
	return xm_pPeer->Read(pData, uSize);
}
void TcpClient::ShutdownRead() noexcept {
	if(xm_pPeer){
		xm_pPeer->ShutdownRead();
	}
}

void TcpClient::Write(const void *pData, std::size_t uSize){
	if(!xm_pPeer){
		MCF_THROW(WSA_INVALID_HANDLE, L"TcpClient 未连接到服务器。");
	}
	return xm_pPeer->Write(pData, uSize);
}
void TcpClient::ShutdownWrite() noexcept {
	if(xm_pPeer){
		xm_pPeer->ShutdownWrite();
	}
}
