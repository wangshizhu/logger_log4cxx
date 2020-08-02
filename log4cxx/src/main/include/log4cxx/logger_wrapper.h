#ifndef LOGGER_WRAPPER_H_

#define LOGGER_WRAPPER_H_

#include <string>

#include "log4cxx/logger.h"
#include "log4cxx/logmanager.h"
#include "log4cxx/net/socketappender.h"
#include "log4cxx/fileappender.h"
#include "log4cxx/helpers/inetaddress.h"
#include "log4cxx/propertyconfigurator.h"
#include "log4cxx/patternlayout.h"
#include "log4cxx/logstring.h"
#include "log4cxx/basicconfigurator.h"
#include "log4cxx/singleton.h"
#include "log4cxx/format.h"

namespace logger
{
	log4cxx::LoggerPtr g_logger(log4cxx::Logger::getLogger(""));

#define LOGGER_LOG4CXX_ERROR(logger, s)	\
	{	\
		try {	\
			LOG4CXX_ERROR(logger, s);	\
		}	\
		catch (const log4cxx::helpers::IOException& ioex) {	\
			printf("IOException: %s\nERROR=%s\n", ioex.what(), s.c_str());	\
		}	\
    }

#define LOGGER_LOG4CXX_WARN(logger, s)	\
	{	\
		try {	\
			LOG4CXX_WARN(logger, s);	\
		}	\
		catch (const log4cxx::helpers::IOException& ioex) {	\
			printf("IOException: %s\nWARN=%s\n", ioex.what(), s.c_str());	\
		}	\
    }

#define LOGGER_LOG4CXX_INFO(logger, s)	\
	{	\
		try {	\
			LOG4CXX_INFO(logger, s);	\
		}	\
		catch (const log4cxx::helpers::IOException& ioex) {	\
			printf("IOException: %s\nINFO=%s\n", ioex.what(), s.c_str());	\
		}	\
    }

#define LOGGER_LOG4CXX_DEBUG(logger, s)	\
	{	\
		try {	\
			LOG4CXX_DEBUG(logger, s);	\
		}	\
		catch (const log4cxx::helpers::IOException& ioex) {	\
			printf("IOException: %s\nDEBUG=%s\n", ioex.what(), s.c_str());	\
		}	\
    }

#define LOGGER_LOG4CXX_FATAL(logger, s)	\
	{	\
		try {	\
			LOG4CXX_FATAL(logger, s);	\
		}	\
		catch (const log4cxx::helpers::IOException& ioex) {	\
			printf("IOException: %s\nFATAL=%s\n", ioex.what(), s.c_str());	\
		}	\
    }

#define LOGGER_LOG4CXX_LOG(logger, level, s)	\
	{	\
		try {	\
			LOG4CXX_LOG(logger, level, s);	\
		}	\
		catch (const log4cxx::helpers::IOException& ioex) {	\
			printf("IOException: %s\nLOG=%s\n", ioex.what(), s.c_str());	\
		}	\
    }

	class Logger : public Singleton<Logger>
	{
	public:
		Logger() 
		{
		}

		~Logger() 
		{
		}

		bool Init(const char* log_name = nullptr,const char* conf = nullptr)
		{
			try
			{
				if (nullptr == log_name)
				{
					g_logger = log4cxx::Logger::getRootLogger();
				}
				else
				{
					g_logger = log4cxx::Logger::getLogger(log_name);
				}
				
				LOG4CXX_INFO(g_logger, "\n");

				if (conf == nullptr)
				{
					log4cxx::BasicConfigurator::configure();
				}
				else
				{
					log4cxx::PropertyConfigurator::configure(conf);
				}

				return true;
			}
			catch (std::exception& e)
			{
				printf("Logger Init failed: %s\n", e.what());
				return false;
			}
			catch (...)
			{
				printf("Logger Init failed");
				return false;
			}
		}
	
	public:

		template<class... Args>
		void DebugInfo(const char* func_name,
			const char* file_name, int line_num,
			const char* msg, Args&&... args) const
		{
			LOGGER_LOG4CXX_DEBUG(g_logger,Formater(func_name,file_name,line_num,msg, std::forward<decltype(args)>(args)...));
		}

		template<class... Args>
		void ErrorInfo(const char* func_name,
			const char* file_name, int line_num,
			const char* msg, Args&&... args) const
		{
			LOGGER_LOG4CXX_ERROR(g_logger, Formater(func_name, file_name, line_num, msg, std::forward<decltype(args)>(args)...));
		}

		template<class... Args>
		void LogInfo(const char* func_name,
			const char* file_name, int line_num,
			const char* msg, Args&&... args) const
		{
			LOGGER_LOG4CXX_INFO(g_logger, Formater(func_name, file_name, line_num, msg, std::forward<decltype(args)>(args)...));
		}

		template<class... Args>
		void FatalInfo(const char* func_name,
			const char* file_name, int line_num,
			const char* msg, Args&&... args) const
		{
			LOGGER_LOG4CXX_FATAL(g_logger, Formater(func_name, file_name, line_num, msg, std::forward<decltype(args)>(args)...));
		}

		template<class... Args>
		void WarnInfo(const char* func_name,
			const char* file_name, int line_num,
			const char* msg, Args&&... args) const
		{
			LOGGER_LOG4CXX_WARN(g_logger, Formater(func_name, file_name, line_num, msg, std::forward<decltype(args)>(args)...));
		}

		template<class... Args>
		std::string Formater(const char* func_name,
			const char* file_name, int line_num,
			const char* msg, Args&&... args) const
		{
			auto&& func_info = fmt::format("file:{0},function:{1},line:{2},msg:{3}", file_name, func_name, line_num, msg);

			return fmt::format(func_info, std::forward<decltype(args)>(args)...);
		}

	};
	
}

#define DEBUG_INFO(m,...) logger::Logger::GetInstancePtr()->DebugInfo(__FUNCTION__,__FILE__,__LINE__,(m),##__VA_ARGS__)
#define ERROR_INFO(m,...) logger::Logger::GetInstancePtr()->ErrorInfo(__FUNCTION__,__FILE__,__LINE__,(m),##__VA_ARGS__)
#define LOG_INFO(m,...) logger::Logger::GetInstancePtr()->LogInfo(__FUNCTION__,__FILE__,__LINE__,(m),##__VA_ARGS__)
#define FATAL_INFO(m,...) logger::Logger::GetInstancePtr()->FatalInfo(__FUNCTION__,__FILE__,__LINE__,(m),##__VA_ARGS__)
#define WARN_INFO(m,...) logger::Logger::GetInstancePtr()->WarnInfo(__FUNCTION__,__FILE__,__LINE__,(m),##__VA_ARGS__)

#endif