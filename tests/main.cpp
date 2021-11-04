#include <iostream>

#include <ILogger.h>

#include "LibTests/Tests.h"

int main() {
	ILogger* logger = ILogger::createLogger("Log.txt");

	std::cout << std::boolalpha;

	std::cout << "### Vector test ###\n\n";
	Tests::vectorTest(logger);

	std::cout << "### Set test ###\n\n";
	Tests::setTest(logger);

	std::cout << "### Compact test ###\n\n";
	Tests::compactTest(logger);

	std::cout << "### Problem test ###\n\n";
	Tests::problemTest(logger);

	std::cout << "### Solver test ###\n\n";
	Tests::solverTest(logger);

	return 0;
}
