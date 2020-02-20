//============================================================================//
// File Name    :  /git/projects/logger.cpp
// Developer    :  Gal Rodnizkey
// Date         :  03/07/19
// Description  :  implementation for logger class
//============================================================================//

#include <chrono>
#include <ctime> 
#include "./logger.hpp"

namespace hrd8
{

// =============================================================================
//                                 Logger
// =============================================================================
Logger::Logger() : m_ostream("./log.txt"), m_log_level(DEBUG), m_mutex()
{
    m_ostream.clear();
}

Logger::~Logger()
{
    m_ostream.close();
}

void Logger::write( enum debug_level log_level,
                    const std::string& str,
                    const char *filename ,
                    int line )
{
    if (log_level >= m_log_level)
    {
        const std::string backspace("  ");
        time_t     now = time(0);
        struct tm  tstruct;
        char       buf[40];
        tstruct = *localtime(&now);
        strftime(buf, sizeof(buf), "%Y-%m-%d  %X", &tstruct);

        std::string tmp =   std::string(buf) + backspace + str + 
                            backspace + filename + backspace + "line: " + 
                            std::to_string(line) + "\n";

        ScopeLock<std::mutex> lock(m_mutex);
        m_ostream.write(tmp.c_str() , tmp.length());
    }
}

void Logger::flush()
{
    m_ostream.flush();
}

}//hrd8    