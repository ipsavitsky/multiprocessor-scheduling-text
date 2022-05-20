#include "time_schedule.hpp"
#include <algorithm>

/**
 * @brief Get the time it takes the schedule to execute
 *
 * @return int
 */
int TimeSchedule::get_time() const {
    int max_time = 0;
    for (auto &[task, time] : task_finish) {
        max_time = std::max(max_time, time);
    }
    return max_time;
}

/**
 * @brief Get the mappping of tasks to processors
 *
 * @return std::map<Schedule::Task, Schedule::Proc>
 */
std::map<Schedule::Task, Schedule::Proc> TimeSchedule::get_procs() const {
    return procs;
}

/**
 * @brief Get the time the task starts
 *
 * @param task The task
 * @return int
 */
int TimeSchedule::get_task_start(const Schedule::Task &task) const {
    return task_start.at(task);
}

/**
 * @brief Get the time the task ends
 *
 * @param task The task
 * @return int
 */
int TimeSchedule::get_task_finish(const Schedule::Task &task) const {
    return task_finish.at(task);
}

/**
 * @brief Add a task to the schedule
 *
 * @param task The task to add
 * @param proc Processor to assign to the task
 * @param start Start of the task
 * @param finish End of the task
 */
void TimeSchedule::add_task(const Schedule::Task &task,
                            const Schedule::Proc &proc, const int &start,
                            const int &finish) {
    procs[task] = proc;
    task_start[task] = start;
    task_finish[task] = finish;
}

/**
 * @brief Test if a task can be added to the schedule
 *
 * @todo Not finished
 * 
 * @param sched Schedule with all dependencies
 * @param task Task to add
 * @param proc Processor to assign to the task
 * @return int
 */
int TimeSchedule::test_add_task(Schedule sched, const Schedule::Task &task,
                                const Schedule::Proc &proc) {
    for (auto it = sched.get_in_edges(task); it.first != it.second;
         ++it.first) {
        auto from = boost::source(*it.first, sched.get_graph());
        LOG_DEBUG << "from: " << from << " to: " << task;
        int trans_time = sched.get_tran_time(from, task);
        LOG_DEBUG << trans_time;
    }
    return 123;
}