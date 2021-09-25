#include "SetControlBlock.h"

SetControlBlock::SetControlBlock(Set* set) { m_set = set; }

RC SetControlBlock::getNext(IVector* const& vec, size_t& index, size_t indexInc) const {
	if (!m_set) {
		return RC::SOURCE_SET_DESTROYED;
	}

	return m_set->getNextVec(vec, index, indexInc);
}

RC SetControlBlock::getPrevious(IVector* const& vec, size_t& index, size_t indexInc) const {
	if (!m_set) {
		return RC::SOURCE_SET_DESTROYED;
	}

	return m_set->getPrevVec(vec, index, indexInc);
}

RC SetControlBlock::getBegin(IVector* const& vec, size_t& index) const {
	if (!m_set) {
		return RC::SOURCE_SET_DESTROYED;
	}

	return m_set->getBeginVec(vec, index);
}

RC SetControlBlock::getEnd(IVector* const& vec, size_t& index) const {
	if (!m_set) {
		return RC::SOURCE_SET_DESTROYED;
	}

	return m_set->getEndVec(vec, index);
}

void SetControlBlock::invalidateSet() { m_set = nullptr; }

ISetControlBlock::~ISetControlBlock() = default;
