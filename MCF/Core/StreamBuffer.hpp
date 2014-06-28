// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2014. LH_Mouse. All wrongs reserved.

#ifndef MCF_STREAM_BUFFER_HPP_
#define MCF_STREAM_BUFFER_HPP_

#include "Utilities.hpp"
#include <forward_list>
#include <functional>
#include <iterator>
#include <cstddef>

namespace MCF {

class StreamBuffer {
private:
	class xDisposableBuffer;

	typedef std::forward_list<xDisposableBuffer> xBufferList;
	typedef xBufferList::iterator xBufferListIterator;

public:
	class ReadIterator;
	class WriteIterator;

private:
	xBufferList xm_lstData;
	xBufferListIterator xm_itTail;
	xBufferList xm_lstPool;
	std::size_t xm_uSize;

public:
	StreamBuffer() noexcept;
	StreamBuffer(const void *pData, std::size_t uSize);
	StreamBuffer(const StreamBuffer &rhs);
	StreamBuffer(StreamBuffer &&rhs) noexcept;
	StreamBuffer &operator=(const StreamBuffer &rhs);
	StreamBuffer &operator=(StreamBuffer &&rhs) noexcept;
	~StreamBuffer() noexcept;

public:
	bool IsEmpty() const noexcept;
	std::size_t GetSize() const noexcept;
	void Clear() noexcept;

	// 如果为空返回 -1。
	int Get() noexcept;
	// 类似于 Get()，但是不从流中删除字节。
	int Peek() const noexcept;
	void Put(unsigned char by);

	// 要么就从头部读取并删除 uSize 个字节并返回 true，要么就返回 false，没有只读取一半的情况。
	bool Extract(void *pData, std::size_t uSize) noexcept;
	bool Discard(std::size_t uSize) noexcept;
	// 追加 uSize 个字节。
	void Insert(const void *pData, std::size_t uSize);

	// 同 Extract()。
	bool CutOut(StreamBuffer &sbufHead, std::size_t uSize);
	void Append(const StreamBuffer &rhs);
	void Append(StreamBuffer &&rhs) noexcept;

	void Swap(StreamBuffer &rhs) noexcept;

	void Traverse(const std::function<void (const unsigned char *, std::size_t)> &fnCallback) const;
	void Traverse(const std::function<void (unsigned char *, std::size_t)> &fnCallback);

	ReadIterator GetReadIterator() noexcept;
	static constexpr ReadIterator GetReadEnd() noexcept;
	WriteIterator GetWriteIterator() noexcept;
};

class StreamBuffer::ReadIterator
	: public std::iterator<std::input_iterator_tag, unsigned char>
{
private:
	StreamBuffer *const xm_psbufOwner;

public:
	constexpr ReadIterator() noexcept
		: xm_psbufOwner(nullptr)
	{
	}
	explicit constexpr ReadIterator(StreamBuffer &sbufOwner) noexcept
		: xm_psbufOwner(&sbufOwner)
	{
	}

public:
	bool operator==(const ReadIterator &rhs) const noexcept {
		if(!rhs.xm_psbufOwner){
			if(!xm_psbufOwner){
				return true;
			} else {
				return xm_psbufOwner->IsEmpty();
			}
		} else {
			if(!xm_psbufOwner){
				return rhs.xm_psbufOwner->IsEmpty();
			} else {
				return xm_psbufOwner == rhs.xm_psbufOwner;
			}
		}
	}
	bool operator!=(const ReadIterator &rhs) const noexcept {
		return !(*this == rhs);
	}
	ReadIterator &operator++() noexcept {
		ASSERT(xm_psbufOwner);
		ASSERT(!xm_psbufOwner->IsEmpty());

		xm_psbufOwner->Get();
		return *this;
	}
	ReadIterator operator++(int) noexcept {
		auto itRet = *this;
		++*this;
		return std::move(itRet);
	}

	unsigned char operator*() const noexcept {
		ASSERT(xm_psbufOwner);
		ASSERT(!xm_psbufOwner->IsEmpty());

		return (unsigned char)xm_psbufOwner->Peek();
	}
};

class StreamBuffer::WriteIterator
	: public std::iterator<std::output_iterator_tag, unsigned char>
{
private:
	StreamBuffer *const xm_psbufOwner;

public:
	explicit constexpr WriteIterator(StreamBuffer &sbufOwner) noexcept
		: xm_psbufOwner(&sbufOwner)
	{
	}

public:
	bool operator==(const WriteIterator &rhs) const noexcept {
		return xm_psbufOwner == rhs.xm_psbufOwner;
	}
	bool operator!=(const WriteIterator &rhs) const noexcept {
		return !(*this == rhs);
	}
	WriteIterator &operator++() noexcept {
		return *this;
	}
	WriteIterator operator++(int) noexcept {
		return *this;
	}

	WriteIterator &operator*() noexcept {
		return *this;
	}
	WriteIterator &operator=(unsigned char by){
		xm_psbufOwner->Put(by);
		return *this;
	}
};

inline StreamBuffer::ReadIterator StreamBuffer::GetReadIterator() noexcept {
	return ReadIterator(*this);
}
inline constexpr StreamBuffer::ReadIterator StreamBuffer::GetReadEnd() noexcept {
	return ReadIterator();
}
inline StreamBuffer::WriteIterator StreamBuffer::GetWriteIterator() noexcept {
	return WriteIterator(*this);
}

}

#endif
