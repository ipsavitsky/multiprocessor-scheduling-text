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
            std::cout << "to: " << child_task
                      << " fictive: " << successors_it.first.to_fictive()
                      << std::endl;
        }
    }
}

void Schedule::set_task_on_tier(Task &task, int &tier) {
    boost::put(tier_t(), graph, task, tier);
    task_by_tier[tier] = task;
}

void Schedule::set_task_on_proc(Task &task, Proc &proc) {
    boost::put(proc_t(), graph, task, proc);
}

Schedule::Task Schedule::get_task_by_tier(const int &tier) const {
    return task_by_tier[tier];
}

int Schedule::get_tier_by_task(const Task &task) const {
    return boost::get(tier_t(), graph, task);
}

Schedule::Proc Schedule::get_proc_by_task(const Task &task) const {
    return boost::get(proc_t(), graph, task);
}

Schedule::Proc Schedule::get_proc_by_tier(const int &tier) const {
    Task task = get_task_by_tier(tier);
    return get_proc_by_task(task);
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
int Schedule::get_number_of_edges() const { return non_fictive_edges; }
void Schedule::add_fictive_edge(const Schedule::Task &t1,
                                const Schedule::Task &t2) {
    boost::add_edge(t1, t2, EdgeProperty(true), graph);
    reinitiate(); // tiers should be reinitiated
}
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

void Schedule::set_lower_bound(int bound) { lower_bound = bound; }

int Schedule::get_lower_bound() const { return lower_bound; }

void Schedule::init_tiers_by_topologic() {
    boost::topological_sort(graph, std::back_inserter(task_by_tier));
    std::reverse(task_by_tier.begin(), task_by_tier.end());
    // task_by_tier was assigned
    if (task_num != task_by_tier.size()) {
        std::cerr << "Wrong size of task_by_tier\n";
        throw 1;
    }
    for (int tier = 0; tier < task_num; ++tier) {
        set_task_on_tier(task_by_tier[tier], tier);
    }
}

void Schedule::init_procs() { // random initialization
    for (int tier = 0; tier < task_num; ++tier) {
        Task task = get_task_by_tier(tier);
        Proc proc = rand() % proc_num;
        set_task_on_proc(task, proc);
    }
}

void Schedule::init_tiers_procs() {
    if (criterion == NO || criterion == BF) {
        task_by_tier = std::vector<Task>(
            boost::num_vertices(graph)); // inititate task_by_tier
        auto cmp_tasks = [this](const Task &t1, const Task &t2) {
            return get_out_degree(t1) > get_out_degree(t2) ||
                   (get_out_degree(t1) == get_out_degree(t2) &&
                    t1 < t2); // unique (because of set)
        };
        std::set<Task, decltype(cmp_tasks)> available_tasks(cmp_tasks);
        GraphTraits::vertex_iterator vi, vi_end;
        std::map<Task, int>
            pred_set; // number of predecessors who has already set in schedule
        for (boost::tie(vi, vi_end) = boost::vertices(graph); vi != vi_end;
             ++vi) {
            pred_set[*vi] = get_in_degree(*vi);
            if (pred_set[*vi] == 0) {
                available_tasks.insert(*vi);
            }
        }
        int curr_tier = 0;
        std::map<Proc, int> times; // current finish time of each processor
        std::map<Task, int> task_start;
        std::map<Task, int> task_finish;
        while (!available_tasks.empty()) {
            Task curr_task = *(available_tasks.begin());
            available_tasks.erase(curr_task);
            for (std::pair<Schedule::Task_out_iterator,
                           Schedule::Task_out_iterator>
                     successors_it = get_successors_of_task(curr_task);
                 successors_it.first != successors_it.second;
                 ++successors_it.first) {
                Task child_task = *(successors_it.first);
                pred_set[child_task]--;
                if (pred_set[child_task] == 0) {
                    available_tasks.insert(child_task);
                }
            }
            set_task_on_tier(curr_task, curr_tier);
            curr_tier++;
            Proc best_proc;
            int best_time = -1;
            int best_min_time = -1;
            for (Proc curr_proc = 0; curr_proc < get_proc_num(); ++curr_proc) {
                if (criterion == BF &&
                    double((proc_load[curr_proc] + 1) * get_proc_num()) /
                                get_task_num() -
                            1 >=
                        BF_BOUND) {
                    continue; // don't check processors that are full by BF
                              // criterion
                }
                int min_time = times[curr_proc];
                for (std::pair<Schedule::Task_in_iterator,
                               Schedule::Task_in_iterator>
                         predecessors_it = get_predecessors_of_task(curr_task);
                     predecessors_it.first != predecessors_it.second;
                     ++predecessors_it.first) {
                    // pass through all parents to finds min_time
                    Task parent_task = *(predecessors_it.first);
                    Proc parent_proc = get_proc_by_task(parent_task);
                    if (predecessors_it.first.from_fictive()) {
                        if (parent_proc != curr_proc) {
                            int total_time = task_start[parent_task];
                            min_time = std::max(min_time, total_time);
                        } else {
                            int total_time = task_finish[parent_task];
                            min_time = std::max(min_time, total_time);
                        }
                    } else {
                        int total_time = task_finish[parent_task] +
                                         get_tran_time(parent_proc, curr_proc);
                        min_time = std::max(min_time, total_time);
                    }
                }
                int time_finish =
                    min_time + get_task_time(curr_proc, curr_task);
                if (best_time == -1 || time_finish < best_time) {
                    best_min_time = min_time;
                    best_time = time_finish;
                    best_proc = curr_proc;
                }
            }
            task_start[curr_task] = best_min_time;
            task_finish[curr_task] = best_time;
            times[best_proc] = best_time;
            set_task_on_proc(curr_task, best_proc);
            if (criterion == BF) {
                proc_load[best_proc]++;
            }
        }
    } else if (criterion == CR) {
        init_tiers_by_topologic();
        Proc start_proc = 0;
        int curr_time = 0;
        std::map<Task, int> task_start;
        std::map<Task, int> task_finish;
        for (int tier = 0; tier < get_task_num(); ++tier) {
            set_task_on_proc(task_by_tier[tier],
                             start_proc); // start all task on processor 0
            task_start[task_by_tier[tier]] = curr_time;
            curr_time += get_task_time(start_proc, task_by_tier[tier]);
        }
        std::map<Proc, int> times; // time of processor to current tier
        for (int tier = 0; tier < get_task_num(); ++tier) {
            int best_start_time;
            int best_new_transmitions, best_new_double_transmitions;
            Proc best_proc = start_proc;
            Task curr_task = get_task_by_tier(tier);
            for (int curr_proc = 0; curr_proc < get_proc_num(); ++curr_proc) {
                int new_transmitions = 0, new_double_transmitions = 0;
                int min_task_time = times[curr_proc];
                for (std::pair<Schedule::Task_in_iterator,
                               Schedule::Task_in_iterator>
                         predecessors_it = get_predecessors_of_task(curr_task);
                     predecessors_it.first != predecessors_it.second;
                     ++predecessors_it.first) {
                    Task parent_task = *(predecessors_it.first);
                    Proc parent_proc = get_proc_by_task(parent_task);
                    if (predecessors_it.first.from_fictive()) {
                        if (parent_proc != curr_proc) {
                            int total_time = task_start[parent_task];
                            min_task_time = std::max(min_task_time, total_time);
                        } else {
                            int total_time = task_finish[parent_task];
                            min_task_time = std::max(min_task_time, total_time);
                        }
                    } else {
                        int total_time = task_finish[parent_task] +
                                         get_tran_time(parent_proc, curr_proc);
                        min_task_time = std::max(min_task_time, total_time);
                        if (parent_proc != curr_proc) {
                            if (parent_proc == start_proc) {
                                if (!is_direct_connection(parent_proc,
                                                          curr_proc)) {
                                    new_double_transmitions++;
                                }
                                new_transmitions++;
                            } else {
                                if (is_direct_connection(parent_proc,
                                                         start_proc) &&
                                    !is_direct_connection(parent_proc,
                                                          curr_proc)) {
                                    new_double_transmitions++;
                                } else if (!is_direct_connection(parent_proc,
                                                                 start_proc) &&
                                           is_direct_connection(parent_proc,
                                                                curr_proc)) {
                                    new_double_transmitions--;
                                }
                            }
                        } else {
                            if (parent_proc != start_proc) {
                                if (!is_direct_connection(parent_proc,
                                                          curr_proc)) {
                                    new_double_transmitions--;
                                }
                                new_transmitions--;
                            }
                        }
                    }
                }
                for (std::pair<Schedule::Task_out_iterator,
                               Schedule::Task_out_iterator>
                         successors_it = get_successors_of_task(curr_task);
                     successors_it.first != successors_it.second;
                     ++successors_it.first) {
                    Task child_task = *(successors_it.first);
                    Proc child_proc = get_proc_by_task(child_task);
                    if (!successors_it.first.to_fictive()) {
                        if (child_proc != curr_proc) {
                            if (!is_direct_connection(curr_proc, child_proc)) {
                                new_double_transmitions++;
                            }
                            new_transmitions++;
                        }
                    }
                }
                int curr_task_finish =
                    min_task_time + get_task_time(curr_proc, curr_task);
                if (curr_proc == 0) { // intitiate best_start_time
                    best_proc = curr_proc;
                    best_start_time = min_task_time;
                    best_new_transmitions = new_transmitions;
                    best_new_double_transmitions = new_double_transmitions;
                } else if ((get_number_of_edges() == 0 ||
                            ((double)(transmitions + new_transmitions) /
                                     get_number_of_edges() <
                                 CR_BOUND &&
                             (double)(double_transmitions +
                                      new_double_transmitions) /
                                     get_number_of_edges() <
                                 CR2_BOUND)) &&
                           curr_task_finish <
                               best_start_time +
                                   get_task_time(curr_proc, curr_task)) {
                    best_proc = curr_proc;
                    best_start_time = min_task_time;
                    best_new_transmitions = new_transmitions;
                    best_new_double_transmitions = new_double_transmitions;
                }
            }
            set_task_on_proc(curr_task, best_proc);
            times[best_proc] =
                best_start_time + get_task_time(best_proc, curr_task);
            task_finish[curr_task] =
                best_start_time + get_task_time(best_proc, curr_task);
            transmitions += best_new_transmitions;
            double_transmitions += best_new_double_transmitions;
        }
    } else {
        throw "Wrong criterion";
    }
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

template <typename edge_it>
Schedule::Schedule(edge_it edge_iterator_start, edge_it edge_iterator_end,
                   int task_num, int proc_num,
                   std::vector<std::vector<int>> &task_times,
                   std::vector<std::vector<int>> &tran_times, int criterion) {
    this->task_num = task_num;
    this->proc_num = proc_num;
    graph = Graph(edge_iterator_start, edge_iterator_end, task_num);
    non_fictive_edges = boost::num_edges(graph);
    this->task_times = task_times;
    init_transmition_matrices(tran_times);
    this->criterion = criterion;
    init_tiers_procs();
}

Schedule::Schedule(const Schedule &schedule) {
    task_num = schedule.task_num;
    proc_num = schedule.proc_num;
    graph = schedule.graph;
    task_by_tier = schedule.task_by_tier;
    task_times = schedule.task_times;
    tran_times = schedule.tran_times;
    long_transmition = schedule.long_transmition;
    criterion = schedule.criterion;
    non_fictive_edges = schedule.non_fictive_edges;
    transmitions = schedule.transmitions;
    proc_load = schedule.proc_load;
}

void Schedule::reinitiate() {
    transmitions = 0;
    proc_load.clear();
    task_by_tier.clear();
    init_tiers_procs();
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

bool Schedule::Task_in_iterator::from_fictive() {
    return boost::get(edge_t(), graph, *in);
}

Schedule::Task Schedule::Task_out_iterator::operator*() {
    return boost::target(*out, graph);
}

Schedule::Task_out_iterator &Schedule::Task_out_iterator::operator++() {
    ++out;
    return *this;
}

bool Schedule::Task_out_iterator::to_fictive() {
    return boost::get(edge_t(), graph, *out);
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
    return std::pair<Schedule::Task_out_iterator, Schedule::Task_out_iterator>(t1, t2);
}

int Schedule::switch_processor(int curr_tier, bool restore) {
    Task curr_task;
    Proc curr_proc, new_proc;
    if (!restore) {
        curr_task = get_task_by_tier(curr_tier);
        curr_proc = get_proc_by_tier(curr_tier);
        new_proc = rand() % (proc_num - 1);
        new_proc = new_proc < curr_proc ? new_proc : new_proc + 1;
    } else {
        new_proc = last_operation.proc_switch.first;
        curr_proc = last_operation.proc_switch.second;
        curr_task = last_operation.last_task;
    }
    if (criterion == BF) {
        proc_load[curr_proc]--; // check BF trying to change schedule
        proc_load[new_proc]++;
        if (calculate_BF() >= BF_BOUND) {
            proc_load[curr_proc]++; // returns to old schedule
            proc_load[new_proc]--;
            return 1; // operation declined
        }
    } else if (criterion == CR) {
        int on_curr_proc = 0,
            on_new_proc = 0; // number of predeccesors and successors on current
                             // or new processor
        int new_double_transmitions = 0; // double transmitions

        for (std::pair<Schedule::Task_in_iterator, Schedule::Task_in_iterator>
                 predecessors_it = get_predecessors_of_task(curr_task);
             predecessors_it.first != predecessors_it.second;
             ++predecessors_it.first) {
            if (!predecessors_it.first.from_fictive()) {
                Task parent_task = *(predecessors_it.first);
                Proc parent_proc = get_proc_by_task(parent_task);
                if (parent_proc == curr_proc) {
                    on_curr_proc++;
                }
                if (parent_proc == new_proc) {
                    on_new_proc++;
                }
                if (is_direct_connection(parent_proc, curr_proc) &&
                    !is_direct_connection(parent_proc, new_proc)) {
                    new_double_transmitions++;
                } else if (!is_direct_connection(parent_proc, curr_proc) &&
                           is_direct_connection(parent_proc, new_proc)) {
                    new_double_transmitions--;
                }
            }
        }
        for (std::pair<Schedule::Task_out_iterator, Schedule::Task_out_iterator>
                 successors_it = get_successors_of_task(curr_task);
             successors_it.first != successors_it.second;
             ++successors_it.first) {
            if (!successors_it.first.to_fictive()) {
                Task child_task = *(successors_it.first);
                Proc child_proc = get_proc_by_task(child_task);
                if (child_proc == curr_proc) {
                    on_curr_proc++;
                }
                if (child_proc == new_proc) {
                    on_new_proc++;
                }
                if (is_direct_connection(child_proc, curr_proc) &&
                    !is_direct_connection(child_proc, new_proc)) {
                    new_double_transmitions++;
                } else if (!is_direct_connection(child_proc, curr_proc) &&
                           is_direct_connection(child_proc, new_proc)) {
                    new_double_transmitions--;
                }
            }
        }
        if (get_number_of_edges() != 0 &&
            (double)(transmitions - on_new_proc + on_curr_proc) /
                    get_number_of_edges() >=
                CR_BOUND &&
            (double)(double_transmitions + new_double_transmitions) /
                    get_number_of_edges() <
                CR2_BOUND) {
            return 1; // operation declined
        }
        transmitions = transmitions - on_new_proc + on_curr_proc;
        double_transmitions += new_double_transmitions;
    }
    set_task_on_proc(curr_task, new_proc);

    last_operation.operation_type = SWITCH_PROCESSOR;
    last_operation.proc_switch = {curr_proc, new_proc};
    last_operation.last_task = curr_task;
    return 0;
}

int Schedule::switch_tasks(int curr_tier, bool restore) {
    // if restore == true no matter what is curr_tier
    if (restore) {
        curr_tier = last_operation.task_switch.second;
    }
    Task curr_task = get_task_by_tier(curr_tier);
    Proc curr_proc = get_proc_by_task(curr_task);
    int min_tier = 0, max_tier = task_num - 1;
    if (!restore) {
        for (std::pair<Schedule::Task_in_iterator, Schedule::Task_in_iterator>
                 predecessors_it = get_predecessors_of_task(curr_task);
             predecessors_it.first != predecessors_it.second;
             ++predecessors_it.first) {
            Task parent_task = *(predecessors_it.first);
            int tier = get_tier_by_task(parent_task);
            min_tier = std::max(min_tier, tier + 1);
        }
        for (std::pair<Schedule::Task_out_iterator, Schedule::Task_out_iterator>
                 successors_it = get_successors_of_task(curr_task);
             successors_it.first != successors_it.second;
             ++successors_it.first) {
            Task child_task = *(successors_it.first);
            int tier = get_tier_by_task(child_task);
            max_tier = std::min(max_tier, tier - 1);
        }
        if (max_tier - min_tier == 0) {
            return 1;
        }
    }
    int new_tier;
    if (!restore) {
        new_tier = rand() % (max_tier - min_tier) + min_tier;
        new_tier = new_tier < curr_tier ? new_tier : new_tier + 1;
    } else {
        new_tier = last_operation.task_switch.first;
    }
    if (new_tier < curr_tier) {
        Task curr_task = get_task_by_tier(curr_tier);
        Task move_task = get_task_by_tier(new_tier);
        set_task_on_tier(curr_task, new_tier);
        for (int tier = new_tier + 1; tier <= curr_tier; ++tier) {
            curr_task = move_task;
            move_task = get_task_by_tier(tier);
            set_task_on_tier(curr_task, tier);
        }
    } else {
        Task curr_task = get_task_by_tier(curr_tier);
        Task move_task = get_task_by_tier(new_tier);
        set_task_on_tier(curr_task, new_tier);
        for (int tier = new_tier - 1; tier >= curr_tier; --tier) {
            curr_task = move_task;
            move_task = get_task_by_tier(tier);
            set_task_on_tier(curr_task, tier);
        }
    }

    last_operation.operation_type = SWITCH_TASK;
    last_operation.task_switch.first = curr_tier;
    last_operation.task_switch.second = new_tier;
    return 0;
}
int Schedule::transform(bool restore) {
    // Returns 1 if nothing changed
    int curr_tier = rand() % task_num;
    if (restore) {
        if (last_operation.operation_type == SWITCH_PROCESSOR) {
            return switch_processor(curr_tier, restore);
        } else {
            return switch_tasks(curr_tier, restore);
        }
    } else {
        if (rand() % 2) {
            // Operation 1
            // Returns 1 if nothing changed
            return switch_processor(curr_tier, restore);
        } else {
            // Operation 2
            // Returns 1 if nothing changed
            return switch_tasks(curr_tier);
        }
    }
}

class TimeDiagram {
    std::map<Schedule::Task, Schedule::Proc> procs;
    std::map<Schedule::Task, int> task_start;
    std::map<Schedule::Task, int> task_finish;

  public:
    TimeDiagram() = default;
    TimeDiagram(const Schedule &schedule);
    int get_time() const {
        int max_time = 0;
        for (auto &[task, time] : task_finish) {
            max_time = std::max(max_time, time);
        }
        return max_time;
    }
    std::map<Schedule::Task, Schedule::Proc> get_procs() const { return procs; }
    int get_task_start(const Schedule::Task &task) { return task_start[task]; }
    int get_task_finish(const Schedule::Task &task) {
        return task_finish[task];
    }
};

TimeDiagram::TimeDiagram(const Schedule &schedule) {
    const int task_num = schedule.get_task_num();
    const int proc_num = schedule.get_proc_num();
    std::map<Schedule::Proc, int>
        times; // current finish time of each processor
    for (int i = 0; i < task_num; ++i) { // i is current tier
        Schedule::Task curr_task = schedule.get_task_by_tier(i);
        Schedule::Proc curr_proc = schedule.get_proc_by_task(curr_task);
        int min_time = times[curr_proc]; // minimum time that is allowed for
                                         // task to start from
        for (std::pair<Schedule::Task_in_iterator, Schedule::Task_in_iterator>
                 predecessors_it = schedule.get_predecessors_of_task(curr_task);
             predecessors_it.first != predecessors_it.second;
             ++predecessors_it.first) {
            // pass through all parents to finds min_time
            Schedule::Task parent_task = *(predecessors_it.first);
            Schedule::Proc parent_proc = schedule.get_proc_by_task(parent_task);
            if (predecessors_it.first.from_fictive()) {
                if (parent_proc != curr_proc) {
                    int total_time = task_start[parent_task];
                    min_time = std::max(min_time, total_time);
                } else {
                    int total_time = task_finish[parent_task];
                    min_time = std::max(min_time, total_time);
                }
            } else {
                int total_time = task_finish[parent_task] +
                                 schedule.get_tran_time(parent_proc, curr_proc);
                min_time = std::max(min_time, total_time);
            }
        }
        procs[curr_task] = curr_proc;
        task_start[curr_task] = min_time;
        task_finish[curr_task] =
            min_time + schedule.get_task_time(curr_proc, curr_task);
        times[curr_proc] =
            min_time + schedule.get_task_time(curr_proc, curr_task);
    }
}
