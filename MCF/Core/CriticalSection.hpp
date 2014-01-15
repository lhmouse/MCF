// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013. LH_Mouse. All wrongs reserved.

#ifndef __MCF_CRITICAL_SECTION_HPP__
#define __MCF_CRITICAL_SECTION_HPP__

#include "NoCopy.hpp"

namespace MCF {

class CriticalSection : NO_COPY {
private:
	class xDelegate;
public:
	class LockHolder {
		friend class CriticalSection;
	private:
		CriticalSection *xm_pOwner;
	private:
		LockHolder(CriticalSection *pOwner) noexcept : xm_pOwner(pOwner) {
			ASSERT(pOwner != nullptr);

			xm_pOwner->xEnter();
		}
	public:
		LockHolder(const LockHolder &rhs) noexcept : xm_pOwner(rhs.xm_pOwner) {
			if(xm_pOwner != nullptr){
				xm_pOwner->xEnter();
			}
		}
		LockHolder(LockHolder &&rhs) noexcept : xm_pOwner(rhs.xm_pOwner) {
			rhs.xm_pOwner = nullptr;

			if(xm_pOwner != nullptr){
				xm_pOwner->xEnter();
			}
		}
		LockHolder &operator=(const LockHolder &rhs) noexcept {
			if(this != &rhs){
				this->~LockHolder();
				new(this) LockHolder(rhs);
			}
			return *this;
		}
		LockHolder &operator=(LockHolder &&rhs) noexcept {
			if(this != &rhs){
				this->~LockHolder();
				new(this) LockHolder(std::move(rhs));
			}
			return *this;
		}
		~LockHolder(){
			if(xm_pOwner != nullptr){
				xm_pOwner->xLeave();
			}
		}
	};
private:
	const std::unique_ptr<xDelegate> xm_pDelegate;
public:
	CriticalSection(unsigned long ulSpinCount = 0x400);
	~CriticalSection();
private:
	void xEnter();
	void xLeave();
public:
	LockHolder Lock() noexcept;
};

}

#endif
