#include "schedule.hpp"
#include "../logging/boost_logger.hpp"
#include <algorithm>
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/dijkstra_shortest_paths.hpp>
#include <boost/graph/graph_traits.hpp>
#include <boost/graph/topological_sort.hpp>
#include <boost/property_map/property_map.hpp>
#include <iostream>
#include <set>
#include <vector>

void Schedule::print_graph() {
    auto fictiveness = get(&VertexData::is_fictive, graph);
    auto sh_path = get(&VertexData::shortest_path_length, graph);
    for (Task curr_task = 0; curr_task < task_num; ++curr_task) {
        std::cout << "from: " << curr_task << '(' << fictiveness[curr_task]
                  << "; " << sh_path[curr_task] << ')' << std::endl;
        for (auto edges = boost::out_edges(curr_task, graph);
             edges.first != edges.second; ++edges.first) {
            Task child_task = boost::target(*edges.first, graph);
            std::cout << "to: " << child_task << std::endl;
        }
    }
}

void Schedule::set_task_on_proc(Task &task, Proc &proc) {
    graph[task].proc = proc;
}

int Schedule::get_tran_time(const Proc &from, const Proc &to) const {
    // TODO: unlinked processors
    return tran_times(from, to);
}

int Schedule::get_task_num() const { return task_num; }

int Schedule::get_proc_num() const { return proc_num; }

int Schedule::get_task_time(const Schedule::Proc &proc,
                            const Schedule::Task &task) const {
    return task_times(proc, task);
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
    return long_transmition(proc1, proc2) == -1;
}

void Schedule::init_transmition_matrices(
    boost::numeric::ublas::matrix<int> tran) {
    tran_times = tran;
    long_transmition.resize(tran.size1(), tran.size2());
    for (int i = 0; i < tran.size1(); ++i) {
        for (int j = 0; j < tran.size2(); ++j) {
            if (tran(i, j) == -1) {
                for (int k = 0; k < tran.size1(); ++k) {
                    if (tran(i, k) != -1 && tran(k, j) != -1 &&
                        (tran_times(i, j) == -1 ||
                         tran(i, k) + tran(k, j) < tran_times(i, j))) {
                        tran_times(i, j) = tran(i, k) + tran(k, j);
                        long_transmition(i, j) = k;
                    }
                }
            } else {
                long_transmition(i, j) = -1;
            }
        }
    }
}

Schedule::Schedule(Schedule::edge_it edge_iterator_start,
                   Schedule::edge_it edge_iterator_end, int task_num,
                   int proc_num, boost::numeric::ublas::matrix<int> &task_times,
                   boost::numeric::ublas::matrix<int> &tran_times,
                   int criteria) {
    this->task_num = task_num;
    this->proc_num = proc_num;
    graph = Graph(edge_iterator_start, edge_iterator_end, task_num);
    edges = boost::num_edges(graph);
    this->task_times = task_times;
    init_transmition_matrices(tran_times);
    this->criteria = criteria;
}

Schedule::Schedule(const Schedule &schedule) {
    task_num = schedule.task_num;
    proc_num = schedule.proc_num;
    graph = schedule.graph;
    task_times = schedule.task_times;
    tran_times = schedule.tran_times;
    long_transmition = schedule.long_transmition;
    criteria = schedule.criteria;
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

std::vector<Schedule::Task> Schedule::get_top_vertices() {
    std::vector<Schedule::Task> top_vertices;
    for (Schedule::Task task = 0; task < task_num; ++task) {
        if (boost::in_degree(task, graph) == 0) {
            top_vertices.push_back(task);
        }
    }
    return top_vertices;
}

void Schedule::create_fictive_node(std::vector<Task> D) {
    auto new_vert = add_vertex({0, 0, true}, graph);
    std::for_each(D.begin(), D.end(), [&](Task task) {
        LOG_DEBUG << "Adding edge from " << new_vert << " to " << task;
        add_edge(new_vert, task, {0}, graph);
    });
    ++task_num;
}

void Schedule::set_up_critical_paths() {
    Task fictive_node;

    for (Task curr_task = 0; curr_task < task_num; ++curr_task) {
        auto out_edges = boost::out_edges(curr_task, graph);
        // TODO: change this!!!
        size_t min_time = std::numeric_limits<size_t>::max();
        if (graph[curr_task].is_fictive) {
            min_time = 0;
            fictive_node = curr_task;
        } else {
            for (auto i = 0; i < proc_num; ++i) {
                if (task_times(i, curr_task) < min_time) {
                    min_time = task_times(i, curr_task);
                }
            }
        }
        LOG_DEBUG << "min time on " << curr_task << " is " << min_time;
        std::for_each(out_edges.first, out_edges.second,
                      [&](auto edge) { graph[edge].min_time = min_time; });
    }
    LOG_DEBUG << "fictive node is " << fictive_node;
    boost::dijkstra_shortest_paths(
        graph, fictive_node,
        distance_map(get(&VertexData::shortest_path_length, graph))
            .weight_map(get(&EdgeData::min_time, graph)));
}

void Schedule::remove_fictive_vertices() {
    for (Task task = 0; task < task_num; ++task) {
        if (graph[task].is_fictive) {
            LOG_DEBUG << "removing fictive node " << task;
            remove_vertex(task);
        }
    }
}

void Schedule::remove_vertex(const Task &task) {
    boost::clear_vertex(task, graph);
    boost::remove_vertex(task, graph);
    --task_num;
}

Schedule::Task Schedule::GC1(std::vector<Task> D) {
    return *std::max_element(D.begin(), D.end(), [&](Task task1, Task task2) {
        return boost::out_degree(task1, graph) <
               boost::out_degree(task2, graph);
    });
}