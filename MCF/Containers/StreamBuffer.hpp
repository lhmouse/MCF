// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2014, LH_Mouse. All wrongs reserved.

#ifndef MCF_STREAM_BUFFER_HPP_
#define MCF_STREAM_BUFFER_HPP_

#include "../Core/Utilities.hpp"
#include "VList.hpp"
#include <forward_list>
#include <functional>
#include <iterator>
#include <cstddef>

namespace MCF {

class StreamBuffer {
private:
	class xDisposableBuffer;

public:
	class ReadIterator;
	class WriteIterator;

private:
	VList<xDisposableBuffer> xm_lstBuffers;
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
	// 类似于 Get()，但是不删除。
	int Peek() const noexcept;
	void Put(unsigned char by);

	// 要么就从头部读取并删除 uSize 个字节并返回 true，要么就返回 false，没有只读取一半的情况。
	bool Extract(void *pData, std::size_t uSize) noexcept;
	// 类似于 Extract()，但是不删除。
	bool CopyOut(void *pData, std::size_t uSize) const noexcept;
	// 类似于 Extract()，但是只删除不输出。
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
	WriteIterator GetWriteIterator() noexcept;

public:
	typedef unsigned char value_type;

	// std::back_insert_iterator
	template<typename Param>
	void push_back(Param &&vParam){
		Put(std::forward<Param>(vParam));
	}
};

class StreamBuffer::ReadIterator
	: public std::iterator<std::input_iterator_tag, unsigned char>
{
private:
	StreamBuffer *xm_psbufOwner;

public:
	explicit constexpr ReadIterator(StreamBuffer &sbufOwner) noexcept
		: xm_psbufOwner(&sbufOwner)
	{
	}

public:
	ReadIterator &operator++() noexcept {
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
		ASSERT(!xm_psbufOwner->IsEmpty());

		return (unsigned char)xm_psbufOwner->Peek();
	}
};

class StreamBuffer::WriteIterator
	: public std::iterator<std::output_iterator_tag, unsigned char>
{
private:
	StreamBuffer *xm_psbufOwner;

public:
	explicit constexpr WriteIterator(StreamBuffer &sbufOwner) noexcept
		: xm_psbufOwner(&sbufOwner)
	{
	}

public:
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
inline StreamBuffer::WriteIterator StreamBuffer::GetWriteIterator() noexcept {
	return WriteIterator(*this);
}

}

#endif
