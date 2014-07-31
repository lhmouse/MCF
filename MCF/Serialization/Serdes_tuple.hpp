// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2014, LH_Mouse. All wrongs reserved.

#ifndef MCF_SERDES_TUPLE_HPP_
#define MCF_SERDES_TUPLE_HPP_

#include "Serdes.hpp"
#include <tuple>

namespace MCF {

namespace Impl {
	template<class Tuple, std::size_t TUPLE_SIZE, std::size_t INDEX>
	struct TupleSerdesHelper {
		typedef TupleSerdesHelper<Tuple, TUPLE_SIZE, INDEX + 1> Next;

		static void Insert(const Tuple &vSource, StreamBuffer &sbufSink){
			std::get<INDEX>(vSource) >>= sbufSink;
			Next::Insert(vSource, sbufSink);
		}
		static void Extract(Tuple &vSink, StreamBuffer &sbufSource){
			std::get<INDEX>(vSink) <<= sbufSource;
			Next::Extract(vSink, sbufSource);
		}
	};
	template<class Tuple, std::size_t TUPLE_SIZE>
	struct TupleSerdesHelper<Tuple, TUPLE_SIZE, TUPLE_SIZE> {
		static void Insert(const Tuple &, StreamBuffer &){
		}
		static void Extract(Tuple &, StreamBuffer &){
		}
	};
}

template<typename... Elements>
void operator>>=(const std::tuple<Elements...> &vSource, StreamBuffer &sbufSink){
	Impl::TupleSerdesHelper<std::tuple<Elements...>,
		sizeof...(Elements), 0
		>::Insert(vSource, sbufSink);
}
template<typename... Elements>
void operator<<=(std::tuple<Elements...> &vSink, StreamBuffer &sbufSource){
	Impl::TupleSerdesHelper<std::tuple<Elements...>,
		sizeof...(Elements), 0
		>::Extract(vSink, sbufSource);
}

}

#endif
