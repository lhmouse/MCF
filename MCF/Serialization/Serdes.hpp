// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2014, LH_Mouse. All wrongs reserved.

#ifndef MCF_SERDES_HPP_
#define MCF_SERDES_HPP_

#include "../Containers/StreamBuffer.hpp"
#include "../Core/Utilities.hpp"
#include "VarIntEx.hpp"
#include <type_traits>
#include <utility>
#include <tuple>
#include <limits>
#include <cstddef>

namespace MCF {

// 辅助函数。
[[noreturn]]
extern void ThrowEndOfStream();
[[noreturn]]
extern void ThrowSizeTooLarge();
[[noreturn]]
extern void ThrowInvalidData();

// 通用接口声明。
template<typename DataType>
void Serialize(StreamBuffer &sbufSink, const DataType &vSource);
template<typename DataType>
void Deserialize(DataType &vSink, StreamBuffer &sbufSource);

template<typename DataType = void, typename InputIterator>
InputIterator Serialize(StreamBuffer &sbufSink, InputIterator itInput, std::size_t uCount);
template<typename DataType = void, typename OutputIterator>
OutputIterator Deserialize(OutputIterator itOutput, std::size_t uCount, StreamBuffer &sbufSource);

namespace Impl {
	template<typename DataType, typename TraitHelper = void>
	struct SerdesTrait {
		static_assert((sizeof(DataType) - 1, false), "Not supported.");

		static void DoSerialize(StreamBuffer &sbufSink, const DataType &vSource);
		static void DoDeserialize(DataType &vSink, StreamBuffer &sbufSource);
	};

	template<>
	struct SerdesTrait<bool, void> {
		static void DoSerialize(StreamBuffer &sbufSink, bool vSource){
			sbufSink.Put((unsigned char)vSource);
		}
		static void DoDeserialize(bool &vSink, StreamBuffer &sbufSource){
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
		static void DoSerialize(StreamBuffer &sbufSink, Char vSource){
			sbufSink.Put((unsigned char)vSource);
		}
		static void DoDeserialize(Char &vSink, StreamBuffer &sbufSource){
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
		static void DoSerialize(StreamBuffer &sbufSink, Integer vSource){
			VarIntEx<Integer> vEncoder;
			vEncoder.Set(vSource);
			auto itOutput = sbufSink.GetWriteIterator();
			vEncoder.Serialize(itOutput);
		}
		static void DoDeserialize(Integer &vSink, StreamBuffer &sbufSource){
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
		static void DoSerialize(StreamBuffer &sbufSink, const FloatingPoint &vSource){
#if __FLOAT_WORD_ORDER__ == __ORDER_BIG_ENDIAN__
			unsigned char abyTemp[sizeof(vSource)];
			ReverseCopyN(abyTemp, sizeof(vSource), (const unsigned char *)(&vSource + 1));
			sbufSink.Insert(abyTemp, sizeof(vSource));
#else
			sbufSink.Insert(&vSource, sizeof(vSource));
#endif
		}
		static void DoDeserialize(FloatingPoint &vSink, StreamBuffer &sbufSource){
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
		static void DoSerialize(StreamBuffer &sbufSink, InputIterator itInput, std::size_t uCount){
			std::size_t i;
			for(i = uCount / 8u; i; --i){
				unsigned char by = 0;
				for(auto j = 8u; j; --j){
					by <<= 1;
					by |= (bool)*itInput;
					++itInput;
				}
				Serialize<unsigned char>(sbufSink, by);
			}
			if((i = uCount % 8u) != 0){
				unsigned char by = 0;
				do {
					by <<= 1;
					by |= (bool)*itInput;
					++itInput;
				} while(--i != 0);
				Serialize<unsigned char>(sbufSink, by);
			}
		}
		template<typename OutputIterator>
		static void DoDeserialize(OutputIterator itOutput, std::size_t uCount, StreamBuffer &sbufSource){
			std::size_t i;
			for(i = uCount / 8u; i; --i){
				unsigned char by;
				Deserialize<unsigned char>(by, sbufSource);
				for(auto j = 8u; j; --j){
					*itOutput = (bool)(by & 0x80);
					++itOutput;
					by <<= 1;
				}
			}
			if((i = uCount % 8u) != 0){
				unsigned char by;
				Deserialize<unsigned char>(by, sbufSource);
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
		static void DoSerialize(StreamBuffer &sbufSink, InputIterator itInput, std::size_t uCount){
			for(auto i = uCount; i; --i){
				Serialize<Element>(sbufSink, *itInput);
				++itInput;
			}
		}
		template<typename OutputIterator>
		static void DoDeserialize(OutputIterator itOutput, std::size_t uCount, StreamBuffer &sbufSource){
			for(auto i = uCount; i; --i){
				Element vTemp;
				Deserialize<Element>(vTemp, sbufSource);
				*itOutput = std::move(vTemp);
				++itOutput;
			}
		}
	};

	template<typename ArrayElement, std::size_t ARRAY_SIZE>
	struct SerdesTrait<ArrayElement [ARRAY_SIZE], void> {
		typedef ArrayElement Array[ARRAY_SIZE];

		static void DoSerialize(StreamBuffer &sbufSink, const Array &vSource){
			Serialize<ArrayElement>(sbufSink, vSource, ARRAY_SIZE);
		}
		static void DoDeserialize(Array &vSink, StreamBuffer &sbufSource){
			Deserialize<ArrayElement>(vSink, ARRAY_SIZE, sbufSource);
		}
	};

	template<typename First, typename Second>
	struct SerdesTrait<std::pair<First, Second>, void> {
		typedef std::pair<First, Second> Pair;

		static void DoSerialize(StreamBuffer &sbufSink, const Pair &vSource){
			Serialize<First>(sbufSink, vSource.first);
			Serialize<Second>(sbufSink, vSource.second);
		}
		static void DoDeserialize(Pair &vSink, StreamBuffer &sbufSource){
			Deserialize<First>(vSink.first, sbufSource);
			Deserialize<Second>(vSink.second, sbufSource);
		}
	};
}

// 辅助函数。
inline void SerializeSize(StreamBuffer &sbufSink, std::size_t uSize){
	Serialize<std::uint64_t>(sbufSink, uSize);
}
inline std::size_t DeserializeSize(StreamBuffer &sbufSource){
	std::uint64_t u64Size;
	Deserialize<std::uint64_t>(u64Size, sbufSource);
	if(u64Size > std::numeric_limits<std::size_t>::max()){
		ThrowSizeTooLarge();
	}
	return u64Size;
}

// 通用接口定义。
template<typename DataType>
void Serialize(StreamBuffer &sbufSink, const DataType &vSource){
	Impl::SerdesTrait<DataType>::DoSerialize(sbufSink, vSource);
}
template<typename DataType>
void Deserialize(DataType &vSink, StreamBuffer &sbufSource){
	Impl::SerdesTrait<DataType>::DoDeserialize(vSink, sbufSource);
}

template<typename DataType, typename InputIterator>
InputIterator Serialize(StreamBuffer &sbufSink, InputIterator itInput, std::size_t uCount){
	Impl::SerdesTrait<
		typename std::conditional<std::is_void<DataType>::value,
			typename std::iterator_traits<InputIterator>::value_type, DataType
			>::type[]
		>::DoSerialize(sbufSink, itInput, uCount);
	return std::move(itInput);
}
template<typename DataType, typename OutputIterator>
OutputIterator Deserialize(OutputIterator itOutput, std::size_t uCount, StreamBuffer &sbufSource){
	Impl::SerdesTrait<
		typename std::conditional<std::is_void<DataType>::value,
			typename std::iterator_traits<OutputIterator>::value_type, DataType
			>::type[]
		>::DoDeserialize(itOutput, uCount, sbufSource);
	return std::move(itOutput);
}

// 特化重载。
template<>
inline void Serialize(StreamBuffer &sbufSink, const StreamBuffer &vSource){
	SerializeSize(sbufSink,  vSource.GetSize());
	sbufSink.Append(vSource);
}
template<>
inline void Deserialize(StreamBuffer &vSink, StreamBuffer &sbufSource){
	const std::size_t uSize = DeserializeSize(sbufSource);
	if(!sbufSource.CutOut(vSink, uSize)){
		ThrowEndOfStream();
	}
}

}

#endif
