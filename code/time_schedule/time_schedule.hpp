/**
 * @file time_schedule.hpp
 * @author Ilya Savitsky (ipsavitsky234@gmail.com)
 * @brief 
 * @version 0.1
 * @date 2022-05-28
 * 
 * @copyright Copyright (c) 2022
 * 
 */
#ifndef TIME_SCHEDULE_HPP
#define TIME_SCHEDULE_HPP

#include "../logging/boost_logger.hpp"
#include "../schedule/schedule.hpp"

#include <boost/numeric/ublas/matrix.hpp>
#include <map>

/**
 * @brief Class representing a time schedule
 *
 */
class TimeSchedule {
  private:
    struct PlacedTask {
        Schedule::Task task;
        std::size_t start;
        std::size_t finish;
    };
    using proc_info = std::vector<PlacedTask>;
    std::vector<proc_info> proc_array;

    std::map<Schedule::Task, Schedule::Proc> fast_mapping;
    std::size_t amount_of_transitions = 0;
    std::size_t amount_of_indirect_transitions = 0;

    double BF_with_task(const Schedule &sched, Schedule::Task task,
                        Schedule::Proc proc) const;
    double CR_with_task(const Schedule &sched, Schedule::Task task,
                        Schedule::Proc proc) const;

  public:
    /**
     * @brief An enum to keep all variations of criteria
     *
     */
    enum class extra_criteria {
        NO, /**< No extra criteria */
        CR, /**< CR criteria */
        BF, /**< BF criteria */
    } criteria; /**< The criteria to use */

    TimeSchedule(std::size_t proc_num);
    int get_time() const;
    void add_task(const Schedule &sched, const Schedule::Task &task,
                  const Schedule::Proc &proc);
    std::size_t test_add_task(const Schedule &sched, const Schedule::Task &task,
                              const Schedule::Proc &proc);
    Schedule::Proc GC2(const Schedule &sched, Schedule::Task task);
    Schedule::Proc GC2_BF(const Schedule &sched, Schedule::Task task, double C1,
                          double C2);
    Schedule::Proc GC2_CR(const Schedule &sched, Schedule::Task task, double C1,
                          double C2, double C3);

    double calculate_BF() const;
    double calculate_CR(const Schedule &sched) const;
    double calculate_CR2(const Schedule &sched) const;
};

#endif