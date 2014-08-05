// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2014, LH_Mouse. All wrongs reserved.

#ifndef MCF_SERDES_HPP_
#define MCF_SERDES_HPP_

#include "../Containers/StreamBuffer.hpp"
#include "../Core/Utilities.hpp"
#include "../Core/Exception.hpp"
#include "VarIntEx.hpp"
#include <type_traits>
#include <iterator>
#include <utility>
#include <limits>
#include <cstddef>

namespace MCF {

// 辅助函数。
[[noreturn]]
inline void ThrowEndOfStream(){
	MCF_THROW(ERROR_HANDLE_EOF, L"遇到文件尾。"_wso);
}
[[noreturn]]
inline void ThrowBadLength(){
	MCF_THROW(ERROR_BAD_LENGTH, L"数据尺寸过大。"_wso);
}
[[noreturn]]
inline void ThrowInvalidData(){
	MCF_THROW(ERROR_INVALID_DATA, L"数据损坏。"_wso);
}

// 这里的运算符分为两种：

// “接口”，全部为非成员函数：
//     StreamBuffer &operator<<(StreamBuffer &, const DataType &);
//     StreamBuffer &operator>>(StreamBuffer &, DataType &);
//     StreamBuffer &&operator<<(StreamBuffer &&, const DataType &);
//     StreamBuffer &&operator>>(StreamBuffer &&, DataType &);

// “实现”，可以是成员函数，也可以不是：
//   非成员函数：
//     void operator>>=(const DataType &, StreamBuffer &);
//     void operator<<=(DataType &, StreamBuffer &);
//   成员函数：
//     void operator>>=(StreamBuffer &) const;
//     void operator<<=(StreamBuffer &);

// “接口”的定义。接口是统一的，这里不需进行特化。
template<typename DataType>
StreamBuffer &operator<<(StreamBuffer &sbufSink, const DataType &vSource){
	vSource >>= sbufSink;
	return sbufSink;
}
template<typename DataType>
StreamBuffer &operator>>(StreamBuffer &sbufSource, DataType &vSink){
	vSink <<= sbufSource;
	return sbufSource;
}

template<typename DataType>
StreamBuffer &&operator<<(StreamBuffer &&sbufSink, const DataType &vSource){
	return std::move(sbufSink << vSource);
}
template<typename DataType>
StreamBuffer &&operator>>(StreamBuffer &&sbufSource, DataType &vSink){
	return std::move(sbufSource >> vSink);
}

// “实现”的定义。此文件内定义针对整型、枚举类型、浮点型、std::pair 以及内建数组类型的特化。
// 这些类型都无法定义成成员函数。
inline void operator>>=(bool bSource, StreamBuffer &sbufSink){
	sbufSink.Put((unsigned char)bSource);
}
inline void operator<<=(bool &bSink, StreamBuffer &sbufSource){
	const int nVal = sbufSource.Get();
	if(nVal == -1){
		ThrowEndOfStream();
	}
	bSink = (nVal != 0);
}

template<typename Char,
	typename std::enable_if<
		std::is_integral<Char>::value && (sizeof(Char) == 1),
		int>::type
	= 0>
void operator>>=(Char chSource, StreamBuffer &sbufSink){
	sbufSink.Put(reinterpret_cast<const unsigned char &>(chSource));
}
template<typename Char,
	typename std::enable_if<
		std::is_integral<Char>::value && (sizeof(Char) == 1),
		int>::type
	= 0>
void operator<<=(Char &chSink, StreamBuffer &sbufSource){
	const int nVal = sbufSource.Get();
	if(nVal == -1){
		ThrowEndOfStream();
	}
	const unsigned char byTemp = nVal;
	chSink = reinterpret_cast<const Char &>(byTemp);
}

template<typename Integral,
	typename std::enable_if<
		std::is_integral<Integral>::value && (sizeof(Integral) > 1),
		int>::type
	= 0>
void operator>>=(Integral vSource, StreamBuffer &sbufSink){
	const VarIntEx<Integral> vEncoder(vSource);
	auto itWrite = sbufSink.GetWriteIterator();
	vEncoder.Serialize(itWrite);
}
template<typename Integral,
	typename std::enable_if<
		std::is_integral<Integral>::value && (sizeof(Integral) > 1),
		int>::type
	= 0>
void operator<<=(Integral &vSink, StreamBuffer &sbufSource){
	VarIntEx<Integral> vDecoder;
	auto itInput = sbufSource.GetReadIterator();
	if(!vDecoder.Deserialize(itInput, sbufSource.GetSize())){
		ThrowEndOfStream();
	}
	vSink = vDecoder.Get();
}

template<typename Enum,
	typename std::enable_if<
		std::is_enum<Enum>::value,
		int>::type
	= 0>
void operator>>=(Enum eSource, StreamBuffer &sbufSink){
	static_cast<typename std::underlying_type<Enum>::type>(eSource) >>= sbufSink;
}
template<typename Enum,
	typename std::enable_if<
		std::is_enum<Enum>::value,
		int>::type
	= 0>
void operator<<=(Enum &eSink, StreamBuffer &sbufSource){
	reinterpret_cast<typename std::underlying_type<Enum>::type &>(eSink) <<= sbufSource;
}

template<typename FloatingPoint,
	typename std::enable_if<
		std::is_floating_point<FloatingPoint>::value,
		int>::type
	= 0>
void operator>>=(const FloatingPoint &vSource, StreamBuffer &sbufSink){
#if __FLOAT_WORD_ORDER__ == __ORDER_BIG_ENDIAN__
	unsigned char abyTemp[sizeof(vSource)];
	ReverseCopyN(abyTemp, sizeof(vSource), reinterpret_cast<const unsigned char *>(&vSource + 1));
	sbufSink.Insert(abyTemp, sizeof(vSource));
#else
	sbufSink.Insert(&vSource, sizeof(vSource));
#endif
}
template<typename FloatingPoint,
	typename std::enable_if<
		std::is_floating_point<FloatingPoint>::value,
		int>::type
	= 0>
void operator<<=(FloatingPoint &vSink, StreamBuffer &sbufSource){
#if __FLOAT_WORD_ORDER__ == __ORDER_BIG_ENDIAN__
	unsigned char abyTemp[sizeof(vSink)];
	if(!sbufSource.Extract(abyTemp, sizeof(vSink))){
		ThrowEndOfStream();
	}
	ReverseCopyBackwardN(reinterpret_cast<unsigned char *>(&vSink + 1), abyTemp, sizeof(vSink));
#else
	if(!sbufSource.Extract(&vSink, sizeof(vSink))){
		ThrowEndOfStream();
	}
#endif
}

// 小工具。
template<typename ValueType, class InputIterator>
class SeqInserter {
private:
	const InputIterator xm_itBegin;
	const std::size_t xm_uCount;

public:
	SeqInserter(InputIterator itBegin, std::size_t uCount)
		: xm_itBegin	(std::move(itBegin))
		, xm_uCount		(uCount)
	{
	}

public:
	void operator>>=(StreamBuffer &sbufSink) const {
		auto itRead = xm_itBegin;
		for(std::size_t i = 0; i < xm_uCount; ++i){
			static_cast<const ValueType &>(*itRead) >>= sbufSink;
			++itRead;
		}
	}
};
template<typename ValueType, class OutputIterator>
class SeqExtractor {
private:
	const OutputIterator xm_itBegin;
	const std::size_t xm_uCount;

public:
	constexpr SeqExtractor(OutputIterator itBegin, std::size_t uCount)
		: xm_itBegin	(std::move(itBegin))
		, xm_uCount		(uCount)
	{
	}

public:
	void operator<<=(StreamBuffer &sbufSink) const {
		auto itWrite = xm_itBegin;
		for(std::size_t i = 0; i < xm_uCount; ++i){
			ValueType vTemp;
			vTemp <<= sbufSink;
			*itWrite = std::move(vTemp);
			++itWrite;
		}
	}
};

template<class InputIterator>
class SeqInserter<bool, InputIterator> {
private:
	const InputIterator xm_itBegin;
	const std::size_t xm_uCount;

public:
	SeqInserter(InputIterator itBegin, std::size_t uCount)
		: xm_itBegin	(std::move(itBegin))
		, xm_uCount		(uCount)
	{
	}

public:
	void operator>>=(StreamBuffer &sbufSink) const {
		auto itRead = xm_itBegin;
		std::size_t i;
		for(i = xm_uCount / 8u; i; --i){
			unsigned char by = 0;
			for(auto j = 8u; j; --j){
				by <<= 1;
				by |= static_cast<bool>(*itRead);
				++itRead;
			}
			by >>= sbufSink;
		}
		if((i = xm_uCount % 8u) != 0){
			unsigned char by = 0;
			do {
				by <<= 1;
				by |= static_cast<bool>(*itRead);
				++itRead;
			} while(--i != 0);
			by >>= sbufSink;
		}
	}
};
template<class OutputIterator>
class SeqExtractor<bool, OutputIterator> {
private:
	const OutputIterator xm_itBegin;
	const std::size_t xm_uCount;

public:
	constexpr SeqExtractor(OutputIterator itBegin, std::size_t uCount)
		: xm_itBegin	(std::move(itBegin))
		, xm_uCount		(uCount)
	{
	}

public:
	void operator<<=(StreamBuffer &sbufSource) const {
		auto itWrite = xm_itBegin;
		std::size_t i;
		for(i = xm_uCount / 8u; i; --i){
			unsigned char by;
			by <<= sbufSource;
			for(auto j = 8u; j; --j){
				*itWrite = static_cast<bool>(by & 0x80);
				++itWrite;
				by <<= 1;
			}
		}
		if((i = xm_uCount % 8u) != 0){
			unsigned char by;
			by <<= sbufSource;
			by <<= (8u - i);
			do {
				*itWrite = static_cast<bool>(by & 0x80);
				++itWrite;
				by <<= 1;
			} while(--i != 0);
		}
	}
};

template<typename ValueType = void, class InputIterator>
auto MakeSeqInserter(InputIterator itBegin, std::size_t uCount){
	return SeqInserter<
		typename std::conditional<std::is_void<ValueType>::value,
			typename std::iterator_traits<InputIterator>::value_type, ValueType
			>::type,
			InputIterator
		>(std::move(itBegin), uCount);
}
template<typename ValueType = void, class OutputIterator>
auto MakeSeqExtractor(OutputIterator itBegin, std::size_t uCount){
	return SeqExtractor<
		typename std::conditional<std::is_void<ValueType>::value,
			typename std::iterator_traits<OutputIterator>::value_type, ValueType
			>::type,
			OutputIterator
		>(std::move(itBegin), uCount);
}

// 特化重载。
template<class Element, std::size_t SIZE>
void operator>>=(const Element (&aSource)[SIZE], StreamBuffer &sbufSink){
	MakeSeqInserter(aSource, SIZE) >>= sbufSink;
}
template<class Element, std::size_t SIZE>
void operator<<=(Element (&aSink)[SIZE], StreamBuffer &sbufSource){
	MakeSeqExtractor(aSink, SIZE) <<= sbufSource;
}

template<typename First, typename Second>
void operator>>=(const std::pair<First, Second> &vSource, StreamBuffer &sbufSink){
	vSource.first >>= sbufSink;
	vSource.second >>= sbufSink;
}
template<typename First, typename Second>
void operator<<=(std::pair<First, Second> &vSink, StreamBuffer &sbufSource){
	vSink.first <<= sbufSource;
	vSink.second <<= sbufSource;
}

inline void operator>>=(const StreamBuffer &sbufSource, StreamBuffer &sbufSink){
	sbufSource.GetSize() >>= sbufSink;
	sbufSink.Append(sbufSource);
}
inline void operator<<=(StreamBuffer &sbufSink, StreamBuffer &sbufSource){
	std::size_t uSize;
	uSize <<= sbufSource;
	sbufSource.CutOut(sbufSink, uSize);
}

}

#endif
