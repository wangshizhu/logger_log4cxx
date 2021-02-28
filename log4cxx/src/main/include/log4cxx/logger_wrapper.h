#ifndef LOGGER_WRAPPER_H_

#define LOGGER_WRAPPER_H_

#include <string>
#include <list>
#include <thread>
#include <atomic>
#include "stdio.h"
#include <condition_variable>

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
#include "concurrentqueue.h"

namespace logger
{

#define MIN_THREAD_NUM 1
#define MAX_THREAD_NUM 4

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

#define LOGGER_LOG4CXX_TRACE(logger, s)	\
	{	\
		try {	\
			LOG4CXX_TRACE(logger, s);	\
		}	\
		catch (const log4cxx::helpers::IOException& ioex) {	\
			printf("IOException: %s\nTRACE=%s\n", ioex.what(), s.c_str());	\
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

	using FunType = std::function<void(void)>;

	class Logger : public Singleton<Logger>
	{
	public:

		Logger():stop_(false), log_msg_lock_free_(1000)
		{
			log_msg_.clear();
			thread_.clear();
		}

		~Logger() 
		{
			fclose(tmp_file_);
		}

	public:

		void Start(int thread_num)
		{
			if (thread_num < MIN_THREAD_NUM)
			{
				thread_num = MIN_THREAD_NUM;
			}
			if (thread_num > MAX_THREAD_NUM)
			{
				thread_num = MAX_THREAD_NUM;
			}

			for (int i = 0; i < thread_num; i++)
			{
				thread_.push_back(std::thread(&Logger::RunNonblockQueue,this));
			}
		}

		void RunNonblockQueue()
		{
			while (true)
			{
				
				if (stop_)
				{
					FunType func;
					bool ok = log_msg_lock_free_.try_dequeue(func);
					if (!ok)
					{
						return;
					}

					func();
				}
				else
				{
					FunType func;
					bool ok = log_msg_lock_free_.try_dequeue(func);
					if (!ok)
					{
						std::unique_lock<std::mutex> lock(mtx_);
						cv_.wait(lock);
						continue;
					}

					func();
				}
			}
		}

		void Run()
		{
			while (true)
			{
				std::unique_lock<std::mutex> lock(mtx_);
				if (stop_)
				{
					if (log_msg_.empty())
					{
						return;
					}
					else
					{
						FunType& func = *log_msg_.begin();

						func();

						log_msg_.pop_front();
					}
				}
				else 
				{
					while (log_msg_.empty())
					{
						if (stop_)
						{
							return;
						}
						cv_.wait(lock);
					}

					FunType& func = *log_msg_.begin();

					func();

					log_msg_.pop_front();
				}
			}
		}

		void Stop()
		{
			if (stop_)
			{
				return;
			}
			stop_ = true;

			cv_.notify_all();
		}

		void Join()
		{
			for (auto&& one : thread_)
			{
				one.join();
			}
		}

		bool Init(const char* conf = nullptr)
		{
			/*tmp_file_ = fopen("../log/test_log.log","w");
			if (tmp_file_ == nullptr)
			{
				return false;
			}*/

			try
			{
				log4cxx::Logger::getRootLogger();

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

	private:

		static log4cxx::LoggerPtr GetLoggerByName(const std::string& name)
		{
			if (name.size() == 0)
			{
				return log4cxx::Logger::getRootLogger();
			}

			return log4cxx::Logger::getLogger(name);
		}

		template<class... Args>
		std::string Formater(const char* func_name,
			const char* file_name, int line_num,
			const char* msg, Args&&... args) const
		{
			auto&& func_info = fmt::format("file:{0},function:{1},line:{2},thread:{3},msg:{4}", file_name, func_name, line_num, std::this_thread::get_id(), msg);

			return fmt::format(func_info, std::forward<decltype(args)>(args)...);
		}
	
	public:

		template<class... Args>
		void NonBlockConcurrentLog(int level, std::string name, const char* func_name,
			const char* file_name, int line_num,
			const char* msg, Args&&... args)
		{
			auto logger = Logger::GetLoggerByName(name);
			if (logger->getLevel()->toInt() > level)
			{
				return;
			}

			auto&& formate_msg = Formater(func_name, file_name, line_num, msg, std::forward<decltype(args)>(args)...);

			bool ok = log_msg_lock_free_.enqueue([lv = level, logger_name = std::move(name), data = std::move(formate_msg)]
			{
				switch (lv)
				{
				case log4cxx::Level::DEBUG_INT:
					LOGGER_LOG4CXX_DEBUG(Logger::GetLoggerByName(logger_name), data);
					break;
				case log4cxx::Level::INFO_INT:
					LOGGER_LOG4CXX_INFO(Logger::GetLoggerByName(logger_name), data);
					break;
				case log4cxx::Level::ERROR_INT:
					LOGGER_LOG4CXX_ERROR(Logger::GetLoggerByName(logger_name), data);
					break;
				case log4cxx::Level::WARN_INT:
					LOGGER_LOG4CXX_WARN(Logger::GetLoggerByName(logger_name), data);
					break;
				case log4cxx::Level::FATAL_INT:
					LOGGER_LOG4CXX_FATAL(Logger::GetLoggerByName(logger_name), data);
					break;
				case log4cxx::Level::TRACE_INT:
					LOGGER_LOG4CXX_TRACE(Logger::GetLoggerByName(logger_name), data);
					break;
				}

			});

			if (!ok)
			{
				std::cout << "enqueue failed" << std::endl;
			}
			else
			{
				cv_.notify_all();
			}
		}

		template<class... Args>
		void ProductThreadLog(int level, 
			std::string name, 
			const char* func_name,
			const char* file_name, 
			int line_num,
			const char* msg, 
			Args&&... args)
		{
			auto logger = Logger::GetLoggerByName(name);
			if (logger->getLevel()->toInt() > level)
			{
				return;
			}

			auto&& formate_msg = Formater(func_name, file_name, line_num, msg, std::forward<decltype(args)>(args)...);

			switch (level)
			{
			case log4cxx::Level::DEBUG_INT:
				LOGGER_LOG4CXX_DEBUG(Logger::GetLoggerByName(name), formate_msg);
				break;
			case log4cxx::Level::INFO_INT:
				LOGGER_LOG4CXX_INFO(Logger::GetLoggerByName(name), formate_msg);
				break;
			case log4cxx::Level::ERROR_INT:
				LOGGER_LOG4CXX_ERROR(Logger::GetLoggerByName(name), formate_msg);
				break;
			case log4cxx::Level::WARN_INT:
				LOGGER_LOG4CXX_WARN(Logger::GetLoggerByName(name), formate_msg);
				break;
			case log4cxx::Level::FATAL_INT:
				LOGGER_LOG4CXX_FATAL(Logger::GetLoggerByName(name), formate_msg);
				break;
			case log4cxx::Level::TRACE_INT:
				LOGGER_LOG4CXX_TRACE(Logger::GetLoggerByName(name), formate_msg);
				break;
			}
		}

		template<class... Args>
		void Log(int level,std::string name, const char* func_name,
			const char* file_name, int line_num,
			const char* msg, Args&&... args)
		{
			auto logger = Logger::GetLoggerByName(name);
			if (logger->getLevel()->toInt() > level)
			{
				return;
			}

			auto&& formate_msg = Formater(func_name, file_name, line_num, msg, std::forward<decltype(args)>(args)...);

			{
				std::unique_lock<std::mutex> lock(mtx_);

				log_msg_.emplace_back([lv = level, logger_name = std::move(name), data = std::move(formate_msg)]
				{
					switch (lv)
					{
					case log4cxx::Level::DEBUG_INT:
						LOGGER_LOG4CXX_DEBUG(Logger::GetLoggerByName(logger_name), data);
						break;
					case log4cxx::Level::INFO_INT:
						LOGGER_LOG4CXX_INFO(Logger::GetLoggerByName(logger_name), data);
						break;
					case log4cxx::Level::ERROR_INT:
						LOGGER_LOG4CXX_ERROR(Logger::GetLoggerByName(logger_name), data);
						break;
					case log4cxx::Level::WARN_INT:
						LOGGER_LOG4CXX_WARN(Logger::GetLoggerByName(logger_name), data);
						break;
					case log4cxx::Level::FATAL_INT:
						LOGGER_LOG4CXX_FATAL(Logger::GetLoggerByName(logger_name), data);
						break;
					case log4cxx::Level::TRACE_INT:
						LOGGER_LOG4CXX_TRACE(Logger::GetLoggerByName(logger_name), data);
						break;
					}

				});
			}

			cv_.notify_one();
		}

	private:

		std::list<FunType> log_msg_;
		moodycamel::ConcurrentQueue<FunType> log_msg_lock_free_;
		std::vector<std::thread> thread_;
		std::condition_variable cv_;
		std::mutex mtx_;
		std::atomic<bool> stop_;
		FILE* tmp_file_;

	};
	
}

#define DEBUG_INFO(m,...) logger::Logger::GetInstancePtr()->Log(log4cxx::Level::DEBUG_INT,"",__FUNCTION__,__FILE__,__LINE__,(m),##__VA_ARGS__)
#define ERROR_INFO(m,...) logger::Logger::GetInstancePtr()->Log(log4cxx::Level::ERROR_INT,"",__FUNCTION__,__FILE__,__LINE__,(m),##__VA_ARGS__)
#define LOG_INFO(m,...) logger::Logger::GetInstancePtr()->Log(log4cxx::Level::INFO_INT,"",__FUNCTION__,__FILE__,__LINE__,(m),##__VA_ARGS__)
#define FATAL_INFO(m,...) logger::Logger::GetInstancePtr()->Log(log4cxx::Level::FATAL_INT,"",__FUNCTION__,__FILE__,__LINE__,(m),##__VA_ARGS__)
#define WARN_INFO(m,...) logger::Logger::GetInstancePtr()->Log(log4cxx::Level::WARN_INT,"",__FUNCTION__,__FILE__,__LINE__,(m),##__VA_ARGS__)

#define NAMED_DEBUG_INFO(name,m,...) logger::Logger::GetInstancePtr()->Log(log4cxx::Level::DEBUG_INT,name,__FUNCTION__,__FILE__,__LINE__,(m),##__VA_ARGS__)
#define NAMED_ERROR_INFO(name,m,...) logger::Logger::GetInstancePtr()->Log(log4cxx::Level::ERROR_INT,name,__FUNCTION__,__FILE__,__LINE__,(m),##__VA_ARGS__)
#define NAMED_LOG_INFO(name,m,...) logger::Logger::GetInstancePtr()->NonBlockConcurrentLog(log4cxx::Level::INFO_INT,name,__FUNCTION__,__FILE__,__LINE__,(m),##__VA_ARGS__)
#define NAMED_FATAL_INFO(name,m,...) logger::Logger::GetInstancePtr()->Log(log4cxx::Level::FATAL_INT,name,__FUNCTION__,__FILE__,__LINE__,(m),##__VA_ARGS__)
#define NAMED_WARN_INFO(name,m,...) logger::Logger::GetInstancePtr()->Log(log4cxx::Level::WARN_INT,name,__FUNCTION__,__FILE__,__LINE__,(m),##__VA_ARGS__)

#endif