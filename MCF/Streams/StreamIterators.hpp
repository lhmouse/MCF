// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2016, LH_Mouse. All wrongs reserved.

#ifndef MCF_STREAMS_STREAM_ITERATORS_HPP_
#define MCF_STREAMS_STREAM_ITERATORS_HPP_

#include "AbstractStreams.hpp"
#include <iterator>

namespace MCF {

class StreamReadIterator : public std::iterator<std::input_iterator_tag, int> {
private:
	AbstractInputStream *x_pstrmOwner;

public:
	explicit constexpr StreamReadIterator(AbstractInputStream &strmOwner) noexcept
		: x_pstrmOwner(&strmOwner)
	{
	}

public:
	int operator*() const {
		return x_pstrmOwner->Peek();
	}
	StreamReadIterator &operator++(){
		x_pstrmOwner->Discard();
		return *this;
	}
	StreamReadIterator operator++(int){
		x_pstrmOwner->Discard();
		return *this;
	}
};

class StreamWriteIterator : public std::iterator<std::output_iterator_tag, unsigned char> {
private:
	AbstractOutputStream *x_pstrmOwner;

public:
	explicit constexpr StreamWriteIterator(AbstractOutputStream &strmOwner) noexcept
		: x_pstrmOwner(&strmOwner)
	{
	}

public:
	StreamWriteIterator &operator=(unsigned char byData){
		x_pstrmOwner->Put(byData);
		return *this;
	}
	StreamWriteIterator &operator*(){
		return *this;
	}
	StreamWriteIterator &operator++(){
		return *this;
	}
	StreamWriteIterator &operator++(int){
		return *this;
	}
};

}

#endif
