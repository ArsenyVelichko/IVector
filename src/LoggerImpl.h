#ifndef IVECTOR_LOGGER_H
#define IVECTOR_LOGGER_H

#include <fstream>

#include "ILogger.h"

namespace {
	class LoggerImpl : public ILogger {
	public:
		static ILogger* createLogger();
		static ILogger* createLogger(const char* const& filename, bool overwrite = true);

		RC log(RC code, Level level) override;
		RC log(RC code,
			   Level level,
			   const char* const& srcfile,
			   const char* const& function,
			   int line) override;

	private:
		LoggerImpl(std::ostream& outputStream);
		LoggerImpl(std::ofstream&& outputFile);

		static const char* getCodeMsg(RC code);
		static const char* warningLvlToStr(Level level);

		std::ofstream m_outputFile;
		std::ostream& m_outputStream;

		static bool m_isMsgMapInit;
		static void setMapMsg(RC code, const char* msg);
		static void initMsgMap();
		static const char* m_msgMap[];
	};
}

#endif //IVECTOR_LOGGER_H
