// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2014, LH_Mouse. All wrongs reserved.

#ifndef MCF_TRANSACTION_HPP_
#define MCF_TRANSACTION_HPP_

#include "../Utilities/NoCopy.hpp"
#include "../Containers/VVector.hpp"
#include <memory>

namespace MCF {

class TransactionItemBase : NO_COPY {
	friend class Transaction;

private:
	virtual bool xLock() = 0;
	virtual void xCommit() noexcept = 0;
	virtual void xUnlock() noexcept = 0;
};

class Transaction : NO_COPY {
private:
	VVector<std::unique_ptr<TransactionItemBase>> xm_vecItems;

public:
	bool IsEmpty() const noexcept;
	void AddItem(std::unique_ptr<TransactionItemBase> pItem);
	void Clear() noexcept;

	bool Commit() const;
};

}

#endif
