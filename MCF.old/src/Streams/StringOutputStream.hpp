// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2017, LH_Mouse. All wrongs reserved.

#ifndef MCF_STREAMS_STRING_OUTPUT_STREAM_HPP_
#define MCF_STREAMS_STRING_OUTPUT_STREAM_HPP_

#include "AbstractOutputStream.hpp"
#include "../Core/String.hpp"

namespace MCF {

class StringOutputStream : public AbstractOutputStream {
private:
	NarrowString x_vString;
	std::size_t x_uOffset;

public:
	StringOutputStream() noexcept
		: x_vString(), x_uOffset(0)
	{ }
	explicit StringOutputStream(NarrowString vString, std::size_t uOffset = 0) noexcept
		: x_vString(std::move(vString)), x_uOffset(uOffset)
	{ }
	~StringOutputStream() override;

public:
	void Put(unsigned char byData) override;
	void Put(const void *pData, std::size_t uSize) override;
	void Flush(bool bHard) override;

	const NarrowString &GetString() const noexcept {
		return x_vString;
	}
	NarrowString &GetString() noexcept {
		return x_vString;
	}
	void SetString(NarrowString &&vString, std::size_t uOffset = 0) noexcept {
		x_vString = std::move(vString);
		x_uOffset = uOffset;
	}

	std::size_t GetOffset() const noexcept {
		return x_uOffset;
	}
	void SetOffset(std::size_t uOffset) noexcept {
		x_uOffset = uOffset;
	}
};

}

#endif
