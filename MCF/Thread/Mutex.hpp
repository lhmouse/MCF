// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2014, LH_Mouse. All wrongs reserved.

#ifndef MCF_THREAD_MUTEX_HPP_
#define MCF_THREAD_MUTEX_HPP_

#include "_LockRaiiTemplate.hpp"
#include "../Utilities/NoCopy.hpp"
#include "../Utilities/Abstract.hpp"
#include "../Core/String.hpp"
#include <memory>

namespace MCF {

class Mutex : NO_COPY, ABSTRACT {
public:
	using Lock = Impl::LockRaiiTemplate<Mutex>;

public:
	static std::unique_ptr<Mutex> Create(const wchar_t *pwszName = nullptr);
	static std::unique_ptr<Mutex> Create(const WideString &wsName);

public:
	Lock TryLock() noexcept;
	Lock GetLock() noexcept;
};

}

#endif
