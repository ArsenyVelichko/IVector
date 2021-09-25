#include "Compact.h"
#include "CompactControlBlock.h"

#include <cstring>
#include <limits>

RC Compact::Iterator::getVectorCopy(IVector*& val) const {
	if (m_placeChanged) {
		RC code = m_controlBlock->get(m_place, m_vector);
		if (code != RC::SUCCESS) {
			return code;
		}
		m_placeChanged = false;
	}

	val = m_vector->clone();
	return val ? RC::SUCCESS : RC::ALLOCATION_ERROR;
}

RC Compact::Iterator::getVectorCoords(IVector* const& val) const {
	if (!m_placeChanged) {
		RC code = m_controlBlock->get(m_place, m_vector);
		if (code != RC::SUCCESS) {
			return code;
		}
		m_placeChanged = false;
	}

	return val->setData(m_vector->getDim(), m_vector->getData());
}

ICompact::IIterator* Compact::Iterator::clone() const {
	IMultiIndex* place = m_place->clone();
	IMultiIndex* byPass = m_order->clone();
	IVector* vector = m_vector->clone();

	if (!place || !byPass || !vector) {
		delete place;
		delete byPass;
		delete vector;
		return nullptr;
	}

	auto* iterator = new (std::nothrow) Compact::Iterator(m_controlBlock, place, byPass, vector);

	if (!iterator) {
		delete place;
		delete byPass;
		delete vector;
		return nullptr;
	}

	iterator->m_placeChanged = m_placeChanged;
	return iterator;
}

Compact::Iterator::Iterator(std::shared_ptr<CompactControlBlock> const& block,
							IMultiIndex* startPos,
							IMultiIndex* byPass,
							IVector* vector) {
	m_controlBlock = block;
	m_place = startPos;
	m_order = byPass;
	m_vector = vector;
	m_placeChanged = true;
}

ICompact::IIterator* Compact::Iterator::getNext() {
	IIterator* copy = clone();
	if (copy) {
		copy->next();
	}
	return copy;
}

bool Compact::Iterator::isValid() const {
	return m_isValid;
}

RC Compact::Iterator::next() {
	RC code = m_controlBlock->get(m_place, m_order);
	m_placeChanged = code == RC::SUCCESS;
	return code;
}

Compact::Iterator::~Iterator() {
	delete m_vector;
	delete m_order;
	delete m_place;
}

ICompact::IIterator::~IIterator() = default;
