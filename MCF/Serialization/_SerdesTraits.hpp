// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2014. LH_Mouse. All wrongs reserved.

// 这个文件包含了一大堆模板，所以很拖编译速度。

#ifndef MCF_SERDES_TRAITS_HPP_
#define MCF_SERDES_TRAITS_HPP_

#include "VarIntEx.hpp"
#include "../Core/StreamBuffer.hpp"
#include <iterator>
#include <type_traits>
#include <array>
#include <bitset>
#include <string>
#include <list>
#include <deque>
#include <vector>
#include <forward_list>
#include <set>
#include <map>
#include <unordered_set>
#include <unordered_map>
#include <functional>
#include <cstddef>

namespace MCF {

namespace Impl {
	[[noreturn]] extern void ThrowOnEof();

	template<typename Object_t, typename = void>
	struct SerdesTrait;

	// 针对 bool 的特化。使用最小的存储。注意 bool 本身的大小是实现定义的，有可能当作 int 了。
	template<>
	struct SerdesTrait<
		bool,
		void
	> {
		void operator()(StreamBuffer &sbufStream, const bool &vObject) const {
			const unsigned char by = vObject;
			sbufStream.Insert(&by, sizeof(by));
		}
		void operator()(bool &vObject, StreamBuffer &sbufStream) const {
			unsigned char by = vObject;
			if(!sbufStream.Extract(&by, sizeof(by))){
				ThrowOnEof();
			}
			vObject = by;
		}
	};

	// 针对比较大的整数类型（bool 除外）的特化。
	template<typename Integral>
	struct SerdesTrait<
		Integral,
		typename std::enable_if<
			std::is_integral<Integral>::value && !std::is_same<Integral, bool>::value && (sizeof(Integral) > 2u)
		>::type
	> {
		void operator()(StreamBuffer &sbufStream, const Integral &vObject) const {
			VarIntEx<Integral> viTemp(vObject);

			unsigned char abyTemp[viTemp.MAX_SERIALIZED_SIZE];
			auto pbyWrite = abyTemp;
			viTemp.Serialize(pbyWrite);

			sbufStream.Insert(abyTemp, (std::size_t)(pbyWrite - abyTemp));
		}
		void operator()(Integral &vObject, StreamBuffer &sbufStream) const {
			VarIntEx<Integral> viTemp;

			auto itRead = sbufStream.GetReadIterator();
			if(!viTemp.Unserialize(itRead, sbufStream.GetReadEnd())){
				ThrowOnEof();
			}

			vObject = viTemp.Get();
		}
	};

	// 针对指针的特化。指针用于持久存储无任何意义，因此指针不能序列化。
	template<typename Object_t>
	struct SerdesTrait<Object_t *, void> {
		static_assert((sizeof(Object_t), false), "Pointers can't be serialized.");
	};

	// 针对 nullptr_t 的特化。什么都不做。这个特化貌似只有理论上的意义。
	template<>
	struct SerdesTrait<std::nullptr_t, void> {
		void operator()(StreamBuffer &, const std::nullptr_t &) const {
		}
		void operator()(std::nullptr_t &, StreamBuffer &) const {
		}
	};

	// 针对枚举类型的特化。当作整数对待。
	template<typename Enum_t>
	struct SerdesTrait<
		Enum_t,
		typename std::enable_if<
			std::is_enum<Enum_t>::value
		>::type
	> {
		typedef typename std::underlying_type<Enum_t>::type UnderlyingType;

		void operator()(StreamBuffer &sbufStream, const Enum_t &vElement) const {
			SerdesTrait<UnderlyingType>()(sbufStream, (const UnderlyingType &)vElement);
		}
		void operator()(Enum_t &vElement, StreamBuffer &sbufStream) const {
			SerdesTrait<UnderlyingType>()((UnderlyingType &)vElement, sbufStream);
		}
	};

	// 针对其他标量类型（较小的整数，浮点数，指向成员的指针等）的特化。
	template<typename Scalar_t>
	struct SerdesTrait<
		Scalar_t,
		typename std::enable_if<
			std::is_scalar<Scalar_t>::value
			&& !std::is_same<Scalar_t, bool>::value
			&& !(std::is_integral<Scalar_t>::value && (sizeof(Scalar_t) > 2u))
			&& !std::is_pointer<Scalar_t>::value
			&& !std::is_null_pointer<Scalar_t>::value
			&& !std::is_enum<Scalar_t>::value
		>::type
	> {
		void operator()(StreamBuffer &sbufStream, const Scalar_t &vElement) const {
			sbufStream.Insert(&vElement, sizeof(vElement));
		}
		void operator()(Scalar_t &vElement, StreamBuffer &sbufStream) const {
			if(!sbufStream.Extract(&vElement, sizeof(vElement))){
				ThrowOnEof();
			}
		}
	};

	// 针对未知大小的内建数组的特化。
	template<typename Element_t>
	struct SerdesTrait<
		Element_t[],
		void
	> {
		void operator()(StreamBuffer &sbufStream, const Element_t *pElements, std::size_t uSize) const {
			for(std::size_t i = 0; i < uSize; ++i){
				SerdesTrait<Element_t>()(sbufStream, pElements[i]);
			}
		}
		void operator()(Element_t *pElements, std::size_t uSize, StreamBuffer &sbufStream) const {
			for(std::size_t i = 0; i < uSize; ++i){
				SerdesTrait<Element_t>()(pElements[i], sbufStream);
			}
		}
	};

	// 针对未知大小的 bool 数组的特化。
	template<>
	struct SerdesTrait<
		bool[],
		void
	> {
		void operator()(StreamBuffer &sbufStream, const bool *pBools, std::size_t uSize) const {
			const std::size_t uByteCount = uSize / 8;
			for(std::size_t i = 0; i < uByteCount; ++i){
				unsigned char by = 0;
				for(std::size_t j = 0; j < 8; ++j){
					by <<= 1;
					by |= *(pBools++);
				}
				SerdesTrait<std::uint8_t>()(sbufStream, by);
			}
			const std::size_t uRemaining = uSize % 8;
			if(uRemaining != 0){
				std::uint8_t by = 0;
				for(std::size_t i = 0; i < uRemaining; ++i){
					by <<= 1;
					by |= *(pBools++);
				}
				SerdesTrait<std::uint8_t>()(sbufStream, by);
			}
		}
		void operator()(bool *pBools, std::size_t uSize, StreamBuffer &sbufStream) const {
			const std::size_t uByteCount = uSize / 8;
			for(std::size_t i = 0; i < uByteCount; ++i){
				std::uint8_t by;
				SerdesTrait<std::uint8_t>()(by, sbufStream);
				for(std::size_t j = 0; j < 8; ++j){
					*(pBools++) = ((by & 0x80) != 0);
					by <<= 1;
				}
			}
			const std::size_t uRemaining = uSize % 8;
			if(uRemaining != 0){
				std::uint8_t by;
				SerdesTrait<std::uint8_t>()(by, sbufStream);
				by <<= 8 - uRemaining;
				for(std::size_t j = 0; j < uRemaining; ++j){
					*(pBools++) = ((by & 0x80) != 0);
					by <<= 1;
				}
			}
		}
	};

	// 针对确定大小的内建数组的特化。
	template<typename Element_t, std::size_t SIZE>
	struct SerdesTrait<
		Element_t[SIZE],
		void
	> {
		void operator()(StreamBuffer &sbufStream, const Element_t (&arElements)[SIZE]) const {
			SerdesTrait<Element_t[]>()(sbufStream, arElements, SIZE);
		}
		void operator()(Element_t (&arElements)[SIZE], StreamBuffer &sbufStream) const {
			SerdesTrait<Element_t[]>()(arElements, SIZE, sbufStream);
		}
	};

	// 针对 pair 的特化。
	template<typename First_t, typename Second_t>
	struct SerdesTrait<
		std::pair<First_t, Second_t>,
		void
	> {
		void operator()(StreamBuffer &sbufStream, const std::pair<First_t, Second_t> &vPair) const {
			SerdesTrait<First_t>()(sbufStream, vPair.first);
			SerdesTrait<Second_t>()(sbufStream, vPair.second);
		}
		void operator()(std::pair<First_t, Second_t> &vPair, StreamBuffer &sbufStream) const {
			SerdesTrait<First_t>()(vPair.first, sbufStream);
			SerdesTrait<Second_t>()(vPair.second, sbufStream);
		}
	};

	// 针对 tuple 的特化。
	template<typename... Elements_t>
	struct SerdesTrait<
		std::tuple<Elements_t...>,
		void
	> {
		template<std::size_t INDEX, typename std::enable_if<(INDEX < sizeof...(Elements_t)), int>::type = 0>
		static void SerializeAll(StreamBuffer &sbufStream, const std::tuple<Elements_t...> &vTuple){
			typedef typename std::tuple_element<INDEX, std::tuple<Elements_t...>>::type ElementType;

			SerdesTrait<ElementType>()(sbufStream, std::get<INDEX>(vTuple));
			SerializeAll<INDEX + 1>(sbufStream, vTuple);
		}
		template<std::size_t INDEX, typename std::enable_if<(INDEX == sizeof...(Elements_t)), int>::type = 0>
		static void SerializeAll(StreamBuffer &, const std::tuple<Elements_t...> &){
		}

		template<std::size_t INDEX, typename std::enable_if<(INDEX < sizeof...(Elements_t)), int>::type = 0>
		static void DeserializeAll(std::tuple<Elements_t...> &vTuple, StreamBuffer &sbufStream){
			typedef typename std::tuple_element<INDEX, std::tuple<Elements_t...>>::type ElementType;

			SerdesTrait<ElementType>()(std::get<INDEX>(vTuple), sbufStream);
			DeserializeAll<INDEX + 1>(vTuple, sbufStream);
		}
		template<std::size_t INDEX, typename std::enable_if<(INDEX == sizeof...(Elements_t)), int>::type = 0>
		static void DeserializeAll(std::tuple<Elements_t...> &, StreamBuffer &){
		}

		void operator()(StreamBuffer &sbufStream, const std::tuple<Elements_t...> &vTuple) const {
			SerializeAll<0>(sbufStream, vTuple);
		}
		void operator()(std::tuple<Elements_t...> &vTuple, StreamBuffer &sbufStream) const {
			DeserializeAll<0>(vTuple, sbufStream);
		}
	};

	// 下面是针对标准库中的容器的特化。
	// 容器分为以下几类：
	// 1. 固定大小的（2 个）：
	//    array, bitset
	// 2. 支持 push_back() 的（5 个）：
	//    basic_string, list, deque, vector, vector<bool>
	// 3. 支持 emplace_front() 的（1 个）：
	//    forward_list
	// 4. 支持 emplace_hint() 的（8 个）：
	//    set, multiset, unordered_set, unordered_multiset,
	//    map, multimap, unordered_map, unordered_multimap

	// 针对 array 的特化。
	// 同内建数组。
	template<typename Element_t, std::size_t SIZE>
	struct SerdesTrait<
		std::array<Element_t, SIZE>,
		void
	> {
		void operator()(StreamBuffer &sbufStream, const std::array<Element_t, SIZE> &vArray) const {
			SerdesTrait<Element_t[]>()(sbufStream, vArray.data(), vArray.size());
		}
		void operator()(std::array<Element_t, SIZE> &vArray, StreamBuffer &sbufStream) const {
			SerdesTrait<Element_t[]>()(vArray.data(), vArray.size(), sbufStream);
		}
	};

	// 针对 bitset 的特化。
	// 注意 SerdesTrait<bool[]> 是特化的。
	template<std::size_t SIZE>
	struct SerdesTrait<
		std::bitset<SIZE>,
		void
	> {
		void operator()(StreamBuffer &sbufStream, const std::bitset<SIZE> &vBitSet) const {
			bool abTemp[SIZE];
			for(std::size_t i = 0; i < SIZE; ++i){
				abTemp[i] = vBitSet[i];
			}
			SerdesTrait<bool[]>()(sbufStream, abTemp, SIZE);
		}
		void operator()(std::bitset<SIZE> &vBitSet, StreamBuffer &sbufStream) const {
			bool abTemp[SIZE];
			SerdesTrait<bool[]>()(abTemp, SIZE, sbufStream);
			for(std::size_t i = 0; i < SIZE; ++i){
				vBitSet[i] = abTemp[i];
			}
		}
	};

	// 针对 basic_string 的特化。
	template<typename Char_t, typename Trait_t, typename Allocator_t>
	struct SerdesTrait<
		std::basic_string<Char_t, Trait_t, Allocator_t>,
		void
	> {
		void operator()(StreamBuffer &sbufStream, const std::basic_string<Char_t, Trait_t, Allocator_t> &vBasicString) const {
			const std::size_t uSize = vBasicString.size();
			SerdesTrait<std::uint64_t>()(sbufStream, uSize);
			SerdesTrait<Char_t[]>()(sbufStream, vBasicString.c_str(), uSize);
		}
		void operator()(std::basic_string<Char_t, Trait_t, Allocator_t> &vBasicString, StreamBuffer &sbufStream) const {
			std::uint64_t u64Size;
			SerdesTrait<std::uint64_t>()(u64Size, sbufStream);
			vBasicString.resize(u64Size);
			SerdesTrait<Char_t[]>()(&(vBasicString[0]), (std::size_t)u64Size, sbufStream);
		}
	};

	// 针对 list, deque, vector 的特化。
	// 检测 emplace_back()。
	template<typename Container_t>
	struct SerdesTrait<
		Container_t,
		typename std::enable_if<
			!(
				std::is_same<std::vector<typename Container_t::value_type>, Container_t>::value
				&& std::is_same<typename Container_t::value_type, bool>::value
			),
			decltype(std::declval<Container_t>().emplace_back(), (void)0)
		>::type
	> {
		typedef typename Container_t::value_type ElementType;

		void operator()(StreamBuffer &sbufStream, const Container_t &vContainer) const {
			const std::size_t uSize = vContainer.size();
			SerdesTrait<std::uint64_t>()(sbufStream, uSize);

			auto itRead = vContainer.begin();
			for(auto i = uSize; i; --i){
				SerdesTrait<ElementType>()(sbufStream, *itRead);
				++itRead;
			}
		}
		void operator()(Container_t &vContainer, StreamBuffer &sbufStream) const {
			std::uint64_t u64Size;
			SerdesTrait<std::uint64_t>()(u64Size, sbufStream);
			vContainer.resize(u64Size);
			auto itWrite = vContainer.begin();
			for(std::size_t i = u64Size; i; --i){
				SerdesTrait<ElementType>()(*itWrite, sbufStream);
				++itWrite;
			}
		}
	};

	// 针对 vector<bool> 的特化。
	template<typename Allocator_t>
	struct SerdesTrait<
		std::vector<bool, Allocator_t>,
		void
	> {
		void operator()(StreamBuffer &sbufStream, const std::vector<bool, Allocator_t> &vVectorBool) const {
			const auto uSize = vVectorBool.size();
			SerdesTrait<std::uint64_t>()(sbufStream, uSize);
			std::unique_ptr<bool[]> pTemp(new bool[uSize]);
			std::copy(vVectorBool.begin(), vVectorBool.end(), pTemp.get());
			SerdesTrait<bool[]>()(sbufStream, pTemp.get(), uSize);
		}
		void operator()(std::vector<bool, Allocator_t> &vVectorBool, StreamBuffer &sbufStream) const {
			std::uint64_t u64Size;
			SerdesTrait<std::uint64_t>()(u64Size, sbufStream);
			std::unique_ptr<bool[]> pTemp(new bool[(std::size_t)u64Size]);
			SerdesTrait<bool[]>()(pTemp.get(), u64Size, sbufStream);
			vVectorBool.resize(u64Size);
			std::copy_n(pTemp.get(), u64Size, vVectorBool.begin());
		}
	};

	// 针对 forward_list 的特化。
	template<typename Element_t, typename Allocator_t>
	struct SerdesTrait<
		std::forward_list<Element_t, Allocator_t>,
		void
	> {
		void operator()(StreamBuffer &sbufStream, const std::forward_list<Element_t, Allocator_t> &vForwardList) const {
			std::size_t uSize = 0;
			StreamBuffer sbufTemp;
			for(const auto vElement : vForwardList){
				SerdesTrait<Element_t>()(sbufTemp, vElement);
				++uSize;
			}
			SerdesTrait<std::uint64_t>()(sbufStream, uSize);
			sbufStream.Append(std::move(sbufTemp));
		}
		void operator()(std::forward_list<Element_t, Allocator_t> &vForwardList, StreamBuffer &sbufStream) const {
			std::uint64_t u64Size;
			SerdesTrait<std::uint64_t>()(u64Size, sbufStream);
			vForwardList.clear();
			for(std::size_t i = u64Size; i; --i){
				vForwardList.emplace_front();
				SerdesTrait<Element_t>()(vForwardList.front(), sbufStream);
			}
			vForwardList.reverse();
		}
	};

	// 针对 set, multiset, unordered_set, unordered_multiset 的特化。
	// 检测 emplace_hint() 返回的类型是否是 const value_type。
	template<typename Container_t>
	struct SerdesTrait<
		Container_t,
		typename std::enable_if<
			std::is_same<
				const typename Container_t::value_type,
				typename std::remove_reference<decltype(
					*std::declval<Container_t>().emplace_hint(
						std::declval<typename Container_t::const_iterator>(),
						std::declval<typename Container_t::value_type>()
					)
				)>::type
			>::value
		>::type
	> {
		typedef typename std::remove_const<typename Container_t::value_type>::type ElementType;

		void operator()(StreamBuffer &sbufStream, const Container_t &vContainer) const {
			const std::size_t uSize = vContainer.size();
			SerdesTrait<std::uint64_t>()(sbufStream, uSize);
			for(const auto &vElement : vContainer){
				SerdesTrait<ElementType>()(sbufStream, vElement);
			}
		}
		void operator()(Container_t &vContainer, StreamBuffer &sbufStream) const {
			std::uint64_t u64Size;
			SerdesTrait<std::uint64_t>()(u64Size, sbufStream);
			ElementType vTempElement;
			vContainer.clear();
			for(std::size_t i = u64Size; i; --i){
				SerdesTrait<ElementType>()(vTempElement, sbufStream);
				vContainer.emplace_hint(vContainer.end(), std::move(vTempElement));
			}
		}
	};

	// 针对 map, multimap, unordered_map, unordered_multimap 的特化。
	// 检测 emplace_hint() 返回的类型是否是 pair, 且其 first 成员的类型为 const value_type::first_type。
	template<typename Container_t>
	struct SerdesTrait<
		Container_t,
		typename std::enable_if<
			std::is_same<
				const typename Container_t::value_type::first_type,
				typename std::remove_reference<decltype(
					std::declval<Container_t>().emplace_hint(
						std::declval<typename Container_t::const_iterator>(),
						std::declval<typename Container_t::value_type>()
					)->first
				)>::type
			>::value
		>::type
	> {
		typedef std::pair<
			typename std::remove_const<typename Container_t::value_type::first_type>::type,
			typename Container_t::value_type::second_type
		> ElementType;

		void operator()(StreamBuffer &sbufStream, const Container_t &vContainer) const {
			const std::size_t uSize = vContainer.size();
			SerdesTrait<std::uint64_t>()(sbufStream, uSize);
			for(const auto &vElement : vContainer){
				SerdesTrait<ElementType>()(sbufStream, vElement);
			}
		}
		void operator()(Container_t &vContainer, StreamBuffer &sbufStream) const {
			std::uint64_t u64Size;
			SerdesTrait<std::uint64_t>()(u64Size, sbufStream);
			ElementType vTempElement;
			vContainer.clear();
			for(std::size_t i = u64Size; i; --i){
				SerdesTrait<ElementType>()(vTempElement, sbufStream);
				vContainer.emplace_hint(vContainer.end(), std::move(vTempElement));
			}
		}
	};

	// 通用版本。需要使用下面定义的宏来建立成员序列化表。
	template<typename Object_t, typename>
	struct SerdesTrait {
		typedef Object_t ObjectType;
		typedef std::pair<
			std::function<void (StreamBuffer &, const Object_t &)>,
			std::function<void (Object_t &, StreamBuffer &)>
		> TableElementType;

		static const TableElementType s_vSerdesTable[];

		void operator()(StreamBuffer &sbufStream, const Object_t &vObject) const {
			auto pCur = s_vSerdesTable;
			while(pCur->first){
				pCur->first(sbufStream, vObject);
				++pCur;
			}
		}
		void operator()(Object_t &vObject, StreamBuffer &sbufStream) const {
			auto pCur = s_vSerdesTable;
			while(pCur->second){
				pCur->second(vObject, sbufStream);
				++pCur;
			}
		}
	};
}

}

#define SERDES_TABLE_BEGIN(cls)	\
	template<>	\
	const typename ::MCF::Impl::SerdesTrait<MACRO_TYPE(cls)>::TableElementType	\
		(::MCF::Impl::SerdesTrait<MACRO_TYPE(cls)>::s_vSerdesTable)[]	\
	= {

#define SERDES_BASE(cls)	\
		TableElementType(	\
			[](auto &stream, const auto &obj){	\
				::MCF::Impl::SerdesTrait<MACRO_TYPE(cls)>()(stream, static_cast<const MACRO_TYPE(cls) &>(obj));	\
			},	\
			[](auto &obj, auto &stream){	\
				::MCF::Impl::SerdesTrait<MACRO_TYPE(cls)>()(static_cast<MACRO_TYPE(cls) &>(obj), stream);	\
			}	\
		),

#define SERDES_MEMBER(member)	\
		TableElementType(	\
			[](auto &stream, const auto &obj){	\
				::MCF::Impl::SerdesTrait<decltype(ObjectType::member)>()(stream, obj.member);	\
			},	\
			[](auto &obj, auto &stream){	\
				::MCF::Impl::SerdesTrait<decltype(ObjectType::member)>()(obj.member, stream);	\
			}	\
		),

// 函数原型：
//   ser(StreamBuffer &, const object &);
//   des(object &, StreamBuffer &);
#define SERDES_CUSTOM(ser, des)	\
		TableElementType(ser, des),

#define SERDES_TABLE_END	\
		TableElementType()	\
	};

#endif
