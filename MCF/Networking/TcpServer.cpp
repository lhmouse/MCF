// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2014. LH_Mouse. All wrongs reserved.

#include "../StdMCF.hpp"
#include "TcpServer.hpp"
#include "TcpPeer.hpp"
#include "../Core/Thread.hpp"
#include "../Core/Exception.hpp"
#include "../Core/Utilities.hpp"
#include "_NetworkingUtils.hpp"
using namespace MCF;

// 嵌套类定义。
class TcpServer::xDelegate : NO_COPY {
private:
	enum class State {
		RUNNING,
		STOPPING,
		STOPPED
	};
private:
	const std::function<void (std::unique_ptr<TcpPeer>)> &xm_fnConnProc;

	State xm_eState;
	__MCF::UniqueSocket xm_sockListen;
	std::unique_ptr<Thread[]> xm_pThreads;
public:
	xDelegate(const std::function<void (std::unique_ptr<TcpPeer>)> &fnConnProc)
		: xm_fnConnProc(fnConnProc)
		, xm_eState(State::STOPPED)
	{
	}
	~xDelegate(){
	}
private:
	void xThreadProc() const noexcept {
		while(xm_eState == State::RUNNING){
			__MCF::UniqueSocket sockClient(::accept(xm_sockListen.Get(), nullptr, nullptr));
			if(!sockClient){
				continue;
			}

			std::unique_ptr<TcpPeer> pClient(new TcpPeer(&sockClient));
			xm_fnConnProc(std::move(pClient));
		}
	}
public:
	bool IsRunning() const noexcept {
		return xm_eState == State::RUNNING;
	}
	void Start(const PeerInfo &vBoundOnto, std::size_t uThreadCount, bool bForceIPv6){
		static const DWORD FALSE_VALUE = 0;

		Stop();

		short shFamily = AF_INET6;
		if(!bForceIPv6 && vBoundOnto.IsIPv4()){
			shFamily = AF_INET;
		}

		xm_sockListen.Reset(::socket(shFamily, SOCK_STREAM, IPPROTO_TCP));
		if(!xm_sockListen){
			MCF_THROW(::WSAGetLastError(), L"::socket() 失败。");
		}
		if(shFamily == AF_INET6){
			if(::setsockopt(xm_sockListen.Get(), IPPROTO_IPV6, IPV6_V6ONLY, (const char *)&FALSE_VALUE, sizeof(FALSE_VALUE))){
				MCF_THROW(::WSAGetLastError(), L"::setsockopt() 失败。");
			}
		}

		SOCKADDR_STORAGE vSockAddr;
		BZero(vSockAddr);
		vSockAddr.ss_family = shFamily;
		if(shFamily == AF_INET){
			auto &vSockAddrIn = reinterpret_cast<SOCKADDR_IN &>(vSockAddr);
			BCopy(vSockAddrIn.sin_addr, vBoundOnto.m_au8IPv4);
			BCopy(vSockAddrIn.sin_port, vBoundOnto.m_u16Port);
		} else {
			auto &vSockAddrIn6 = reinterpret_cast<SOCKADDR_IN6 &>(vSockAddr);
			BCopy(vSockAddrIn6.sin6_addr, vBoundOnto.m_au16IPv6);
			BCopy(vSockAddrIn6.sin6_port, vBoundOnto.m_u16Port);
		}
		if(::bind(xm_sockListen.Get(), (const SOCKADDR *)&vSockAddr, sizeof(vSockAddr))){
			MCF_THROW(::WSAGetLastError(), L"::bind() 失败。");
		}

		if(::listen(xm_sockListen.Get(), SOMAXCONN)){
			MCF_THROW(::WSAGetLastError(), L"::listen() 失败。");
		}

		xm_pThreads.reset(new Thread[uThreadCount]);
		for(std::size_t i = 0; i < uThreadCount; ++i){
			xm_pThreads[i].Start(std::bind(&xDelegate::xThreadProc, this));
		}

		xm_eState = State::RUNNING;
	}
	void Stop() noexcept {
		if(xm_eState != State::STOPPED){
			xm_eState = State::STOPPING;

			xm_pThreads.reset();
			xm_sockListen.Reset();

			xm_eState = State::STOPPED;
		}
	}
};

// 构造函数和析构函数。
TcpServer::TcpServer(std::function<void (std::unique_ptr<TcpPeer>)> fnConnProc)
	: xm_fnConnProc(std::move(fnConnProc))
	, xm_pDelegate(new xDelegate(xm_fnConnProc))
{
	WSADATA vWsaData;
	if(::WSAStartup(MAKEWORD(2, 2), &vWsaData)){
		MCF_THROW(::WSAGetLastError(), L"::WSAStartup() 失败。");
	}
}
TcpServer::~TcpServer(){
	::WSACleanup();
}

// 其他非静态成员函数。
bool TcpServer::IsRunning() const noexcept {
	return xm_pDelegate->IsRunning();
}
void TcpServer::Start(const PeerInfo &vBoundOnto, std::size_t uThreadCount, bool bForceIPv6){
	xm_pDelegate->Start(vBoundOnto, uThreadCount, bForceIPv6);
}
void TcpServer::Stop() noexcept {
	xm_pDelegate->Stop();
}
