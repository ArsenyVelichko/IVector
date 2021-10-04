#include <cmath>
#include "Problem.h"

RC IProblem::setLogger(ILogger* const logger) {
	return LogContainer<Problem>::setInstance(logger);
}

RC IDiffProblem::setLogger(ILogger* const logger) {
	return IProblem::setLogger(logger);
}

RC Problem::setArgsDomain(ICompact const* const& args, ILogger* logger) {
	setLogger(logger);

	if (!args) {
		delete _curArgs;
		delete _argsDomain;
		_curArgs = nullptr;
		_argsDomain = nullptr;
		return RC::SUCCESS;
	}

	if (args->getDim() != ARGS::ARGS_NUM) {
		log_warning(RC::MISMATCHING_DIMENSIONS);
		return RC::MISMATCHING_DIMENSIONS;
	}

	delete _argsDomain;
	if (!args || _curArgs && !args->isInside(_curArgs)) {
		delete _curArgs;
		_curArgs = nullptr;
	}
	if (!args) {
		_argsDomain = nullptr;
		return RC::SUCCESS;
	}
	_argsDomain = args->clone();
	return _argsDomain ? RC::SUCCESS : RC::ALLOCATION_ERROR;
}

RC Problem::setParamsDomain(ICompact const* const& params) {
	if (!params) {
		delete _curParams;
		delete _paramsDomain;
		_paramsDomain = nullptr;
		_curParams = nullptr;
		return RC::SUCCESS;
	}

	if (params->getDim() != PARAMS::PARAMS_NUM) {
		log_warning(RC::MISMATCHING_DIMENSIONS);
		return RC::MISMATCHING_DIMENSIONS;
	}
	IVector* leftBound = nullptr;
	RC code = params->getLeftBoundary(leftBound);
	if (code != RC::SUCCESS) {
		log_warning(code);
		return code;
	}
	const double* data = leftBound->getData();
	if (data[PARAMS::A] <= 0 || data[PARAMS::B] <= 0) {
		delete leftBound;
		log_warning(RC::INVALID_ARGUMENT);
		return RC::INVALID_ARGUMENT;
	}

	delete _paramsDomain;
	if (_curParams && !params->isInside(_curParams)) {
		delete _curParams;
		_curParams = nullptr;
	}
	_paramsDomain = params->clone();
	return _paramsDomain ? RC::SUCCESS : RC::ALLOCATION_ERROR;
}

IDiffProblem* IDiffProblem::createDiffProblem() {
	return new (std::nothrow) Problem();
}

IDiffProblem* Problem::clone() const {
	IDiffProblem* copy = new(std::nothrow) Problem();

	if (!copy) {
		log_warning(RC::ALLOCATION_ERROR);
		return nullptr;
	}

	if (copy->setParamsDomain(_paramsDomain) != RC::SUCCESS ||
		copy->setParams(_curParams) != RC::SUCCESS ||
		copy->setArgsDomain(_argsDomain) != RC::SUCCESS ||
		copy->setArgs(_curArgs) != RC::SUCCESS) {
		delete copy;
		log_warning(RC::ALLOCATION_ERROR);
		return nullptr;
	}
	return copy;
}

bool Problem::isValidParams(IVector const* const& params) const {
	return _paramsDomain->isInside(params);
}

bool Problem::isValidArgs(IVector const* const& args) const {
	return _argsDomain->isInside(args);
}

RC Problem::setParams(IVector const* const& params) {
	if (!params) {
		delete _curParams;
		_curParams = nullptr;
		return RC::SUCCESS;
	}

	if (!isValidParams(params)) {
		log_warning(RC::INVALID_ARGUMENT);
		return RC::INVALID_ARGUMENT;
	}

	if (!_curParams) {
		_curParams = params->clone();
		if (!_curParams) {
			return RC::ALLOCATION_ERROR;
		}
		return RC::SUCCESS;
	}
	return _curParams->setData(params->getDim(), params->getData());
}

RC Problem::setArgs(IVector const* const& args) {
	if (!args) {
		delete _curArgs;
		_curArgs = nullptr;
		return RC::SUCCESS;
	}

	if (!isValidArgs(args)) {
		log_warning(RC::INVALID_ARGUMENT);
		return RC::INVALID_ARGUMENT;
	}

	if (!_curArgs) {
		_curArgs = args->clone();
		if (!_curArgs) {
			return RC::ALLOCATION_ERROR;
		}
		return RC::SUCCESS;
	}
	return _curArgs->setData(args->getDim(), args->getData());
}

double Problem::eval(double const* params, double const* args) const {
	return params[PARAMS::A] * pow(args[ARGS::X], 2) + params[PARAMS::B] * pow(args[ARGS::Y], 2)
		   + params[PARAMS::C] * sin(params[PARAMS::D] * args[ARGS::X]);
}

double Problem::evalByParams(IVector const* const& params) const {

	if (params->getDim() != PARAMS::PARAMS_NUM) {
		log_warning(RC::MISMATCHING_DIMENSIONS);
		return NAN;
	}
	if (!isValidParams(params)) {
		log_warning(RC::INVALID_ARGUMENT);
		return NAN;
	}
	if (!_curArgs) {
		log_warning(RC::NO_ARGS_SET);
		return NAN;
	}

	return eval(params->getData(), _curArgs->getData());
}

double Problem::evalByArgs(IVector const* const& args) const {

	if (args->getDim() != ARGS::ARGS_NUM) {
		log_warning(RC::MISMATCHING_DIMENSIONS);
		return NAN;
	}
	if (!isValidArgs(args)) {
		log_warning(RC::INVALID_ARGUMENT);
		return NAN;
	}
	if (!_curParams) {
		log_warning(RC::NO_PARAMS_SET);
		return NAN;
	}

	return eval(_curParams->getData(), args->getData());
}

double Problem::evalDerivativeByArgs(IVector const* const& args, IMultiIndex const* const& index) const {

	if (index->getDim() != ARGS::ARGS_NUM || args->getDim() != ARGS::ARGS_NUM) {
		log_warning(RC::MISMATCHING_DIMENSIONS);
		return NAN;
	}
	if (!isValidArgs(args)) {
		log_warning(RC::INVALID_ARGUMENT);
		return NAN;
	}
	if (!_curParams) {
		log_warning(RC::NO_PARAMS_SET);
		return NAN;
	}

	const size_t* derOrder = index->getData();
	size_t dx = derOrder[ARGS::X];
	size_t dy = derOrder[ARGS::Y];
	const double* argsData = args->getData();
	const double* paramsData = _curParams->getData();
	int powX = 2 - dx;
	int powY = 2 - dy;
	int fact = 1;
	for (int i = 2; i > powX; i--) {
		fact *= i;
	}
	double parA = dy > 0 ? 0 : paramsData[PARAMS::A] * fact;
	fact = 1;
	for (int i = 2; i > powY; i--) {
		fact *= i;
	}
	double parB = dx > 0 ? 0 : paramsData[PARAMS::B] * fact;
	double parC = dy > 0 ? 0 : paramsData[PARAMS::C] * pow(-1, dx / 2) * pow(paramsData[PARAMS::D], dx);
	bool fSin = dx % 2 == 0 ? true : false;
	double res = parA * pow(argsData[ARGS::X], powX) + parB * pow(argsData[ARGS::Y], powY);
	res += fSin ? parC * sin(paramsData[PARAMS::D] * argsData[ARGS::X]) : parC * cos(paramsData[PARAMS::D] * argsData[ARGS::X]);
	return res;
}

double Problem::evalDerivativeByParams(IVector const* const& params, IMultiIndex const* const& index) const {

	if (index->getDim() != PARAMS::PARAMS_NUM || params->getDim() != PARAMS::PARAMS_NUM) {
		log_warning(RC::MISMATCHING_DIMENSIONS);
	}
	if (!isValidParams(params)) {
		log_warning(RC::INVALID_ARGUMENT);
		return NAN;
	}
	if (!_curArgs) {
		log_warning(RC::NO_ARGS_SET);
		return NAN;
	}

	const size_t* derOrder = index->getData();
	const double* argsData = _curArgs->getData();
	const double* paramsData = params->getData();
	double term1, term2, term3;

	if (derOrder[PARAMS::C] > 0 || derOrder[PARAMS::D] > 0) {
		term1 = term2 = 0;
		if (derOrder[PARAMS::C] > 1) {
			term3 = 0;
		}
		else {
			term3 = derOrder[PARAMS::C] == 0 ? paramsData[PARAMS::C] : 1;
			term3 *= pow(argsData[ARGS::X], derOrder[PARAMS::D]);
			term3 *= pow(-1, derOrder[PARAMS::D] / 2);
			term3 *= derOrder[PARAMS::D] % 2 == 0 ? sin(paramsData[PARAMS::D] * argsData[ARGS::X]) : cos(sin(paramsData[PARAMS::D] * argsData[ARGS::X]));
		}
	}
	else {
		if (derOrder[PARAMS::A] > 0) {
			term1 = derOrder[PARAMS::A] == 1 ? pow(argsData[ARGS::X], 2) : 0;
			term2 = 0;
			term3 = 0;
		}
		else {
			if (derOrder[PARAMS::B] == 0) {
				term1 = paramsData[PARAMS::A] * pow(argsData[ARGS::X], 2);
				term2 = paramsData[PARAMS::B] * pow(argsData[ARGS::Y], 2);
				term3 = paramsData[PARAMS::C] * sin(paramsData[PARAMS::D] * argsData[ARGS::X]);
			}
			else {
				term1 = 0;
				term2 = derOrder[PARAMS::B] == 1 ? pow(argsData[ARGS::Y], 2) : 0;
				term3 = 0;
			}
		}
	}
	return term1 + term2 + term3;
}

RC Problem::evalGradientByArgs(IVector const* const& args, IVector* const& val) const {

	if (args->getDim() != ARGS::ARGS_NUM || val->getDim() != ARGS::ARGS_NUM) {
		log_warning(RC::MISMATCHING_DIMENSIONS);
		return RC::MISMATCHING_DIMENSIONS;
	}
	if (!isValidArgs(args)) {
		log_warning(RC::INVALID_ARGUMENT);
		return RC::INVALID_ARGUMENT;
	}
	if (!_curParams) {
		log_warning(RC::NO_PARAMS_SET);
		return RC::NO_PARAMS_SET;
	}

	const double* argsData = args->getData();
	const double* paramsData = _curParams->getData();
	val->setCoord(ARGS::X, 2 * paramsData[PARAMS::A] * argsData[ARGS::X] + paramsData[PARAMS::C] * paramsData[PARAMS::D] * cos(paramsData[PARAMS::D] * argsData[ARGS::X]));
	val->setCoord(ARGS::Y, 2 * paramsData[PARAMS::B] * argsData[ARGS::Y]);
	return RC::SUCCESS;
}

RC Problem::evalGradientByParams(IVector const* const& params, IVector* const& val) const {

	if (val->getDim() != PARAMS::PARAMS_NUM || params->getDim() != PARAMS::PARAMS_NUM) {
		log_warning(RC::MISMATCHING_DIMENSIONS);
	}
	if (!isValidParams(params)) {
		log_warning(RC::INVALID_ARGUMENT);
		return RC::INVALID_ARGUMENT;
	}
	if (!_curArgs) {
		log_warning(RC::NO_ARGS_SET);
		return RC::NO_ARGS_SET;
	}

	const double x = _curArgs->getData()[ARGS::X];
	const double y = _curArgs->getData()[ARGS::Y];
	const double* paramsData = params->getData();
	val->setCoord(PARAMS::A, x * x);
	val->setCoord(PARAMS::B, y * y);
	val->setCoord(PARAMS::C, sin(paramsData[PARAMS::D] * x));
	val->setCoord(PARAMS::D, paramsData[PARAMS::C] * x * cos(paramsData[PARAMS::D] * x));
	return RC::SUCCESS;
}

Problem::~Problem() {
	delete _paramsDomain;
	delete _argsDomain;
	delete _curParams;
	delete _curArgs;
}

IDiffProblem::~IDiffProblem() = default;

IProblem::~IProblem() = default;
