#include <fstream>
#include <iostream>

#include "Logger.h"

bool Logger::m_isMsgMapInit = false;
const char* Logger::m_msgMap[static_cast<int>(RC::AMOUNT)];

ILogger* Logger::createLogger() { return new (std::nothrow) Logger(); }

ILogger* Logger::createLogger(const char* const& filename, bool overwrite) {
	auto mode = overwrite ? std::ios_base::out : std::ios_base::app;
	std::ofstream fstream(filename, mode);

	if (fstream.fail()) {
		return nullptr;
	}
	return new (std::nothrow) Logger(std::move(fstream));
}

Logger::Logger() : m_outputStream(std::cout) {}

Logger::Logger(std::ofstream&& outputFile) :
	m_outputStream(m_outputFile), m_outputFile(std::move(outputFile)) {}

ILogger* ILogger::createLogger() { return Logger::createLogger(); }

ILogger* ILogger::createLogger(const char* const& filename, bool overwrite) {
	return Logger::createLogger(filename, overwrite);
}

RC Logger::log(RC code, ILogger::Level level) {

	m_outputStream << warningLvlToStr(level) << ": " //
				   << getCodeMsg(code)				 //
				   << std::endl;					 //

	return RC::SUCCESS;
}

RC Logger::log(RC code,
			   ILogger::Level level,
			   const char* const& srcfile,
			   const char* const& function,
			   int line) {

	m_outputStream << warningLvlToStr(level) << ": " //
				   << getCodeMsg(code)				 //
				   << " in " << srcfile				 //
				   << " " << function				 //
				   << " line: " << line				 //
				   << std::endl;

	return RC::SUCCESS;
}

const char* Logger::getCodeMsg(RC code) {
	initMsgMap();

	if (code == RC::AMOUNT) {
		return "";
	}
	return m_msgMap[(int)code];
}

const char* Logger::warningLvlToStr(ILogger::Level level) {
	switch (level) {
	case Level::WARNING:
		return "Warning";

	case Level::SEVERE:
		return "Severe";

	case Level::INFO:
		return "Info";

	default:
		return "";
	}
}

void Logger::setMapMsg(RC code, const char* msg) {
	int intCode = static_cast<int>(code);
	m_msgMap[intCode] = msg;
}

void Logger::initMsgMap() {
	if (m_isMsgMapInit) {
		return;
	}

	setMapMsg(RC::UNKNOWN, "Unknown error");
	setMapMsg(RC::SUCCESS, "Successful execution");
	setMapMsg(RC::INVALID_ARGUMENT, "Invalid argument");
	setMapMsg(RC::MISMATCHING_DIMENSIONS, "Dimensions mismatch");
	setMapMsg(RC::INDEX_OUT_OF_BOUND, "Index out of bounds");
	setMapMsg(RC::INFINITY_OVERFLOW, "Infinity overflow");
	setMapMsg(RC::NOT_NUMBER, "NaN");
	setMapMsg(RC::ALLOCATION_ERROR, "Memory allocation error");
	setMapMsg(RC::NULLPTR_ERROR, "Null pointer exception");
	setMapMsg(RC::FILE_NOT_FOUND, "File not found");
	setMapMsg(RC::VECTOR_NOT_FOUND, "Set doesn't contains this vector instance");
	setMapMsg(RC::IO_ERROR, "Input/output error");
	setMapMsg(RC::MEMORY_INTERSECTION, "Found intersecting memory while copying instance");
	setMapMsg(RC::SOURCE_SET_DESTROYED, "Source set was destroyed");
	setMapMsg(RC::SOURCE_SET_EMPTY, "Source set is empty");
	setMapMsg(RC::VECTOR_ALREADY_EXIST, "Vector already exists");

	m_isMsgMapInit = true;
}

ILogger::~ILogger() = default;
