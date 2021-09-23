#pragma once

#include <ILogger.h>

template<class T>
class LogProducer {
public:
	static RC setLogger(ILogger* logger);
	static ILogger* getLogger();

	static RC safeLog(RC code, ILogger::Level level, const char* file, const char* function, int line);

private:
	static ILogger* m_logger;
};

#include "LogProducer.tpp"

#define log_info(code) safeLog((code), ILogger::Level::INFO, __FILE__, __FUNCTION__, __LINE__)

#define log_warning(code) safeLog((code), ILogger::Level::WARNING, __FILE__, __FUNCTION__, __LINE__)

#define log_severe(code) safeLog((code), ILogger::Level::SEVERE, __FILE__, __FUNCTION__, __LINE__)
