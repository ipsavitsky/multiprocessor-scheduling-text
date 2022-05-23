#include "time_schedule.hpp"
#include <algorithm>

TimeSchedule::TimeSchedule(size_t proc_num) { proc_array.resize(proc_num); }

/**
 * @brief Get the time it takes the schedule to execute
 *
 * @return int
 */
int TimeSchedule::get_time() const {
    std::vector<int> task_time;
    std::for_each(proc_array.begin(), proc_array.end(),
                  [&task_time](const proc_info &proc) {
                      task_time.push_back(proc.back().finish);
                  });
    return *std::max_element(task_time.begin(), task_time.end());
}

/**
 * @brief Add a task to the schedule
 *
 * @param task The task to add
 * @param proc Processor to assign to the task
 * @param start Start of the task
 * @param finish End of the task
 */
void TimeSchedule::add_task(const Schedule &sched, const Schedule::Task &task,
                            const Schedule::Proc &proc) {
    auto x = test_add_task(sched, task, proc);
    PlacedTask placed_task{task, x, x + sched.get_task_time(proc, task)};
    proc_array[proc].push_back(placed_task);
    fast_mapping[task] = proc;
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
    std::vector<int> times;
    for (auto it = sched.get_in_edges(task); it.first != it.second;
         ++it.first) {
        auto from = boost::source(*it.first, sched.get_graph());
        LOG_DEBUG << "from: " << from << " to: " << task;
        int trans_time = sched.get_task_time(proc, task);
        int task_time = sched.get_tran_time(fast_mapping[from], proc);
        LOG_DEBUG << "trans_time: " << trans_time
                  << "; tran_time: " << task_time;
        times.push_back(trans_time + task_time);
    }
    auto first_available_dependencies =
        *std::max_element(times.begin(), times.end());

    auto stuff = proc_array[proc];
    int first_available_processor;
    if (stuff.begin() != stuff.end()) {
        first_available_processor =
            std::max_element(proc_array[proc].begin(), proc_array[proc].end(),
                             [](const PlacedTask &a, const PlacedTask &b) {
                                 return a.finish < b.finish;
                             })
                ->finish;
    } else {
        first_available_processor = 0;
    }
    return std::max(first_available_dependencies, first_available_processor);
}

Schedule::Proc TimeSchedule::GC2(Schedule sched, Schedule::Task task) {
    std::vector<std::pair<Schedule::Proc, int>> times;
    for (int i = 0; i < proc_array.size(); i++) {
        times.push_back({i, test_add_task(sched, task, i)});
    }
    auto best_proc = std::min_element(
        times.begin(), times.end(),
        [](const auto &a, const auto &b) { return a.second < b.second; });
    return best_proc->first;
}

Schedule::Proc TimeSchedule::GC2_CR(Schedule sched, Schedule::Task task,
                                    double C1, double C2, double C3) {
    throw std::runtime_error("Not implemented");
}

Schedule::Proc TimeSchedule::GC2_BF(Schedule sched, Schedule::Task task,
                                    double C1, double C2) {
    std::vector<std::pair<Schedule::Proc, int>> times;
    for (int i = 0; i < proc_array.size(); i++) {
        times.push_back({i, C1 * test_add_task(sched, task, i) +
                                C2 * BF_with_task(sched, task, i)});
    }
    return std::min_element(
               times.begin(), times.end(),
               [](const auto &a, const auto &b) { return a.second < b.second; })
        ->first;
}

double TimeSchedule::BF_with_task(Schedule sched, Schedule::Task task,
                                  Schedule::Proc proc) {
    TimeSchedule copy(*this);
    copy.add_task(sched, task, proc);
    return copy.calculate_BF();
}

double TimeSchedule::calculate_BF() const {
    auto max_tasks =
        std::max_element(proc_array.begin(), proc_array.end(),
                         [](const proc_info &a, const proc_info &b) {
                             return a.size() < b.size();
                         })
            ->size();
    size_t amount_of_tasks = 0;
    for (auto &proc : proc_array) {
        amount_of_tasks += proc.size();
    }
    double BF = 100 * ((max_tasks * proc_array.size() / amount_of_tasks) - 1);
    return std::ceil(BF);
}

double TimeSchedule::calculate_CR(Schedule sched) const {
    return amount_of_transitions / boost::num_edges(sched.get_graph());
}

double TimeSchedule::calculate_CR2() const {
    throw std::runtime_error("Not implemented");
}