template<class T>
ILogger* LogContainer<T>::m_logger = nullptr;

template<class T>
ILogger* LogContainer<T>::getInstance() {
	return m_logger;
}

template<class T>
RC LogContainer<T>::setInstance(ILogger* logger) {
	m_logger = logger;
	return RC::SUCCESS;
}
