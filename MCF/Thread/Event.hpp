// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2014, LH_Mouse. All wrongs reserved.

#ifndef MCF_THREAD_EVENT_HPP_
#define MCF_THREAD_EVENT_HPP_

#include "../Utilities/Noncopyable.hpp"
#include "../Core/String.hpp"
#include "Win32Handle.hpp"

namespace MCF {

class Event : Noncopyable {
private:
	const UniqueWin32Handle xm_hEvent;

public:
	explicit Event(bool bInitSet, const wchar_t *pwszName = nullptr);
	Event(bool bInitSet, const WideString &wsName);

public:
	bool Wait(unsigned long long ullMilliSeconds) const noexcept;
	void Wait() const noexcept;
	bool IsSet() const noexcept;
	void Set() noexcept;
	void Clear() noexcept;
};

}

#endif
