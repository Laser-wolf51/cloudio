//============================================================================//
// File Name    :  singleton.hpp
// Developer    :  Eyal Weizman
// Date         :  2019-07-11
// Description  :
// creating a Singleton<T> object creates a single object T and returns a 
// pointer to it. T must have a default Ctor + Dtor.
// at the end of the program - the singleton will be destroyed.
//============================================================================//
#ifndef __ILRD_SINGLETON_H__
#define __ILRD_SINGLETON_H__

#include <mutex>
#include <atomic>
#include "scope_lock.hpp"

namespace hrd8
{

//===================== class Singleton ========================= //
template <typename T>
class Singleton
{
public:
    static T* get_instance();
    
    // deleted: all special functions
    ~Singleton() = delete;
    Singleton() = delete;
    Singleton(const Singleton&) = delete;
    Singleton& operator=(const Singleton&) = delete;
    Singleton(const Singleton &&) = delete;
    Singleton& operator=(const Singleton&&) = delete;

private:
    static std::atomic<T*> s_instance;
    static std::mutex s_lock; // with hrd8::ScopeLock
    
    // class declaration
    class Singleton_Destroyer;
};

//===================== class Singleton_Destroyer ========================= //
template <typename T>
class Singleton<T>::Singleton_Destroyer
{
public:
    ~Singleton_Destroyer();
};

// ============================= Singleton =================================
// static member init
template < typename T>
std::atomic<T*> Singleton<T>::s_instance(0);
template < typename T>
std::mutex Singleton<T>::s_lock{};

// get_instance
template <typename T>
T* Singleton<T>::get_instance()
{
    T* temp = s_instance.load(std::memory_order_acquire);
    std::atomic_thread_fence(std::memory_order_acquire);
    
    if (nullptr == temp)
    {
        ScopeLock<std::mutex> lock(s_lock);
        temp = s_instance.load(std::memory_order_relaxed);
        if (nullptr == temp)
        {
            s_instance.store(new T);
            temp = s_instance.load();
            
            // create a single static object: will be Dtored at the end of the
            // program.
            static Singleton_Destroyer s_instance_destroyer;
        }
    }
    
    return (temp);
}

// ====================== Singleton_Destroyer =================================
template <typename T>
Singleton<T>::Singleton_Destroyer::~Singleton_Destroyer()
{
    delete Singleton<T>::s_instance.exchange(nullptr);
}

} //hrd8

#endif //__ILRD_SINGLETON_H__