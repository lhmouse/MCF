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
	struct XChunk;

public:
	class ConstChunkEnumerator;

	class ChunkEnumerator {
		friend ConstChunkEnumerator;

	private:
		XChunk *x_pChunk;

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
		explicit operator bool() const noexcept {
			return x_pChunk != nullptr;
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
		const XChunk *x_pChunk;

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
		explicit operator bool() const noexcept {
			return x_pChunk != nullptr;
		}

		ConstChunkEnumerator &operator++() noexcept;
		ConstChunkEnumerator operator++(int) noexcept {
			auto ceRet = *this;
			++*this;
			return ceRet;
		}
	};

private:
	XChunk *__restrict__ x_pFirst;
	XChunk *__restrict__ x_pLast;
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
		std::swap(x_pFirst, rhs.x_pFirst);
		std::swap(x_pLast, rhs.x_pLast);
		std::swap(x_uSize, rhs.x_uSize);
	}
};

inline void swap(StreamBuffer &lhs, StreamBuffer &rhs) noexcept {
	lhs.Swap(rhs);
}

}

#endif
