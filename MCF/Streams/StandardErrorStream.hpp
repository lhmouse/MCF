// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2016, LH_Mouse. All wrongs reserved.

#ifndef MCF_STREAMS_STANDARD_ERROR_STREAM_HPP_
#define MCF_STREAMS_STANDARD_ERROR_STREAM_HPP_

#include "AbstractOutputStream.hpp"

namespace MCF {

class StandardErrorStream : public AbstractOutputStream {
public:
	constexpr StandardErrorStream() noexcept = default;
	~StandardErrorStream() override;

	StandardErrorStream(StandardErrorStream &&) noexcept = default;
	StandardErrorStream& operator=(StandardErrorStream &&) noexcept = default;

public:
	void Put(unsigned char byData) override;

	void Put(const void *pData, std::size_t uSize) override;

	void Flush(bool bHard) override;

	void Swap(StandardErrorStream & /* rhs */) noexcept {
		using std::swap;
	}

	friend void swap(StandardErrorStream &lhs, StandardErrorStream &rhs) noexcept {
		lhs.Swap(rhs);
	}
};

}

#endif
