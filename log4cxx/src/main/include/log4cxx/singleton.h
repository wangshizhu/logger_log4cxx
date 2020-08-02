#ifndef	SINGLETON_H_
#define SINGLETON_H_

#include <assert.h>

template <class T>
class Singleton
{
public:
	Singleton()
	{
		assert(!m_pT);
		m_pT = static_cast<T*>(this);
	}
	virtual ~Singleton()
	{
		assert(m_pT);
		m_pT = nullptr;
	}

	Singleton(const Singleton&) = delete;
	Singleton& operator=(const Singleton&) = delete;

public:
	static T* GetInstancePtr()
	{
		return m_pT;
	}

private:
	static T* m_pT;
};

template <class T> T*  Singleton<T>::m_pT = nullptr;

#endif // !SINGLETON_H_