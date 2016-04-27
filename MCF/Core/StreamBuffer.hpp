// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2016, LH_Mouse. All wrongs reserved.

#ifndef MCF_CORE_STREAM_BUFFER_HPP_
#define MCF_CORE_STREAM_BUFFER_HPP_

#include "../Containers/List.hpp"
#include "../Containers/Vector.hpp"
#include <iterator>
#include <utility>
#include <cstddef>

namespace MCF {

template<std::size_t>
class FixedSizeAllocator;

class StreamBuffer {
private:
	struct X_Chunk {
		struct FromBeginning { };
		struct FromEnd       { };

		unsigned uBegin;
		unsigned uEnd;
		unsigned char abyData[256];

		explicit X_Chunk(FromBeginning)
			: uBegin(0), uEnd(uBegin)
		{
		}
		explicit X_Chunk(FromEnd)
			: uBegin(sizeof(abyData)), uEnd(uBegin)
		{
		}
	};
	struct X_ChunkAllocator {
		void *operator()(std::size_t uSize);
		void operator()(void *pBlock) noexcept;
	};
	using X_ChunkList = List<X_Chunk, X_ChunkAllocator>;

	static FixedSizeAllocator<X_ChunkList::kNodeSize> xs_vPool;

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

private:
	X_ChunkList x_lstChunks;
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
	StreamBuffer(unsigned char byData, std::size_t uSize)
		: StreamBuffer()
	{
		Put(byData, uSize);
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

	int Peek() noexcept;
	int Get() noexcept;
	bool Discard() noexcept;
	void Put(unsigned char byData);

	int Unput() noexcept;
	void Unget(unsigned char byData);

	std::size_t Peek(void *pData, std::size_t uSize) noexcept;
	std::size_t Get(void *pData, std::size_t uSize) noexcept;
	std::size_t Discard(std::size_t uSize) noexcept;
	void Put(const void *pData, std::size_t uSize);
	void Put(unsigned char byData, std::size_t uSize);

	// 拆分成两部分，返回 [0, uSize) 部分，[uSize, -) 部分仍保存于当前对象中。
	StreamBuffer CutOff(std::size_t uSize);
	// CutOff() 的逆操作。该函数返回后 rhs 为空。
	void Splice(StreamBuffer &rhs) noexcept;
	void Splice(StreamBuffer &&rhs) noexcept {
		Splice(rhs);
	}

	void Dump(Vector<unsigned char> &vecData) const;

	template<typename FuncT>
	bool Iterate(FuncT &&vFunc) const {
		for(auto pChunk = x_lstChunks.GetFirst(); pChunk; pChunk = x_lstChunks.GetNext(pChunk)){
			if(!std::forward<FuncT>(vFunc)(pChunk->abyData + pChunk->uBegin, pChunk->uEnd - pChunk->uBegin)){
				return false;
			}
		}
		return true;
	}
	template<typename FuncT>
	bool Iterate(FuncT &&vFunc){
		for(auto pChunk = x_lstChunks.GetFirst(); pChunk; pChunk = x_lstChunks.GetNext(pChunk)){
			if(!std::forward<FuncT>(vFunc)(pChunk->abyData + pChunk->uBegin, pChunk->uEnd - pChunk->uBegin)){
				return false;
			}
		}
		return true;
	}

	void Swap(StreamBuffer &rhs) noexcept {
		using std::swap;
		swap(x_lstChunks, rhs.x_lstChunks);
		swap(x_uSize,     rhs.x_uSize);
	}

public:
	friend void swap(StreamBuffer &lhs, StreamBuffer &rhs) noexcept {
		lhs.Swap(rhs);
	}
};

}

#endif
