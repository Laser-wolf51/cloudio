// =============================================================================
// File name  : epoll.hpp
// Developer  : Asaf Halbani Batan (Edited)
// Date		  : 2019-06-25
// Description: warraper for epoll functions
// =============================================================================

#ifndef __ILRD_EPOLL_HPP__
#define __ILRD_EPOLL_HPP__

#include <sys/epoll.h>  /* epoll  */
#include <unistd.h>     /* close  */
#include <vector>       /* vector */

namespace hrd8
{

typedef struct 
{
    unsigned int events;
    int fd;
} res_data;

class Epoll 
{

public:
    // special member function
    Epoll(int maxevents = 10); // throw runtime_error
    ~Epoll();
    Epoll(Epoll &&) = default;
    Epoll& operator=(Epoll &&) = default;
    Epoll(const Epoll&) = delete;
    Epoll operator=(const Epoll&) = delete;

    // member function
    bool add(int fd, unsigned int events); // throw std::runtime_error
    bool remove(int del_fd); // throw std::runtime_error

    // returns the number of file descriptors ready for the requested I/O
    int wait(int timeout);
    res_data operator[](size_t index);

private:
    int m_epfd;
    int m_max_events;
    struct epoll_event m_vec[10];
};

}//hrd8
#endif //__ILRD_EPOLL_HPP__
    