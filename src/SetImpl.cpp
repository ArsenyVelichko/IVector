// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com

#include <cstring>
#include <cmath>
#include <limits>
#include "Set.h"
#include "SetControlBlock.h"


constexpr size_t basicSize = 100;
constexpr size_t maxEnlarger = 1000;

ILogger* Set::_logger = nullptr;

inline size_t Set::vecDataSize() const {
	return _dim * sizeof(double);
}

Set::Set() : _cBlock(new(std::nothrow) SetControlBlock(this)) {
	_size = 0;
	_dim = 0;
	_allocated = 0;
	_nextHash = 0;
	_data = nullptr;
	_hash = nullptr;
}

size_t Set::getDim() const {
	return _dim;
}

size_t Set::getSize() const {
	return _size;
}

RC Set::getCopy(size_t index, IVector *& val) const {
#ifndef FAST_MATH
	if (index >= _size) {
		if (_logger) {
			_logger->warning(RC::INDEX_OUT_OF_BOUND);
		}
		return RC::INDEX_OUT_OF_BOUND;
	}
#endif
	IVector* vector = IVector::createVector(_dim, _data + index * _dim);
#ifndef FAST_MATH
	if (!vector) {
		if (_logger) {
			_logger->warning(RC::ALLOCATION_ERROR);
		}
		return RC::ALLOCATION_ERROR;
	}
#endif
	val = vector;
	return RC::SUCCESS;
}

RC Set::findFirst(IVector const * const& pat, IVector::NORM n, double tol, IVector * const& val, size_t& index) const {
#ifndef FAST_MATH
	if (val->getDim() != _dim || pat->getDim() != _dim) {
		if (_logger) {
			_logger->warning(RC::MISMATCHING_DIMENSIONS);
		}
		return RC::MISMATCHING_DIMENSIONS;
	}
#endif
	if (_size == 0) {
		// reason of unwanted log messages in other methods
		/*if (_logger) {
			_logger->info(RC::VECTOR_NOT_FOUND);
		}*/
		return RC::VECTOR_NOT_FOUND;
	}
	for (size_t i = 0; i < _size; i++) {
		val->setData(_dim, _data + i * _dim);
		val->dec(pat);
		if (val->norm(n) < tol) {
			index = i;
			return RC::SUCCESS;
		}
	}
	// reason of unwanted log messages in other methods
	/*if (_logger) {
		_logger->info(RC::VECTOR_NOT_FOUND);
	}*/
	return RC::VECTOR_NOT_FOUND;
}

RC Set::findFirstAndCopy(IVector const * const& pat, IVector::NORM n, double tol, IVector *& val) const {
	size_t index = 0;
	if (_size == 0) {
		if (_logger) {
			_logger->info(RC::VECTOR_NOT_FOUND);
		}
		return RC::VECTOR_NOT_FOUND;
	}
	IVector* vec = IVector::createVector(_dim, _data);
	RC code = findFirst(pat, n, tol, vec, index);
	if (code != RC::SUCCESS) {
		delete vec;
		return code;
	}
	val = vec;
	return RC::SUCCESS;
}

RC Set::findFirstAndCopyCoords(const IVector *const &pat, IVector::NORM n, double tol, IVector *const &val) const {
	size_t index = 0;
	if (_size == 0) {
		if (_logger) {
			_logger->info(RC::VECTOR_NOT_FOUND);
		}
		return RC::VECTOR_NOT_FOUND;
	}
	RC code = findFirst(pat, n, tol, val, index);
	if (code != RC::SUCCESS) {
		return code;
	}
	return RC::SUCCESS;
}

RC Set::getCoords(size_t index, IVector * const& val) const {
#ifndef FAST_MATH
	if (index >= _size) {
		if (_logger) {
			_logger->warning(RC::INDEX_OUT_OF_BOUND);
		}
		return RC::INDEX_OUT_OF_BOUND;
	}
	if (val->getDim() != _dim) {
		if (_logger) {
			_logger->warning(RC::MISMATCHING_DIMENSIONS);
		}
		return RC::MISMATCHING_DIMENSIONS;
	}
#endif
	val->setData(_dim, _data + index * _dim);
	return RC::SUCCESS;
}

bool Set::allocate() {
	size_t enlarger = fmin(_allocated, maxEnlarger);
	size_t newSize = _allocated + enlarger;
	double* newData = new(std::nothrow) double[newSize * _dim];
	size_t* newHash = new(std::nothrow) size_t[newSize];
	if (!newData || !newHash) {
		delete[] newData;
		delete[] newHash;
		return false;
	}
	if (_data) {
		memcpy(newData, _data, _allocated * vecDataSize());
		delete[] _data;
	}
	if (_hash) {
		memcpy(newHash, _hash, _allocated * sizeof(size_t));
		delete[] _hash;
	}
	_allocated = newSize;
	_data = newData;
	_hash = newHash;
	return true;
}

RC Set::insert(IVector const * const& val, IVector::NORM n, double tol) {
	if (_allocated == 0) {
		_allocated = basicSize / 2;
		_dim = val->getDim();
		allocate();
	}
#ifndef FAST_MATH
	if (val->getDim() != _dim) {
		if (_logger) {
			_logger->warning(RC::MISMATCHING_DIMENSIONS);
		}
		return RC::MISMATCHING_DIMENSIONS;
	}
#endif
	if (_size == 0) {
		memmove(_data, val->getData(), _dim * sizeof(double));
		_hash[0] = _nextHash;
		_nextHash++;
		_size++;
		return RC::SUCCESS;
	}
	if (_size == _allocated) {
		if (!allocate()) {
			if (_logger) {
				_logger->warning(RC::ALLOCATION_ERROR);
			}
			return RC::ALLOCATION_ERROR;
		}
	}
	IVector* curVec = IVector::createVector(_dim, _data);
	size_t index = 0;
	if (findFirst(val, n, tol, curVec, index) != RC::SUCCESS) {
		memmove(_data + _size * _dim, val->getData(), _dim * sizeof(double));
		_hash[_size] = _nextHash;
		_nextHash++;
		_size++;
	}
	delete curVec;
	return RC::SUCCESS;
}

RC Set::remove(size_t index) {
#ifndef FAST_MATH
	if (index >= _size) {
		if (_logger) {
			_logger->warning(RC::INDEX_OUT_OF_BOUND);
		}
		return RC::INDEX_OUT_OF_BOUND;
	}
#endif
	memcpy(_data + index * _dim, _data + (index + 1) * _dim, _allocated * vecDataSize() - vecDataSize() * (index + 1));
	memmove(_hash + index, _hash + index + 1, sizeof(size_t) * _allocated  - sizeof(size_t) * (index + 1));
	_size--;
	return RC::SUCCESS;
}

RC Set::remove(IVector const * const& pat, IVector::NORM n, double tol) {
#ifndef FAST_MATH
	if (_size == 0) {
		if (_logger) {
			_logger->info(RC::VECTOR_NOT_FOUND);
		}
		return RC::VECTOR_NOT_FOUND;
	}
#endif
	IVector* vec = IVector::createVector(_dim, _data);
#ifndef FAST_MATH
	if (!vec) {
		if (_logger) {
			_logger->warning(RC::ALLOCATION_ERROR);
		}
		return RC::ALLOCATION_ERROR;
	}
#endif
	//RC code = findFirst(pat, n, tol, vec, index);
	RC code = RC::VECTOR_NOT_FOUND;
	for (size_t i = 0; i < _size; i++) {
		vec->setData(_dim, _data + i * _dim);
		vec->dec(pat);
		if (vec->norm(n) <= tol) {
			remove(i);
			i--;
			code = RC::SUCCESS;
		}
	}
	delete vec;
	return code;
}

Set::~Set() {
	_cBlock->invalidateSet();
	delete[] _data;
	delete[] _hash;
}

RC ISet::setLogger(ILogger* const logger) {
	Set::_logger = logger;
	return RC::SUCCESS;
}

ISet* ISet::createSet() {
	return new(std::nothrow) Set();
}

ISet* ISet::makeIntersection(ISet const * const& op1, ISet const * const& op2, IVector::NORM n, double tol) {
#ifndef FAST_MATH
	if (op1->getDim() != op2->getDim()) {
		if (Set::_logger) {
			Set::_logger->warning(RC::MISMATCHING_DIMENSIONS);
		}
		return nullptr;
	}
#endif
	if (op1->getSize() == 0 || op2->getSize() == 0) {
		return createSet();
	}
	IVector* vec1 = nullptr;
	IVector* vec2 = nullptr;
	if (op1->getCopy(0, vec1) != RC::SUCCESS || op2->getCopy(0, vec2) != RC::SUCCESS) {
		return nullptr;
	}
	ISet* intersection = createSet();
	if (!intersection) {
		return nullptr;
	}
	size_t size1 = op1->getSize();
	size_t size2 = op2->getSize();
	for (size_t i = 0; i < size1; i++) {
		op1->getCoords(i, vec1);
		for (size_t j = 0; j < size2; j++) {
			op2->getCoords(j, vec2);
			vec2->dec(vec1);
			if (vec2->norm(n) <= tol) {
				if (intersection->insert(vec1, n, tol) != RC::SUCCESS) {
					delete vec1;
					delete vec2;
					delete intersection;
					return nullptr;
				}
			}
		}
	}
	delete vec1;
	delete vec2;
	return intersection;
}

ISet* ISet::makeUnion(ISet const * const& op1, ISet const * const& op2, IVector::NORM n, double tol) {
#ifndef FAST_MATH
	if (op1->getDim() != op2->getDim()) {
		if (Set::_logger) {
			Set::_logger->warning(RC::MISMATCHING_DIMENSIONS);
		}
		return nullptr;
	}
#endif
	ISet* unionSet = createSet();
#ifndef FAST_MATH
	if (!unionSet) {
		return nullptr;
	}
#endif
	if (op1->getSize() == 0 && op2->getSize() == 0) {
		return unionSet;
	}
	IVector* buf = nullptr;
	if (op1->getSize() != 0) {
		op1->getCopy(0, buf);
	}
	else {
		op2->getCopy(0, buf);
	}
	const ISet* sets[2] = {op1, op2};
	size_t size;
	for (auto set : sets) {
		size = set->getSize();
		for (size_t i = 0; i < size; i++) {
			set->getCoords(i, buf);
			if (unionSet->insert(buf, n, tol) != RC::SUCCESS) {
				delete buf;
				delete unionSet;
				return nullptr;
			}
		}
	}
	delete buf;
	return unionSet;
}

ISet* ISet::sub(ISet const * const& op1, ISet const * const& op2, IVector::NORM n, double tol) {
	ISet* sub = createSet();
	if (!sub || op1->getSize() == 0) {
		return sub;
	}
	size_t size = op1->getSize();
	IVector* buf = nullptr;
	op1->getCopy(0, buf);
	for (size_t i = 0; i < size; i++) {
		op1->getCoords(i, buf);
		if (sub->insert(buf, n, tol) != RC::SUCCESS) {
			delete sub;
			delete buf;
			return nullptr;
		}
	}
	size = op2->getSize();
	for (size_t i = 0; i < size; i++) {
		op2->getCoords(i, buf);
		sub->remove(buf, n, tol);
	}
	delete buf;
	return sub;
}

ISet* ISet::symSub(ISet const * const& op1, ISet const * const& op2, IVector::NORM n, double tol) {
	ISet* unionSet = Set::makeUnion(op1, op2, n, tol);
	if (!unionSet || unionSet->getSize() == 0) {
		return unionSet;
	}
	ISet* intersection = Set::makeIntersection(op1, op2, n, tol);
#ifndef FAST_MATH
	if (!intersection) {
		return nullptr;
	}
#endif
	size_t size = intersection->getSize();
	IVector* buf = nullptr;
	intersection->getCopy(0, buf);
	if (!buf) {
		delete intersection;
		return nullptr;
	}
	for (size_t i = 0; i < size; i++) {
		intersection->getCoords(i, buf);
		unionSet->remove(buf, n, tol);
	}
	delete buf;
	delete intersection;
	return unionSet;
}

bool ISet::equals(ISet const * const& op1, ISet const * const& op2, IVector::NORM n, double tol) {
	return subSet(op1, op2, n, tol) && subSet(op2, op1, n, tol);
}

bool ISet::subSet(ISet const * const& op1, ISet const * const& op2, IVector::NORM n, double tol) {
	size_t size1 = op1->getSize();
	size_t size2 = op2->getSize();
	IVector* vec1 = nullptr;
	IVector* vec2 = nullptr;
	op1->getCopy(0, vec1);
	op2->getCopy(0, vec2);
	if (!vec1 || !vec2) {
		delete vec1;
		delete vec2;
		return false;
	}
	bool foundEq = false;
	for (size_t i = 0; i < size1; i++) {
		foundEq = false;
		op1->getCoords(i, vec1);
		for (size_t j = 0; j < size2; j++) {
			op2->getCoords(j, vec2);
			vec2->dec(vec1);
			if (vec2->norm(n) <= tol) {
				foundEq = true;
				break;
			}
		}
		if (!foundEq) {
			delete vec1;
			delete vec2;
			return false;
		}
	}
	delete vec1;
	delete vec2;
	return true;
}

ISet* Set::clone() const {
	Set* copy = new(std::nothrow) Set();
	if (!copy) {
		return copy;
	}
	copy->_dim = _dim;
	copy->_allocated = _allocated;
	copy->_nextHash = _nextHash;
	copy->_logger = _logger;
	copy->_size = _size;
	copy->_data = new(std::nothrow) double[_dim * _allocated];
	copy->_hash = new(std::nothrow) size_t[_allocated];
	if (!copy->_data || !copy->_hash) {
		if (_logger) {
			_logger->warning(RC::ALLOCATION_ERROR);
		}
		delete[] copy->_data;
		delete[] copy->_hash;
		delete copy;
		return nullptr;
	}
	memcpy(copy->_data, _data, vecDataSize() * _allocated);
	memcpy(copy->_hash, _hash, sizeof(size_t) * _allocated);
	return copy;
}


ISet::~ISet() = default;

void Set::getNextVec(IVector *vector, size_t &key, size_t inc) {
	if (key == std::numeric_limits<size_t>::max()) {
		return;
	}
	else if (inc >= _size || key >= _hash[_size - inc]) {
		key = std::numeric_limits<size_t>::max();
	}
	else {
		// todo: binary search
		size_t i;
		for (i = 0; i < _size; i++) {
			if (_hash[i] > key) {
				break;
			}
		}
		key = _hash[i + inc - 1];
		vector->setData(_dim, _data + (i + inc - 1) * _dim);
	}
}

void Set::getPrevVec(IVector *vector, size_t &key, size_t dec) {
	if (key == std::numeric_limits<size_t>::max()) {
		return;
	}
	else if (dec >= _size || key <= _hash[dec]) {
		key = std::numeric_limits<size_t>::max();
	}
	else {
		// todo: binary search
		size_t i;
		for (i = _size - 1; i > 0; i--) {
			if (_hash[i] < key) {
				break;
			}
		}
		key = _hash[i - dec + 1];
		vector->setData(_dim, _data + (i - dec + 1) * _dim);
	}
}

void Set::getBeginVec(IVector* vector, size_t& key) {
	if (_size == 0) {
		key = std::numeric_limits<size_t>::max();
		return;
	}
	vector->setData(_dim, _data);
	key = _hash[0];
}
void Set::getEndVec(IVector* vector, size_t& key) {
	if (_size == 0) {
		key = std::numeric_limits<size_t>::max();
		return;
	}
	vector->setData(_dim, _data + _dim * (_size - 1));
	key = _hash[_size - 1];
}

ISet::IIterator* Set::getIterator(size_t index) const {
	if (index >= _size) {
		if (_logger) {
			_logger->warning(RC::INDEX_OUT_OF_BOUND);
		}
	}
	IVector* vec = IVector::createVector(_dim, _data + _dim * index);
	if (!vec) {
		if (_logger) {
			_logger->warning(RC::ALLOCATION_ERROR);
		}
		return  nullptr;
	}
	auto iterator = new(std::nothrow) Set::Iterator(_cBlock, vec, _hash[index]);
	if (!iterator) {
		if (_logger) {
			_logger->warning(RC::ALLOCATION_ERROR);
		}
		delete vec;
		return nullptr;
	}
	return iterator;
}

ISet::IIterator* Set::getBegin() const {
	if (_size == 0) {
		return nullptr;
	}
	IVector* vec = IVector::createVector(_dim, _data);
	if (!vec) {
		if (_logger) {
			_logger->warning(RC::ALLOCATION_ERROR);
		}
		return  nullptr;
	}
	auto iterator = new(std::nothrow) Set::Iterator(_cBlock, vec, _hash[0]);
	if (!iterator) {
		if (_logger) {
			_logger->warning(RC::ALLOCATION_ERROR);
		}
		delete vec;
		return nullptr;
	}
	return iterator;
}

ISet::IIterator* Set::getEnd() const {
	if (_size == 0) {
		return nullptr;
	}
	IVector* vec = IVector::createVector(_dim, _data + _dim * (_size - 1));
	if (!vec) {
		if (_logger) {
			_logger->warning(RC::ALLOCATION_ERROR);
		}
		return nullptr;
	}
	auto iterator = new(std::nothrow) Set::Iterator(_cBlock, vec, _hash[_size - 1]);
	if (!iterator) {
		if (_logger) {
			_logger->warning(RC::ALLOCATION_ERROR);
		}
		delete vec;
		return nullptr;
	}
	return iterator;
}

