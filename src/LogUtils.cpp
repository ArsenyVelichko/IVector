#include "LogUtils.h"

RC LogUtils::safeLog(ILogger* logger,
					 RC code,
					 ILogger::Level level,
					 const char* srcfile,
					 const char* function,
					 int line) {
	if (logger) {
		return logger->log(code, level, srcfile, function, line);
	}
	return RC::NULLPTR_ERROR;
}
