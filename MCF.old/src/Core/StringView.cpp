// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2018, LH_Mouse. All wrongs reserved.

#include "StringView.hpp"

namespace MCF {

template class StringView<Impl_StringTraits::Type::kUtf8>;
template class StringView<Impl_StringTraits::Type::kUtf16>;
template class StringView<Impl_StringTraits::Type::kUtf32>;
template class StringView<Impl_StringTraits::Type::kCesu8>;
template class StringView<Impl_StringTraits::Type::kAnsi>;
template class StringView<Impl_StringTraits::Type::kModifiedUtf8>;
template class StringView<Impl_StringTraits::Type::kNarrow>;
template class StringView<Impl_StringTraits::Type::kWide>;

}
