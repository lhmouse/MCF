// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2014, LH_Mouse. All wrongs reserved.

#include "../StdMCF.hpp"
#include "StringObserver.hpp"
using namespace MCF;

namespace MCF {

template class StringObserver<StringTypes::NARROW>;
template class StringObserver<StringTypes::WIDE>;
template class StringObserver<StringTypes::UTF8>;
template class StringObserver<StringTypes::UTF16>;
template class StringObserver<StringTypes::UTF32>;
template class StringObserver<StringTypes::CESU8>;
template class StringObserver<StringTypes::ANSI>;

}
