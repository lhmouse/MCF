// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2014, LH_Mouse. All wrongs reserved.

#ifndef MCF_CORE_STREAM_BUFFER_HPP_
#define MCF_CORE_STREAM_BUFFER_HPP_

#include "../Containers/List.hpp"
#include <iterator>
#include <utility>
#include <memory>
#include <cstddef>

namespace MCF {

struct StreamBufferChunk;

class StreamBuffer {
public:
	class ReadIterator;
	class WriteIterator;

	class TraverseContext;

private:
	List<StreamBufferChunk> xm_lstBuffers;
	std::size_t xm_uSize;

public:
	StreamBuffer() noexcept;
	StreamBuffer(const void *pData, std::size_t uSize);
	StreamBuffer(const char *pszData);
	StreamBuffer(const StreamBuffer &rhs);
	StreamBuffer(StreamBuffer &&rhs) noexcept;
	StreamBuffer &operator=(const StreamBuffer &rhs);
	StreamBuffer &operator=(StreamBuffer &&rhs) noexcept;
	~StreamBuffer();

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
	int Unput() noexcept;
	void Unget(unsigned char by);

	std::size_t Peek(void *pData, std::size_t uSize) const noexcept;
	std::size_t Get(void *pData, std::size_t uSize) noexcept;
	std::size_t Discard(std::size_t uSize) noexcept;
	void Put(const void *pData, std::size_t uSize);
	void Put(const char *pszData);

	// 拆分成两部分，返回 [0, uSize) 部分，[uSize, -) 部分仍保存于当前对象中。
	StreamBuffer Cut(std::size_t uSize);
	// Cut() 的逆操作。该函数返回后 src 为空。
	void Splice(StreamBuffer &rhs) noexcept;
	void Splice(StreamBuffer &&rhs) noexcept {
		Splice(rhs);
	}

	bool Traverse(const TraverseContext *&pContext,
		std::pair<const void *, std::size_t> &vBlock) const noexcept;
	bool Traverse(TraverseContext *&pContext,
		std::pair<void *, std::size_t> &vBlock) noexcept;

	template<typename CallbackT>
	void Traverse(CallbackT &&vCallback) const {
		const TraverseContext *pContext = nullptr;
		std::pair<const void *, std::size_t> vBlock;
		while(Traverse(pContext, vBlock)){
			std::forward<CallbackT>(vCallback)(vBlock.first, vBlock.second);
		}
	}
	template<typename CallbackT>
	void Traverse(CallbackT &&vCallback){
		TraverseContext *pContext = nullptr;
		std::pair<void *, std::size_t> vBlock;
		while(Traverse(pContext, vBlock)){
			std::forward<CallbackT>(vCallback)(vBlock.first, vBlock.second);
		}
	}

	ReadIterator GetReadIterator() noexcept;
	WriteIterator GetWriteIterator() noexcept;

	void Swap(StreamBuffer &rhs) noexcept {
		xm_lstBuffers.Swap(rhs.xm_lstBuffers);
		std::swap(xm_uSize, rhs.xm_uSize);
	}

public:
	using value_type = unsigned char;

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

	unsigned char operator*() const noexcept {
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
