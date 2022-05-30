/**
 * @file huawei_parser.hpp
 * @author Ilya Savitsky (ipsavitsky234@gmail.com)
 * @brief 
 * @version 0.1
 * @date 2022-05-28
 * 
 * @copyright Copyright (c) 2022
 * 
 */
#ifndef HUAWEI_PARSER_HPP
#define HUAWEI_PARSER_HPP

#include "../schedule/schedule.hpp"

#include <string>

Schedule input_schedule_class_1(std::string path);
Schedule input_schedule_class_2(std::string path);
Schedule input_schedule_regular(std::string path);

#endif