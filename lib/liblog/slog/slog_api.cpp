#include "slog_api.h"
#include "log4cxx/logger.h"
#include "log4cxx/basicconfigurator.h"
#include "log4cxx/propertyconfigurator.h"
#include "log4cxx/helpers/exception.h"
#include <stdarg.h>
using namespace log4cxx;

#define MAX_LOG_LENGTH 1024 * 10

class CSLogObject
{
public:
	CSLogObject(const char *module_name, int delay) {}

	virtual ~CSLogObject() {}

	virtual void Trace(const char *loginfo) {}

	virtual void Debug(const char *loginfo) {}

	virtual void Info(const char *loginfo) {}

	virtual void Warn(const char *loginfo) {}

	virtual void Error(const char *loginfo) {}

	virtual void Fatal(const char *loginfo) {}
};

class CLog4CXX : public CSLogObject
{
public:
	CLog4CXX(const char *module_name, int delay);
	virtual ~CLog4CXX();

	void Trace(const char *loginfo);

	void Debug(const char *loginfo);

	void Info(const char *loginfo);

	void Warn(const char *loginfo);

	void Error(const char *loginfo);

	void Fatal(const char *loginfo);

private:
	LoggerPtr m_logger;
};

CLog4CXX::CLog4CXX(const char *module_name, int delay) : CSLogObject(module_name, delay)
{
	PropertyConfigurator::configureAndWatch("log4cxx.properties", delay);

	m_logger = Logger::getLogger(module_name);
}

CLog4CXX::~CLog4CXX()
{}

void CLog4CXX::Trace(const char *loginfo)
{
	m_logger->trace(loginfo);
	// LOG4CXX_TRACE(m_logger, loginfo);
}

void CLog4CXX::Debug(const char *loginfo)
{
	m_logger->debug(loginfo);
}

void CLog4CXX::Info(const char *loginfo)
{
	m_logger->info(loginfo);
}

void CLog4CXX::Warn(const char *loginfo)
{
	m_logger->warn(loginfo);
}

void CLog4CXX::Error(const char *loginfo)
{
	m_logger->error(loginfo);
}

void CLog4CXX::Fatal(const char *loginfo)
{
	m_logger->fatal(loginfo);
}

struct CSLog *CSLog_create(const char *module_name, int delay)
{
	struct CSLog *log = new struct CSLog;

	log->m_log = new CLog4CXX(module_name, delay);
	return log;
}

void CSLog_destroy(struct CSLog *log)
{
	delete log->m_log;
	delete log;
}

void Trace(struct CSLog *log, const char *format, ...)
{
	va_list args;

	va_start(args, format);
	char szBuffer[MAX_LOG_LENGTH];
	vsnprintf(szBuffer, sizeof(szBuffer), format, args);
	va_end(args);
	log->m_log->Trace(szBuffer);
}

void Debug(struct CSLog *log, const char *format, ...)
{
	va_list args;

	va_start(args, format);
	char szBuffer[MAX_LOG_LENGTH];
	vsnprintf(szBuffer, sizeof(szBuffer), format, args);
	va_end(args);
	log->m_log->Debug(szBuffer);
}

void Info(struct CSLog *log, const char *format, ...)
{
	va_list args;

	va_start(args, format);
	char szBuffer[MAX_LOG_LENGTH];
	vsnprintf(szBuffer, sizeof(szBuffer), format, args);
	va_end(args);
	log->m_log->Info(szBuffer);
}

void Warn(struct CSLog *log, const char *format, ...)
{
	va_list args;

	va_start(args, format);
	char szBuffer[MAX_LOG_LENGTH];
	vsnprintf(szBuffer, sizeof(szBuffer), format, args);
	va_end(args);
	log->m_log->Warn(szBuffer);
}

void Error(struct CSLog *log, const char *format, ...)
{
	va_list args;

	va_start(args, format);
	char szBuffer[MAX_LOG_LENGTH];
	vsnprintf(szBuffer, sizeof(szBuffer), format, args);
	va_end(args);
	log->m_log->Error(szBuffer);
}

void Fatal(struct CSLog *log, const char *format, ...)
{
	va_list args;

	va_start(args, format);
	char szBuffer[MAX_LOG_LENGTH];
	vsnprintf(szBuffer, sizeof(szBuffer), format, args);
	va_end(args);
	log->m_log->Fatal(szBuffer);
}

