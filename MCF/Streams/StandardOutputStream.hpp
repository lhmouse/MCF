// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2016, LH_Mouse. All wrongs reserved.

#ifndef MCF_STREAMS_STANDARD_OUTPUT_STREAM_HPP_
#define MCF_STREAMS_STANDARD_OUTPUT_STREAM_HPP_

#include "AbstractOutputStream.hpp"
#include "../Core/StreamBuffer.hpp"

namespace MCF {

class StandardOutputStream : public AbstractOutputStream {
public:
	static void FlushAll(bool bHard);

public:
	using Handle = void *;

private:
	StandardOutputStream *x_pPrev;
	StandardOutputStream *x_pNext;

	Handle x_hPipe;

	mutable StreamBuffer x_vBuffer;

public:
	StandardOutputStream();
	~StandardOutputStream() override;

	StandardOutputStream(StandardOutputStream &&) noexcept = default;
	StandardOutputStream& operator=(StandardOutputStream &&) noexcept = default;

public:
	void Put(unsigned char byData) override;

	void Put(const void *pData, std::size_t uSize) override;

	void Flush(bool bHard) const override;

	Handle GetHandle() const noexcept {
		return x_hPipe;
	}

	void Swap(StandardOutputStream &rhs) noexcept {
		using std::swap;
		swap(x_hPipe,   rhs.x_hPipe);
		swap(x_vBuffer, rhs.x_vBuffer);
	}

	friend void swap(StandardOutputStream &lhs, StandardOutputStream &rhs) noexcept {
		lhs.Swap(rhs);
	}
};

}

#endif
