#pragma once

#include <memory>

#include <IBroker.h>

namespace {
	class Broker : public IBroker {
	public:
		static Broker* getInstance();

		bool canCastTo(INTERFACE_IMPL impl) const override;
		void* getInterfaceImpl(INTERFACE_IMPL impl) const override;
		void release() override;

	private:
		Broker() = default;

		static std::unique_ptr<Broker> m_instance;
	};
};
