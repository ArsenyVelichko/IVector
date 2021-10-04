#include "CompactControlBlock.h"

CompactControlBlock::CompactControlBlock(Compact* compact) : m_compact(compact) {}

void CompactControlBlock::invalidateCompact() { m_compact = nullptr; }

RC CompactControlBlock::get(IMultiIndex* const& currentIndex,
							const IMultiIndex* const& bypassOrder) const {
	if (!m_compact) {
		return RC::SOURCE_SET_DESTROYED;
	}
	return m_compact->advance(currentIndex, bypassOrder);
}

RC CompactControlBlock::get(const IMultiIndex* const& currentIndex, IVector* const& val) const {
	if (!m_compact) {
		return RC::SOURCE_SET_DESTROYED;
	}
	return m_compact->getVectorCoords(currentIndex, val);
}

ICompactControlBlock::~ICompactControlBlock() = default;
