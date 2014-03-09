// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2014. LH_Mouse. All wrongs reserved.

#ifndef __MCF_EVENT_HPP__
#define __MCF_EVENT_HPP__

#include "NoCopy.hpp"
#include <memory>

namespace MCF {

class Event : NO_COPY {
private:
	class xDelegate;

private:
	std::unique_ptr<xDelegate> xm_pDelegate;

public:
	explicit Event(bool bInitSet, const wchar_t *pwszName = nullptr);
	Event(Event &&rhs) noexcept;
	Event &operator=(Event &&rhs) noexcept;
	~Event();

public:
	bool IsSet() const noexcept;
	void Set() noexcept;
	void Clear() noexcept;

	bool WaitTimeout(unsigned long ulMilliSeconds) const noexcept;
	void Wait() const noexcept;
};

}

#endif
