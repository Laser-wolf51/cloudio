//============================================================================//
// File Name    :  logger.hpp
// Developer    :  Gal Rodnizkey
// Date         :  03/07/2019
// Description  :  Basic logging component with all basic logging prpoerties. 
//                 Logger can only be created as a singleton, therefore, it is 
//                 not possible for the ofstream member to be asssociated with 
//                 different files.
//============================================================================//

#ifndef __ILRD_LOGGER_H__
#define __ILRD_LOGGER_H__

#include <mutex>
#include <fstream>
#include "handleton.hpp"

namespace hrd8
{

class Logger
{
    public:
        enum debug_level
        {
            DEBUG,
            INFO,
            ERROR
        };

    void write( enum debug_level log_level,
                const std::string& str,
                const char *filename ,
                int line );
    
    void flush();

    Logger(Logger const&) = delete;
    Logger(Logger const&&) = delete;
    Logger& operator=(Logger const&) = delete;
    Logger& operator=(Logger const&&) = delete;
    
    private:
        Logger();   // Logger can only be created as a singleton
        ~Logger();

        friend class Handleton<Logger>;

        std::ofstream m_ostream;
        enum debug_level m_log_level;
        std::mutex m_mutex;
};

} //hrd8

#endif //__ILRD_LOGGER_H__