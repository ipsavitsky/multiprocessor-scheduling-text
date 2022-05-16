#ifndef BOOST_LOGGER_H_
#define BOOST_LOGGER_H_

#include <ostream> 

enum severity_level {
    normal,
    notification,
    warning,
    error,
    critical
};

void init_logging();

std::ostream& operator<< (std::ostream& strm, severity_level level); 

#endif