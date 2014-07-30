// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2014, LH_Mouse. All wrongs reserved.

#ifndef MCF_SERDES_TUPLE_HPP_
#define MCF_SERDES_TUPLE_HPP_

#include "Serdes.hpp"
#include <tuple>
#include <iterator>

namespace MCF {

namespace Impl {
	template<class Tuple, std::size_t TUPLE_SIZE, std::size_t INDEX>
	struct TupleSerdesHelper {
		typedef typename std::tuple_element<INDEX, Tuple>::type Element;
		typedef TupleSerdesHelper<Tuple, TUPLE_SIZE, INDEX + 1> Next;

		static void DoSerialize(StreamBuffer &sbufSink, const Tuple &vSource){
			Serialize<Element>(sbufSink, std::get<INDEX>(vSource));
			Next::DoSerialize(sbufSink, vSource);
		}
		static void DoDeserialize(Tuple &vSink, StreamBuffer &sbufSource){
			Deserialize<Element>(std::get<INDEX>(vSink), sbufSource);
			Next::DoDeserialize(vSink, sbufSource);
		}
	};
	template<class Tuple, std::size_t TUPLE_SIZE>
	struct TupleSerdesHelper<Tuple, TUPLE_SIZE, TUPLE_SIZE> {
		static void DoSerialize(StreamBuffer &, const Tuple &){
		}
		static void DoDeserialize(Tuple &, StreamBuffer &){
		}
	};
}

template<typename... Elements>
void Serialize(StreamBuffer &sbufSink, const std::tuple<Elements...> &vSource){
	Impl::TupleSerdesHelper<std::tuple<Elements...>,
		sizeof...(Elements), 0
		>::DoSerialize(sbufSink, vSource);
}
template<typename... Elements>
void Deserialize(std::tuple<Elements...> &vSink, StreamBuffer &sbufSource){
	Impl::TupleSerdesHelper<std::tuple<Elements...>,
		sizeof...(Elements), 0
		>::DoDeserialize(vSink, sbufSource);
}

}

#endif
