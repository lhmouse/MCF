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
	std::array<unsigned char, 256u> xm_arrSmall;
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
	bool ExtractNoThrow(void *pData, std::size_t uSize) noexcept;
	// 和上面类似，但是如果数据少于请求的字节数会抛出异常 MCF::Exception(ERROR_HANDLE_EOF)。
	void Extract(void *pData, std::size_t uSize);

	void Append(const StreamBuffer &sbufOther);
	void Append(StreamBuffer &&sbufOther);
	void Swap(StreamBuffer &sbufOther) noexcept;

	void Traverse(std::function<void (const unsigned char *, std::size_t)> fnCallback) const;
	void Traverse(std::function<void (unsigned char *, std::size_t)> fnCallback);

	ReadIterator GetReadIterator() noexcept;
	static constexpr ReadIterator GetReadEnd() noexcept;
	// 返回一个和任何有效迭代器都不相等的迭代器，用于从输入流中一直读取。
	// 如果读到流的末尾会引发异常。
	static constexpr ReadIterator GetReadForeverEnd() noexcept;
	WriteIterator GetWriteIterator() noexcept;
};

class StreamBufferReadIterator
	: public std::iterator<std::input_iterator_tag, unsigned char>
{
	friend StreamBuffer;

private:
	static constexpr const auto xFOREVER = (StreamBuffer *)(std::intptr_t)-1;

private:
	StreamBuffer *const xm_psbufOwner;

private:
	explicit constexpr StreamBufferReadIterator(StreamBuffer *psbufOwner) noexcept
		: xm_psbufOwner(psbufOwner)
	{
	}

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
		if((xm_psbufOwner == xFOREVER) || (rhs.xm_psbufOwner == xFOREVER)){
			return false;
		}
		if(rhs.xm_psbufOwner){
			if(xm_psbufOwner){
				return xm_psbufOwner == rhs.xm_psbufOwner;
			} else {
				return rhs.xm_psbufOwner->IsEmpty();
			}
		} else {
			if(xm_psbufOwner){
				return xm_psbufOwner->IsEmpty();
			} else {
				return true;
			}
		}
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

	unsigned char operator*(){
		unsigned char by;
		xm_psbufOwner->Extract(&by, sizeof(by));
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
inline constexpr StreamBuffer::ReadIterator StreamBuffer::GetReadEnd() noexcept {
	return ReadIterator();
}
inline constexpr StreamBuffer::ReadIterator StreamBuffer::GetReadForeverEnd() noexcept {
	return ReadIterator(ReadIterator::xFOREVER);
}
inline StreamBuffer::WriteIterator StreamBuffer::GetWriteIterator() noexcept {
	return WriteIterator(*this);
}

}

#endif
