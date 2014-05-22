// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2014. LH_Mouse. All wrongs reserved.

// 这个文件包含了一大堆模板，所以很拖编译速度。

#ifndef MCF_SERDES_TRAITS_HPP_
#define MCF_SERDES_TRAITS_HPP_

#include "DataBuffer.hpp"
#include "VarIntEx.hpp"
#include <iterator>
#include <type_traits>
#include <array>
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
	template<typename Object_t, typename = void>
	struct SerdesTrait;

	// 针对 bool 的特化。使用最小的存储。注意 bool 本身的大小是实现定义的，有可能当作 int 了。
	template<>
	struct SerdesTrait<
		bool,
		void
	> {
		void operator()(DataBuffer &dbufStream, const bool &vObject) const {
			unsigned char by = vObject;
			dbufStream.Insert(&by, sizeof(by));
		}
		void operator()(bool &vObject, DataBuffer &dbufStream) const {
			unsigned char by;
			dbufStream.Extract(&by, sizeof(by));
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
		void operator()(DataBuffer &dbufStream, const Integral &vObject) const {
			VarIntEx<Integral> viTemp(vObject);

			unsigned char abyTemp[viTemp.MAX_SERIALIZED_SIZE];
			auto pbyWrite = abyTemp;
			viTemp.Serialize(pbyWrite);

			dbufStream.Insert(abyTemp, (std::size_t)(pbyWrite - abyTemp));
		}
		void operator()(Integral &vObject, DataBuffer &dbufStream) const {
			VarIntEx<Integral> viTemp;

			auto itRead = dbufStream.GetReadIterator();
			viTemp.Unserialize(itRead, dbufStream.GetReadForeverEnd());

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
		void operator()(DataBuffer &, const std::nullptr_t &) const {
		}
		void operator()(std::nullptr_t &, DataBuffer &) const {
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

		void operator()(DataBuffer &dbufStream, const Enum_t &vElement) const {
			SerdesTrait<UnderlyingType>()(dbufStream, (const UnderlyingType &)vElement);
		}
		void operator()(Enum_t &vElement, DataBuffer &dbufStream) const {
			SerdesTrait<UnderlyingType>()((UnderlyingType &)vElement, dbufStream);
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
		void operator()(DataBuffer &dbufStream, const Scalar_t &vElement) const {
			dbufStream.Insert(&vElement, sizeof(vElement));
		}
		void operator()(Scalar_t &vElement, DataBuffer &dbufStream) const {
			dbufStream.Extract(&vElement, sizeof(vElement));
		}
	};

	// 针对内建数组的特化。从第一个开始迭代处理每一项即可。
	template<typename Element_t, std::size_t COUNT>
	struct SerdesTrait<
		Element_t [COUNT],
		void
	> {
		void operator()(DataBuffer &dbufStream, const Element_t (&arrElements)[COUNT]) const {
			for(const auto &vElement : arrElements){
				SerdesTrait<Element_t>()(dbufStream, vElement);
			}
		}
		void operator()(Element_t (&arrElements)[COUNT], DataBuffer &dbufStream) const {
			for(auto &vElement : arrElements){
				SerdesTrait<Element_t>()(vElement, dbufStream);
			}
		}
	};

	// 针对 pair 的特化。
	template<typename First_t, typename Second_t>
	struct SerdesTrait<
		std::pair<First_t, Second_t>,
		void
	> {
		void operator()(DataBuffer &dbufStream, const std::pair<First_t, Second_t> &vPair) const {
			SerdesTrait<First_t>()(dbufStream, vPair.first);
			SerdesTrait<Second_t>()(dbufStream, vPair.second);
		}
		void operator()(std::pair<First_t, Second_t> &vPair, DataBuffer &dbufStream) const {
			SerdesTrait<First_t>()(vPair.first, dbufStream);
			SerdesTrait<Second_t>()(vPair.second, dbufStream);
		}
	};

	// 针对 tuple 的特化。
	template<typename... Elements_t>
	struct SerdesTrait<
		std::tuple<Elements_t...>,
		void
	> {
		template<std::size_t INDEX, typename std::enable_if<(INDEX < sizeof...(Elements_t)), int>::type = 0>
		static void SerializeAll(DataBuffer &dbufStream, const std::tuple<Elements_t...> &vTuple){
			typedef typename std::tuple_element<INDEX, std::tuple<Elements_t...>>::type ElementType;

			SerdesTrait<ElementType>()(dbufStream, std::get<INDEX>(vTuple));
			SerializeAll<INDEX + 1>(dbufStream, vTuple);
		}
		template<std::size_t INDEX, typename std::enable_if<(INDEX == sizeof...(Elements_t)), int>::type = 0>
		static void SerializeAll(DataBuffer &, const std::tuple<Elements_t...> &){
		}

		template<std::size_t INDEX, typename std::enable_if<(INDEX < sizeof...(Elements_t)), int>::type = 0>
		static void DeserializeAll(std::tuple<Elements_t...> &vTuple, DataBuffer &dbufStream){
			typedef typename std::tuple_element<INDEX, std::tuple<Elements_t...>>::type ElementType;

			SerdesTrait<ElementType>()(std::get<INDEX>(vTuple), dbufStream);
			DeserializeAll<INDEX + 1>(vTuple, dbufStream);
		}
		template<std::size_t INDEX, typename std::enable_if<(INDEX == sizeof...(Elements_t)), int>::type = 0>
		static void DeserializeAll(std::tuple<Elements_t...> &, DataBuffer &){
		}

		void operator()(DataBuffer &dbufStream, const std::tuple<Elements_t...> &vTuple) const {
			SerializeAll<0>(dbufStream, vTuple);
		}
		void operator()(std::tuple<Elements_t...> &vTuple, DataBuffer &dbufStream) const {
			DeserializeAll<0>(vTuple, dbufStream);
		}
	};

	// 下面是针对标准库中的容器的特化。
	// 容器分为以下几类：
	// 1. 固定大小的（1 个）：
	//    array
	// 2. 支持 push_back() 的（5 个）：
	//    basic_string, list, deque, vector, vector<bool>
	// 3. 支持 emplace_front() 的（1 个）：
	//    forward_list
	// 4. 支持 emplace_hint() 的（8 个）：
	//    set, multiset, unordered_set, unordered_multiset,
	//    map, multimap, unordered_map, unordered_multimap

	// 针对 array 的特化。
	// 同内建数组。
	template<typename Element_t, std::size_t COUNT>
	struct SerdesTrait<
		std::array<Element_t, COUNT>,
		void
	> {
		void operator()(DataBuffer &dbufStream, const std::array<Element_t, COUNT> &vContainer) const {
			for(const auto &vElement : vContainer){
				SerdesTrait<Element_t>()(dbufStream, vElement);
			}
		}
		void operator()(std::array<Element_t, COUNT> &vContainer, DataBuffer &dbufStream) const {
			for(auto &vElement : vContainer){
				SerdesTrait<Element_t>()(vElement, dbufStream);
			}
		}
	};

	// 针对 basic_string, list, deque, vector 的特化。
	// 检测 push_back()。注意 basic_string 没有 emplace_back()。
	template<typename Container_t>
	struct SerdesTrait<
		Container_t,
		decltype(std::declval<Container_t>().push_back(), (void)0)
	> {
		typedef typename Container_t::value_type ElementType;

		void operator()(DataBuffer &dbufStream, const Container_t &vContainer) const {
			const std::size_t uSize = vContainer.size();
			SerdesTrait<std::uint64_t>()(dbufStream, uSize);

			for(const auto &vElement : vContainer){
				SerdesTrait<ElementType>()(dbufStream, vElement);
			}
		}
		void operator()(Container_t &vContainer, DataBuffer &dbufStream) const {
			std::uint64_t u64Temp;
			SerdesTrait<std::uint64_t>()(u64Temp, dbufStream);
			const std::size_t uSize = u64Temp;

			vContainer.resize(uSize);
			for(auto &vElement : vContainer){
				SerdesTrait<ElementType>()(vElement, dbufStream);
			}
		}
	};

	// 针对 vector<bool> 的特化。
	template<typename Allocator_t>
	struct SerdesTrait<
		std::vector<bool, Allocator_t>,
		void
	> {
		void operator()(DataBuffer &dbufStream, const std::vector<bool, Allocator_t> &vContainer) const {
			const std::size_t uSize = vContainer.size();
			SerdesTrait<std::uint64_t>()(dbufStream, uSize);

			auto itRead = vContainer.cbegin();
			const std::size_t uByteCount = uSize / 8;
			for(std::size_t i = 0; i < uByteCount; ++i){
				unsigned char by = 0;
				for(std::size_t j = 0; j < 8; ++j){
					by <<= 1;
					by |= *itRead;
					++itRead;
				}
				SerdesTrait<std::uint8_t>()(dbufStream, by);
			}
			const std::size_t uRemaining = uSize % 8;
			if(uRemaining != 0){
				std::uint8_t by = 0;
				for(std::size_t i = 0; i < uRemaining; ++i){
					by <<= 1;
					by |= *itRead;
					++itRead;
				}
				SerdesTrait<std::uint8_t>()(dbufStream, by);
			}
		}
		void operator()(std::vector<bool, Allocator_t> &vContainer, DataBuffer &dbufStream) const {
			std::uint64_t u64Temp;
			SerdesTrait<std::uint64_t>()(u64Temp, dbufStream);
			const std::size_t uSize = u64Temp;

			vContainer.resize(uSize);
			auto itWrite = vContainer.begin();
			const std::size_t uByteCount = uSize / 8;
			for(std::size_t i = 0; i < uByteCount; ++i){
				std::uint8_t by;
				SerdesTrait<std::uint8_t>()(by, dbufStream);
				for(std::size_t j = 0; j < 8; ++j){
					*itWrite = ((by & 0x80) != 0);
					++itWrite;
					by <<= 1;
				}
			}
			const std::size_t uRemaining = uSize % 8;
			if(uRemaining != 0){
				std::uint8_t by;
				SerdesTrait<std::uint8_t>()(by, dbufStream);
				by <<= 8 - uRemaining;
				for(std::size_t j = 0; j < uRemaining; ++j){
					*itWrite = ((by & 0x80) != 0);
					++itWrite;
					by <<= 1;
				}
			}
		}
	};

	// 针对 forward_list 的特化。
	template<typename Element_t, typename Allocator_t>
	struct SerdesTrait<
		std::forward_list<Element_t, Allocator_t>,
		void
	> {
		void operator()(DataBuffer &dbufStream, const std::forward_list<Element_t, Allocator_t> &vContainer) const {
			std::size_t uSize = 0;
			DataBuffer dbufTemp;
			for(const auto vElement : vContainer){
				SerdesTrait<Element_t>()(dbufTemp, vElement);
				++uSize;
			}
			SerdesTrait<std::uint64_t>()(dbufStream, uSize);
			dbufStream.Append(std::move(dbufTemp));
		}
		void operator()(std::forward_list<Element_t, Allocator_t> &vContainer, DataBuffer &dbufStream) const {
			std::uint64_t u64Temp;
			SerdesTrait<std::uint64_t>()(u64Temp, dbufStream);
			const std::size_t uSize = u64Temp;

			for(std::size_t i = 0; i < uSize; ++i){
				vContainer.emplace_front();
				SerdesTrait<Element_t>()(vContainer.front(), dbufStream);
			}
			vContainer.reverse();
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

		void operator()(DataBuffer &dbufStream, const Container_t &vContainer) const {
			const std::size_t uSize = vContainer.size();
			SerdesTrait<std::uint64_t>()(dbufStream, uSize);

			for(const auto &vElement : vContainer){
				SerdesTrait<ElementType>()(dbufStream, vElement);
			}
		}
		void operator()(Container_t &vContainer, DataBuffer &dbufStream) const {
			std::uint64_t u64Temp;
			SerdesTrait<std::uint64_t>()(u64Temp, dbufStream);
			const std::size_t uSize = u64Temp;

			vContainer.clear();
			ElementType vTempElement;
			for(std::size_t i = 0; i < uSize; ++i){
				SerdesTrait<ElementType>()(vTempElement, dbufStream);
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

		void operator()(DataBuffer &dbufStream, const Container_t &vContainer) const {
			const std::size_t uSize = vContainer.size();
			SerdesTrait<std::uint64_t>()(dbufStream, uSize);

			for(const auto &vElement : vContainer){
				SerdesTrait<ElementType>()(dbufStream, vElement);
			}
		}
		void operator()(Container_t &vContainer, DataBuffer &dbufStream) const {
			std::uint64_t u64Temp;
			SerdesTrait<std::uint64_t>()(u64Temp, dbufStream);
			const std::size_t uSize = u64Temp;

			vContainer.clear();
			ElementType vTempElement;
			for(std::size_t i = 0; i < uSize; ++i){
				SerdesTrait<ElementType>()(vTempElement, dbufStream);
				vContainer.emplace_hint(vContainer.end(), std::move(vTempElement));
			}
		}
	};

	// 通用版本。需要使用下面定义的宏来建立成员序列化表。
	template<typename Object_t, typename>
	struct SerdesTrait {
		typedef Object_t ObjectType;
		typedef std::pair<
			std::function<void (DataBuffer &, const Object_t &)>,
			std::function<void (Object_t &, DataBuffer &)>
		> TableElementType;

		static const TableElementType s_SerdesTable[];

		void operator()(DataBuffer &dbufStream, const Object_t &vObject) const {
			auto pCur = s_SerdesTable;
			while(pCur->first){
				pCur->first(dbufStream, vObject);
				++pCur;
			}
		}
		void operator()(Object_t &vObject, DataBuffer &dbufStream) const {
			auto pCur = s_SerdesTable;
			while(pCur->second){
				pCur->second(vObject, dbufStream);
				++pCur;
			}
		}
	};
}

}

#define SERDES_FRIEND_DECL(...)	\
	friend ::MCF::Impl::SerdesTrait<__VA_ARGS__>;

#define SERDES_TABLE_BEGIN(...)	\
	template<>	\
	const typename ::MCF::Impl::SerdesTrait<__VA_ARGS__>::TableElementType	\
		(::MCF::Impl::SerdesTrait<__VA_ARGS__>::s_SerdesTable)[]	\
	= {

#define SERDES_BASE(...)	\
		TableElementType(	\
			[](auto &stream, const auto &obj){	\
				::MCF::Impl::SerdesTrait<__VA_ARGS__>()(stream, static_cast<const __VA_ARGS__ &>(obj));	\
			},	\
			[](auto &obj, auto &stream){	\
				::MCF::Impl::SerdesTrait<__VA_ARGS__>()(static_cast<__VA_ARGS__ &>(obj), stream);	\
			}	\
		),

#define SERDES_MEMBER(...)	\
		TableElementType(	\
			[](auto &stream, const auto &obj){	\
				::MCF::Impl::SerdesTrait<decltype(ObjectType::__VA_ARGS__)>()(stream, obj.__VA_ARGS__);	\
			},	\
			[](auto &obj, auto &stream){	\
				::MCF::Impl::SerdesTrait<decltype(ObjectType::__VA_ARGS__)>()(obj.__VA_ARGS__, stream);	\
			}	\
		),

#define SERDES_TABLE_END	\
		TableElementType()	\
	};

#endif
