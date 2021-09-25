#pragma once

#include "Compact.h"
#include "ICompactControlBlock.h"

class CompactControlBlock : public ICompactControlBlock {
public:
	CompactControlBlock(Compact* parent);

	RC get(IMultiIndex* const& currentIndex, IMultiIndex const* const& bypassOrder) const override;
	RC get(IMultiIndex const* const& currentIndex, IVector* const& val) const override;

	void invalidateCompact();

	~CompactControlBlock();

private:
	Compact* m_compact;
};
