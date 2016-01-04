// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2016, LH_Mouse. All wrongs reserved.

#ifndef MCF_CORE_STREAM_BUFFER_HPP_
#define MCF_CORE_STREAM_BUFFER_HPP_

#include <utility>
#include <cstddef>
#include <iterator>

namespace MCF {

namespace Impl_StreamBuffer {
	struct Chunk;
}

class StreamBuffer {
public:
	class ConstChunkEnumerator;

	class ChunkEnumerator : public std::iterator<std::forward_iterator_tag, const std::pair<unsigned char *, unsigned char *>> {
		friend ConstChunkEnumerator;

	private:
		Impl_StreamBuffer::Chunk *x_pChunk;

	public:
		explicit ChunkEnumerator(StreamBuffer &rhs) noexcept
			: x_pChunk(rhs.x_pFirst)
		{
		}

	public:
		unsigned char *GetBegin() const noexcept;
		unsigned char *GetEnd() const noexcept;

		unsigned char *GetData() const noexcept {
			return GetBegin();
		}
		unsigned GetSize() const noexcept {
			return static_cast<unsigned>(GetEnd() - GetBegin());
		}

	public:
		bool operator==(const ChunkEnumerator &rhs) const noexcept {
			return x_pChunk == rhs.x_pChunk;
		}
		bool operator!=(const ChunkEnumerator &rhs) const noexcept {
			return !(*this == rhs);
		}

		ChunkEnumerator &operator++() noexcept;
		ChunkEnumerator operator++(int) noexcept {
			auto ceRet = *this;
			++*this;
			return ceRet;
		}

		const std::pair<unsigned char *, unsigned char *> operator*() const noexcept {
			return std::make_pair(GetBegin(), GetEnd());
		}

		explicit operator bool() const noexcept {
			return !!x_pChunk;
		}
	};

	class ConstChunkEnumerator : public std::iterator<std::forward_iterator_tag, const std::pair<const unsigned char *, const unsigned char *>> {
	private:
		const Impl_StreamBuffer::Chunk *x_pChunk;

	public:
		explicit ConstChunkEnumerator(const StreamBuffer &rhs) noexcept
			: x_pChunk(rhs.x_pFirst)
		{
		}
		ConstChunkEnumerator(ChunkEnumerator &rhs) noexcept
			: x_pChunk(rhs.x_pChunk)
		{
		}

	public:
		const unsigned char *GetBegin() const noexcept;
		const unsigned char *GetEnd() const noexcept;

		const unsigned char *GetData() const noexcept {
			return GetBegin();
		}
		std::size_t GetSize() const noexcept {
			return static_cast<std::size_t>(GetEnd() - GetBegin());
		}

	public:
		bool operator==(const ConstChunkEnumerator &rhs) const noexcept {
			return x_pChunk == rhs.x_pChunk;
		}
		bool operator!=(const ConstChunkEnumerator &rhs) const noexcept {
			return !(*this == rhs);
		}

		ConstChunkEnumerator &operator++() noexcept;
		ConstChunkEnumerator operator++(int) noexcept {
			auto ceRet = *this;
			++*this;
			return ceRet;
		}

		const std::pair<const unsigned char *, const unsigned char *> operator*() const noexcept {
			return std::make_pair(GetBegin(), GetEnd());
		}

		explicit operator bool() const noexcept {
			return !!x_pChunk;
		}
	};

private:
	Impl_StreamBuffer::Chunk *x_pFirst;
	Impl_StreamBuffer::Chunk *x_pLast;
	std::size_t x_uSize;

public:
	constexpr StreamBuffer() noexcept
		: x_pFirst(nullptr), x_pLast(nullptr), x_uSize(0)
	{
	}

	StreamBuffer(const void *pData, std::size_t uSize);
	StreamBuffer(const char *pszData);
	StreamBuffer(const StreamBuffer &rhs);
	StreamBuffer(StreamBuffer &&rhs) noexcept;
	StreamBuffer &operator=(const StreamBuffer &rhs);
	StreamBuffer &operator=(StreamBuffer &&rhs) noexcept;
	~StreamBuffer();

public:
	bool IsEmpty() const noexcept {
		return x_uSize == 0;
	}
	std::size_t GetSize() const noexcept {
		return x_uSize;
	}

	// 如果为空返回 -1。
	int PeekFront() const noexcept;
	int PeekBack() const noexcept;

	void Clear() noexcept;

	int Peek() const noexcept {
		return PeekFront();
	}
	int Get() noexcept;
	void Put(unsigned char by);
	int Unput() noexcept;
	void Unget(unsigned char by);

	std::size_t Peek(void *pData, std::size_t uSize) const noexcept;
	std::size_t Get(void *pData, std::size_t uSize) noexcept;
	std::size_t Discard(std::size_t uSize) noexcept;
	void Put(const void *pData, std::size_t uSize);
	void Put(const char *pszData);

	ConstChunkEnumerator EnumerateFirstChunk() const noexcept {
		return ConstChunkEnumerator(*this);
	}
	ChunkEnumerator EnumerateFirstChunk() noexcept {
		return ChunkEnumerator(*this);
	}
	ConstChunkEnumerator EnumerateConstFirstChunk() const noexcept {
		return ConstChunkEnumerator(*this);
	}

	// 拆分成两部分，返回 [0, uSize) 部分，[uSize, -) 部分仍保存于当前对象中。
	StreamBuffer CutOff(std::size_t uSize);
	// CutOff() 的逆操作。该函数返回后 src 为空。
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
};

inline void swap(StreamBuffer &lhs, StreamBuffer &rhs) noexcept {
	lhs.Swap(rhs);
}

}

#endif
