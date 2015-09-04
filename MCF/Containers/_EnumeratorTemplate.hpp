// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2015, LH_Mouse. All wrongs reserved.

#ifndef MCF_CONTAINERS_ENUMERATOR_TEMPLATE_HPP_
#define MCF_CONTAINERS_ENUMERATOR_TEMPLATE_HPP_

#include "../Utilities/Assert.hpp"
#include <iterator>

namespace MCF {

namespace Impl_EnumeratorTemplate {
	template<typename ContainerT, typename AdvanceOnceT>
	class ConstEnumerator;

	template<typename ContainerT, typename AdvanceOnceT>
	class Enumerator : public std::iterator<std::forward_iterator_tag, typename ContainerT::ElementType> {
		template<typename, typename>
		friend class ConstEnumerator;

	public:
		using ElementType = typename ContainerT::ElementType;

	private:
		ContainerT *x_pContainer;
		ElementType *x_pElement;

	public:
		explicit constexpr Enumerator() noexcept
			: x_pContainer(nullptr), x_pElement(nullptr)
		{
		}
		explicit constexpr Enumerator(ContainerT &vContainer, ElementType *pElement) noexcept
			: x_pContainer(&vContainer), x_pElement(pElement)
		{
		}

	public:
		bool operator==(const Enumerator &rhs) const noexcept {
			return x_pElement == rhs.x_pElement;
		}
		bool operator!=(const Enumerator &rhs) const noexcept {
			return !(*this == rhs);
		}

		Enumerator &operator++(){
			ASSERT(x_pContainer);
			ASSERT(x_pElement);

			AdvanceOnceT()(*x_pContainer, x_pElement);
			return *this;
		}
		Enumerator operator++(int){
			auto enRet = *this;
			++(*this);
			return enRet;
		}

		ElementType &operator*() const noexcept {
			ASSERT(x_pElement);

			return *x_pElement;
		}

		explicit operator bool() const noexcept {
			__asm__(""); // 这里可以优化掉一个分支。
			return !!x_pElement;
		}
	};

	template<typename ContainerT, typename AdvanceOnceT>
	class ConstEnumerator : public std::iterator<std::forward_iterator_tag, const typename ContainerT::ElementType> {
	public:
		using ElementType = const typename ContainerT::ElementType;

	private:
		const ContainerT *x_pContainer;
		const ElementType *x_pElement;

	public:
		explicit constexpr ConstEnumerator() noexcept
			: x_pContainer(nullptr), x_pElement(nullptr)
		{
		}
		explicit constexpr ConstEnumerator(const ContainerT &vContainer, const ElementType *pElement) noexcept
			: x_pContainer(&vContainer), x_pElement(pElement)
		{
		}
		constexpr ConstEnumerator(const Enumerator<ContainerT, AdvanceOnceT> &rhs) noexcept
			: x_pContainer(rhs.x_pContainer), x_pElement(rhs.x_pElement)
		{
		}

	public:
		bool operator==(const ConstEnumerator &rhs) const noexcept {
			return x_pElement == rhs.x_pElement;
		}
		bool operator!=(const ConstEnumerator &rhs) const noexcept {
			return !(*this == rhs);
		}

		ConstEnumerator &operator++(){
			ASSERT(x_pContainer);
			ASSERT(x_pElement);

			AdvanceOnceT()(*x_pContainer, x_pElement);
			return *this;
		}
		ConstEnumerator operator++(int){
			auto enRet = *this;
			++(*this);
			return enRet;
		}

		const ElementType &operator*() const noexcept {
			ASSERT(x_pElement);

			return *x_pElement;
		}

		explicit operator bool() const noexcept {
			__asm__(""); // 这里可以优化掉一个分支。
			return !!x_pElement;
		}
	};

}

}

#endif
