#pragma once

#include <cmath>

#include <ILogger.h>

namespace Tests {
	inline bool fuzzyCompare(double x, double y, double tol) { return fabs(x - y) < tol; }

	void vectorTest(ILogger* logger);
	void setTest(ILogger* logger);
	void compactTest(ILogger* logger);
	void problemTest(ILogger* logger);
	void solverTest(ILogger* logger);
}