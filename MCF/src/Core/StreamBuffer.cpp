// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2016, LH_Mouse. All wrongs reserved.

#include "../StdMCF.hpp"
#include "StreamBuffer.hpp"
#include "MinMax.hpp"
#include "CopyMoveFill.hpp"

/*
#include "precompiled.hpp"
#include "stream_buffer.hpp"
#include "atomic.hpp"

namespace Poseidon {

struct StreamBuffer::Chunk FINAL {
	static volatile bool s_pool_lock;
	static Chunk *s_pool_head;

	static void *operator new(std::size_t bytes){
		assert(bytes == sizeof(Chunk));

		while(atomic_exchange(s_pool_lock, true, ATOMIC_ACQ_REL) == true){
			atomic_pause();
		}
		const AUTO(head, s_pool_head);
		if(!head){
			atomic_store(s_pool_lock, false, ATOMIC_RELEASE);
			return ::operator new(bytes);
		}
		s_pool_head = head->prev;
		atomic_store(s_pool_lock, false, ATOMIC_RELEASE);
		return head;
	}
	static void operator delete(void *p) NOEXCEPT {
		if(!p){
			return;
		}
		const AUTO(head, static_cast<Chunk *>(p));

		while(atomic_exchange(s_pool_lock, true, ATOMIC_ACQ_REL) == true){
			atomic_pause();
		}
		head->prev = s_pool_head;
		s_pool_head = head;
		atomic_store(s_pool_lock, false, ATOMIC_RELEASE);
	}

	__attribute__((__destructor__(101)))
	static void pool_destructor() NOEXCEPT {
		while(s_pool_head){
			const AUTO(head, s_pool_head);
			s_pool_head = head->prev;
			::operator delete(head);
		}
	}

	Chunk *prev;
	Chunk *next;
	unsigned begin;
	unsigned end;
	unsigned char data[0x100];
};

volatile bool StreamBuffer::Chunk::s_pool_lock = false;
StreamBuffer::Chunk *StreamBuffer::Chunk::s_pool_head = NULLPTR;

unsigned char *StreamBuffer::ChunkEnumerator::begin() const NOEXCEPT {
	assert(m_chunk);

	return m_chunk->data + m_chunk->begin;
}
unsigned char *StreamBuffer::ChunkEnumerator::end() const NOEXCEPT {
	assert(m_chunk);

	return m_chunk->data + m_chunk->end;
}

StreamBuffer::ChunkEnumerator &StreamBuffer::ChunkEnumerator::operator++() NOEXCEPT {
	assert(m_chunk);

	m_chunk = m_chunk->next;
	return *this;
}

const unsigned char *StreamBuffer::ConstChunkEnumerator::begin() const NOEXCEPT {
	assert(m_chunk);

	return m_chunk->data + m_chunk->begin;
}
const unsigned char *StreamBuffer::ConstChunkEnumerator::end() const NOEXCEPT {
	assert(m_chunk);

	return m_chunk->data + m_chunk->end;
}

StreamBuffer::ConstChunkEnumerator &StreamBuffer::ConstChunkEnumerator::operator++() NOEXCEPT {
	assert(m_chunk);

	m_chunk = m_chunk->next;
	return *this;
}

// 构造函数和析构函数。
StreamBuffer::StreamBuffer(const void *data, std::size_t bytes)
	: m_first(NULLPTR), m_last(NULLPTR), m_size(0)
{
	put(data, bytes);
}
StreamBuffer::StreamBuffer(const char *str)
	: m_first(NULLPTR), m_last(NULLPTR), m_size(0)
{
	put(str);
}
StreamBuffer::StreamBuffer(const std::string &str)
	: m_first(NULLPTR), m_last(NULLPTR), m_size(0)
{
	put(str);
}
StreamBuffer::StreamBuffer(const StreamBuffer &rhs)
	: m_first(NULLPTR), m_last(NULLPTR), m_size(0)
{
	for(AUTO(ce, rhs.get_chunk_enumerator()); ce; ++ce){
		put(ce.data(), ce.size());
	}
}
StreamBuffer &StreamBuffer::operator=(const StreamBuffer &rhs){
	StreamBuffer(rhs).swap(*this);
	return *this;
}
#ifdef POSEIDON_CXX11
StreamBuffer::StreamBuffer(StreamBuffer &&rhs) noexcept
	: StreamBuffer()
{
	swap(rhs);
}
StreamBuffer &StreamBuffer::operator=(StreamBuffer &&rhs) noexcept {
	rhs.swap(*this);
	return *this;
}
#endif
StreamBuffer::~StreamBuffer(){
	clear();
}

// 其他非静态成员函数。
int StreamBuffer::front() const NOEXCEPT {
	if(m_size == 0){
		return -1;
	}

	int ret = -1;
	AUTO(chunk, m_first);
	do {
		if(chunk->end != chunk->begin){
			ret = chunk->data[chunk->begin];
		}
		chunk = chunk->next;
	} while(ret < 0);
	return ret;
}
int StreamBuffer::back() const NOEXCEPT {
	if(m_size == 0){
		return -1;
	}

	int ret = -1;
	AUTO(chunk, m_last);
	do {
		if(chunk->end != chunk->begin){
			ret = chunk->data[chunk->end - 1];
		}
		chunk = chunk->prev;
	} while(ret < 0);
	return ret;
}

void StreamBuffer::clear() NOEXCEPT {
	while(m_first){
		const AUTO(chunk, m_first);
		m_first = chunk->next;
		delete chunk;
	}
	m_last = NULLPTR;
	m_size = 0;
}

int StreamBuffer::get() NOEXCEPT {
	if(m_size == 0){
		return -1;
	}

	int ret = -1;
	AUTO(chunk, m_first);
	do {
		if(chunk->end != chunk->begin){
			ret = chunk->data[chunk->begin];
			++(chunk->begin);
		}
		if(chunk->begin == chunk->end){
			chunk = chunk->next;
			delete m_first;
			m_first = chunk;

			if(chunk){
				chunk->prev = NULLPTR;
			} else {
				m_last = NULLPTR;
			}
		}
	} while(ret < 0);
	m_size -= 1;
	return ret;
}
void StreamBuffer::put(unsigned char by){
	std::size_t last_avail = 0;
	if(m_last){
		last_avail = sizeof(m_last->data) - m_last->end;
	}
	Chunk *last_chunk = NULLPTR;
	if(last_avail != 0){
		last_chunk = m_last;
	} else {
		AUTO(chunk, new Chunk);
		chunk->next = NULLPTR;
		// chunk->prev = NULLPTR;
		chunk->begin = 0;
		chunk->end = 0;

		if(m_last){
			m_last->next = chunk;
		} else {
			m_first = chunk;
		}
		chunk->prev = m_last;
		m_last = chunk;

		if(!last_chunk){
			last_chunk = chunk;
		}
	}

	AUTO(chunk, last_chunk);
	chunk->data[chunk->end] = by;
	++(chunk->end);
	++m_size;
}
int StreamBuffer::unput() NOEXCEPT {
	if(m_size == 0){
		return -1;
	}

	int ret = -1;
	AUTO(chunk, m_last);
	do {
		if(chunk->end != chunk->begin){
			--(chunk->end);
			ret = chunk->data[chunk->end];
		}
		if(chunk->begin == chunk->end){
			chunk = chunk->prev;
			delete m_last;
			m_last = chunk;

			if(chunk){
				chunk->next = NULLPTR;
			} else {
				m_first = NULLPTR;
			}
		}
	} while(ret < 0);
	m_size -= 1;
	return ret;
}
void StreamBuffer::unget(unsigned char by){
	std::size_t first_avail = 0;
	if(m_first){
		first_avail = m_first->begin;
	}
	Chunk *first_chunk = NULLPTR;
	if(first_avail != 0){
		first_chunk = m_first;
	} else {
		AUTO(chunk, new Chunk);
		// chunk->next = NULLPTR;
		chunk->prev = NULLPTR;
		chunk->begin = sizeof(chunk->data);
		chunk->end = sizeof(chunk->data);

		if(m_first){
			m_first->prev = chunk;
		} else {
			m_last = chunk;
		}
		chunk->next = m_first;
		m_first = chunk;

		if(!first_chunk){
			first_chunk = chunk;
		}
	}

	AUTO(chunk, first_chunk);
	--(chunk->begin);
	chunk->data[chunk->begin] = by;
	++m_size;
}

std::size_t StreamBuffer::peek(void *data, std::size_t bytes) const NOEXCEPT {
	const AUTO(bytes_to_copy, std::min(bytes, m_size));
	if(bytes_to_copy == 0){
		return 0;
	}

	std::size_t bytes_copied = 0;
	AUTO(chunk, m_first);
	do {
		const AUTO(write, static_cast<unsigned char *>(data) + bytes_copied);
		const AUTO(bytes_to_copy_this_time, std::min<std::size_t>(bytes_to_copy - bytes_copied, chunk->end - chunk->begin));
		std::memcpy(write, chunk->data + chunk->begin, bytes_to_copy_this_time);
		bytes_copied += bytes_to_copy_this_time;
		chunk = chunk->next;
	} while(bytes_copied < bytes_to_copy);
	return bytes_copied;
}
std::size_t StreamBuffer::get(void *data, std::size_t bytes) NOEXCEPT {
	const AUTO(bytes_to_copy, std::min(bytes, m_size));
	if(bytes_to_copy == 0){
		return 0;
	}

	std::size_t bytes_copied = 0;
	AUTO(chunk, m_first);
	do {
		const AUTO(write, static_cast<unsigned char *>(data) + bytes_copied);
		const AUTO(bytes_to_copy_this_time, std::min<std::size_t>(bytes_to_copy - bytes_copied, chunk->end - chunk->begin));
		std::memcpy(write, chunk->data + chunk->begin, bytes_to_copy_this_time);
		bytes_copied += bytes_to_copy_this_time;
		chunk->begin += bytes_to_copy_this_time;
		if(chunk->begin == chunk->end){
			chunk = chunk->next;
			delete m_first;
			m_first = chunk;

			if(chunk){
				chunk->prev = NULLPTR;
			} else {
				m_last = NULLPTR;
			}
		}
	} while(bytes_copied < bytes_to_copy);
	m_size -= bytes_to_copy;
	return bytes_copied;
}
std::size_t StreamBuffer::discard(std::size_t bytes) NOEXCEPT {
	const AUTO(bytes_to_copy, std::min(bytes, m_size));
	if(bytes_to_copy == 0){
		return 0;
	}

	std::size_t bytes_copied = 0;
	AUTO(chunk, m_first);
	do {
		const AUTO(bytes_to_copy_this_time, std::min<std::size_t>(bytes_to_copy - bytes_copied, chunk->end - chunk->begin));
		bytes_copied += bytes_to_copy_this_time;
		chunk->begin += bytes_to_copy_this_time;
		if(chunk->begin == chunk->end){
			chunk = chunk->next;
			delete m_first;
			m_first = chunk;

			if(chunk){
				chunk->prev = NULLPTR;
			} else {
				m_last = NULLPTR;
			}
		}
	} while(bytes_copied < bytes_to_copy);
	m_size -= bytes_to_copy;
	return bytes_copied;
}
void StreamBuffer::put(const void *data, std::size_t bytes){
	const AUTO(bytes_to_copy, bytes);
	if(bytes_to_copy == 0){
		return;
	}

	std::size_t last_avail = 0;
	if(m_last){
		last_avail = sizeof(m_last->data) - m_last->end;
	}
	Chunk *last_chunk = NULLPTR;
	if(last_avail != 0){
		last_chunk = m_last;
	}
	if(bytes_to_copy > last_avail){
		const AUTO(new_chunks, (bytes_to_copy - last_avail - 1) / sizeof(last_chunk->data) + 1);
		assert(new_chunks != 0);

		AUTO(chunk, new Chunk);
		chunk->next = NULLPTR;
		chunk->prev = NULLPTR;
		chunk->begin = 0;
		chunk->end = 0;

		AUTO(splice_first, chunk), splice_last = chunk;
		try {
			for(std::size_t i = 1; i < new_chunks; ++i){
				chunk = new Chunk;
				chunk->next = NULLPTR;
				chunk->prev = splice_last;
				chunk->begin = 0;
				chunk->end = 0;

				splice_last->next = chunk;
				splice_last = chunk;
			}
		} catch(...){
			do {
				chunk = splice_first;
				splice_first = chunk->next;
				delete chunk;
			} while(splice_first);

			throw;
		}
		if(m_last){
			m_last->next = splice_first;
		} else {
			m_first = splice_first;
		}
		splice_first->prev = m_last;
		m_last = splice_last;

		if(!last_chunk){
			last_chunk = splice_first;
		}
	}

	std::size_t bytes_copied = 0;
	AUTO(chunk, last_chunk);
	do {
		const AUTO(read, static_cast<const unsigned char *>(data) + bytes_copied);
		const AUTO(bytes_to_copy_this_time, std::min<std::size_t>(bytes_to_copy - bytes_copied, sizeof(chunk->data) - chunk->end));
		std::memcpy(chunk->data + chunk->end, read, bytes_to_copy_this_time);
		chunk->end += bytes_to_copy_this_time;
		bytes_copied += bytes_to_copy_this_time;
		chunk = chunk->next;
	} while(bytes_copied < bytes_to_copy);
	m_size += bytes_to_copy;
}
void StreamBuffer::put(const char *str){
	put(str, std::strlen(str));
}
void StreamBuffer::put(const std::string &str){
	put(str.data(), str.size());
}

StreamBuffer StreamBuffer::cut_off(std::size_t bytes){
	StreamBuffer ret;

	const AUTO(bytes_to_copy, std::min(bytes, m_size));
	if(bytes_to_copy == 0){
		return ret;
	}

	if(m_size <= bytes_to_copy){
		ret.swap(*this);
		return ret;
	}

	std::size_t bytes_copied = 0;
	AUTO(cut_end, m_first);
	for(;;){
		const AUTO(bytes_remaining, bytes_to_copy - bytes_copied);
		const AUTO(bytes_avail, cut_end->end - cut_end->begin);
		if(bytes_remaining <= bytes_avail){
			if(bytes_remaining == bytes_avail){
				cut_end = cut_end->next;
			} else {
				const AUTO(chunk, new Chunk);
				chunk->next = cut_end;
				chunk->prev = cut_end->prev;
				chunk->begin = 0;
				chunk->end = bytes_remaining;

				std::memcpy(chunk->data, cut_end->data + cut_end->begin, bytes_remaining);
				cut_end->begin += bytes_remaining;

				if(cut_end->prev){
					cut_end->prev->next = chunk;
				} else {
					m_first = chunk;
				}
				cut_end->prev = chunk;
			}
			break;
		}
		bytes_copied += bytes_avail;
		cut_end = cut_end->next;
	}

	const AUTO(cut_first, m_first);
	const AUTO(cut_last, cut_end->prev);
	cut_last->next = NULLPTR;
	cut_end->prev = NULLPTR;

	m_first = cut_end;
	m_size -= bytes_to_copy;

	ret.m_first = cut_first;
	ret.m_last = cut_last;
	ret.m_size = bytes_to_copy;
	return ret;
}
void StreamBuffer::splice(StreamBuffer &rhs) NOEXCEPT {
	if(&rhs == this){
		return;
	}
	if(!rhs.m_first){
		return;
	}

	if(m_last){
		m_last->next = rhs.m_first;
	} else {
		m_first = rhs.m_first;
	}
	rhs.m_first->prev = m_last;
	m_last = rhs.m_last;
	m_size += rhs.m_size;

	rhs.m_first = NULLPTR;
	rhs.m_last = NULLPTR;
	rhs.m_size = 0;
}

}
*/
