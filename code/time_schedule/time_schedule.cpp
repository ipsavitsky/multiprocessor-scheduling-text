#include "time_schedule.hpp"
#include <algorithm>

int TimeSchedule::get_time() const {
    int max_time = 0;
    for (auto &[task, time] : task_finish) {
        max_time = std::max(max_time, time);
    }
    return max_time;
}

std::map<Schedule::Task, Schedule::Proc> TimeSchedule::get_procs() const {
    return procs;
}

int TimeSchedule::get_task_start(const Schedule::Task &task) const {
    return task_start.at(task);
}

int TimeSchedule::get_task_finish(const Schedule::Task &task) const {
    return task_finish.at(task);
}

void TimeSchedule::add_task(const Schedule::Task &task,
                            const Schedule::Proc &proc, const int &start,
                            const int &finish) {
    procs[task] = proc;
    task_start[task] = start;
    task_finish[task] = finish;
}