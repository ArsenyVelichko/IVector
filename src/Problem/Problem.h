#pragma once

#include <IDiffProblem.h>

#include "LogUtils.h"

using LogUtils::LogContainer;

class Problem: public IDiffProblem, public LogContainer<Problem>{
public:
	enum ARGS {
		X,
		Y,
		ARGS_NUM,
		};
	enum PARAMS {
		A,
		B,
		C,
		D,
		PARAMS_NUM,
		};

	//A * x^2 + B * y^2 + C * sin(D * x), A > 0, B > 0
	static Problem* createProblem(ICompact const* const& params, ICompact const* const& args);

	IDiffProblem* clone() const override;

	bool isValidParams(IVector const* const& params) const override;
	bool isValidArgs(IVector const* const& args) const override;

	RC setParams(IVector const* const& params) override;
	RC setArgs(IVector const* const& args) override;

	RC setArgsDomain(ICompact const* const& args, ILogger* logger = nullptr) override;
	RC setParamsDomain(ICompact const* const& params) override;

	double evalByParams(IVector const* const& params) const override;
	double evalByArgs(IVector const* const& args) const override;

	double evalDerivativeByArgs(IVector const* const& args, IMultiIndex const* const& index) const override;
	double evalDerivativeByParams(IVector const* const& params, IMultiIndex const* const& index) const override;

	RC evalGradientByArgs(IVector const* const& args, IVector* const& val) const override;
	RC evalGradientByParams(IVector const* const& params, IVector* const& val) const override;

	~Problem() override;
private:
	double eval(double const* params, double const* args) const;

	ICompact* _paramsDomain = nullptr;
	ICompact* _argsDomain = nullptr;
	IVector* _curArgs = nullptr;
	IVector* _curParams = nullptr;
};
