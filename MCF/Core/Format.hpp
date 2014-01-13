// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2012 - 2013. LH_Mouse. All wrongs reserved.

#ifndef __MCF_FORMAT_HPP__
#define __MCF_FORMAT_HPP__

#include "../MCFCRT/MCFCRT.h"
#include "String.hpp"
#include <utility>

namespace MCF {

template<typename CHAR_T, StringEncoding CHAR_ENC>
struct StringInserterBase {
	typedef struct tagParams {
		std::size_t uIndex;
		CHAR_T chType;
		CHAR_T chPadding;
		int nWidth1;
		int nWidth2;
	} PARAMS;

	virtual ~StringInserterBase(){ }

	virtual std::size_t operator()(const PARAMS &Params) = 0;
	virtual bool operator()(CHAR_T *&pchDst, const PARAMS &Params) const = 0;
};

template<typename SOURCE_T, typename CHAR_T, StringEncoding CHAR_ENC>
struct StringInserter : public StringInserterBase<CHAR_T, CHAR_ENC> {
	using typename StringInserterBase<CHAR_T, CHAR_ENC>::PARAMS;

	StringInserter(const SOURCE_T &Source);

	virtual std::size_t operator()(const PARAMS &Params);
	virtual bool operator()(CHAR_T *&pchDst, const PARAMS &Params) const;
};

template<typename CHAR_T, StringEncoding CHAR_ENC, class INSERTER_ITER>
bool FormatVector(
	GenericString<CHAR_T, CHAR_ENC> &strRet,
	const CHAR_T *pszFormat,
	INSERTER_ITER *pInserterIterators,
	std::size_t uInserterCount
){
	typedef StringInserterBase<CHAR_T, CHAR_ENC> InserterBase;
	typedef typename InserterBase::PARAMS PARAMS;

	bool bNoError = true;
/*	VVector<PARAMS> vecParams;

	auto pchFormatRead = pszFormat;
	std::size_t uSizeNeeded = 0;
	auto iterInserter = pInserterIterators;
	for(std::size_t i = 0; i < uInserterCount; ++i){
	}
	// calculate size
	// do format
*/
	return bNoError;
}

namespace __MCF {
	template<typename CHAR_T, StringEncoding CHAR_ENC, class... INSERTER_T>
	static inline __attribute__((always_inline)) bool FormatHelper(
		GenericString<CHAR_T, CHAR_ENC> &strRet,
		const CHAR_T *pszFormat,
		INSERTER_T &&... Inserter
	){
		VVector<StringInserterBase<CHAR_T, CHAR_ENC> *> vecInserters;
		NOOP((vecInserters.Push(&static_cast<StringInserterBase<CHAR_T, CHAR_ENC> &>(Inserter)), 0)...);
		return FormatVector(strRet, pszFormat, vecInserters.GetData(), vecInserters.GetSize());
	}
}

template<typename CHAR_T, StringEncoding CHAR_ENC, class... PARAM_T>
bool Format(GenericString<CHAR_T, CHAR_ENC> &strRet, const CHAR_T *pszFormat, const PARAM_T &... Param){
	return __MCF::FormatHelper(strRet, pszFormat, StringInserter<PARAM_T, CHAR_T, CHAR_ENC>(Param)...);
}

}

#endif
