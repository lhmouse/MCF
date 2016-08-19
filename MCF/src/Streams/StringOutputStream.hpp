// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2016, LH_Mouse. All wrongs reserved.

#ifndef MCF_STREAMS_STRING_OUTPUT_STREAM_HPP_
#define MCF_STREAMS_STRING_OUTPUT_STREAM_HPP_

#include "AbstractOutputStream.hpp"
#include "../Core/String.hpp"

namespace MCF {

class StringOutputStream : public AbstractOutputStream {
private:
	NarrowString x_vString;

public:
	explicit StringOutputStream(NarrowString vString = NarrowString()) noexcept
		: x_vString(std::move(vString))
	{
	}
	~StringOutputStream() override;

	StringOutputStream(StringOutputStream &&) noexcept = default;
	StringOutputStream &operator=(StringOutputStream &&) noexcept = default;

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
	void SetString(NarrowString vString) noexcept {
		x_vString = std::move(vString);
	}

	void Swap(StringOutputStream &rhs) noexcept {
		using std::swap;
		swap(x_vString, rhs.x_vString);
	}

public:
	friend void swap(StringOutputStream &lhs, StringOutputStream &rhs) noexcept {
		lhs.Swap(rhs);
	}
};

}

#endif
