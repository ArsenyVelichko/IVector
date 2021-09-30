#include "Compact.h"
#include "CompactControlBlock.h"

#include <cstring>
#include <limits>

struct Compact::Iterator::IteratorDef {
	IMultiIndex* order = nullptr;
	IMultiIndex* pos = nullptr;
	IVector* vector = nullptr;

	bool isValid() const;
	void clear();
};

ICompact::IIterator* Compact::getIterator(const IMultiIndex* const& index,
										  const IMultiIndex* const& bypassOrder) const {
	if (!isOrderValid(bypassOrder)) {
		log_warning(RC::INVALID_ARGUMENT);
		return nullptr;
	}

	Iterator::IteratorDef def;
	def.order = bypassOrder->clone();
	def.pos = index->clone();
	getVectorCopy(index, def.vector);

	if (!def.isValid()) {
		def.clear();
		return nullptr;
	}

	auto res = new (std::nothrow) Iterator(def, m_controlBlock);
	if (!res) {
		log_warning(RC::ALLOCATION_ERROR);
		def.clear();
		return nullptr;
	}

	return res;
}

ICompact::IIterator* Compact::getEnd(const IMultiIndex* const& bypassOrder) const {
	std::shared_ptr<IMultiIndex> endIndex(m_nodeQuantities->clone());

	auto endIndexData = endIndex->getData();
	for (size_t i = 0; i < getDim(); i++) {
		endIndex->setAxisIndex(i, endIndexData[i] - 1);
	}

	return getIterator(endIndex.get(), bypassOrder);
}

ICompact::IIterator* Compact::getBegin(const IMultiIndex* const& bypassOrder) const {
	std::shared_ptr<size_t> zeroData(new (std::nothrow) size_t[getDim()]());
	std::shared_ptr<IMultiIndex> beginIdx(IMultiIndex::createMultiIndex(getDim(), zeroData.get()));
	return getIterator(beginIdx.get(), bypassOrder);
}

RC ICompact::IIterator::setLogger(ILogger* const pLogger) {
	return LogContainer<Compact::Iterator>::setInstance(pLogger);
}

ILogger* ICompact::IIterator::getLogger() { //
	return LogContainer<Compact::Iterator>::getInstance();
}

RC Compact::Iterator::getVectorCopy(IVector*& val) const {
	if (m_placeChanged) {
		RC code = m_controlBlock->get(m_pos, m_vector);
		if (code != RC::SUCCESS) {
			return code;
		}
		m_placeChanged = false;
	}

	IVector* cloneVec = m_vector->clone();

	if (cloneVec == nullptr) {
		return RC::ALLOCATION_ERROR;
	}

	val = cloneVec;
	return RC::SUCCESS;
}

RC Compact::Iterator::getVectorCoords(IVector* const& val) const {
	if (m_placeChanged) {
		RC code = m_controlBlock->get(m_pos, m_vector);
		if (code != RC::SUCCESS) {
			return code;
		}
		m_placeChanged = false;
	}

	return val->setData(m_vector->getDim(), m_vector->getData());
}

ICompact::IIterator* Compact::Iterator::clone() const {
	IteratorDef def;

	def.pos = m_pos->clone();
	def.order = m_order->clone();
	def.vector = m_vector->clone();

	if (!def.isValid()) {
		def.clear();
		return nullptr;
	}

	auto* iterator = new (std::nothrow) Compact::Iterator(def, m_controlBlock);

	if (!iterator) {
		log_warning(RC::ALLOCATION_ERROR);
		def.clear();
		return nullptr;
	}

	iterator->m_placeChanged = m_placeChanged;
	iterator->m_isValid = m_isValid;
	return iterator;
}

ICompact::IIterator* Compact::Iterator::getNext() {
	IIterator* copy = clone();
	if (copy) {
		copy->next();
	}
	return copy;
}

Compact::Iterator::Iterator(const IteratorDef& def,
							const std::shared_ptr<CompactControlBlock>& controlBlock) :
		m_order(def.order), m_pos(def.pos), m_vector(def.vector), m_controlBlock(controlBlock) {}

bool Compact::Iterator::isValid() const { return m_isValid; }

RC Compact::Iterator::next() {
	RC code = m_controlBlock->get(m_pos, m_order);

	if (code == RC::SUCCESS) {
		m_placeChanged = true;
	} else {
		m_isValid = false;
	}

	return code;
}

bool Compact::Iterator::IteratorDef::isValid() const { return order && pos && vector; }

void Compact::Iterator::IteratorDef::clear() {
	delete order;
	order = nullptr;

	delete pos;
	pos = nullptr;

	delete vector;
	vector = nullptr;
}

Compact::Iterator::~Iterator() {
	delete m_vector;
	delete m_order;
	delete m_pos;
}

ICompact::IIterator::~IIterator() = default;
