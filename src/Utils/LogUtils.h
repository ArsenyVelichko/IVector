#pragma once

#include <ILogger.h>

namespace LogUtils {

	RC safeLog(ILogger* logger,
			   RC code,
			   ILogger::Level level,
			   const char* file,
			   const char* function,
			   int line);

	template<class T>
	class LogContainer {
	public:
		static RC setInstance(ILogger* logger);
		static ILogger* getInstance();

	private:
		static ILogger* m_logger;
	};

#include "LogUtils.tpp"

} // namespace LogUtils

#define safe_log_macro(logger, code, level)                                                                \
	LogUtils::safeLog((logger), (code), (level), __FILE__, __FUNCTION__, __LINE__)

#define log_info_in(logger, code) safe_log_macro(logger, code, ILogger::Level::INFO)
#define log_warning_in(logger, code) safe_log_macro(logger, code, ILogger::Level::WARNING)
#define log_severe_in(logger, code) safe_log_macro(logger, code, ILogger::Level::SEVERE)

#define log_info(code) log_info_in(getLogger(), code)
#define log_warning(code) log_warning_in(getLogger(), code)
#define log_severe(code) log_severe_in(getLogger(), code)
