/**
 * @file boost_logger.hpp
 * @author Ilya Savitsky (ipsavitsky234@gmail.com)
 * @brief
 * @version 0.1
 * @date 2022-05-28
 *
 * @copyright Copyright (c) 2022
 *
 */
#ifndef LOGGER_HPP
#define LOGGER_HPP

#include <boost/log/expressions.hpp>
#include <boost/log/sources/logger.hpp>
#include <boost/log/support/date_time.hpp>
#include <boost/log/trivial.hpp>
#include <boost/log/utility/setup.hpp>

/**
 * @brief A macro to log with "debug" severity
 *
 */
#define LOG_DEBUG BOOST_LOG_SEV(logger::log, boost::log::trivial::debug)
/**
 * @brief A macro to log with "info" severity
 *
 */
#define LOG_INFO BOOST_LOG_SEV(logger::log, boost::log::trivial::info)
/**
 * @brief A macro to log with "warning" severity
 *
 */
#define LOG_WARNING BOOST_LOG_SEV(logger::log, boost::log::trivial::warning)
/**
 * @brief A macro to log with "error" severity
 *
 */
#define LOG_ERROR BOOST_LOG_SEV(logger::log, boost::log::trivial::error)

namespace logger {
extern boost::log::sources::severity_logger<boost::log::trivial::severity_level>
    log;
extern bool verbose;
extern bool debug;

void init();
} // namespace logger

#endif // LOGGER_HPP