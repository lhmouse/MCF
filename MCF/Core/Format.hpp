// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2012 - 2013. LH_Mouse. All wrongs reserved.

#ifndef __MCF_FORMAT_HPP__
#define __MCF_FORMAT_HPP__

#include "../StdMCF.hpp"

#include "String.hpp"

namespace MCF {
/*	namespace String {
		template<typename CHAR_T, String::ENCODING CHAR_ENC>
		class InserterBase {
		protected:
			virtual std::size_t xQuerySize() const = 0;
			virtual void Format(CHAR_T *pchBuffer) = 0;
		};

		template<typename SOURCE_T, typename CHAR_T, String::ENCODING CHAR_ENC>
		class Inserter final : public InserterBase<CHAR_T, CHAR_ENC> {
		protected:
			const SOURCE_T &xm_Src;
		public:
			explicit Inserter(const SOURCE_T &Src) : xm_Src(Src) { }
		protected:
			virtual std::size_t xQuerySize() const;
			virtual void Format(CHAR_T *pchBuffer);
		};
	}

	namespace __MCF {
		template<typename... P>
		void Noop(const P &...){
		}
	}

	template<typename CHAR_T, String::ENCODING CHAR_ENC, typename... PARAM_T>
	void Format(GenericString<CHAR_T, CHAR_ENC> &strResult, const GenericString<CHAR_T, CHAR_ENC> &strFormat, const PARAM_T &... Params){
		Format(strResult, strFormat.GetCStr(), Params...);
	}
	template<typename CHAR_T, String::ENCODING CHAR_ENC, typename... PARAM_T>
	void Format(GenericString<CHAR_T, CHAR_ENC> &strResult, const CHAR_T *pszFormat, const PARAM_T &... Params){
	}*/
}

#endif
