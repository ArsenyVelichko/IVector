#pragma once

#include <IBroker.h>

#include "LibUtils.h"

class DllLoader {
public:
	~DllLoader();

	bool loadLibrary(const char* libName);
	void* loadImplementation(IBroker::INTERFACE_IMPL impl);

private:
	IBroker* m_broker;
	bool m_isLoaded = false;
	LIB m_dllHandler;
};
