// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2014, LH_Mouse. All wrongs reserved.

#ifndef MCF_EVENT_HPP_
#define MCF_EVENT_HPP_

#include "../Utilities/NoCopy.hpp"
#include "../Utilities/Abstract.hpp"
#include "../Core/String.hpp"
#include <memory>

namespace MCF {

class Event : NO_COPY, ABSTRACT {
public:
	static std::unique_ptr<Event> Create(bool bInitSet, const WideStringObserver &wsoName = nullptr);
	static std::unique_ptr<Event> Create(bool bInitSet, const WideString &wcsName);

public:
	bool IsSet() const noexcept;
	void Set() noexcept;
	void Clear() noexcept;

	bool WaitTimeout(unsigned long long ullMilliSeconds) const noexcept;
	void Wait() const noexcept;
};

}

#endif
