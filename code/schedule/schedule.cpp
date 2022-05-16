#include "schedule.hpp"
#include <algorithm>
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/graph_traits.hpp>
#include <boost/graph/topological_sort.hpp>
#include <boost/property_map/property_map.hpp>
#include <iostream>
#include <set>
#include <vector>

void Schedule::print_graph() {
    for (Task curr_task = 0; curr_task < task_num; ++curr_task) {
        std::cout << "from: " << curr_task << std::endl;
        for (auto successors_it = get_successors_of_task(curr_task);
             successors_it.first != successors_it.second;
             ++successors_it.first) {
            Task child_task = *(successors_it.first);
            std::cout << "to: " << child_task << std::endl;
        }
    }
}

void Schedule::set_task_on_proc(Task &task, Proc &proc) {
    graph[task].proc = proc;
}

int Schedule::get_tran_time(const Proc &from, const Proc &to) const {
    return tran_times[from][to]; // change later for unlinked processors
}

int Schedule::get_task_num() const { return task_num; }

int Schedule::get_proc_num() const { return proc_num; }

int Schedule::get_task_time(const Schedule::Proc &proc,
                            const Schedule::Task &task) const {
    return task_times[proc][task];
}
int Schedule::get_out_degree(const Schedule::Task &task) const {
    return boost::out_degree(task, graph);
}

int Schedule::get_in_degree(const Schedule::Task &task) const {
    return boost::in_degree(task, graph);
}

int Schedule::get_number_of_edges() const { return edges; }

bool Schedule::is_direct_connection(const Schedule::Proc &proc1,
                                    const Schedule::Proc &proc2) {
    return long_transmition[proc1][proc2] == -1;
}
double Schedule::calculate_BF() {
    int max_tasks = 0;
    for (Proc proc = 0; proc < proc_num; ++proc) {
        max_tasks = std::max(
            max_tasks, proc_load[proc]); // could be optimized - store max_tasks
                                         // in schedule. But isn't required
                                         // (compexicity will not be changed)
    }
    return (double)max_tasks / task_num;
}

double Schedule::calculate_CR() const {
    return (double)transmitions / get_number_of_edges();
}

double Schedule::calculate_CR2() const {
    return (double)double_transmitions / get_number_of_edges();
}

void Schedule::init_transmition_matrices(std::vector<std::vector<int>> tran) {
    tran_times = tran;
    long_transmition = std::vector<std::vector<int>>(
        tran.size(), std::vector<int>(tran[0].size()));
    for (int i = 0; i < tran.size(); ++i) {
        for (int j = 0; j < tran[i].size(); ++j) {
            if (tran[i][j] == -1) {
                for (int k = 0; k < tran.size(); ++k) {
                    if (tran[i][k] != -1 && tran[k][j] != -1 &&
                        (tran_times[i][j] == -1 ||
                         tran[i][k] + tran[k][j] < tran_times[i][j])) {
                        tran_times[i][j] = tran[i][k] + tran[k][j];
                        long_transmition[i][j] = k;
                    }
                }
            } else {
                long_transmition[i][j] = -1;
            }
        }
    }
}

Schedule::Schedule(Schedule::edge_it edge_iterator_start, Schedule::edge_it edge_iterator_end,
                   int task_num, int proc_num,
                   std::vector<std::vector<int>> &task_times,
                   std::vector<std::vector<int>> &tran_times, int criterion) {
    this->task_num = task_num;
    this->proc_num = proc_num;
    graph = Graph(edge_iterator_start, edge_iterator_end, task_num);
    edges = boost::num_edges(graph);
    this->task_times = task_times;
    init_transmition_matrices(tran_times);
    this->criterion = criterion;
}

Schedule::Schedule(const Schedule &schedule) {
    task_num = schedule.task_num;
    proc_num = schedule.proc_num;
    graph = schedule.graph;
    task_times = schedule.task_times;
    tran_times = schedule.tran_times;
    long_transmition = schedule.long_transmition;
    criterion = schedule.criterion;
    edges = schedule.edges;
    transmitions = schedule.transmitions;
    proc_load = schedule.proc_load;
}

Schedule::Task Schedule::Task_in_iterator::operator*() {
    return boost::source(*in, graph);
}

Schedule::Task_in_iterator &Schedule::Task_in_iterator::operator++() {
    ++in;
    return *this;
}

bool Schedule::Task_in_iterator::operator!=(
    const Schedule::Task_in_iterator &rhs) {
    return in != rhs.in; // it doesn't check graph (but information about it
                         // could be in in_edge_iterator)
}

Schedule::Task Schedule::Task_out_iterator::operator*() {
    return boost::target(*out, graph);
}

Schedule::Task_out_iterator &Schedule::Task_out_iterator::operator++() {
    ++out;
    return *this;
}

bool Schedule::Task_out_iterator::operator!=(
    const Schedule::Task_out_iterator &rhs) {
    return out != rhs.out;
}

std::pair<Schedule::Task_in_iterator, Schedule::Task_in_iterator>
Schedule::get_predecessors_of_task(Task task) const {
    auto edges = boost::in_edges(task, graph);
    Task_in_iterator t1(edges.first, graph), t2(edges.second, graph);
    return {t1, t2};
}

std::pair<Schedule::Task_out_iterator, Schedule::Task_out_iterator>
Schedule::get_successors_of_task(Schedule::Task task) const {
    auto edges = boost::out_edges(task, graph);
    Task_out_iterator t1(edges.first, graph), t2(edges.second, graph);
    return std::pair<Schedule::Task_out_iterator, Schedule::Task_out_iterator>(
        t1, t2);
}

// class TimeDiagram {
//     std::map<Schedule::Task, Schedule::Proc> procs;
//     std::map<Schedule::Task, int> task_start;
//     std::map<Schedule::Task, int> task_finish;

//   public:
//     TimeDiagram() = default;
//     TimeDiagram(const Schedule &schedule);
//     int get_time() const {
//         int max_time = 0;
//         for (auto &[task, time] : task_finish) {
//             max_time = std::max(max_time, time);
//         }
//         return max_time;
//     }
//     std::map<Schedule::Task, Schedule::Proc> get_procs() const { return
//     procs; } int get_task_start(const Schedule::Task &task) { return
//     task_start[task]; } int get_task_finish(const Schedule::Task &task) {
//         return task_finish[task];
//     }
// };

// TimeDiagram::TimeDiagram(const Schedule &schedule) {
//     const int task_num = schedule.get_task_num();
//     const int proc_num = schedule.get_proc_num();
//     std::map<Schedule::Proc, int>
//         times; // current finish time of each processor
//     for (int i = 0; i < task_num; ++i) { // i is current tier
//         Schedule::Task curr_task = schedule.get_task_by_tier(i);
//         Schedule::Proc curr_proc = schedule.get_proc_by_task(curr_task);
//         int min_time = times[curr_proc]; // minimum time that is allowed for
//                                          // task to start from
//         for (std::pair<Schedule::Task_in_iterator,
//         Schedule::Task_in_iterator>
//                  predecessors_it =
//                  schedule.get_predecessors_of_task(curr_task);
//              predecessors_it.first != predecessors_it.second;
//              ++predecessors_it.first) {
//             // pass through all parents to finds min_time
//             Schedule::Task parent_task = *(predecessors_it.first);
//             Schedule::Proc parent_proc =
//             schedule.get_proc_by_task(parent_task); if
//             (predecessors_it.first.from_fictive()) {
//                 if (parent_proc != curr_proc) {
//                     int total_time = task_start[parent_task];
//                     min_time = std::max(min_time, total_time);
//                 } else {
//                     int total_time = task_finish[parent_task];
//                     min_time = std::max(min_time, total_time);
//                 }
//             } else {
//                 int total_time = task_finish[parent_task] +
//                                  schedule.get_tran_time(parent_proc,
//                                  curr_proc);
//                 min_time = std::max(min_time, total_time);
//             }
//         }
//         procs[curr_task] = curr_proc;
//         task_start[curr_task] = min_time;
//         task_finish[curr_task] =
//             min_time + schedule.get_task_time(curr_proc, curr_task);
//         times[curr_proc] =
//             min_time + schedule.get_task_time(curr_proc, curr_task);
//     }
// }
