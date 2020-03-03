// -----------------------------------------------------------------------------
// File name  : wpq.hpp
// Developer  : Eyal Weizman
// Date       : 2019-07-11
// Description: wpq header
// -----------------------------------------------------------------------------
#ifndef __WPQ_HPP__
#define __WPQ_HPP__

//=== Include
#include <queue>
#include <mutex>
#include <chrono>

#include "../../utils/semaphore/semaphore.hpp"

//=== Namespace
namespace hrd8
{

//====================== class PriorityQueue ===================================
template<
    typename T,
    typename Container = std::vector<T>,
    typename Compare = std::less<typename Container::value_type>
>
class PriorityQueue
{
public:
    explicit PriorityQueue(const Compare& compare = Compare(), 
                           const Container& container = Container());
    ~PriorityQueue() = default;
    
    // API:
    void push(const T& value);
    void push(T&& value);
    const T& top() const;
    
    template <typename TimeUnit = std::chrono::milliseconds>
    bool pop(T& out_object, TimeUnit timeout);
    size_t size() const;
    bool empty() const;
    
    // deleted
    PriorityQueue(const PriorityQueue& other) = delete;
    PriorityQueue& operator=(const PriorityQueue& other) = delete;
    PriorityQueue(PriorityQueue&& other) = delete;
    PriorityQueue& operator=(PriorityQueue&& other) = delete;

private:
    std::priority_queue<T, Container, Compare> m_queue;
    std::mutex m_lock; 
    Semaphore m_sem;
};


// ========================== Implementation ===================================
// Ctor
    template<typename T, typename Container, typename Compare>
    PriorityQueue<T, Container, Compare>::
    PriorityQueue(const Compare& compare,const Container& container):
        m_queue(compare, container)
    {}

// Push copy
    template<typename T, typename Container, typename Compare>
    void PriorityQueue<T, Container, Compare>::
    push(const T& value)
    {
        m_lock.lock();
        m_queue.push(value);
        m_lock.unlock();
        m_sem.post();
    }

// Push move
    template<typename T, typename Container, typename Compare>
    void PriorityQueue<T, Container, Compare>::
    push(T&& value)
    {
        m_lock.lock();
        m_queue.push(value);
        m_lock.unlock();
        m_sem.post();
    }

// top
    template<typename T, typename Container, typename Compare>
    const T& PriorityQueue<T, Container, Compare>::
    top() const
    {
        return (m_queue.top());
    }

// pop
    template<typename T, typename Container, typename Compare>
    template <typename TimeUnit>
    bool PriorityQueue<T, Container, Compare>::
    pop(T& out_object, TimeUnit timeout)
    {
        bool ret_val = false;
        
        // wait untill the Q is not empty / timeout
        if (m_sem.timedwait<TimeUnit>(timeout) == Semaphore::success)
        {
            m_lock.lock();
            // save the top element in the out-param
            out_object = m_queue.top();
            m_queue.pop();
            m_lock.unlock();
            
            ret_val = true;
        }
        
        return (ret_val);
    }

// size
    template<typename T, typename Container, typename Compare>
    size_t PriorityQueue<T, Container, Compare>::
    size() const
    {
        return (m_queue.size());
    }

// empty
    template<typename T, typename Container, typename Compare>
    bool PriorityQueue<T, Container, Compare>::
    empty() const
    {
        return (m_queue.empty());
    }

} // hrd8
#endif //__WPQ_HPP__
