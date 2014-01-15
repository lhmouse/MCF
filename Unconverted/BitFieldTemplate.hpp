// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2012 - 2013. LH_Mouse. All wrongs reserved.

#ifndef __MCF_BIT_FIELD_TEMPLATE_HPP__
#define __MCF_BIT_FIELD_TEMPLATE_HPP__

#include "StdMCF.hpp"

namespace MCF {
	template<typename BASE_TYPE>
	class BitFieldTemplate {
		static_assert(std::is_arithmetic<BASE_TYPE>::value, "BASE_TYPE must be an aithmetic type.");
	private:
		static std::uintptr_t inline __attribute__((always_inline)) xMakeMask(std::size_t uIndexBegin, std::size_t uBitCount){
			return (std::uintptr_t)((((std::uintptr_t)1 << uBitCount) - 1) << uIndexBegin);
		}
	private:
		BASE_TYPE xm_Base;
	public:
		BitFieldTemplate(BASE_TYPE Base = 0){
			xm_Base = Base;
		}
		BitFieldTemplate &operator=(BASE_TYPE Base){
			xm_Base = Base;
			return *this;
		}
	public:
		template<typename VAL_TYPE>
		VAL_TYPE inline __attribute__((always_inline)) Test(std::size_t uIndexBegin, std::size_t uBitCount) const {
			static_assert(std::is_arithmetic<VAL_TYPE>::value, "VAL_TYPE must be an aithmetic type.");

			return (VAL_TYPE)((((std::uintptr_t)xm_Base) & xMakeMask(uIndexBegin, uBitCount)) >> uIndexBegin);
		}
		template<typename VAL_TYPE>
		VAL_TYPE inline __attribute__((always_inline)) Extract(std::size_t uIndexBegin, std::size_t uBitCount){
			static_assert(std::is_arithmetic<VAL_TYPE>::value, "VAL_TYPE must be an aithmetic type.");

			const std::uintptr_t uMask = xMakeMask(uIndexBegin, uBitCount);
			const VAL_TYPE Ret = (VAL_TYPE)((((std::uintptr_t)xm_Base) & uMask) >> uIndexBegin);
			xm_Base &= (BASE_TYPE)~uMask;
			return Ret;
		}
		template<typename VAL_TYPE>
		void inline __attribute__((always_inline)) Combine(std::size_t uIndexBegin, std::size_t uBitCount, VAL_TYPE Val){
			static_assert(std::is_arithmetic<VAL_TYPE>::value, "VAL_TYPE must be an aithmetic type.");

			const std::uintptr_t uMask = xMakeMask(uIndexBegin, uBitCount);
			// xm_Base = (xm_Base & ~uMask) | (((std::uintptr_t)Val << uIndexBegin) & uMask);
			xm_Base = (BASE_TYPE)(((((std::uintptr_t)Val << uIndexBegin) ^ xm_Base) & uMask) ^ xm_Base);
		}
	public:
		operator const BASE_TYPE &() const {
			return xm_Base;
		}
		operator BASE_TYPE &(){
			return xm_Base;
		}
	};
}

#endif
