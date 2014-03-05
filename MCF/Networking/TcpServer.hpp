// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2014. LH_Mouse. All wrongs reserved.

#ifndef __MCF_TCP_SERVER_HPP__
#define __MCF_TCP_SERVER_HPP__

#include "TcpPeer.hpp"
#include "../Core/NoCopy.hpp"
#include <functional>
#include <memory>
#include <cstddef>
#include <cstdint>

namespace MCF {

class TcpServer : NO_COPY {
public:
	enum : std::size_t {
		MAX_THREAD_COUNT = 16
	};
private:
	class xDelegate;
private:
	const std::function<void (std::unique_ptr<TcpPeer>)> xm_fnConnProc;
	const std::unique_ptr<xDelegate> xm_pDelegate;
public:
	TcpServer(std::function<void (std::unique_ptr<TcpPeer>)> fnConnProc);
	~TcpServer();
public:
	bool IsRunning() const noexcept;
	void Start(const PeerInfoIPv4 &vPeerInfo, std::size_t uThreadCount);
	void Stop() noexcept;
};

}

#endif
