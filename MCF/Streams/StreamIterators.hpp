// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2016, LH_Mouse. All wrongs reserved.

#ifndef MCF_STREAMS_STREAM_ITERATORS_HPP_
#define MCF_STREAMS_STREAM_ITERATORS_HPP_

#include "AbstractStreams.hpp"
#include <iterator>

namespace MCF {

class InputStreamIterator : public std::iterator<std::input_iterator_tag, int> {
private:
	AbstractInputStream *x_pstrmOwner;

public:
	explicit constexpr InputStreamIterator(AbstractInputStream &strmOwner) noexcept
		: x_pstrmOwner(&strmOwner)
	{
	}

public:
	int operator*() const {
		return x_pstrmOwner->Peek();
	}
	InputStreamIterator &operator++(){
		x_pstrmOwner->Discard();
		return *this;
	}
	InputStreamIterator operator++(int){
		x_pstrmOwner->Discard();
		return *this;
	}
};

class OutputStreamIterator : public std::iterator<std::output_iterator_tag, unsigned char> {
private:
	AbstractOutputStream *x_pstrmOwner;

public:
	explicit constexpr OutputStreamIterator(AbstractOutputStream &strmOwner) noexcept
		: x_pstrmOwner(&strmOwner)
	{
	}

public:
	OutputStreamIterator &operator=(unsigned char byData){
		x_pstrmOwner->Put(byData);
		return *this;
	}
	OutputStreamIterator &operator*(){
		return *this;
	}
	OutputStreamIterator &operator++(){
		return *this;
	}
	OutputStreamIterator &operator++(int){
		return *this;
	}
};

}

#endif
