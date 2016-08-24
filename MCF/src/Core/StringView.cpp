// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2016, LH_Mouse. All wrongs reserved.

#include "StringView.hpp"

namespace MCF {

template class StringView<StringType::kUtf8>;
template class StringView<StringType::kUtf16>;
template class StringView<StringType::kUtf32>;
template class StringView<StringType::kCesu8>;
template class StringView<StringType::kAnsi>;
template class StringView<StringType::kNarrow>;
template class StringView<StringType::kWide>;

}
