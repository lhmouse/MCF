// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2016, LH_Mouse. All wrongs reserved.

#ifndef MCF_STREAMS_STANDARD_OUTPUT_STREAM_HPP_
#define MCF_STREAMS_STANDARD_OUTPUT_STREAM_HPP_

#include "AbstractOutputStream.hpp"

namespace MCF {

class StandardOutputStream : public AbstractOutputStream {
public:
	StandardOutputStream() noexcept = default;
	~StandardOutputStream() override;

	StandardOutputStream(StandardOutputStream &&) noexcept = default;
	StandardOutputStream& operator=(StandardOutputStream &&) noexcept = default;

public:
	void Put(unsigned char byData) override;

	void Put(const void *pData, std::size_t uSize) override;

	void Flush(bool bHard) override;

	bool IsBuffered() const noexcept;
	void SetBuffered(bool bBuffered);

	void Swap(StandardOutputStream & /* rhs */) noexcept {
		using std::swap;
	}

	friend void swap(StandardOutputStream &lhs, StandardOutputStream &rhs) noexcept {
		lhs.Swap(rhs);
	}
};

}

#endif
