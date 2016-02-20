// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2016, LH_Mouse. All wrongs reserved.

#ifndef MCF_STREAMS_ABSTRACT_STREAM_FILTERS_HPP_
#define MCF_STREAMS_ABSTRACT_STREAM_FILTERS_HPP_

#include "AbstractStreams.hpp"
#include "../SmartPointers/UniquePtr.hpp"

namespace MCF {

class AbstractInputStreamFilter : public AbstractInputStream {
protected:
	UniquePtr<AbstractInputStream> x_pInputStream;

public:
	constexpr AbstractInputStreamFilter() noexcept
		: x_pInputStream()
	{
	}
	explicit AbstractInputStreamFilter(UniquePtr<AbstractInputStream> pInputStream) noexcept
		: AbstractInputStreamFilter()
	{
		SetInputStream(std::move(pInputStream));
	}
	~AbstractInputStreamFilter() override;

public:
	const UniquePtr<AbstractInputStream> &GetInputStream() const noexcept {
		return x_pInputStream;
	}
	UniquePtr<AbstractInputStream> &GetInputStream() noexcept {
		return x_pInputStream;
	}
	void SetInputStream(UniquePtr<AbstractInputStream> pInputStream) noexcept {
		x_pInputStream = std::move(pInputStream);
	}
};

class AbstractOutputStreamFilter : public AbstractOutputStream {
protected:
	UniquePtr<AbstractOutputStream> x_pOutputStream;

public:
	constexpr AbstractOutputStreamFilter() noexcept
		: x_pOutputStream()
	{
	}
	explicit AbstractOutputStreamFilter(UniquePtr<AbstractOutputStream> pOutputStream) noexcept
		: AbstractOutputStreamFilter()
	{
		SetOutputStream(std::move(pOutputStream));
	}
	~AbstractOutputStreamFilter() override;

public:
	const UniquePtr<AbstractOutputStream> &GetOutputStream() const noexcept {
		return x_pOutputStream;
	}
	UniquePtr<AbstractOutputStream> &GetOutputStream() noexcept {
		return x_pOutputStream;
	}
	void SetOutputStream(UniquePtr<AbstractOutputStream> pOutputStream) noexcept {
		x_pOutputStream = std::move(pOutputStream);
	}
};

}

#endif
