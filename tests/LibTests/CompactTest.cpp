#include <cassert>
#include <iostream>
#include <vector>

#include "Tests.h"
#include "PrintUtils.h"

void Tests::compactTest(ILogger* logger) {
	ICompact::setLogger(logger);
	ICompact::IIterator::setLogger(logger);
	IMultiIndex::setLogger(logger);

	size_t dim = 2;
	double tol = 1.0e-10;

	IMultiIndex* steps = IMultiIndex::createMultiIndex(dim, std::vector<size_t>{3, 2}.data());

	IVector* bound1 = IVector::createVector(dim, std::vector<double>{1, 5}.data());
	IVector* bound2 = IVector::createVector(dim, std::vector<double>{-4, 2}.data());

	IMultiIndex* order = IMultiIndex::createMultiIndex(dim, std::vector<size_t>{0, 1}.data());

	std::cout << "Creating Compact A:" << std::endl;
	ICompact* compact1 = ICompact::createCompact(bound1, bound2, steps);
	PrintUtils::printCompact(compact1, order);
	assert(compact1->getDim() == dim);

	std::cout << "Compact A with reverse iteration:" << std::endl;
	order->setAxisIndex(0, 1);
	order->setAxisIndex(1, 0);
	PrintUtils::printCompact(compact1, order);

	std::cout << "Trying to create iterator with invalid order: ";
	order->setAxisIndex(0, 1);
	order->setAxisIndex(1, 1);
	assert(compact1->getBegin(order) == nullptr);
	std::cout << "failed" << std::endl;
	order->setAxisIndex(0, 0);

	steps->setAxisIndex(0, 3);
	steps->setAxisIndex(1, 2);
	bound1->setCoord(0, -3);
	bound1->setCoord(1, 5);
	bound2->setCoord(0, 2);
	bound2->setCoord(1, 10);

	std::cout << "Creating Compact B:" << std::endl;
	ICompact* compact2 = ICompact::createCompact(bound1, bound2, steps);
	PrintUtils::printCompact(compact2, order);
	assert(compact2->getDim() == dim);

	steps->setAxisIndex(1, 3);

	std::cout << "Intersection of Compact A and Compact B:" << std::endl;
	auto intersection = ICompact::createIntersection(compact1, compact2, steps, tol);
	PrintUtils::printCompact(intersection, order);

	IVector* minBound;
	intersection->getLeftBoundary(minBound);
	IVector* correctMinBound = IVector::createVector(dim, std::vector<double>{-3, 5}.data());
	assert(IVector::equals(correctMinBound, minBound, IVector::NORM::SECOND, tol));

	delete correctMinBound;
	delete minBound;

	IVector* maxBound;
	intersection->getRightBoundary(maxBound);
	IVector* correctMaxBound = IVector::createVector(dim, std::vector<double>{1, 5}.data());
	assert(IVector::equals(correctMaxBound, maxBound, IVector::NORM::SECOND, tol));

	delete correctMaxBound;
	delete maxBound;

	delete intersection;

	std::cout << "Span of Compact A and Compact B:" << std::endl;
	auto span = ICompact::createCompactSpan(compact1, compact2, steps);
	PrintUtils::printCompact(span, order);

	delete span;

	std::cout << "Is point inside Compact A: ";
	auto vec = IVector::createVector(dim, std::vector<double>{1, 1}.data());
	PrintUtils::printVector(vec);
	bool isInside = compact1->isInside(vec);
	std::cout << isInside << std::endl;
	assert(isInside == false);

	std::cout << "Is point inside Compact A: ";
	vec->setCoord(1, 3);
	PrintUtils::printVector(vec);
	isInside = compact1->isInside(vec);
	std::cout << isInside << std::endl;
	assert(isInside == true);

	delete vec;

	delete compact1;
	delete compact2;
	delete order;
	delete steps;
	delete bound1;
	delete bound2;

	std::cout << "Compact test successfully finished\n\n";
}