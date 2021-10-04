#pragma once

#include <memory>

#include <ICompact.h>

#include "LogUtils.h"

using LogUtils::LogContainer;
class CompactControlBlock;

class Compact : public ICompact, public LogContainer<Compact> {
public:
	struct CompactDef;

	static Compact* createCompact(const CompactDef& def);

	static Compact* createCompact(IVector const* vec1,
								  IVector const* vec2,
								  IMultiIndex const* nodeQuantities);

	ICompact* clone() const override;

	size_t getDim() const override;
	IMultiIndex* getGrid() const override;

	bool isInside(IVector const* const& vec) const override;

	RC getVectorCopy(IMultiIndex const* index, IVector*& val) const override;
	RC getVectorCoords(IMultiIndex const* index, IVector* const& val) const override;

	RC getLeftBoundary(IVector*& vec) const override;
	RC getRightBoundary(IVector*& vec) const override;

	class Iterator : public ICompact::IIterator, public LogContainer<Iterator> {
	public:
		struct IteratorDef;

		Iterator(const IteratorDef& def, std::shared_ptr<CompactControlBlock> const& controlBlock);

		bool isValid() const override;

		IIterator* getNext() override;
		IIterator* clone() const override;

		RC next() override;

		RC getVectorCopy(IVector*& val) const override;
		RC getVectorCoords(IVector* const& val) const override;

		~Iterator() override;

	private:
		IMultiIndex* m_order;
		IMultiIndex* m_pos;
		IVector* m_vector;
		std::shared_ptr<CompactControlBlock> m_controlBlock;

		mutable bool m_placeChanged = false;
		bool m_isValid = true;
	};

	IIterator* getIterator(IMultiIndex const* const& index,
						   IMultiIndex const* const& bypassOrder) const override;

	IIterator* getBegin(IMultiIndex const* const& bypassOrder) const override;
	IIterator* getEnd(IMultiIndex const* const& bypassOrder) const override;

	RC advance(IMultiIndex* const& pos, IMultiIndex const* const& bypassOrder);

	~Compact() override;

private:
	explicit Compact(const CompactDef& def);

	bool isIndexValid(const IMultiIndex* index) const;
	bool isOrderValid(const IMultiIndex* order) const;

	IVector* m_minBound;
	IVector* m_maxBound;
	IMultiIndex* m_nodeQuantities;
	std::shared_ptr<CompactControlBlock> m_controlBlock;
};
