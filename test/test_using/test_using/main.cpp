
#include <iostream>

#include "log4cxx/logger_wrapper.h"

int main()
{
	{
		if (!logger::Logger::GetInstancePtr()->Init("../config/log4j.properties"))
		{
			std::cout << "logger init failed" << std::endl;
			return 0;
		}

		LOG_INFO("logger test using");
		NAMED_DEBUG_INFO("test","logger test using in test");
		NAMED_DEBUG_INFO("testex", "debug logger test using in testex");
		NAMED_LOG_INFO("testex", "logger test using in testex");
	}

	system("pause");
	return 0;
}