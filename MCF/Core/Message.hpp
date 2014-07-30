// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2014, LH_Mouse. All wrongs reserved.

#ifndef MCF_MESSAGE_HPP_
#define MCF_MESSAGE_HPP_

#include "String.hpp"
#include <functional>
#include <memory>
#include <cstdint>

namespace MCF {

class Message {
public:
	// 返回 true 则表示消息已被处理，不会继续传递。
	// 否则消息传递给前一个注册的响应器。
	typedef std::function<bool (Message &)> HandlerProc;

public:
	static std::shared_ptr<void> RegisterHandler(const Utf8StringObserver &u8soName, HandlerProc fnProc);

public:
	const Utf8String m_u8sName;
	const std::size_t m_uType;

public:
	Message(Utf8String u8sName, std::size_t uType) noexcept
		: m_u8sName	(std::move(u8sName))
		, m_uType	(uType)
	{
	}
	virtual ~Message() noexcept = default;

public:
	void Dispatch();
};

}

#endif
