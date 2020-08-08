
#include <iostream>

#include "log4cxx/logger_wrapper.h"

int main()
{
	{
		if (!logger::Logger::GetInstancePtr()->Init("test", "../config/log4j.properties"))
		{
			std::cout << "logger init failed" << std::endl;
			return 0;
		}

		LOG_INFO("logger test using");
		LOG_INFO("logger test using again");
	}

	system("pause");
	return 0;
}