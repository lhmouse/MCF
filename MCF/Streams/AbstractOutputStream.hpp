// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2016, LH_Mouse. All wrongs reserved.

#ifndef MCF_STREAMS_ABSTRACT_OUTPUT_STREAM_HPP_
#define MCF_STREAMS_ABSTRACT_OUTPUT_STREAM_HPP_

#include "../SmartPointers/PolyIntrusivePtr.hpp"
#include <cstddef>

namespace MCF {

class AbstractOutputStream : public PolyIntrusiveBase {
public:
	AbstractOutputStream() noexcept = default;
	~AbstractOutputStream() override = 0;

	AbstractOutputStream(AbstractOutputStream &&) noexcept = default;
	AbstractOutputStream &operator=(AbstractOutputStream &&) noexcept = default;

public:
	virtual void Put(unsigned char byData) = 0;
	virtual void Put(const void *pData, std::size_t uSize) = 0;
	virtual void Flush(bool bHard) = 0;
};

}

#endif
