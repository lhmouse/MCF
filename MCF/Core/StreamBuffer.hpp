// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2014. LH_Mouse. All wrongs reserved.

#ifndef MCF_STREAM_BUFFER_HPP_
#define MCF_STREAM_BUFFER_HPP_

#include "Utilities.hpp"
#include <array>
#include <vector>
#include <deque>
#include <functional>
#include <iterator>
#include <cstddef>

namespace MCF {

class StreamBufferReadIterator;
class StreamBufferWriteIterator;

class StreamBuffer {
public:
	typedef StreamBufferReadIterator	ReadIterator;
	typedef StreamBufferWriteIterator	WriteIterator;

private:
	std::array<unsigned char, 256u> xm_abySmall;
	std::size_t xm_uSmallSize;
	std::deque<std::vector<unsigned char>> xm_deqLarge;
	std::size_t xm_uSize;

public:
	StreamBuffer() noexcept;
	StreamBuffer(const void *pData, std::size_t uSize);
	StreamBuffer(const StreamBuffer &rhs);
	StreamBuffer(StreamBuffer &&rhs) noexcept;
	StreamBuffer &operator=(const StreamBuffer &rhs);
	StreamBuffer &operator=(StreamBuffer &&rhs) noexcept;

public:
	bool IsEmpty() const noexcept;
	std::size_t GetSize() const noexcept;
	void Clear() noexcept;

	// 追加 uSize 个字节。
	void Insert(const void *pData, std::size_t uSize);
	// 要么就从头部读取并删除 uSize 个字节并返回 true，要么就返回 false。
	// 没有只读取一半的情况。
	bool Extract(void *pData, std::size_t uSize) noexcept;

	void Append(const StreamBuffer &rhs);
	void Append(StreamBuffer &&rhs);
	void Swap(StreamBuffer &rhs) noexcept;

	void Traverse(std::function<void (const unsigned char *, std::size_t)> fnCallback) const;
	void Traverse(std::function<void (unsigned char *, std::size_t)> fnCallback);

	ReadIterator GetReadIterator() noexcept;
	ReadIterator GetReadEnd() const noexcept;
	WriteIterator GetWriteIterator() noexcept;
};

class StreamBufferReadIterator
	: public std::iterator<std::input_iterator_tag, unsigned char>
{
	friend StreamBuffer;

private:
	StreamBuffer *xm_psbufOwner;

public:
	constexpr StreamBufferReadIterator() noexcept
		: xm_psbufOwner(nullptr)
	{
	}
	explicit constexpr StreamBufferReadIterator(StreamBuffer &sbufOwner) noexcept
		: xm_psbufOwner(&sbufOwner)
	{
	}

public:
	bool operator==(const StreamBufferReadIterator &rhs) const noexcept {
		return xm_psbufOwner == rhs.xm_psbufOwner;
	}
	bool operator!=(const StreamBufferReadIterator &rhs) const noexcept {
		return !(*this == rhs);
	}
	StreamBufferReadIterator &operator++() noexcept {
		return *this;
	}
	StreamBufferReadIterator operator++(int) noexcept {
		return *this;
	}

	int operator*() noexcept {
		ASSERT(xm_psbufOwner);

		unsigned char by;
		if(!xm_psbufOwner->Extract(&by, sizeof(by))){
			return -1;
		}
		if(xm_psbufOwner->IsEmpty()){
			xm_psbufOwner = nullptr;
		}
		return by;
	}
};

class StreamBufferWriteIterator
	: public std::iterator<std::output_iterator_tag, unsigned char>
{
	friend StreamBuffer;

private:
	StreamBuffer *const xm_psbufOwner;

public:
	explicit constexpr StreamBufferWriteIterator(StreamBuffer &sbufOwner) noexcept
		: xm_psbufOwner(&sbufOwner)
	{
	}

public:
	bool operator==(const StreamBufferWriteIterator &rhs) const noexcept {
		return xm_psbufOwner == rhs.xm_psbufOwner;
	}
	bool operator!=(const StreamBufferWriteIterator &rhs) const noexcept {
		return !(*this == rhs);
	}
	StreamBufferWriteIterator &operator++() noexcept {
		return *this;
	}
	StreamBufferWriteIterator operator++(int) noexcept {
		return *this;
	}

	StreamBufferWriteIterator &operator*() noexcept {
		return *this;
	}
	StreamBufferWriteIterator &operator=(unsigned char by){
		xm_psbufOwner->Insert(&by, sizeof(by));
		return *this;
	}
};

inline StreamBuffer::ReadIterator StreamBuffer::GetReadIterator() noexcept {
	return ReadIterator(*this);
}
inline StreamBuffer::ReadIterator StreamBuffer::GetReadEnd() const noexcept {
	return ReadIterator();
}
inline StreamBuffer::WriteIterator StreamBuffer::GetWriteIterator() noexcept {
	return WriteIterator(*this);
}

}

#endif
