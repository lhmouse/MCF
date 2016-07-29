// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2016, LH_Mouse. All wrongs reserved.

#ifndef MCF_CORE_STREAM_BUFFER_HPP_
#define MCF_CORE_STREAM_BUFFER_HPP_

#include "_Enumerator.hpp"
#include <iterator>
#include <utility>
#include <cstddef>

namespace MCF {
/*
class StreamBuffer {
public:
	class ReadIterator;
	class WriteIterator;

	class ChunkEnumerator;
	class ConstChunkEnumerator;

public:
	struct Chunk {
		unsigned uBegin;
		unsigned uEnd;
		unsigned char abyData[256];
	};

	using Element = Chunk;

private:
	Chunk *x_pFirst;
	Chunk *x_pLast;
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
	~StreamBuffer();

public:
	constexpr const Element *GetFirst() const noexcept {
		return x_pFirst;
	}


	//   const Element * GetFirst (               ) const noexcept;
	//         Element * GetFirst (               )       noexcept;
	//   const Element * GetLast  (               ) const noexcept;
	//         Element * GetLast  (               )       noexcept;
	//   const Element * GetPrev  (const Element *) const noexcept;
	//         Element * GetNext  (      Element *)       noexcept;
	//   const Element * GetPrev  (const Element *) const noexcept;
	//         Element * GetNext  (      Element *)       noexcept;

	constexpr bool IsEmpty() const noexcept {
		return x_uSize == 0;
	}
	constexpr std::size_t GetSize() const noexcept {
		return x_uSize;
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

class StreamBuffer::ConstChunkEnumerator : public std::iterator<std::bidirectional_iterator_tag, int> {
private:
	const StreamBuffer *x_psbufOwner;

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
class StreamBuffer::ChunkEnumerator : public std::iterator<std::bidirectional_iterator_tag, int> {
	friend ConstChunkEnumerator;

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



class StreamBuffer {
private:
	struct Chunk;

public:

	class ReadIterator : public std::iterator<std::input_iterator_tag, int> {
	private:
		StreamBuffer *m_owner;

	public:
		explicit ReadIterator(StreamBuffer &owner)
			: m_owner(&owner)
		{
		}

	public:
		int operator*() const {
			return m_owner->peek();
		}
		ReadIterator &operator++(){
			m_owner->get();
			return *this;
		}
		ReadIterator operator++(int){
			m_owner->get();
			return *this;
		}
	};

	class WriteIterator : public std::iterator<std::output_iterator_tag, unsigned char> {
	private:
		StreamBuffer *m_owner;

	public:
		explicit WriteIterator(StreamBuffer &owner)
			: m_owner(&owner)
		{
		}

	public:
		WriteIterator &operator=(unsigned char byte){
			m_owner->put(byte);
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
	Chunk *m_first;
	Chunk *m_last;
	std::size_t m_size;

public:
	CONSTEXPR StreamBuffer() NOEXCEPT
		: m_first(NULLPTR), m_last(NULLPTR), m_size(0)
	{
	}

	StreamBuffer(const void *data, std::size_t bytes);
	StreamBuffer(const char *str);
	StreamBuffer(const std::string &str);
	StreamBuffer(const StreamBuffer &rhs);
	StreamBuffer &operator=(const StreamBuffer &rhs);
#ifdef POSEIDON_CXX11
	StreamBuffer(StreamBuffer &&rhs) noexcept;
	StreamBuffer &operator=(StreamBuffer &&rhs) noexcept;
#endif
	~StreamBuffer();

public:
	bool empty() const NOEXCEPT {
		return m_size == 0;
	}
	std::size_t size() const NOEXCEPT {
		return m_size;
	}

	// 如果为空返回 -1。
	int front() const NOEXCEPT;
	int back() const NOEXCEPT;

	void clear() NOEXCEPT;

	int peek() const NOEXCEPT {
		return front();
	}
	int get() NOEXCEPT;
	void put(unsigned char by);
	int unput() NOEXCEPT;
	void unget(unsigned char by);

	std::size_t peek(void *data, std::size_t bytes) const NOEXCEPT;
	std::size_t get(void *data, std::size_t bytes) NOEXCEPT;
	std::size_t discard(std::size_t bytes) NOEXCEPT;
	void put(const void *data, std::size_t bytes);
	void put(const char *str);
	void put(const std::string &str);

	ConstChunkEnumerator get_chunk_enumerator() const NOEXCEPT {
		return ConstChunkEnumerator(*this);
	}
	ChunkEnumerator get_chunk_enumerator() NOEXCEPT {
		return ChunkEnumerator(*this);
	}
	ConstChunkEnumerator get_const_chunk_enumerator() const NOEXCEPT {
		return ConstChunkEnumerator(*this);
	}

	// 拆分成两部分，返回 [0, bytes) 部分，[bytes, -) 部分仍保存于当前对象中。
	StreamBuffer cut_off(std::size_t bytes);
	// cut_off() 的逆操作。该函数返回后 src 为空。
	void splice(StreamBuffer &rhs) NOEXCEPT;
#ifdef POSEIDON_CXX11
	void splice(StreamBuffer &&rhs) noexcept {
		splice(rhs);
	}
#endif

	void swap(StreamBuffer &rhs) NOEXCEPT {
		std::swap(m_first, rhs.m_first);
		std::swap(m_last, rhs.m_last);
		std::swap(m_size, rhs.m_size);
	}

	void dump(std::string &str) const {
		str.resize(size());
		if(!str.empty()){
			peek(&str[0], str.size());
		}
	}
	std::string dump() const {
		std::string str;
		dump(str);
		return str;
	}

public:
	typedef unsigned char value_type;

	// std::front_insert_iterator
	void push_front(unsigned char by){
		unget(by);
	}

	// std::back_insert_iterator
	void push_back(unsigned char by){
		put(by);
	}
};

inline void swap(StreamBuffer &lhs, StreamBuffer &rhs) NOEXCEPT {
	lhs.swap(rhs);
}
*/
}

#endif
