#pragma once
#include "../include/ISet.h"
#include "ISetControlBlock.h"
#include <memory>

class SetControlBlock;

class Set : public ISet {
public:
	static ILogger* _logger;

	Set();

	ISet* clone() const override;

	size_t getDim() const override;
	size_t getSize() const override;
	RC getCopy(size_t index, IVector*& val) const override;

	RC findFirstAndCopy(IVector const* const& pat,
						IVector::NORM n,
						double tol,
						IVector*& val) const override;

	RC getCoords(size_t index, IVector* const& val) const override;

	RC findFirstAndCopyCoords(IVector const* const& pat,
							  IVector::NORM n,
							  double tol,
							  IVector* const& val) const override;

	RC insert(IVector const* const& val, IVector::NORM n, double tol) override;

	RC remove(size_t index) override;
	RC remove(IVector const* const& pat, IVector::NORM n, double tol) override;

	class Iterator : public ISet::IIterator {
	public:
		static ILogger* _logger;

		Iterator(const std::shared_ptr<ISetControlBlock>& controlBlock,
				 IVector* vector,
				 size_t index);

		/*
		 * Create iterator associated with next/previous position
		 *
		 * @param [in] indexInc Quantity of steps forward
		 */
		IIterator* getNext(size_t indexInc = 1) const override;
		IIterator* getPrevious(size_t indexInc = 1) const override;
		IIterator* clone() const override;

		/*
		 * Moves iterator forward/backward
		 */
		RC next(size_t indexInc = 1) override;
		RC previous(size_t indexInc = 1) override;

		bool isValid() const override;

		RC makeBegin() override;
		RC makeEnd() override;

		/*
		 * Getter of value (same semantic as ISet::getCopy)
		 */
		RC getVectorCopy(IVector*& val) const override;
		/*
		 * Getter of value (same semantic as ISet::getCoords)
		 */
		RC getVectorCoords(IVector* const& val) const override;

		~Iterator() override;

	private:
		IVector* _vector = nullptr;
		size_t _index = 0;
		std::shared_ptr<ISetControlBlock> _cBlock;
	};

	IIterator* getIterator(size_t index) const override;
	IIterator* getBegin() const override;
	IIterator* getEnd() const override;

	void getNextVec(IVector* vector, size_t& key, size_t inc);
	void getPrevVec(IVector* vector, size_t& key, size_t dec);
	void getBeginVec(IVector* vector, size_t& key);
	void getEndVec(IVector* vector, size_t& key);

	~Set();

private:
	double* _data;
	size_t* _hash;
	size_t _nextHash;

	size_t _dim;
	size_t _allocated;
	size_t _size;

	std::shared_ptr<SetControlBlock> _cBlock;

	size_t vecDataSize() const;

	RC findFirst(IVector const* const& pat,
				 IVector::NORM n,
				 double tol,
				 IVector* const& val,
				 size_t& index) const;

	bool allocate();
};
