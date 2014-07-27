// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2014 LH_Mouse. All wrongs reserved.

#ifndef MCF_SERDES_HPP_
#define MCF_SERDES_HPP_

#include "../Core/StreamBuffer.hpp"
#include "../Core/Utilities.hpp"
#include "VarIntEx.hpp"
#include <type_traits>
#include <deque>
#include <utility>
#include <tuple>
#include <cstddef>

namespace MCF {

namespace Impl {
	[[noreturn]]
	extern void ThrowEndOfStream();

	[[noreturn]]
	extern void ThrowInvalidData();

	template<typename DataType, typename TraitHelper = void>
	struct SerdesTrait {
		static_assert((sizeof(DataType) - 1, false), "Not supported.");

		static void Serialize(StreamBuffer &sbufSink, const DataType &vSource);
		static void Deserialize(DataType &vSink, StreamBuffer &sbufSource);
	};

	template<>
	struct SerdesTrait<bool, void> {
		static void Serialize(StreamBuffer &sbufSink, bool vSource){
			sbufSink.Put((unsigned char)vSource);
		}
		static void Deserialize(bool &vSink, StreamBuffer &sbufSource){
			const auto nVal = sbufSource.Get();
			if(nVal == -1){
				ThrowEndOfStream();
			}
			vSink = (nVal != 0);
		}
	};
	template<typename Char>
	struct SerdesTrait<Char,
		typename std::enable_if<
			std::is_integral<Char>::value && (sizeof(Char) == 1)
			>::type>
	{
		static void Serialize(StreamBuffer &sbufSink, Char vSource){
			sbufSink.Put((unsigned char)vSource);
		}
		static void Deserialize(Char &vSink, StreamBuffer &sbufSource){
			const auto nVal = sbufSource.Get();
			if(nVal == -1){
				ThrowEndOfStream();
			}
			vSink = (Char)(unsigned char)nVal;
		}
	};
	template<typename Integer>
	struct SerdesTrait<Integer,
		typename std::enable_if<
			std::is_integral<Integer>::value && (sizeof(Integer) > 1)
			>::type>
	{
		static void Serialize(StreamBuffer &sbufSink, Integer vSource){
			VarIntEx<Integer> vEncoder;
			vEncoder.Set(vSource);
			auto itOutput = sbufSink.GetWriteIterator();
			vEncoder.Serialize(itOutput);
		}
		static void Deserialize(Integer &vSink, StreamBuffer &sbufSource){
			VarIntEx<Integer> vDecoder;
			auto itInput = sbufSource.GetReadIterator();
			if(!vDecoder.Deserialize(itInput, sbufSource.GetReadEnd())){
				ThrowEndOfStream();
			}
			vSink = vDecoder.Get();
		}
	};
	template<typename FloatingPoint>
	struct SerdesTrait<FloatingPoint,
		typename std::enable_if<
			std::is_floating_point<FloatingPoint>::value
			>::type>
	{
		static void Serialize(StreamBuffer &sbufSink, const FloatingPoint &vSource){
#if __FLOAT_WORD_ORDER__ == __ORDER_BIG_ENDIAN__
			unsigned char abyTemp[sizeof(vSource)];
			ReverseCopyN(abyTemp, sizeof(vSource), (const unsigned char *)(&vSource + 1));
			sbufSink.Insert(abyTemp, sizeof(vSource));
#else
			sbufSink.Insert(&vSource, sizeof(vSource));
#endif
		}
		static void Deserialize(FloatingPoint &vSink, StreamBuffer &sbufSource){
#if __FLOAT_WORD_ORDER__ == __ORDER_BIG_ENDIAN__
			unsigned char abyTemp[sizeof(vSink)];
			if(!sbufSource.Extract(abyTemp, sizeof(vSink))){
				ThrowEndOfStream();
			}
			ReverseCopyBackwardN((unsigned char *)(&vSink + 1), abyTemp, sizeof(vSink));
#else
			if(!sbufSource.Extract(&vSink, sizeof(vSink))){
				ThrowEndOfStream();
			}
#endif
		}
	};

	template class SerdesTrait<bool>;
	template class SerdesTrait<char>;
	template class SerdesTrait<signed char>;
	template class SerdesTrait<unsigned char>;
	template class SerdesTrait<short>;
	template class SerdesTrait<unsigned short>;
	template class SerdesTrait<int>;
	template class SerdesTrait<unsigned int>;
	template class SerdesTrait<long>;
	template class SerdesTrait<unsigned long>;
	template class SerdesTrait<long long>;
	template class SerdesTrait<unsigned long long>;

	template class SerdesTrait<float>;
	template class SerdesTrait<double>;
	template class SerdesTrait<long double>;

	template class SerdesTrait<wchar_t>;
	template class SerdesTrait<char16_t>;
	template class SerdesTrait<char32_t>;

	template<>
	struct SerdesTrait<bool[], void> {
		template<typename InputIterator>
		static void Serialize(StreamBuffer &sbufSink, InputIterator itInput, std::size_t uCount){
			std::size_t i;
			for(i = uCount / 8u; i; --i){
				unsigned char by = 0;
				for(auto j = 8u; j; --j){
					by <<= 1;
					by |= (bool)*itInput;
					++itInput;
				}
				SerdesTrait<unsigned char>::Serialize(sbufSink, by);
			}
			if((i = uCount % 8u) != 0){
				unsigned char by = 0;
				do {
					by <<= 1;
					by |= (bool)*itInput;
					++itInput;
				} while(--i != 0);
				SerdesTrait<unsigned char>::Serialize(sbufSink, by);
			}
		}
		template<typename OutputIterator>
		static void Deserialize(OutputIterator itOutput, std::size_t uCount, StreamBuffer &sbufSource){
			std::size_t i;
			for(i = uCount / 8u; i; --i){
				unsigned char by;
				SerdesTrait<unsigned char>::Deserialize(by, sbufSource);
				for(auto j = 8u; j; --j){
					*itOutput = (bool)(by & 0x80);
					++itOutput;
					by <<= 1;
				}
			}
			if((i = uCount % 8u) != 0){
				unsigned char by;
				SerdesTrait<unsigned char>::Deserialize(by, sbufSource);
				by <<= (8u - i);
				do {
					*itOutput = (bool)(by & 0x80);
					++itOutput;
					by <<= 1;
				} while(--i != 0);
			}
		}
	};
	template<typename Element>
	struct SerdesTrait<Element[], void> {
		template<typename InputIterator>
		static void Serialize(StreamBuffer &sbufSink, InputIterator itInput, std::size_t uCount){
			for(auto i = uCount; i; --i){
				SerdesTrait<Element>::Serialize(sbufSink, *itInput);
				++itInput;
			}
		}
		template<typename OutputIterator>
		static void Deserialize(OutputIterator itOutput, std::size_t uCount, StreamBuffer &sbufSource){
			for(auto i = uCount; i; --i){
				SerdesTrait<Element>::Deserialize(*itOutput, sbufSource);
				++itOutput;
			}
		}
	};

	template<typename ArrayElement, std::size_t ARRAY_SIZE>
	struct SerdesTrait<ArrayElement [ARRAY_SIZE], void> {
		typedef ArrayElement Array[ARRAY_SIZE];
		typedef SerdesTrait<ArrayElement[]> Delegate;

		static void Serialize(StreamBuffer &sbufSink, const Array &vSource){
			Delegate::Serialize(sbufSink, vSource, ARRAY_SIZE);
		}
		static void Deserialize(Array &vSink, StreamBuffer &sbufSource){
			Delegate::Deserialize(vSink, ARRAY_SIZE, sbufSource);
		}
	};

	template<typename First, typename Second>
	struct SerdesTrait<std::pair<First, Second>, void> {
		typedef std::pair<First, Second> Pair;

		static void Serialize(StreamBuffer &sbufSink, const Pair &vSource){
			SerdesTrait<First>::Serialize(sbufSink, vSource.first);
			SerdesTrait<Second>::Serialize(sbufSink, vSource.second);
		}
		static void Deserialize(Pair &vSink, StreamBuffer &sbufSource){
			SerdesTrait<First>::Deserialize(vSink.first, sbufSource);
			SerdesTrait<Second>::Deserialize(vSink.second, sbufSource);
		}
	};

	template<class Tuple, std::size_t TUPLE_SIZE, std::size_t INDEX>
	struct TupleSerdesHelper {
		typedef typename std::tuple_element<INDEX, Tuple>::type Element;
		typedef TupleSerdesHelper<Tuple, TUPLE_SIZE, INDEX + 1> Next;

		static void Serialize(StreamBuffer &sbufSink, const Tuple &vSource){
			SerdesTrait<Element>::Serialize(sbufSink, std::get<INDEX>(vSource));
			Next::Serialize(sbufSink, vSource);
		}
		static void Deserialize(Tuple &vSink, StreamBuffer &sbufSource){
			SerdesTrait<Element>::Deserialize(std::get<INDEX>(vSink), sbufSource);
			Next::Deserialize(vSink, sbufSource);
		}
	};
	template<class Tuple, std::size_t TUPLE_SIZE>
	struct TupleSerdesHelper<Tuple, TUPLE_SIZE, TUPLE_SIZE> {
		static void Serialize(StreamBuffer &, const Tuple &){
		}
		static void Deserialize(Tuple &, StreamBuffer &){
		}
	};

	template<typename... TupleElements>
	struct SerdesTrait<std::tuple<TupleElements...>, void> {
		typedef std::tuple<TupleElements...> Tuple;
		typedef TupleSerdesHelper<Tuple, std::tuple_size<Tuple>::value, 0> First;

		static void Serialize(StreamBuffer &sbufSink, const Tuple &vSource){
			First::Serialize(sbufSink, vSource);
		}
		static void Deserialize(Tuple &vSink, StreamBuffer &sbufSource){
			First::Deserialize(vSink, sbufSource);
		}
	};

	template<typename QueueElement>
	struct SerdesTrait<std::deque<QueueElement>, void> {
		typedef std::deque<QueueElement> Queue;
		typedef SerdesTrait<QueueElement[]> Delegate;

		static void Serialize(StreamBuffer &sbufSink, const Queue &vSource){
			const auto uSize = vSource.size();
			SerdesTrait<unsigned long long>::Serialize(sbufSink, uSize);
			Delegate::Serialize(sbufSink, vSource.begin(), uSize);
		}
		static void Deserialize(Queue &vSink, StreamBuffer &sbufSource){
			unsigned long long ullSize;
			SerdesTrait<unsigned long long>::Deserialize(ullSize, sbufSource);
			if(ullSize > std::numeric_limits<std::size_t>::max()){
				ThrowInvalidData();
			}
			vSink.clear();
			Delegate::Deserialize(std::back_inserter(vSink), ullSize, sbufSource);
		}
	};

	template<>
	struct SerdesTrait<StreamBuffer, void> {
		static void Serialize(StreamBuffer &sbufSink, const StreamBuffer &vSource){
			SerdesTrait<unsigned long long>::Serialize(sbufSink, vSource.GetSize());
			sbufSink.Append(vSource);
		}
		static void Deserialize(StreamBuffer &vSink, StreamBuffer &sbufSource){
			unsigned long long ullSize;
			SerdesTrait<unsigned long long>::Deserialize(ullSize, sbufSource);
			if(ullSize > std::numeric_limits<std::size_t>::max()){
				ThrowInvalidData();
			}
			if(!sbufSource.CutOut(vSink, ullSize)){
				ThrowEndOfStream();
			}
		}
	};
}

template<typename DataType>
void Serialize(StreamBuffer &sbufSink, const DataType &vSource){
	Impl::SerdesTrait<DataType>::Serialize(sbufSink, vSource);
}
template<typename DataType>
void Deserialize(DataType &vSink, StreamBuffer &sbufSource){
	Impl::SerdesTrait<DataType>::Deserialize(vSink, sbufSource);
}

template<typename InputIterator>
InputIterator Serialize(StreamBuffer &sbufSink, InputIterator itInput, std::size_t uCount){
	Impl::SerdesTrait<
		typename std::remove_reference<decltype(*itInput)>::type[]
		>::Serialize(sbufSink, itInput, uCount);
	return std::move(itInput);
}
template<typename OutputIterator>
OutputIterator Deserialize(OutputIterator itOutput, std::size_t uCount, StreamBuffer &sbufSource){
	Impl::SerdesTrait<
		typename std::remove_reference<decltype(*itOutput)>::type[]
		>::Deserialize(itOutput, uCount, sbufSource);
	return std::move(itOutput);
}

}

#endif
