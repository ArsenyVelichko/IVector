#include <string>

template<class T>
ILogger* LogProducer<T>::m_logger = nullptr;

template<class T>
ILogger* LogProducer<T>::getLogger() {
	return m_logger;
}

template<class T>
RC LogProducer<T>::setLogger(ILogger* logger) {
	m_logger = logger;
	return RC::SUCCESS;
}

template<class T>
RC LogProducer<T>::safeLog(
	RC code, ILogger::Level level, const char* file, const char* function, int line) {
	if (m_logger) {
		auto memberFunction = std::string(typeid(T).name()) + "::" + function;
		return getLogger()->log(code, level, file, memberFunction.c_str(), line);
	}
	return RC::NULLPTR_ERROR;
}
