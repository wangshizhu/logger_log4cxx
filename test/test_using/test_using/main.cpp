
#include <iostream>
#include <thread>

#include "log4cxx/logger_wrapper.h"

#define PRODUCT_THREAD_NUM 8
#define CONSUMER_THREAD_NUM 4

void TestFun()
{
	for (size_t i = 0; i < 1000; i++)
	{
		NAMED_LOG_INFO("testex", "logger test using in testex");

		//std::this_thread::sleep_for(std::chrono::milliseconds(2));
	}
}

int main()
{
	{
		logger::Logger log;
		if (!logger::Logger::GetInstancePtr()->Init("../config/log4j.properties"))
		{
			std::cout << "logger init failed" << std::endl;
			return 0;
		}

		std::thread t[PRODUCT_THREAD_NUM];
		for (int i = 0; i < PRODUCT_THREAD_NUM; i++)
		{
			t[i] = std::thread(TestFun);
		}

		logger::Logger::GetInstancePtr()->Start(CONSUMER_THREAD_NUM);

		for (int i = 0; i < PRODUCT_THREAD_NUM; i++)
		{
			t[i].join();
		}

		logger::Logger::GetInstancePtr()->Stop();
		logger::Logger::GetInstancePtr()->Join();

		/*LOG_INFO("logger test using");
		NAMED_DEBUG_INFO("test","logger test using in test");
		NAMED_DEBUG_INFO("testex", "debug logger test using in testex");
		NAMED_LOG_INFO("testex", "logger test using in testex");*/
	}

	system("pause");
	return 0;
}