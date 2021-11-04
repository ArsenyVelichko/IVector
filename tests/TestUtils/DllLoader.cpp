#include <stdexcept>

#include "DllLoader.h"

bool DllLoader::loadLibrary(const char* libName) {
	if (m_isLoaded) {
		CLOSE_LIBRARY(m_dllHandler);
		m_isLoaded = false;
	}

	m_dllHandler = LOAD_LIBRARY(libName);
	if (!m_dllHandler) {
		return false;
	}

	auto getBroker = reinterpret_cast<GetBrokerFunc>(GET_BROKER(m_dllHandler));
	if (!getBroker) {
		CLOSE_LIBRARY(m_dllHandler);
		return false;
	}

	m_broker = reinterpret_cast<IBroker*>(getBroker());
	m_isLoaded = true;
	return true;
}

DllLoader::~DllLoader() {
	if (m_isLoaded) {
		CLOSE_LIBRARY(m_dllHandler);
	}
}

void* DllLoader::loadImplementation(IBroker::INTERFACE_IMPL impl) {
	if (!m_isLoaded) { return nullptr; }
	return m_broker->getInterfaceImpl(impl);;
}