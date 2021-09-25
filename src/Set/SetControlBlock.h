#pragma once

#include <ISetControlBlock.h>

#include "Set.h"

class SetControlBlock : public ISetControlBlock {
public:
	SetControlBlock(Set* set);

	void invalidateSet();

	RC getNext(IVector* const& vec, size_t& index, size_t indexInc = 1) const override;
	RC getPrevious(IVector* const& vec, size_t& index, size_t indexInc = 1) const override;

	RC getBegin(IVector* const& vec, size_t& index) const override;
	RC getEnd(IVector* const& vec, size_t& index) const override;

	~SetControlBlock() = default;

private:
	Set* m_set;
};
