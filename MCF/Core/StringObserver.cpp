// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2015, LH_Mouse. All wrongs reserved.

#include "../StdMCF.hpp"
#include "StringObserver.hpp"

namespace MCF {

template class StringObserver<StringType::NARROW>;
template class StringObserver<StringType::WIDE>;
template class StringObserver<StringType::UTF8>;
template class StringObserver<StringType::UTF16>;
template class StringObserver<StringType::UTF32>;
template class StringObserver<StringType::CESU8>;
template class StringObserver<StringType::ANSI>;

}
