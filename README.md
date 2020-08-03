### Windows集成
--------------------------------------------------

* 修改log4cxx.h

	替换如下代码：
		
		#define LOG4CXX_LIST_DEF(N, T) \
		template class LOG4CXX_EXPORT std::allocator<T>; \
		template class LOG4CXX_EXPORT std::vector<T>; \
		typedef std::vector<T> N
		
	替换为：
	
		#define LOG4CXX_LIST_DEF(N, T) \
		typedef std::vector<T> N

* 修改multicast.c
	
	替换如下代码：
		
		#if MCAST_JOIN_SOURCE_GROUP
		
	替换为：
	
		#if defined(group_source_req)
		
* 修改stringhelper.cpp
	
	增加`#include <iterator>`

* 增加预编译宏定义 LOG4CXX_STATIC 宏

* 增加静态库链接
	
		log4cxx.lib
		apr.lib
		aprutil.lib
		xml.lib
		Ws2_32.lib
		Mswsock.lib
	