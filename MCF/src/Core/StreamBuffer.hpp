// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2017, LH_Mouse. All wrongs reserved.

#ifndef MCF_CORE_STREAM_BUFFER_HPP_
#define MCF_CORE_STREAM_BUFFER_HPP_

#include <iterator>
#include <cstddef>

namespace MCF {

class StreamBuffer {
public:
	class ReadIterator;
	class WriteIterator;

private:
	struct X_ChunkHeader;
	X_ChunkHeader *x_pLast = nullptr;
	X_ChunkHeader *x_pFirst = nullptr;
	std::size_t x_uSize = 0;

public:
	constexpr StreamBuffer() noexcept = default;
	StreamBuffer(unsigned char byData, std::size_t uSize){
		Put(byData, uSize);
	}
	StreamBuffer(const void *pData, std::size_t uSize){
		Put(pData, uSize);
	}
	StreamBuffer(const StreamBuffer &vOther);
	StreamBuffer(StreamBuffer &&vOther) noexcept {
		Swap(vOther);
	}
	StreamBuffer &operator=(const StreamBuffer &vOther){
		StreamBuffer(vOther).Swap(*this);
		return *this;
	}
	StreamBuffer &operator=(StreamBuffer &&vOther) noexcept {
		vOther.Swap(*this);
		return *this;
	}
	~StreamBuffer();

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

	void *Squash();

	StreamBuffer CutOff(std::size_t uOffsetEnd);
	void Splice(StreamBuffer &vOther) noexcept;
	void Splice(StreamBuffer &&vOther) noexcept {
		Splice(vOther);
	}

	void Swap(StreamBuffer &vOther) noexcept {
		using std::swap;
		swap(x_pLast,  vOther.x_pLast);
		swap(x_pFirst, vOther.x_pFirst);
		swap(x_uSize,  vOther.x_uSize);
	}

public:
	friend void swap(StreamBuffer &vSelf, StreamBuffer &vOther) noexcept {
		vSelf.Swap(vOther);
	}
};

class StreamBuffer::ReadIterator : public std::iterator<std::input_iterator_tag, int> {
private:
	StreamBuffer *x_psbufParent;

public:
	explicit constexpr ReadIterator(StreamBuffer &sbufParent) noexcept
		: x_psbufParent(&sbufParent)
	{
	}

public:
	int operator*() const {
		return x_psbufParent->Peek();
	}
	ReadIterator &operator++(){
		x_psbufParent->Discard();
		return *this;
	}
	ReadIterator operator++(int){
		x_psbufParent->Discard();
		return *this;
	}
};
class StreamBuffer::WriteIterator : public std::iterator<std::output_iterator_tag, unsigned char> {
private:
	StreamBuffer *x_psbufParent;

public:
	explicit constexpr WriteIterator(StreamBuffer &sbufParent) noexcept
		: x_psbufParent(&sbufParent)
	{
	}

public:
	WriteIterator &operator=(unsigned char byData){
		x_psbufParent->Put(byData);
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
