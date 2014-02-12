// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2014. LH_Mouse. All wrongs reserved.

#ifndef __MCF_Z_HPP__
#define __MCF_Z_HPP__

#include <memory>
#include <functional>
#include <utility>
#include <cstddef>

namespace MCF {

class ZEncoder {
private:
	class xDelegate;
private:
	const std::unique_ptr<xDelegate> xm_pDelegate;
public:
	ZEncoder(std::function<std::pair<void *, std::size_t>(std::size_t)> fnDataCallback, int nLevel = 6);
	~ZEncoder();
public:
	void Abort() noexcept;
	void Update(const void *pData, std::size_t uSize);
	void Finalize();
};

class ZDecoder {
private:
	class xDelegate;
private:
	const std::unique_ptr<xDelegate> xm_pDelegate;
public:
	ZDecoder(std::function<std::pair<void *, std::size_t>(std::size_t)> fnDataCallback);
	~ZDecoder();
public:
	void Abort() noexcept;
	void Update(const void *pData, std::size_t uSize);
	void Finalize();
};

}

#endif
