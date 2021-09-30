#include "LogUtils.h"

RC LogUtils::safeLog(ILogger* logger,
					 RC code,
					 ILogger::Level level,
					 const char* file,
					 const char* function,
					 int line) {
	if (logger) {
		return logger->log(code, level, file, function, line);
	}
	return RC::NULLPTR_ERROR;
}
