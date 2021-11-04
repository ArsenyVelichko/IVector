#include "Broker.h"
#include "Solver.h"

std::unique_ptr<Broker> Broker::m_instance;

Broker* Broker::getInstance() {
	if (!m_instance) {
		m_instance.reset(new (std::nothrow) Broker());
	}
	return m_instance.get();
}

bool Broker::canCastTo(INTERFACE_IMPL impl) const {
	return impl == INTERFACE_IMPL::ISOLVER;
}

void* Broker::getInterfaceImpl(INTERFACE_IMPL impl) const {
	if (!canCastTo(impl)) {
		return nullptr;
	}
	return ISolver::createSolver();
}

void Broker::release() {
	m_instance.reset(nullptr);
}

IBroker::~IBroker() = default;

void* getBroker() {
	return Broker::getInstance();
}