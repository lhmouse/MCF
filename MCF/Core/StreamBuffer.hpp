// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2014, LH_Mouse. All wrongs reserved.

#ifndef MCF_STREAM_BUFFER_HPP_
#define MCF_STREAM_BUFFER_HPP_

#include "../Containers/VList.hpp"
#include <iterator>
#include <utility>
#include <memory>
#include <functional>
#include <cstddef>

namespace MCF {

namespace Impl {
	struct DisposableBuffer;
}

class StreamBuffer {
public:
	class ReadIterator;
	class WriteIterator;

private:
	VList<Impl::DisposableBuffer> xm_lstBuffers;
	std::size_t xm_uSize;

public:
	StreamBuffer() noexcept;
	StreamBuffer(const void *pData, std::size_t uSize);
	StreamBuffer(const char *pszData);
	StreamBuffer(const StreamBuffer &rhs);
	StreamBuffer(StreamBuffer &&rhs) noexcept;
	StreamBuffer &operator=(const StreamBuffer &rhs);
	StreamBuffer &operator=(StreamBuffer &&rhs) noexcept;
	~StreamBuffer() noexcept;

public:
	bool IsEmpty() const noexcept {
		return xm_uSize == 0;
	}
	std::size_t GetSize() const noexcept {
		return xm_uSize;
	}
	void Clear() noexcept;

	// 如果为空返回 -1。
	int Peek() const noexcept;
	int Get() noexcept;
	void Put(unsigned char by);

	std::size_t Peek(void *pData, std::size_t uSize) const noexcept;
	std::size_t Get(void *pData, std::size_t uSize) noexcept;
	std::size_t Discard(std::size_t uSize) noexcept;
	void Put(const void *pData, std::size_t uSize);
	void Put(const char *pszData);

	ReadIterator GetReadIterator() noexcept;
	WriteIterator GetWriteIterator() noexcept;

	void Traverse(const std::function<void (const unsigned char *, std::size_t)> &fnCallback) const;
	void Traverse(const std::function<void (unsigned char *, std::size_t)> &fnCallback);

	// 拆分成两部分，返回 [0, uSize) 部分，[uSize, -) 部分仍保存于当前对象中。
	StreamBuffer Cut(std::size_t uSize);
	// Cut() 的逆操作。该函数返回后 src 为空。
	void Splice(StreamBuffer &rhs) noexcept;
	void Splice(StreamBuffer &&rhs) noexcept {
		Splice(rhs);
	}

	void Swap(StreamBuffer &rhs) noexcept;

public:
	typedef unsigned char value_type;

	// std::back_insert_iterator
	void push_back(unsigned char byParam){
		Put(byParam);
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

	int operator*() const noexcept {
		ASSERT(!xm_psbufOwner->IsEmpty());

		return xm_psbufOwner->Peek();
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

inline void swap(StreamBuffer &lhs, StreamBuffer &rhs) noexcept {
	lhs.Swap(rhs);
}

}

#endif
