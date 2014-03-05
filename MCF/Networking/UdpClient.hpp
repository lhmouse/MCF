// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2014. LH_Mouse. All wrongs reserved.

#ifndef __MCF_UDP_CLIENT_HPP__
#define __MCF_UDP_CLIENT_HPP__

#include "../Core/NoCopy.hpp"
#include <memory>
#include <cstddef>
#include <cstdint>

namespace MCF {

class UdpClient : NO_COPY {
private:
	class xDelegate;
private:
	std::unique_ptr<xDelegate> xm_pDelegate;
public:
	UdpClient() noexcept;
	~UdpClient();
public:
};

}

#endif
