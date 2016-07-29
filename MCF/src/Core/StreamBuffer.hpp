// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2016, LH_Mouse. All wrongs reserved.

#ifndef MCF_CORE_STREAM_BUFFER_HPP_
#define MCF_CORE_STREAM_BUFFER_HPP_

#include <iterator>
#include <utility>
#include <cstddef>

namespace MCF {

/*
class StreamBuffer {
private:
	struct Chunk;

public:
	class ConstChunkEnumerator;

	class ChunkEnumerator {
		friend ConstChunkEnumerator;

	private:
		Chunk *m_chunk;

	public:
		explicit ChunkEnumerator(StreamBuffer &rhs) NOEXCEPT
			: m_chunk(rhs.m_first)
		{
		}

	public:
		unsigned char *begin() const NOEXCEPT;
		unsigned char *end() const NOEXCEPT;

		unsigned char *data() const NOEXCEPT {
			return begin();
		}
		unsigned size() const NOEXCEPT {
			return static_cast<unsigned>(end() - begin());
		}

	public:
#ifdef POSEIDON_CXX11
		explicit operator bool() const noexcept {
			return !!m_chunk;
		}
#else
		typedef bool (StreamBuffer::*DummyBool_)() const;
		operator DummyBool_() const NOEXCEPT {
			return !!m_chunk ? &StreamBuffer::empty : 0;
		}
#endif

		ChunkEnumerator &operator++() NOEXCEPT;
		ChunkEnumerator operator++(int) NOEXCEPT {
			AUTO(ret, *this);
			++*this;
			return ret;
		}
	};

	class ConstChunkEnumerator {
	private:
		const Chunk *m_chunk;

	public:
		explicit ConstChunkEnumerator(const StreamBuffer &rhs) NOEXCEPT
			: m_chunk(rhs.m_first)
		{
		}
		ConstChunkEnumerator(ChunkEnumerator &rhs) NOEXCEPT
			: m_chunk(rhs.m_chunk)
		{
		}

	public:
		const unsigned char *begin() const NOEXCEPT;
		const unsigned char *end() const NOEXCEPT;

		const unsigned char *data() const NOEXCEPT {
			return begin();
		}
		std::size_t size() const NOEXCEPT {
			return static_cast<std::size_t>(end() - begin());
		}

	public:
#ifdef POSEIDON_CXX11
		explicit operator bool() const noexcept {
			return !!m_chunk;
		}
#else
		typedef bool (StreamBuffer::*DummyBool_)() const;
		operator DummyBool_() const NOEXCEPT {
			return !!m_chunk ? &StreamBuffer::empty : 0;
		}
#endif

		ConstChunkEnumerator &operator++() NOEXCEPT;
		ConstChunkEnumerator operator++(int) NOEXCEPT {
			AUTO(ret, *this);
			++*this;
			return ret;
		}
	};

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
