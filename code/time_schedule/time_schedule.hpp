#ifndef TIME_SCHEDULE_HPP
#define TIME_SCHEDULE_HPP

#include "../logging/boost_logger.hpp"
#include "../schedule/schedule.hpp"
#include <boost/numeric/ublas/io.hpp>
#include <boost/numeric/ublas/matrix.hpp>
#include <map>

/**
 * @brief Class representing a time schedule
 *
 */
class TimeSchedule {
  private:
    std::map<Schedule::Task, Schedule::Proc> procs;
    std::map<Schedule::Task, int> task_start;
    std::map<Schedule::Task, int> task_finish;

  public:
    /**
     * @brief An enum to keep all variations of criteria
     *
     */
    enum class extra_criteria {
        NO,
        CR,
        BF,
    };

    TimeSchedule() = default;
    int get_time() const;
    std::map<Schedule::Task, Schedule::Proc> get_procs() const;
    int get_task_start(const Schedule::Task &task) const;
    int get_task_finish(const Schedule::Task &task) const;
    void add_task(const Schedule::Task &task, const Schedule::Proc &proc,
                  const int &start, const int &finish);
    int test_add_task(Schedule sched, const Schedule::Task &task,
                      const Schedule::Proc &proc);
};

#endif