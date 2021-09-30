#pragma once

#include <IMultiIndex.h>

#include "LogUtils.h"

using LogUtils::LogContainer;

namespace {

	class MultiIndex : public IMultiIndex, public LogContainer<MultiIndex> {
	public:
		static MultiIndex* createMultiIndex(size_t dim, const size_t* indices);

		IMultiIndex* clone() const override;

		size_t getDim() const override;
		const size_t* getData() const override;
		RC setData(size_t dim, size_t const* const& ptr_data) override;

		RC getAxisIndex(size_t axisIndex, size_t& val) const override;
		RC setAxisIndex(size_t axisIndex, size_t val) override;

		RC incAxisIndex(size_t axisIndex, size_t val) override;

		~MultiIndex() override;

	private:
		MultiIndex(size_t dim);

		size_t* getData();

		size_t m_dim;
	};

} // namespace
