// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2014, LH_Mouse. All wrongs reserved.

#ifndef MCF_SERDES_BITSET_HPP_
#define MCF_SERDES_BITSET_HPP_

#include "Serdes.hpp"
#include <bitset>
#include <iterator>

namespace MCF {

namespace Impl {
	template<std::size_t BIT_COUNT>
	class BitSetReadIterator :
		public std::iterator<std::input_iterator_tag, bool>
	{
	private:
		const std::bitset<BIT_COUNT> &xm_bsetOwner;
		std::size_t xm_uIndex;

	public:
		explicit BitSetReadIterator(const std::bitset<BIT_COUNT> &bsetOwner, std::size_t uIndex = 0)
			: xm_bsetOwner	(bsetOwner)
			, xm_uIndex		(uIndex)
		{
		}

	public:
		bool operator==(const BitSetReadIterator &rhs) const noexcept {
			ASSERT(&xm_bsetOwner == &rhs.xm_bsetOwner);
			return xm_uIndex == rhs.xm_uIndex;
		}
		bool operator!=(const BitSetReadIterator &rhs) const noexcept {
			return !(*this == rhs);
		}
		BitSetReadIterator &operator++() noexcept {
			++xm_uIndex;
			return *this;
		}
		BitSetReadIterator operator++(int) noexcept {
			auto itRet = *this;
			++*this;
			return std::move(itRet);
		}

		bool operator*() const noexcept {
			ASSERT(xm_uIndex < BIT_COUNT);
			return xm_bsetOwner[xm_uIndex];
		}
	};
	template<std::size_t BIT_COUNT>
	class BitSetWriteIterator :
		public std::iterator<std::output_iterator_tag, bool>
	{
	private:
		std::bitset<BIT_COUNT> &xm_bsetOwner;
		std::size_t xm_uIndex;

	public:
		explicit BitSetWriteIterator(std::bitset<BIT_COUNT> &bsetOwner)
			: xm_bsetOwner	(bsetOwner)
			, xm_uIndex		(0)
		{
		}

	public:
		bool operator==(const BitSetWriteIterator &rhs) const noexcept {
			ASSERT(&xm_bsetOwner == &rhs.xm_bsetOwner);
			return true;
		}
		bool operator!=(const BitSetWriteIterator &rhs) const noexcept {
			return !(*this == rhs);
		}
		BitSetWriteIterator &operator++() noexcept {
			++xm_uIndex;
			return *this;
		}
		BitSetWriteIterator operator++(int) noexcept {
			auto itRet = *this;
			++*this;
			return std::move(itRet);
		}

		BitSetWriteIterator &operator*() noexcept {
			return *this;
		}
		BitSetWriteIterator &operator=(bool b) noexcept {
			ASSERT(xm_uIndex < BIT_COUNT);
			xm_bsetOwner.set(xm_uIndex, b);
			return *this;
		}
	};
}

template<std::size_t BIT_COUNT>
void Serialize(StreamBuffer &sbufSink, const std::bitset<BIT_COUNT> &vSource){
	Serialize<bool>(
		sbufSink,
		Impl::BitSetReadIterator<BIT_COUNT>(vSource, 0),
		BIT_COUNT
	);
}
template<std::size_t BIT_COUNT>
void Deserialize(std::bitset<BIT_COUNT> &vSink, StreamBuffer &sbufSource){
	Deserialize<bool>(
		Impl::BitSetWriteIterator<BIT_COUNT>(vSink),
		BIT_COUNT,
		sbufSource
	);
}

}

#endif
