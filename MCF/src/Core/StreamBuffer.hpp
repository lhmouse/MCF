// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2016, LH_Mouse. All wrongs reserved.

#ifndef MCF_CORE_STREAM_BUFFER_HPP_
#define MCF_CORE_STREAM_BUFFER_HPP_

#include <iterator>
#include <utility>
#include <cstddef>

namespace MCF {

namespace Impl_StreamBuffer {
	struct Chunk;
}

class StreamBuffer {
public:
	class ReadIterator;
	class WriteIterator;

private:
	Impl_StreamBuffer::Chunk *x_pFirst;
	Impl_StreamBuffer::Chunk *x_pLast;
	std::size_t x_uSize;

public:
	constexpr StreamBuffer() noexcept
		: x_pFirst(nullptr), x_pLast(nullptr), x_uSize(0)
	{
	}
	StreamBuffer(unsigned char byData, std::size_t uSize)
		: StreamBuffer()
	{
		Put(byData, uSize);
	}
	StreamBuffer(const void *pData, std::size_t uSize)
		: StreamBuffer()
	{
		Put(pData, uSize);
	}
	StreamBuffer(const StreamBuffer &rhs);
	StreamBuffer(StreamBuffer &&rhs) noexcept
		: StreamBuffer()
	{
		Swap(rhs);
	}
	StreamBuffer &operator=(const StreamBuffer &rhs){
		StreamBuffer(rhs).Swap(*this);
		return *this;
	}
	StreamBuffer &operator=(StreamBuffer &&rhs) noexcept {
		rhs.Swap(*this);
		return *this;
	}
	~StreamBuffer(){
		Clear();
	}

public:
	bool IsEmpty() const noexcept {
		return x_uSize == 0;
	}
	std::size_t GetSize() const noexcept {
		return x_uSize;
	}
	void Clear() noexcept;

	int PeekFront() const noexcept;
	int PeekBack() const noexcept;

	int Peek() const noexcept;
	int Get() noexcept;
	bool Discard() noexcept;
	void Put(unsigned char byData);
	int Unput() noexcept;
	void Unget(unsigned char byData);

	std::size_t Peek(void *pData, std::size_t uSize) const noexcept;
	std::size_t Get(void *pData, std::size_t uSize) noexcept;
	std::size_t Discard(std::size_t uSize) noexcept;
	void Put(unsigned char byData, std::size_t uSize);
	void Put(const void *pData, std::size_t uSize);

	StreamBuffer CutOff(std::size_t uOffsetEnd);
	void Splice(StreamBuffer &rhs) noexcept;
	void Splice(StreamBuffer &&rhs) noexcept {
		Splice(rhs);
	}

	void Swap(StreamBuffer &rhs) noexcept {
		using std::swap;
		swap(x_pFirst, rhs.x_pFirst);
		swap(x_pLast,  rhs.x_pLast);
		swap(x_uSize,  rhs.x_uSize);
	}

public:
	friend void swap(StreamBuffer &lhs, StreamBuffer &rhs) noexcept {
		lhs.Swap(rhs);
	}
};

class StreamBuffer::ReadIterator : public std::iterator<std::input_iterator_tag, int> {
private:
	StreamBuffer *x_psbufOwner;

public:
	explicit constexpr ReadIterator(StreamBuffer &sbufOwner) noexcept
		: x_psbufOwner(&sbufOwner)
	{
	}

public:
	int operator*() const {
		return x_psbufOwner->Peek();
	}
	ReadIterator &operator++(){
		x_psbufOwner->Discard();
		return *this;
	}
	ReadIterator operator++(int){
		x_psbufOwner->Discard();
		return *this;
	}
};
class StreamBuffer::WriteIterator : public std::iterator<std::output_iterator_tag, unsigned char> {
private:
	StreamBuffer *x_psbufOwner;

public:
	explicit constexpr WriteIterator(StreamBuffer &sbufOwner) noexcept
		: x_psbufOwner(&sbufOwner)
	{
	}

public:
	WriteIterator &operator=(unsigned char byData){
		x_psbufOwner->Put(byData);
		return *this;
	}
	WriteIterator &operator*(){
		return *this;
	}
	WriteIterator &operator++(){
		return *this;
	}
	WriteIterator &operator++(int){
		return *this;
	}
};

}

#endif
