//============================================================================//
// File Name    :  factory.hpp
// Developer    :  Eyal Weizman
// Date         :  09/07/2019
// Description  :  Factory class to create differnet object types of the same 
//                 family at runtime.
//============================================================================//
#ifndef __ILRD_FACTORY_H__
#define __ILRD_FACTORY_H__

#include <vector>
#include <functional>
#include <unordered_map>
#include <memory>
#include <stdexcept>
#include <string>
#include <iostream>

//====================== Class Factory =========================================
namespace hrd8
{
template <typename Base, typename Key, typename Args>
class Factory
{

public:
	// Factory(){std::cout << "Factory Ctor. this  = " << this << std::endl;}
	Factory() = default;
	~Factory() = default;

	Factory(const Factory&) = delete;
	Factory& operator=(const Factory&) = delete;
	Factory(Factory&&) = delete;
	Factory& operator=(Factory&&) = delete;

	enum add_result
	{
		ADDED,
		REPLACED
	};

	// the signature of the create functions the user need to supply when 
	// adding to the factory.
	using CreatorFunc = std::function<std::unique_ptr<Base>(Args args)>;

	// adds the <key, create_func> to the map.
	// in case key already exist, REPLACED will be returned.
	// otherwise, <key, create_func> will be added and ADDED will be returned.  
	// if add fails, it throws  BadAdd
	enum add_result add(const Key& key, CreatorFunc create_func);

	// Returns a pointer to newly allocated object.
	// In case of failure, throws exception of incorrect key or bad allocation.
	std::unique_ptr<Base> create(const Key& key, Args args);
		
	// clear all the functions from the factory.
	// 
	// (added in order to prevent core dump when destructed, happens when the 
	// functions are from the DllLoader.
	// the ~DllLoader - calls dlsym and makes those functions invalid. factory
	// is a handleton and thus destroyed at the end of the program.)
	void clear_all();

private:
    // Search, insertion, and removal of elements in O(1)
    std::unordered_map<Key, CreatorFunc> m_funcs_map;
};

//============================= Exceptions =====================================
// BadAdd
class BadAdd : public std::runtime_error
{
public:
    BadAdd(const std::string what): runtime_error(what) {}
};

// BadCreate
class BadCreate : public std::runtime_error
{
public:
    BadCreate(const std::string what): runtime_error(what) {}
};

// BadKey
class BadKey : public std::logic_error
{
public:
    BadKey(const std::string what): logic_error(what) {}
};


//============================= Implementation =================================

// add
template <typename Base, typename Key, typename Args>
typename Factory<Base, Key, Args>::add_result Factory<Base, Key, Args>::
    add(const Key& key, CreatorFunc create_func)
{
	add_result status = ADDED;
	if (m_funcs_map.find(key) != m_funcs_map.end())
	{
		status = REPLACED;
	}
	
	try
	{
		m_funcs_map[key] = create_func;
	}
	catch(const std::bad_alloc& ex)
	{
		throw(BadAdd(std::string("factory: add: failed to add ") + ex.what()));
	}
	
	// std::cout << "factory:add: key = " << key << std::endl; //debug

	return (status);
}

// create
template <typename Base, typename Key, typename Args>
std::unique_ptr<Base> 
    Factory<Base, Key, Args>::create(const Key& key, Args args)
{
    try
    {
        // std::cout << "factory:create: key = " << key << std::endl; // debug
		
		auto func = m_funcs_map.at(key);
		auto ret_val = func(std::move(args));
		return(ret_val);
    }
    catch (const std::bad_alloc& ex)
    {
        throw (BadCreate(std::string("factory: failed to create. ")+ex.what()));
    }
    catch (const std::out_of_range& ex)
    {
        throw (BadKey(std::string("factory: create failed. ") + ex.what()));
    }
}

// clear_all
template <typename Base, typename Key, typename Args>
void Factory<Base, Key, Args>::clear_all()
{
	m_funcs_map.clear();
}

} //hrd8

#endif //__ILRD_FACTORY_H__



// graveyard
	// const auto func = m_funcs_map.at(key);
	// const char* runner = (char *)&func;
	// for (size_t i = 0; i < sizeof(func); i++)
	// {
	// 	printf("%3hhx ", *runner);
	// 	if (((i+1)%8) == 0)
	// 	{
	// 		printf("\n");
	// 	}
		
	// 	++runner;
	// }
		



