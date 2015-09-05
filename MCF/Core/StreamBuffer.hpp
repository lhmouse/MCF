// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2015, LH_Mouse. All wrongs reserved.

#ifndef MCF_CORE_STREAM_BUFFER_HPP_
#define MCF_CORE_STREAM_BUFFER_HPP_

#include <utility>
#include <cstddef>

namespace MCF {

class StreamBuffer {
private:
	struct $Chunk;

public:
	class ConstChunkEnumerator;

	class ChunkEnumerator {
		friend ConstChunkEnumerator;

	private:
		$Chunk *$pChunk;

	public:
		explicit ChunkEnumerator(StreamBuffer &rhs) noexcept
			: $pChunk(rhs.$pFirst)
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
		explicit operator bool() const noexcept {
			return $pChunk != nullptr;
		}

		ChunkEnumerator &operator++() noexcept;
		ChunkEnumerator operator++(int) noexcept {
			auto ceRet = *this;
			++*this;
			return ceRet;
		}
	};

	class ConstChunkEnumerator {
	private:
		const $Chunk *$pChunk;

	public:
		explicit ConstChunkEnumerator(const StreamBuffer &rhs) noexcept
			: $pChunk(rhs.$pFirst)
		{
		}
		ConstChunkEnumerator(ChunkEnumerator &rhs) noexcept
			: $pChunk(rhs.$pChunk)
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
		explicit operator bool() const noexcept {
			return $pChunk != nullptr;
		}

		ConstChunkEnumerator &operator++() noexcept;
		ConstChunkEnumerator operator++(int) noexcept {
			auto ceRet = *this;
			++*this;
			return ceRet;
		}
	};

private:
	$Chunk *__restrict__ $pFirst;
	$Chunk *__restrict__ $pLast;
	std::size_t $uSize;

public:
	constexpr StreamBuffer() noexcept
		: $pFirst(nullptr), $pLast(nullptr), $uSize(0)
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
		return $uSize == 0;
	}
	std::size_t GetSize() const noexcept {
		return $uSize;
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

	ConstChunkEnumerator GetChunkEnumerator() const noexcept {
		return ConstChunkEnumerator(*this);
	}
	ChunkEnumerator GetChunkEnumerator() noexcept {
		return ChunkEnumerator(*this);
	}
	ConstChunkEnumerator GetConstChunkEnumerator() const noexcept {
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
		std::swap($pFirst, rhs.$pFirst);
		std::swap($pLast, rhs.$pLast);
		std::swap($uSize, rhs.$uSize);
	}
};

inline void swap(StreamBuffer &lhs, StreamBuffer &rhs) noexcept {
	lhs.Swap(rhs);
}

}

#endif
