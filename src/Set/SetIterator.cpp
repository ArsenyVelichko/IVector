#include "Set.h"
#include "SetControlBlock.h"

RC ISet::IIterator::setLogger(ILogger* const logger) {
	return LogProducer<Set::Iterator>::setLogger(logger);
}

ILogger* ISet::IIterator::getLogger() { //
	return LogProducer<Set::Iterator>::getLogger();
}

Set::Iterator::Iterator(const std::shared_ptr<SetControlBlock>& controlBlock,
						IVector* vector,
						size_t index) :
	m_controlBlock(controlBlock), m_vector(vector), m_hash(index) {}

ISet::IIterator* Set::Iterator::getNext(size_t indexInc) const {
	auto iterator = clone();
	if (!iterator) {
		return iterator;
	}
	iterator->next(indexInc);
	return iterator;
}

ISet::IIterator* Set::Iterator::getPrevious(size_t indexInc) const {
	auto iterator = clone();
	if (!iterator) {
		return iterator;
	}
	iterator->previous(indexInc);
	return iterator;
}

ISet::IIterator* Set::Iterator::clone() const {
	IVector* vec = m_vector->clone();
	if (!vec) {
		return nullptr;
	}

	auto iterator = new (std::nothrow) Set::Iterator(m_controlBlock, vec, m_hash);
	if (!iterator) {
		log_warning(RC::ALLOCATION_ERROR);
		delete vec;
		return nullptr;
	}
	return iterator;
}

RC Set::Iterator::next(size_t indexInc) {
	RC rc = m_controlBlock->getNext(m_vector, m_hash, indexInc);
	if (rc != RC::SUCCESS) {
		m_isValid = false;
	}
	return rc;
}

RC Set::Iterator::previous(size_t indexInc) {
	RC rc = m_controlBlock->getPrevious(m_vector, m_hash, indexInc);
	if (rc != RC::SUCCESS) {
		m_isValid = false;
	}
	return rc;
}

bool Set::Iterator::isValid() const { return m_isValid; }

RC Set::Iterator::makeBegin() {
	RC rc = m_controlBlock->getBegin(m_vector, m_hash);
	if (rc != RC::SUCCESS) {
		m_isValid = false;
	}
	return rc;
}

RC Set::Iterator::makeEnd() {
	RC rc = m_controlBlock->getEnd(m_vector, m_hash);
	if (rc != RC::SUCCESS) {
		m_isValid = false;
	}
	return rc;
}

RC Set::Iterator::getVectorCopy(IVector*& val) const {
	auto copy = m_vector->clone();

	if (!copy) {
		return RC::ALLOCATION_ERROR;
	}
	val = copy;

	return RC::SUCCESS;
}

RC Set::Iterator::getVectorCoords(IVector* const& val) const {
	return val->setData(m_vector->getDim(), m_vector->getData());
}

ISet::IIterator::~IIterator() = default;

Set::Iterator::~Iterator() { delete m_vector; }
