
#include <iostream>
#include <thread>

#include "log4cxx/logger_wrapper.h"

#define PRODUCT_THREAD_NUM 8
#define CONSUMER_THREAD_NUM 8

void TestFun()
{
	for (size_t i = 0; i < 2000; i++)
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

		auto start = std::chrono::steady_clock::now();
		auto start_since_epoch = start.time_since_epoch();
		auto start_ms = std::chrono::duration_cast<std::chrono::milliseconds>(start_since_epoch).count();

		for (int i = 0; i < PRODUCT_THREAD_NUM; i++)
		{
			t[i].join();
		}

		logger::Logger::GetInstancePtr()->Stop();
		logger::Logger::GetInstancePtr()->Join();

		auto end = std::chrono::steady_clock::now();
		auto end_since_epoch = end.time_since_epoch();
		auto end_ms = std::chrono::duration_cast<std::chrono::milliseconds>(end_since_epoch).count();

		std::cout << end_ms - start_ms << std::endl;

		/*LOG_INFO("logger test using");
		NAMED_DEBUG_INFO("test","logger test using in test");
		NAMED_DEBUG_INFO("testex", "debug logger test using in testex");
		NAMED_LOG_INFO("testex", "logger test using in testex");*/
	}

	system("pause");
	return 0;
}