//============================================================================//
// File Name    :  handleton.hpp
// Developer    :  Eyal Weizman
// Date         :  2019-07-21
// Description  :
// T must have a default Ctor + Dtor.
// at the end of the program - the handleton will be destroyed.
// How To Use   :   T* t_ptr = Handleton<T>::get_instance();
//============================================================================//
#ifndef __ILRD_HANDLETON_H__
#define __ILRD_HANDLETON_H__

#include <mutex>
#include <atomic>
#include <iostream>

#include "../scope_lock.hpp"

namespace hrd8
{

//===================== class Handleton ========================= //
template <typename T>
class Handleton
{
public:
	template <typename... ARGS>
	static T* get_instance(ARGS... args);
	
	// deleted: all special functions
	~Handleton() = delete;
	Handleton() = delete;
	Handleton(const Handleton&) = delete;
	Handleton& operator=(const Handleton&) = delete;
	Handleton(const Handleton &&) = delete;
	Handleton& operator=(const Handleton&&) = delete;

private:
    static std::atomic<T*> s_instance;
    static std::mutex s_mutex; // with hrd8::ScopeLock
    
    // class declaration
    class Handleton_Destroyer;
};

/******************************** static memebers Init ************************/
#ifndef _INIT_HANDLETON_
#define INIT_HANDLETON(type) template<> \
    std::atomic<type*> Handleton<type>::s_instance{nullptr}; \
    template<> \
    std::mutex Handleton<type>::s_mutex{};
#endif


//===================== class Handleton_Destroyer ========================= //
template <typename T>
class Handleton<T>::Handleton_Destroyer
{
public:
    ~Handleton_Destroyer();
};

// ============================= Handleton =================================
// static member init
template < typename T>
std::atomic<T*> Handleton<T>::s_instance(0);
template < typename T>
std::mutex Handleton<T>::s_mutex{};

// get_instance
template <typename T>
template <typename... ARGS>
T* Handleton<T>::get_instance(ARGS... args)
{
	T* temp = s_instance.load(std::memory_order_relaxed);
	std::atomic_thread_fence(std::memory_order_acquire);
	// std::cout << "Handleton begin - temp = " << temp << std::endl;
	if (nullptr == temp)
	{
		ScopeLock<std::mutex> lock(s_mutex);
		temp = s_instance.load(std::memory_order_relaxed);
		if (nullptr == temp)
		{
			// create a single static object: will be Dtored at the end of the
			// program.
			static Handleton_Destroyer s_instance_destroyer;

			std::atomic_thread_fence(std::memory_order_release);

			s_instance.store(new T(args...));
			temp = s_instance.load();
			
			// std::cout << "Handleton. temp = " << temp << std::endl;
		}
	}
	
	return (temp);
}

// ====================== Handleton_Destroyer =================================
template <typename T>
Handleton<T>::Handleton_Destroyer::~Handleton_Destroyer()
{
    delete Handleton<T>::s_instance.exchange(nullptr);
}

} //hrd8

#endif //__ILRD_HANDLETON_H__