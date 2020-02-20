// -----------------------------------------------------------------------------
// File name  : observer.hpp
// Developer  : Eyal Weizman
// Date       : 2019-07-18
// Description: observer header
// -----------------------------------------------------------------------------
#ifndef __OBSERVER_HPP__
#define __OBSERVER_HPP__

//=== Include
#include <vector>       //vector
#include <memory>       //not yet
#include <mutex>        //mutex
#include <algorithm>    //find

#include "scope_lock.hpp"

//=== Namespace
namespace hrd8
{

template <typename Message>
class Dispatcher;

// -----------------------------------------------------------------------------
//                            class CallbackBase                                 
// -----------------------------------------------------------------------------
template <typename Message>
class CallbackBase
{
public:
    explicit CallbackBase(Dispatcher<Message>*);
    virtual ~CallbackBase();

    CallbackBase(const CallbackBase& other) = delete;
    CallbackBase& operator=(const CallbackBase& other) = delete;
    CallbackBase(CallbackBase&& other) = delete;
    CallbackBase& operator=(CallbackBase&& other) = delete;

protected:
    // dispatcher notifies callback of his premature death 
    virtual void notify_service_out();

private:
    Dispatcher<Message>* m_dispatcher;
    
    virtual void notify(const Message& message) = 0;
    friend class Dispatcher<Message>;
};
// -----------------------------------------------------------------------------
//                              class Callback                                   
// -----------------------------------------------------------------------------
template <typename Client, typename Message>
class Callback : public CallbackBase<Message>
{
public:
    using action_t = void(Client::*)(const Message&);
    using stop_t   = void(Client::*)();

    Callback(Dispatcher<Message>* dispatcher,
             Client& client,
             action_t notify, 
             stop_t stop = nullptr);

    virtual ~Callback() noexcept = default;
    
private:
    Client& m_client;
    action_t m_notify_func;
    stop_t m_stop_func;

    void notify(const Message& message) override;

    // dispatcher notifies callback of his premature death 
    virtual void notify_service_out() override;
    friend class Dispatcher<Message>;
};
// -----------------------------------------------------------------------------
//                              class Dispatcher                                
// -----------------------------------------------------------------------------
template <typename Message>
class Dispatcher final
{
public:
    Dispatcher() = default;
    ~Dispatcher();

    void broadcast(const Message& message);

    Dispatcher(const Dispatcher& other) = delete;
    Dispatcher& operator=(const Dispatcher& other) = delete;
    Dispatcher(Dispatcher&& other) = delete;
    Dispatcher& operator=(Dispatcher&& other) = delete;

private:
    std::vector<CallbackBase<Message>*> m_callbacks;
    std::mutex m_lock;

    // Apply dispatcher_death
    void broadcast_death();
    // subscribe/unsubscribe
    void add(CallbackBase<Message>* callback);
    void remove(CallbackBase<Message>* callback);

    friend class CallbackBase<Message>; // check

};

// ===========================================================================//
//========================= Implementation ===================================//
// ===========================================================================//
//========================== CallbackBase ====================================//
// Ctor
	template <typename Message>
	CallbackBase<Message>::CallbackBase(Dispatcher<Message>* dispatcher):
	    m_dispatcher(dispatcher)
	{
        m_dispatcher->add(this);
    }

// Dtor
	template <typename Message>
	CallbackBase<Message>::~CallbackBase()
	{
        if (m_dispatcher != nullptr)
        {
            m_dispatcher->remove(this);
        }        
    }

// notify_service_out
	template <typename Message>
	void CallbackBase<Message>::notify_service_out()
	{
        m_dispatcher = nullptr;
    }

//========================== derived Callback ================================//
// Ctor
	template <typename Client, typename Message>
	Callback<Client, Message>::
	Callback(Dispatcher<Message>* dispatcher,
	Client& client,
	action_t notify, 
	stop_t stop):
	    CallbackBase<Message>(dispatcher),
	    m_client(client),
	    m_notify_func(notify),
	    m_stop_func(stop)
	{}

// notify
	template <typename Client, typename Message>
	void Callback<Client, Message>::notify(const Message& message)
	{
        (m_client.*m_notify_func)(message);
    }

// notify_service_out
	template <typename Client, typename Message>
	void Callback<Client, Message>::notify_service_out()
	{
        CallbackBase<Message>::notify_service_out();
        if (m_stop_func != nullptr)
        {
            (m_client.*m_stop_func)();
        }        
    }

//========================== Dispatcher ======================================//
// Dtor
	template <typename Message>
	Dispatcher<Message>::~Dispatcher()
	{
        broadcast_death();
    }

// add
	template <typename Message>
	void Dispatcher<Message>::add(CallbackBase<Message>* callback)
	{
        m_callbacks.push_back(callback);
    }

// broadcast
	template <typename Message>
	void Dispatcher<Message>::broadcast(const Message& message)
	{
        for (auto& it : m_callbacks)
        {
            it->notify(message);
        }
    }

// broadcast_death
	template <typename Message>
	void Dispatcher<Message>::broadcast_death()
	{
        for (auto& it : m_callbacks)
        {
            it->notify_service_out();
        }
    }

// remove
	template <typename Message>
	void Dispatcher<Message>::remove(CallbackBase<Message>* callback)
	{
        m_callbacks.erase(std::find(m_callbacks.begin(),
                                    m_callbacks.end(),
                                    callback));
    }


}// hrd8

#endif // __OBSERVER_HPP__