// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2016, LH_Mouse. All wrongs reserved.

#include "../StdMCF.hpp"
#include "Rcnts.hpp"

namespace MCF {

template class Rcnts<char>;
template class Rcnts<wchar_t>;
template class Rcnts<char16_t>;
template class Rcnts<char32_t>;

}
