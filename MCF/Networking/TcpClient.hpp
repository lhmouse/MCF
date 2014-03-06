// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2014. LH_Mouse. All wrongs reserved.

#ifndef __MCF_TCP_CLIENT_HPP__
#define __MCF_TCP_CLIENT_HPP__

#include "TcpPeer.hpp"
#include "../Core/NoCopy.hpp"
#include <memory>
#include <cstddef>
#include <cstdint>

namespace MCF {

class TcpClient : NO_COPY {
private:
	std::unique_ptr<TcpPeer> xm_pPeer;
public:
	TcpClient();
	~TcpClient();
public:
	bool IsConnected() const noexcept;
	unsigned long Connect(const PeerInfo &vServerInfo);
	void Shutdown() noexcept;

	PeerInfo GetPeerInfo() const noexcept;

	std::size_t Read(void *pData, std::size_t uSize);
	void ShutdownRead() noexcept;

	void Write(const void *pData, std::size_t uSize);
	void ShutdownWrite() noexcept;
};

}

#endif
