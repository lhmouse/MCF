// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2016, LH_Mouse. All wrongs reserved.

#ifndef MCF_STREAMS_NULL_OUTPUT_STREAM_HPP_
#define MCF_STREAMS_NULL_OUTPUT_STREAM_HPP_

#include "../Config.hpp"
#include "AbstractOutputStream.hpp"

namespace MCF {

class MCF_HAS_EXPORTED_RTTI NullOutputStream : public AbstractOutputStream {
public:
	NullOutputStream() noexcept = default;
	~NullOutputStream() override;

	NullOutputStream(NullOutputStream &&) noexcept = default;
	NullOutputStream &operator=(NullOutputStream &&) noexcept = default;

public:
	void Put(unsigned char byData) noexcept override;
	void Put(const void *pData, std::size_t uSize) noexcept override;
	void Flush(bool bHard) noexcept override;

	void Swap(NullOutputStream &rhs) noexcept {
		using std::swap;
		(void)rhs;
	}

public:
	friend void swap(NullOutputStream &lhs, NullOutputStream &rhs) noexcept {
		lhs.Swap(rhs);
	}
};

}

#endif
