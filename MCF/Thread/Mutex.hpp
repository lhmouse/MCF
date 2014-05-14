// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2014. LH_Mouse. All wrongs reserved.

#ifndef MCF_MUTEX_HPP_
#define MCF_MUTEX_HPP_

#include "_LockRaiiTemplate.hpp"
#include "../Core/Utilities.hpp"
#include "../Core/StringObserver.hpp"
#include <memory>

namespace MCF {

class Mutex : NO_COPY, ABSTRACT {
public:
	typedef Impl::LockRaiiTemplateTemplate<Mutex> Lock;

public:
	static std::unique_ptr<Mutex> Create(const WideStringObserver &wsoName = nullptr);

public:
	Lock GetLock() noexcept;
};

}

#endif
