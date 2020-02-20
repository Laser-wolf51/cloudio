// =============================================================================
// File name  : epoll.cpp
// Developer  : Asaf Halbani Batan (Edited)
// Date		  : 2019-06-25
// Description: warraper for epoll functions
// =============================================================================
#include <cstring>
#include <iostream>
#include "epoll.hpp"
namespace hrd8
{

    
Epoll::Epoll(int maxevents): m_max_events(maxevents)
{

    if ((m_epfd = epoll_create1(0)) == -1)
    {
        throw std::runtime_error ("Epoll creation failed\n");
    }
}

Epoll::~Epoll()
{
    close (m_epfd);
    // (Dtor shell never throw exceptions)
}

bool Epoll::add(int fd, unsigned int events)
{
    struct epoll_event event;
    memset(&event, 0, sizeof(event));
    event.events = events;
    event.data.fd = fd;
   

    if((epoll_ctl(m_epfd, EPOLL_CTL_ADD, fd, &event)) == -1)
    {
       throw std::runtime_error("epoll add failed\n");
    }
    return true;
}

bool Epoll::remove(int del_fd)
{
    if((epoll_ctl(m_epfd, EPOLL_CTL_DEL, del_fd, reinterpret_cast<struct epoll_event *>(&m_epfd))) == -1)
    {
       throw std::runtime_error("epoll add failed\n");
    }
    return true;
}

int Epoll::wait(int timeout)
{
    return epoll_wait(m_epfd, m_vec, m_max_events, timeout);
}

res_data Epoll::operator[](size_t index)
{
    res_data ret_val;
    
    ret_val.events = m_vec[index].events;
    ret_val.fd = m_vec[index].data.fd;
    
    return ret_val;
}
}

//     while(running)
//     {
//         printf ("\nEpoll waiting for input...\n");
//         event_count = epoll_wait(epoll_fd, events, MAX_EVENTS, 30000); // timeout 30 seconds
//         for (i = 0; i < event_count; ++i)
//         {
//             printf ("Reading file descriptor %d --\n", events[i].data.fd);
//             bytes_read = read(events[i].data.fd, read_buffer, READ_SIZE);
//             printf ("Bytes read = %ld\n", bytes_read);
//             read_buffer[bytes_read] = '\0';
//             printf ("Read = %s\n", read_buffer);

//             if (!strncmp(read_buffer, "stop\n", 5))
//             {
//                 running = 0;
//             }
//         }
//     }






    // if (close(epoll_fd))
    // {
    //     fprintf (stderr, "failed to close epoll\n");
    //     return (-1);
    // }

    // return 0;
