// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2015, LH_Mouse. All wrongs reserved.

#ifndef MCF_CORE_PRINT_HPP_
#define MCF_CORE_PRINT_HPP_

#include "String.hpp"
#include <type_traits>
#include <memory>
#include <cstddef>
#include <climits>

namespace MCF {

namespace Printers {
	template<typename OutputIteratorT, typename EndPredT, unsigned kRadixT, bool bUpperCaseT>
	class IntegralPrinter {
		static_assert((kRadixT != 0) && (kRadixT <= 36), "kRadixT is invalid");

	private:
		OutputIteratorT &x_itOutput;
		EndPredT &x_fnEndPred;

	public:
		IntegralPrinter(OutputIteratorT &itOutput, EndPredT &fnEndPred)
			: x_itOutput(itOutput), x_fnEndPred(fnEndPred)
		{
		}

	private:
		template<typename ParamT>
		void xDoPrintUnsigned(ParamT vParam){
			static_assert(std::is_unsigned<ParamT>::value, "ParamT must be an unsigned type");

			static constexpr char kAsciiDigits[] = "00112233445566778899aAbBcCdDeEfFgGhHiIjJkKlLmMnNoOpPqQrRsStTuUvVwWxXyYzZ";

			char achTemp[sizeof(ParamT) * CHAR_BIT];
			auto pchWrite = achTemp;
			do {
				const unsigned uDigit = vParam % kRadixT;
				vParam /= kRadixT;
				*pchWrite = kAsciiDigits[uDigit * 2 + static_cast<unsigned>(bUpperCaseT)];
				++pchWrite;
			} while(vParam != 0);

			do {
				if(x_fnEndPred()){
					return;
				}
				--pchWrite;
				*x_itOutput = *pchWrite;
				++x_itOutput;
			} while(pchWrite != achTemp);
		}
		template<typename ParamT>
		void xDoPrintSigned(ParamT vParam){
			static_assert(std::is_signed<ParamT>::value, "ParamT must be a signed type");

			if(vParam < 0){
				if(x_fnEndPred()){
					return;
				}
				*x_itOutput = '-';
				++x_itOutput;
				vParam = -vParam;
			}
			xDoPrintUnsigned(static_cast<std::make_unsigned_t<ParamT>>(vParam));
		}

	public:
		void operator()(bool v){
			xDoPrintUnsigned(static_cast<unsigned>(v));
		}
		void operator()(signed char v){
			xDoPrintSigned(static_cast<int>(v));
		}
		void operator()(unsigned char v){
			xDoPrintUnsigned(static_cast<unsigned>(v));
		}
		void operator()(short v){
			xDoPrintSigned(static_cast<int>(v));
		}
		void operator()(unsigned short v){
			xDoPrintUnsigned(static_cast<unsigned>(v));
		}
		void operator()(int v){
			xDoPrintSigned(v);
		}
		void operator()(unsigned v){
			xDoPrintUnsigned(v);
		}
		void operator()(long v){
			xDoPrintSigned(v);
		}
		void operator()(unsigned long v){
			xDoPrintUnsigned(v);
		}
		void operator()(long long v){
			xDoPrintSigned(v);
		}
		void operator()(unsigned long long v){
			xDoPrintUnsigned(v);
		}
	};

	template<typename Tx, typename Ty>
	using b = IntegralPrinter<Tx, Ty,  2, false>;
	template<typename Tx, typename Ty>
	using o = IntegralPrinter<Tx, Ty,  8, false>;
	template<typename Tx, typename Ty>
	using d = IntegralPrinter<Tx, Ty, 10, false>;
	template<typename Tx, typename Ty>
	using x = IntegralPrinter<Tx, Ty, 16, false>;

	template<typename Tx, typename Ty>
	using B = IntegralPrinter<Tx, Ty,  2, true>;
	template<typename Tx, typename Ty>
	using O = IntegralPrinter<Tx, Ty,  8, true>;
	template<typename Tx, typename Ty>
	using D = IntegralPrinter<Tx, Ty, 10, true>;
	template<typename Tx, typename Ty>
	using X = IntegralPrinter<Tx, Ty, 16, true>;
}

namespace Impl_Print {
	template<
		template<typename, typename>
			class ...PrintersT,
		typename OutputIteratorT, typename EndPredT, typename ...ParamsT>
	OutputIteratorT PrintPred(OutputIteratorT itOutput, EndPredT fnEndPred, const ParamsT &... vParams){
		static_assert(sizeof...(PrintersT) == sizeof...(ParamsT), "Count of formatters and count of parameters mismatch");

		const int anDummy[] = { (PrintersT<OutputIteratorT, EndPredT>(itOutput, fnEndPred)(vParams), 1)... };
		static_cast<void>(anDummy);
		return itOutput;
	}
}

template<
	template<typename, typename>
		class ...PrintersT,
	typename OutputIteratorT, typename ...ParamsT>
OutputIteratorT Print(OutputIteratorT itOutput, std::common_type_t<OutputIteratorT> itEnd, const ParamsT &... vParams){
	return Impl_Print::PrintPred<PrintersT...>(itOutput, [&](const auto &it){ return it == itEnd; }, vParams...);
}
template<
	template<typename, typename>
		class ...PrintersT,
	typename OutputIteratorT, typename ...ParamsT>
OutputIteratorT Print(OutputIteratorT itOutput, const ParamsT &... vParams){
	return Impl_Print::PrintPred<PrintersT...>(itOutput, []{ return false; }, vParams...);
}

template<
	template<typename, typename>
		class ...PrintersT,
	StringType kStringTypeT, typename ...ParamsT>
String<kStringTypeT> &PrintStr(String<kStringTypeT> &strDst, const ParamsT &... vParams){
	Impl_Print::PrintPred<PrintersT...>(std::back_inserter(strDst), []{ return false; }, vParams...);
	return strDst;
}
template<
	template<typename, typename>
		class ...PrintersT,
	StringType kStringTypeT, typename ...ParamsT>
String<kStringTypeT> &&PrintStr(String<kStringTypeT> &&strDst, const ParamsT &... vParams){
	Impl_Print::PrintPred<PrintersT...>(std::back_inserter(strDst), []{ return false; }, vParams...);
	return std::move(strDst);
}

}

#endif
