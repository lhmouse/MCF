// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2014, LH_Mouse. All wrongs reserved.

#ifndef MCF_CORE_TRANSACTION_HPP_
#define MCF_CORE_TRANSACTION_HPP_

#include "../Utilities/Noncopyable.hpp"
#include "../Containers/Vector.hpp"
#include <memory>

namespace MCF {

class TransactionItemBase : NONCOPYABLE {
public:
	virtual ~TransactionItemBase();

public:
	virtual bool Lock() = 0;
	virtual void Commit() noexcept = 0;
	virtual void Unlock() noexcept = 0;
};

class Transaction : NONCOPYABLE {
private:
	Vector<std::unique_ptr<TransactionItemBase>> xm_vecItems;

public:
	bool IsEmpty() const noexcept;
	void AddItem(std::unique_ptr<TransactionItemBase> &&pItem);
	void Clear() noexcept;

	bool Commit() const;
};

}

#endif
