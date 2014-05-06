// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2014. LH_Mouse. All wrongs reserved.

#ifndef MCF_TRANSACTION_HPP_
#define MCF_TRANSACTION_HPP_

#include "Utilities.hpp"
#include "VVector.hpp"
#include <memory>

namespace MCF {

class OperationBase : NO_COPY, ABSTRACT {
public:
	virtual void Lock() = 0;
	virtual void Commit() noexcept = 0;
	virtual void Unlock() noexcept = 0;
};

class Transaction : NO_COPY {
private:
	VVector<std::unique_ptr<OperationBase>> xm_vecOperations;

public:
	bool IsEmpty() const noexcept;
	void AddOperation(std::unique_ptr<OperationBase> pOperation);
	void Clear();

	void CommitAll();
};

}

#endif
