#include <iostream>
#include <functional>

#include "PrintUtils.h"

void PrintUtils::printVector(IVector* vector) {
	std::function<void(double)> print = [](double x) { std::cout << x << ' '; };
	std::cout << "( ";
	vector->foreach (print);
	std::cout << ")" << std::endl;
}

void PrintUtils::printMultiIndex(IMultiIndex* index) {
	auto indexData = index->getData();
	std::cout << "( ";
	for (size_t i = 0; i < index->getDim(); i++) {
		std::cout << indexData[i] << ' ';
	}
	std::cout << ")" << std::endl;
}

void PrintUtils::printSet(ISet* set) {
	if (set->getSize() == 0) {
		std::cout << "[ empty ]" << std::endl;
		return;
	}

	IVector* vec;

	std::cout << "[ " << std::endl;

	auto it = set->getBegin();
	for (; it->isValid(); it->next()) {
		it->getVectorCopy(vec);
		printVector(vec);
		delete vec;
	}
	delete it;

	std::cout << "]" << std::endl;
}

void PrintUtils::printCompact(ICompact* compact, IMultiIndex* byPass) {
	ICompact::IIterator* it = compact->getBegin(byPass);
	IVector* vec;

	std::cout << "{ " << std::endl;
	for (; it->isValid(); it->next()) {
		it->getVectorCopy(vec);
		printVector(vec);
		delete vec;
	}
	std::cout << "}" << std::endl;

	delete it;
}