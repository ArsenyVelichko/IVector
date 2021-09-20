#pragma once
#include <IVector.h>

#include "LogUtils.h"

constexpr int foo() { return 0; }

namespace {

	class VectorImpl : public IVector {
	public:
		IVector* clone() const override;
		double const* getData() const override;
		RC setData(size_t dim, double const* const& data) override;

		RC getCord(size_t index, double& val) const override;
		RC setCord(size_t index, double val) override;
		RC scale(double multiplier) override;
		size_t getDim() const override;

		RC inc(IVector const* const& op) override;
		RC dec(IVector const* const& op) override;

		double norm(NORM n) const override;

		RC applyFunction(const std::function<double(double)>& fun) override;
		RC foreach (const std::function<void(double)>& fun) const override;

		size_t sizeAllocated() const override;

		~VectorImpl();

		static VectorImpl* createVector(size_t dim, const double* const& data);
		static RC setLogger(ILogger* logger);
		static ILogger* getLogger();

	private:
		VectorImpl(size_t dim);

		double* getData();

		double infiniteNorm() const;
		double firstNorm() const;
		double secondNorm() const;

		size_t m_dim;
		static ILogger* m_logger;
	};

} // namespace
