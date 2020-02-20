// -----------------------------------------------------------------------------
// File name  : observer_test.cpp
// Developer  : Eyal Weizman
// Date       : 2019-07-18
// Description: observer test file
// -----------------------------------------------------------------------------
#include <iostream>
#include "observer.hpp"

using namespace hrd8;

class Publisher
{
public:
	Publisher() :
		m_dispatcher_ptr(new Dispatcher<std::string>)
	{}
	
	~Publisher() = default;
	
	Dispatcher<std::string>* get_dispatcher() {return (m_dispatcher_ptr.get());}
	void broadcast(const std::string& message) {m_dispatcher_ptr->broadcast(message);}
	
    std::shared_ptr<Dispatcher<std::string>> m_dispatcher_ptr;
private:
};



class Subscriber
{
public:
	Subscriber(Dispatcher<std::string>* disp):
		m_callback_ptr(new Callback<Subscriber, std::string>
		(disp, *this, &Subscriber::action_func, &Subscriber::stop_t))
	{}
	
	~Subscriber() = default;
	
	void action_func(const std::string& message)
	{
		std::cout << "notify test:\t\t" << message << std::endl;
	}
	
	void stop_t()
	{
		m_callback_ptr.reset();
	}
	
    std::shared_ptr<Callback<Subscriber, std::string>> m_callback_ptr; // auto deleted
private:
};

//=========================== MAIN ===========================================//
int main()
{
	std::cout << "\n============= observer test ==============\n" << std::endl;
	
	Publisher publisher;
	Subscriber sub_1(publisher.get_dispatcher());
	// now the sub is subscribed
	
	publisher.broadcast("SUCCESS");
	// should activate 2 Dtors + remove
	sub_1.m_callback_ptr.reset();
	
	Subscriber sub_2(publisher.get_dispatcher());
	std::cout << "dispatcher Dtor:\t";
	publisher.m_dispatcher_ptr.reset();
	(sub_2.m_callback_ptr.get() == nullptr)
	?
	std::cout << "SUCCESS" << std::endl : std::cout << "FAIL" << std::endl;
	
	std::cout << "\n--------------------------------------------\n" << std::endl;
	
	return (0);
}
