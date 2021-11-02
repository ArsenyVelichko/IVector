#pragma once

#include <IDiffProblem.h>

#include "LogUtils.h"

using LogUtils::LogContainer;

class DiffProblem: public IDiffProblem, public LogContainer<DiffProblem> {
public:
    static DiffProblem* createProblem();

    IDiffProblem* clone() const override;

    bool isValidParams(IVector const* const& params) const override;
    bool isValidArgs(IVector const* const& args) const override;

    RC setParams(IVector const* const& params) override;
    RC setArgs(IVector const* const& args) override;

    RC setArgsDomain(ICompact const* const& args, ILogger* logger) override;
    RC setParamsDomain(ICompact const* const& params) override;

    double evalByParams(IVector const* const& params) const override;
    double evalByArgs(IVector const* const& args) const override;

    double evalDerivativeByArgs(IVector const* const& args, IMultiIndex const* const& index) const override;
    double evalDerivativeByParams(IVector const* const& params, IMultiIndex const* const& index) const override;

    RC evalGradientByArgs(IVector const* const& args, IVector* const& val) const override;
    RC evalGradientByParams(IVector const* const& params, IVector* const& val) const override;

    ~DiffProblem() override;

private:
	enum Args {
		X,
		Y,
		ARGS_NUM,
	};

	static const int m_argsPows[];

	enum Params {
		A,
		B,
		C,
		D,
		PARAMS_NUM,
	};

	DiffProblem() = default;

    static double eval(const IVector* params, const IVector* args);
	static size_t intervalFactorial(size_t min, size_t max);

    ICompact* m_paramsDomain = nullptr;
    ICompact* m_argsDomain = nullptr;
    IVector* m_args = nullptr;
    IVector* m_params = nullptr;
};
