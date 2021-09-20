#pragma once

#include <ILogger.h>

namespace LogUtils {
	RC safeLog(ILogger* logger,
			   RC code,
			   ILogger::Level level,
			   const char* srcfile,
			   const char* function,
			   int line);
}

#define log_info(logger, code)                                                                     \
	LogUtils::safeLog((logger), (code), ILogger::Level::INFO, __FILE__, __FUNCTION__, __LINE__)

#define log_warning(logger, code)                                                                  \
	LogUtils::safeLog((logger), (code), ILogger::Level::WARNING, __FILE__, __FUNCTION__, __LINE__)

#define log_severe(logger, code)                                                                   \
	LogUtils::safeLog((logger), (code), ILogger::Level::SEVERE, __FILE__, __FUNCTION__, __LINE__)
