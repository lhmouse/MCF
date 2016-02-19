// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2016, LH_Mouse. All wrongs reserved.

#ifndef MCF_CORE_STREAM_BUFFER_HPP_
#define MCF_CORE_STREAM_BUFFER_HPP_

#include "../Containers/List.hpp"
#include <iterator>
#include <utility>
#include <cstddef>

namespace MCF {

namespace Impl_StreamBuffer {
	struct Chunk {
		struct FromBeginning { };
		struct FromEnd       { };

		static void *operator new(std::size_t uSize);
		static void operator delete(void *pRaw) noexcept;

		unsigned uBegin;
		unsigned uEnd;
		unsigned char abyData[256];

		explicit Chunk(FromBeginning)
			: uBegin(0), uEnd(uBegin)
		{
		}
		explicit Chunk(FromEnd)
			: uBegin(sizeof(abyData)), uEnd(uBegin)
		{
		}
	};
}

class StreamBuffer {
public:
	class ReadIterator : public std::iterator<std::input_iterator_tag, int> {
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

	class WriteIterator : public std::iterator<std::output_iterator_tag, unsigned char> {
	private:
		StreamBuffer *x_psbufOwner;

	public:
		explicit constexpr WriteIterator(StreamBuffer &sbufOwner) noexcept
			: x_psbufOwner(&sbufOwner)
		{
		}

	public:
		WriteIterator &operator=(unsigned char by){
			x_psbufOwner->Put(by);
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

private:
	List<Impl_StreamBuffer::Chunk> x_lstChunks;
	std::size_t x_uSize;

public:
	constexpr StreamBuffer() noexcept
		: x_lstChunks(), x_uSize(0)
	{
	}
	StreamBuffer(const void *pData, std::size_t uSize)
		: StreamBuffer()
	{
		Put(pData, uSize);
	}
	StreamBuffer(unsigned char by, std::size_t uSize)
		: StreamBuffer()
	{
		Put(by, uSize);
	}

public:
	bool IsEmpty() const noexcept {
		return x_uSize == 0;
	}
	std::size_t GetSize() const noexcept {
		return x_uSize;
	}
	void Clear() noexcept {
		x_lstChunks.Clear();
		x_uSize = 0;
	}

	 // 如果为空返回 -1。
	int PeekFront() const noexcept;
	int PeekBack() const noexcept;

	int Peek() const noexcept;
	int Get() noexcept;
	void Discard() noexcept;
	void Put(unsigned char by);

	int Unput() noexcept;
	void Unget(unsigned char by);

	std::size_t Peek(void *pData, std::size_t uSize) const noexcept;
	std::size_t Get(void *pData, std::size_t uSize) noexcept;
	std::size_t Discard(std::size_t uSize) noexcept;
	void Put(const void *pData, std::size_t uSize);
	void Put(unsigned char by, std::size_t uSize);

	// 拆分成两部分，返回 [0, uSize) 部分，[uSize, -) 部分仍保存于当前对象中。
	StreamBuffer CutOff(std::size_t uSize);
	// CutOff() 的逆操作。该函数返回后 rhs 为空。
	void Splice(StreamBuffer &rhs) noexcept;
	void Splice(StreamBuffer &&rhs) noexcept {
		Splice(rhs);
	}

	template<typename FuncT>
	void Iterate(FuncT &&vFunc) const {
		for(auto pChunk = x_lstChunks.GetFirst(); pChunk; pChunk = x_lstChunks.GetNext(pChunk)){
			std::forward<FuncT>(vFunc)(pChunk->abyData + pChunk->uBegin, pChunk->uEnd - pChunk->uBegin);
		}
	}
	template<typename FuncT>
	void Iterate(FuncT &&vFunc){
		for(auto pChunk = x_lstChunks.GetFirst(); pChunk; pChunk = x_lstChunks.GetNext(pChunk)){
			std::forward<FuncT>(vFunc)(pChunk->abyData + pChunk->uBegin, pChunk->uEnd - pChunk->uBegin);
		}
	}

	void Swap(StreamBuffer &rhs) noexcept {
		using std::swap;
		swap(x_lstChunks, rhs.x_lstChunks);
		swap(x_uSize,     rhs.x_uSize);
	}

	friend void swap(StreamBuffer &lhs, StreamBuffer &rhs) noexcept {
		lhs.Swap(rhs);
	}
};

}

#endif
