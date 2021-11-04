#include <iostream>
#include <vector>
#include <cassert>

#include <IDiffProblem.h>
#include <ISolver.h>

#include "Tests.h"
#include "DllLoader.h"
#include "PrintUtils.h"

void Tests::solverTest(ILogger* logger) {
	auto impl = IBroker::INTERFACE_IMPL::IPROBLEM;

	DllLoader problemLoader;
	assert(problemLoader.loadLibrary("libProblem"));
	auto problem = reinterpret_cast<IDiffProblem*>(problemLoader.loadImplementation(impl));

	impl = IBroker::INTERFACE_IMPL::ISOLVER;

	DllLoader solverLoader;
	assert(solverLoader.loadLibrary("libSolver"));
	auto solver = reinterpret_cast<ISolver*>(solverLoader.loadImplementation(impl));

	double tol = 1.0e-6;
	size_t argDim = 2;

	std::vector<double> leftBoundData = { -20, -20 };
	std::vector<double> rightBoundData = { 20, 20 };
	std::vector<size_t> gridData = { 100, 100 };

	IVector* argsLeftBound = IVector::createVector(argDim, leftBoundData.data());
	IVector* argsRightBound = IVector::createVector(argDim, rightBoundData.data());
	IMultiIndex* argsGrid = IMultiIndex::createMultiIndex(argDim, gridData.data());

	ICompact* argsDomain = ICompact::createCompact(argsLeftBound, argsRightBound, argsGrid);

	delete argsLeftBound;
	delete argsRightBound;
	delete argsGrid;

	size_t paramDim = 4;

	leftBoundData = { -10, -10, -10, -10 };
	rightBoundData = { 10, 10, 10, 10 };
	gridData = { 50, 50, 50, 50 };

	IVector* paramsLeftBound = IVector::createVector(paramDim, leftBoundData.data());
	IVector* paramsRightBound = IVector::createVector(paramDim, rightBoundData.data());
	IMultiIndex* paramsGrid = IMultiIndex::createMultiIndex(paramDim, gridData.data());

	ICompact* paramsDomain = ICompact::createCompact(paramsLeftBound, paramsRightBound, paramsGrid);

	delete paramsLeftBound;
	delete paramsRightBound;
	delete paramsGrid;

	problem->setArgsDomain(argsDomain, logger);
	problem->setParamsDomain(paramsDomain);

	std::cout << "Problem: 3x^4 + 2y^2 - sin(4x)" << std::endl;
	std::cout << "This problem has 2 local minima: (-0.666, 0) and (0.358, 0)" << std::endl;

	auto paramPoint = IVector::createVector(paramDim, std::vector<double>{ 3, 2, -1, 4 }.data());
	problem->setParams(paramPoint);

	delete paramPoint;

	solver->setProblem(problem);
	solver->setArgsDomain(argsDomain, logger);
	solver->setParamsDomain(paramsDomain);

	delete argsDomain;
	delete paramsDomain;

	auto solverParams = IVector::createVector(4, std::vector<double>{ tol, 0.9, 0.5, 0.5 }.data());
	std::cout << "Solver params: ";
	PrintUtils::printVector(solverParams);

	auto findMinima = [&] (const std::vector<double>& startPointData) {
		auto startPoint = IVector::createVector(argDim, startPointData.data());

		std::cout << "Start point: ";
		PrintUtils::printVector(startPoint);

		solver->solveByArgs(startPoint, solverParams);

		IVector* solution = nullptr;
		solver->getSolution(solution);
		assert(solution);

		std::cout << "Solutuion: ";
		PrintUtils::printVector(solution);

		IVector* solutionGrad = IVector::createVector(argDim, std::vector<double>(argDim).data());
		problem->evalGradientByArgs(solution, solutionGrad);

		double gradNorm = solutionGrad->norm(IVector::NORM::SECOND);
		std::cout << "Solution gradient norm: " << gradNorm << std::endl;
		assert(gradNorm < tol);

		delete solutionGrad;
		delete solution;
		delete startPoint;
	};

	findMinima({ -0.5, -2.0 });
	findMinima({ 1.0, 1.0 });

	delete solverParams;
	delete problem;
	delete solver;

	std::cout << "Solver test successfully finished\n\n";
}