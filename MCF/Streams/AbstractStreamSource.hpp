// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2016, LH_Mouse. All wrongs reserved.

#ifndef MCF_STREAMS_ABSTRACT_STREAM_SOURCE_HPP_
#define MCF_STREAMS_ABSTRACT_STREAM_SOURCE_HPP_

#include <cstddef>

namespace MCF {

class AbstractStreamSource {
public:
	virtual ~AbstractStreamSource();

public:
	virtual int Peek() const = 0;
	virtual int Get() = 0;
	virtual void Discard() = 0;

	virtual std::size_t Peek(void *pData, std::size_t uSize) const = 0;
	virtual std::size_t Get(void *pData, std::size_t uSize) = 0;
	virtual std::size_t Discard(std::size_t uSize) = 0;
};

}

#endif
