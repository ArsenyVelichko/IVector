#include <vector>
#include <iostream>
#include <cassert>
#include <limits>

#include "Tests.h"
#include "PrintUtils.h"

void Tests::vectorTest(ILogger* logger) {
	IVector::setLogger(logger);

	double tol = 1.0e-10;
	size_t dim = 3;

	IVector* vec1 = IVector::createVector(dim, std::vector<double>{1, 2, 3}.data());
	IVector* vec2 = IVector::createVector(dim, std::vector<double>{-1, -2, -3}.data());

	std::cout << "Vector A: ";
	PrintUtils::printVector(vec1);

	std::cout << "Vector B: ";
	PrintUtils::printVector(vec2);

	IVector* subRes = IVector::sub(vec1, vec2);
	IVector* correctSubRes = IVector::createVector(dim, std::vector<double>{2, 4, 6}.data());
	std::cout << "Subtraction: ";
	PrintUtils::printVector(subRes);
	assert(IVector::equals(subRes, correctSubRes, IVector::NORM::SECOND, tol));

	delete subRes;
	delete correctSubRes;

	double dotProduct = IVector::dot(vec1, vec2);
	std::cout << "Dot product: " << dotProduct << std::endl;
	assert(Tests::fuzzyCompare(dotProduct, -14, tol));

	double firstNorm = vec1->norm(IVector::NORM::FIRST);
	std::cout << "First norm of Vector A: " << firstNorm << std::endl;
	assert(Tests::fuzzyCompare(firstNorm, 6, tol));

	double infiniteNorm = vec2->norm(IVector::NORM::CHEBYSHEV);
	std::cout << "Infinite norm of Vector B: " << infiniteNorm << std::endl;
	assert(Tests::fuzzyCompare(infiniteNorm, 3, tol));

	std::cout << "Setting coord 1 of Vector B to -7 :";
	vec2->setCoord(1, -7);
	IVector* correctSetRes = IVector::createVector(dim, std::vector<double>{-1, -7, -3}.data());
	PrintUtils::printVector(vec2);
	assert(IVector::equals(vec2, correctSetRes, IVector::NORM::SECOND, tol));

	delete correctSetRes;

	vec1->inc(vec2);
	IVector* correctIncRes = IVector::createVector(dim, std::vector<double>{0, -5, 0}.data());
	std::cout << "Incrementing Vector A with Vector B: ";
	PrintUtils::printVector(vec1);
	assert(IVector::equals(vec1, correctIncRes, IVector::NORM::SECOND, tol));

	delete correctIncRes;

	std::cout << "Copying Vector A to Vector B: ";
	IVector::copyInstance(vec2, vec1);
	PrintUtils::printVector(vec2);
	assert(IVector::equals(vec2, vec1, IVector::NORM::SECOND, tol));

	std::cout << "Moving Vector A to Vector B: ";
	IVector::moveInstance(vec2, vec1);
	PrintUtils::printVector(vec2);
	assert(vec1 == nullptr);

	double inf = std::numeric_limits<double>::infinity();
	std::cout << "Trying set infinity coord value in Vector B: ";
	RC rc = vec2->setCoord(1, inf);
	PrintUtils::printVector(vec2);
	assert(rc != RC::SUCCESS);

	delete vec2;

	std::cout << "Vector test successfully finished\n\n";
}