// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2016, LH_Mouse. All wrongs reserved.

#ifndef MCF_STREAMS_STANDARD_INPUT_STREAM_HPP_
#define MCF_STREAMS_STANDARD_INPUT_STREAM_HPP_

#include "AbstractInputStream.hpp"
#include "../Core/StreamBuffer.hpp"
#include "../Containers/Vector.hpp"

namespace MCF {

class StandardInputStream : public AbstractInputStream {
public:
	using Handle = void *;

private:
	Handle x_hPipe;

	mutable StreamBuffer x_vBuffer;
	mutable Vector<unsigned char> x_vecBackBuffer;

public:
	StandardInputStream();
	~StandardInputStream() override;

	StandardInputStream(StandardInputStream &&) noexcept = default;
	StandardInputStream& operator=(StandardInputStream &&) noexcept = default;

public:
	int Peek() const override;
	int Get() override;
	bool Discard() override;

	std::size_t Peek(void *pData, std::size_t uSize) const override;
	std::size_t Get(void *pData, std::size_t uSize) override;
	std::size_t Discard(std::size_t uSize) override;

	Handle GetHandle() const noexcept {
		return x_hPipe;
	}

	void Swap(StandardInputStream &rhs) noexcept {
		using std::swap;
		swap(x_hPipe,         rhs.x_hPipe);
		swap(x_vBuffer,       rhs.x_vBuffer);
		swap(x_vecBackBuffer, rhs.x_vecBackBuffer);
	}

	friend void swap(StandardInputStream &lhs, StandardInputStream &rhs) noexcept {
		lhs.Swap(rhs);
	}
};

}

#endif
