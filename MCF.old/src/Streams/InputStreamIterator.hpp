// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2018, LH_Mouse. All wrongs reserved.

#ifndef MCF_STREAMS_INPUT_STREAM_ITERATOR_HPP_
#define MCF_STREAMS_INPUT_STREAM_ITERATOR_HPP_

#include "AbstractInputStream.hpp"
#include <iterator>

namespace MCF {

class InputStreamIterator : public std::iterator<std::input_iterator_tag, int> {
private:
	AbstractInputStream *x_pstrmOwner;

public:
	explicit InputStreamIterator(AbstractInputStream &strmOwner) noexcept
		: x_pstrmOwner(&strmOwner)
	{ }

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

	int operator()(){
		const int nRet = *(*this);
		++(*this);
		return nRet;
	}
};

}

#endif
