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
	UniqueSocket xm_sockListen;
	std::array<Thread, MAX_THREAD_COUNT> xm_arThreads;
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
			UniqueSocket sockClient;

			SOCKADDR vSockAddr;
			int nAddrLen = sizeof(vSockAddr);
			sockClient.Reset(::accept(xm_sockListen.Get(), &vSockAddr, &nAddrLen));
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
	void Start(const PeerInfoIPv4 &vPeerInfo, std::size_t uThreadCount){
		Stop();

		xm_sockListen.Reset(::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP));
		if(!xm_sockListen){
			MCF_THROW(::WSAGetLastError(), L"::socket() 失败。");
		}

		SOCKADDR_IN vSockAddrIn;
		vSockAddrIn.sin_family = AF_INET;
		vSockAddrIn.sin_port = htons(vPeerInfo.u16Port);
		BCopy(vSockAddrIn.sin_addr, vPeerInfo.au8IP);
		Zero(vSockAddrIn.sin_zero);
		if(::bind(xm_sockListen.Get(), (const SOCKADDR *)&vSockAddrIn, sizeof(vSockAddrIn))){
			MCF_THROW(::WSAGetLastError(), L"::bind() 失败。");
		}

		if(::listen(xm_sockListen.Get(), SOMAXCONN)){
			MCF_THROW(::WSAGetLastError(), L"::listen() 失败。");
		}

		for(std::size_t i = 0; i < uThreadCount && i < xm_arThreads.size(); ++i){
			xm_arThreads[i].Start(std::bind(&xDelegate::xThreadProc, this));
		}

		xm_eState = State::RUNNING;
	}
	void Stop() noexcept {
		if(xm_eState != State::STOPPED){
			xm_eState = State::STOPPING;
			for(auto &vThread : xm_arThreads){
				vThread.Join();
			}
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
void TcpServer::Start(const PeerInfoIPv4 &vPeerInfo, std::size_t uThreadCount){
	xm_pDelegate->Start(vPeerInfo, uThreadCount);
}
void TcpServer::Stop() noexcept {
	xm_pDelegate->Stop();
}
