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
unsigned long TcpClient::Connect(const PeerInfoIPv4 &vPeerInfo){
	UniqueSocket sockServer;

	sockServer.Reset(::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP));
	if(!sockServer){
		return ::WSAGetLastError();
	}

	SOCKADDR_IN vSockAddrIn;
	vSockAddrIn.sin_family = AF_INET;
	vSockAddrIn.sin_port = htons(vPeerInfo.u16Port);
	BCopy(vSockAddrIn.sin_addr, vPeerInfo.au8IP);
	Zero(vSockAddrIn.sin_zero);
	if(::connect(sockServer.Get(), (const SOCKADDR *)&vSockAddrIn, sizeof(vSockAddrIn))){
		return ::WSAGetLastError();
	}

	xm_pPeer.reset(new TcpPeer(&sockServer));
	return ERROR_SUCCESS;
}
void TcpClient::Shutdown() noexcept {
	xm_pPeer.reset();
}

PeerInfoIPv4 TcpClient::GetPeerInfo() const noexcept {
	PeerInfoIPv4 vRet;
	if(xm_pPeer){
		vRet = xm_pPeer->GetPeerInfo();
	} else {
		Zero(vRet);
	}
	return std::move(vRet);
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
