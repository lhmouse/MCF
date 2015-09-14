// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2015, LH_Mouse. All wrongs reserved.

#ifndef MCF_CONTAINERS_ENUMERATOR_TEMPLATE_HPP_
#define MCF_CONTAINERS_ENUMERATOR_TEMPLATE_HPP_

#include "../Utilities/Assert.hpp"
#include <iterator>

namespace MCF {

namespace Impl_EnumeratorTemplate {
	// ConstEnumerator 和 Enumerator 需求
	// ------------------------------------------------------------
	// ContainerT 必须具有：
	// 0) 成员类型：
	//   Element
	// 1) 成员函数：
	//   const Element * GetFirst (               ) const noexcept;
	//         Element * GetFirst (               )       noexcept;
	//   const Element * GetLast  (               ) const noexcept;
	//         Element * GetLast  (               )       noexcept;
	//   const Element * GetPrev  (const Element *) const noexcept;
	//         Element * GetNext  (      Element *)       noexcept;
	//   const Element * GetPrev  (const Element *) const noexcept;
	//         Element * GetNext  (      Element *)       noexcept;

	template<typename ContainerT>
	class ConstEnumerator;

	template<typename ContainerT>
	class Enumerator : public std::iterator<std::bidirectional_iterator_tag, typename ContainerT::Element> {
		friend ConstEnumerator<ContainerT>;

	public:
		using Element = typename ContainerT::Element;

	private:
		ContainerT *x_pContainer;
		Element *x_pElement;

	public:
		explicit constexpr Enumerator() noexcept
			: x_pContainer(nullptr), x_pElement(nullptr)
		{
		}
		explicit constexpr Enumerator(ContainerT &vContainer, Element *pElement) noexcept
			: x_pContainer(&vContainer), x_pElement(pElement)
		{
		}

	public:
		Element *GetPtr() const noexcept {
			return x_pElement;
		}

	public:
		bool operator==(const Enumerator &rhs) const noexcept {
			return x_pElement == rhs.x_pElement;
		}
		bool operator!=(const Enumerator &rhs) const noexcept {
			return !(*this == rhs);
		}

		Enumerator &operator++() noexcept {
			ASSERT(x_pContainer);

			if(x_pElement){
				x_pElement = x_pContainer->GetNext(x_pElement);
			} else {
				x_pElement = x_pContainer->GetFirst();
			}
			return *this;
		}
		Enumerator &operator--() noexcept {
			ASSERT(x_pContainer);

			if(x_pElement){
				x_pElement = x_pContainer->GetPrev(x_pElement);
			} else {
				x_pElement = x_pContainer->GetLast();
			}
			return *this;
		}

		Enumerator operator++(int) noexcept {
			auto enRet = *this;
			++(*this);
			return enRet;
		}
		Enumerator operator--(int) noexcept {
			auto enRet = *this;
			--(*this);
			return enRet;
		}

		Element &operator*() const noexcept {
			ASSERT(x_pElement);

			return *x_pElement;
		}
		Element *operator->() const noexcept {
			return x_pElement;
		}

		explicit operator bool() const noexcept {
			return !!x_pElement;
		}
	};

	template<typename ContainerT>
	class ConstEnumerator : public std::iterator<std::bidirectional_iterator_tag, const typename ContainerT::Element> {
	public:
		using Element = const typename ContainerT::Element;

	private:
		const ContainerT *x_pContainer;
		const Element *x_pElement;

	public:
		explicit constexpr ConstEnumerator() noexcept
			: x_pContainer(nullptr), x_pElement(nullptr)
		{
		}
		explicit constexpr ConstEnumerator(const ContainerT &vContainer, const Element *pElement) noexcept
			: x_pContainer(&vContainer), x_pElement(pElement)
		{
		}
		constexpr ConstEnumerator(const Enumerator<ContainerT> &rhs) noexcept
			: x_pContainer(rhs.x_pContainer), x_pElement(rhs.x_pElement)
		{
		}

	public:
		const Element *GetPtr() const noexcept {
			return x_pElement;
		}

	public:
		bool operator==(const ConstEnumerator &rhs) const noexcept {
			return x_pElement == rhs.x_pElement;
		}
		bool operator!=(const ConstEnumerator &rhs) const noexcept {
			return !(*this == rhs);
		}

		ConstEnumerator &operator++() noexcept {
			ASSERT(x_pContainer);

			if(x_pElement){
				x_pElement = x_pContainer->GetNext(x_pElement);
			} else {
				x_pElement = x_pContainer->GetFirst();
			}
			return *this;
		}
		ConstEnumerator &operator--() noexcept {
			ASSERT(x_pContainer);

			if(x_pElement){
				x_pElement = x_pContainer->GetPrev(x_pElement);
			} else {
				x_pElement = x_pContainer->GetLast();
			}
			return *this;
		}

		ConstEnumerator operator++(int) noexcept {
			auto enRet = *this;
			++(*this);
			return enRet;
		}
		ConstEnumerator operator--(int) noexcept {
			auto enRet = *this;
			--(*this);
			return enRet;
		}

		const Element &operator*() const noexcept {
			ASSERT(x_pElement);

			return *x_pElement;
		}
		const Element *operator->() const noexcept {
			return x_pElement;
		}

		explicit operator bool() const noexcept {
			return !!x_pElement;
		}
	};

}

}

#endif
