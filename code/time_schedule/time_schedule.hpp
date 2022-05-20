#ifndef TIME_SCHEDULE_HPP
#define TIME_SCHEDULE_HPP

#include "../schedule/schedule.hpp"
#include <map>

class TimeSchedule {
  private:
    std::map<Schedule::Task, Schedule::Proc> procs;
    std::map<Schedule::Task, int> task_start;
    std::map<Schedule::Task, int> task_finish;

  public:
    TimeSchedule() = default;
    int get_time() const;
    std::map<Schedule::Task, Schedule::Proc> get_procs() const;
    int get_task_start(const Schedule::Task &task) const;
    int get_task_finish(const Schedule::Task &task) const;

    void add_task(const Schedule::Task &task, const Schedule::Proc &proc,
                  const int &start, const int &finish);
};

#endif