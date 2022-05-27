#include "schedule.hpp"
#include "../logging/boost_logger.hpp"

#include <boost/graph/dijkstra_shortest_paths.hpp>
#include <boost/graph/graph_traits.hpp>
#include <boost/graph/topological_sort.hpp>
#include <boost/property_map/property_map.hpp>

#include <algorithm>
#include <iostream>
#include <set>

/**
 * @brief Output graph to stdout
 */
void Schedule::print_graph() const {
    auto fictiveness = get(&VertexData::is_fictive, graph);
    auto sh_path = get(&VertexData::shortest_path_length, graph);
    for (Task curr_task = 0; curr_task < task_num; ++curr_task) {
        std::cout << std::boolalpha << "from: " << curr_task << " ("
                  << fictiveness[curr_task] << "; " << sh_path[curr_task] << ')'
                  << std::noboolalpha << std::endl;
        for (auto edges = boost::out_edges(curr_task, graph);
             edges.first != edges.second; ++edges.first) {
            Task child_task = boost::target(*edges.first, graph);
            std::cout << "to: " << child_task << std::endl;
        }
    }
}

/**
 * @brief Get data transfer time between two processors
 *
 * @param from Source processor
 * @param to Destination processor
 * @return Time to transfer data from `from` to `to`
 */
int Schedule::get_tran_time(const Proc &from, const Proc &to) const {
    return tran_times(from, to);
}

/**
 * @brief Get amount of vertices in task graph
 *
 * @return Amount of tasks
 */
int Schedule::get_task_num() const { return task_num; }

/**
 * @brief Get amount of processors
 *
 * @return Amount of processors
 */
int Schedule::get_proc_num() const { return proc_num; }

/**
 * @brief Get time that it takes to execute a task on a processor
 *
 * @param proc Processor on which to execute task
 * @param task Task to execute
 * @return int
 */
int Schedule::get_task_time(const Schedule::Proc &proc,
                            const Schedule::Task &task) const {
    return task_times(proc, task);
}

/**
 * @brief Get amount of child tasks
 *
 * @param task Source task
 * @return int
 */
int Schedule::get_out_degree(const Schedule::Task &task) const {
    return boost::out_degree(task, graph);
}

/**
 * @brief Get amount of parent tasks
 *
 * @param task Child task
 * @return int
 */
int Schedule::get_in_degree(const Schedule::Task &task) const {
    return boost::in_degree(task, graph);
}

/**
 * @brief Get number of edges in task graph
 *
 * @return int
 */
int Schedule::get_number_of_edges() const { return edges; }

/**
 * @brief Check if the connection between two processors is direct
 *
 * @param proc1 Source processor
 * @param proc2 Destination processor
 *
 * @return `true` if direct connection
 * @return `false` if connected through third processor
 */
bool Schedule::is_direct_connection(const Schedule::Proc &proc1,
                                    const Schedule::Proc &proc2) const {
    return long_transmition(proc1, proc2) == -1;
}

/**
 * @brief Initialize transmittion matrices. Fill in connections with through
 * third processors
 *
 * @param tran Transmittion matrix `D`
 */
void Schedule::init_transmition_matrices(
    boost::numeric::ublas::matrix<int> tran) {
    tran_times = tran;
    long_transmition.resize(tran.size1(), tran.size2(), false);
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

/**
 * @brief Construct a new Schedule object
 *
 * @param edge_vec Start of edge iterator
 * @param task_times Task completion matrix \f$ C \f$
 * @param tran_times Transmittion matrix \f$ D \f$
 */
Schedule::Schedule(std::vector<Edge> &edge_vec,
                   boost::numeric::ublas::matrix<int> &task_times,
                   boost::numeric::ublas::matrix<int> &tran_times) {
    this->task_num = task_times.size2();
    this->proc_num = tran_times.size1();
    graph = Graph(edge_vec.begin(), edge_vec.end(), task_num);
    edges = boost::num_edges(graph);
    this->task_times = task_times;
    init_transmition_matrices(tran_times);
}

/**
 * @brief Copy constructor of Schedule
 *
 * @param schedule The other schedule
 */
Schedule::Schedule(const Schedule &schedule) {
    task_num = schedule.task_num;
    proc_num = schedule.proc_num;
    graph = schedule.graph;
    task_times = schedule.task_times;
    tran_times = schedule.tran_times;
    long_transmition = schedule.long_transmition;
    edges = schedule.edges;
}

/**
 * @brief Get all vertices that have no parent nodes, which means that they are
 * ready to be added to the schedule
 *
 * @return std::vector<Schedule::Task>
 */
std::vector<Schedule::Task> Schedule::get_top_vertices() {
    std::vector<Schedule::Task> top_vertices;
    for (Schedule::Task task = 0; task < task_num; ++task) {
        bool has_real_parents = false;
        for (auto edges = boost::in_edges(task, graph);
             edges.first != edges.second; ++edges.first) {
            auto src = boost::source(*edges.first, graph);
            if (graph[src].is_existent == true) {
                has_real_parents = true;
                break;
            }
        }
        if (!has_real_parents && graph[task].is_existent == true)
            top_vertices.push_back(task);
    }
    return top_vertices;
}

/**
 * @brief Create a fictive node
 *
 * @param D Nodes that have to be child nodes of a fictive node
 */
void Schedule::create_fictive_node(std::vector<Task> D) {
    auto new_vert = add_vertex({0, true, true}, graph);
    std::for_each(D.begin(), D.end(), [&](Task task) {
        LOG_DEBUG << "Adding edge from " << new_vert << " to " << task;
        add_edge(new_vert, task, {0}, graph);
    });
    ++task_num;
}

/**
 * @brief Calculate critical paths
 *
 * This functions does several things:
 *  1. For each edge set its property to minimum execution time of the source
 * task on any processor
 *  2. Run Dijkstra's algorithm on the graph to calculate critical paths
 *
 * @todo change `std::numeric_limits<int>::max()` to something more reasonable
 * @todo change calculating minimum to matrix-aware code
 */
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

/**
 * @brief Remove all fictive vertices from graph
 *
 */
void Schedule::hard_remove_fictive_vertices() {
    for (Task task = 0; task < task_num; ++task) {
        if (graph[task].is_fictive) {
            LOG_DEBUG << "removing fictive node " << task;
            remove_vertex(task);
        }
    }
}

/**
 * @brief Remove a vertex from the graph
 *
 * @param task
 */
void Schedule::remove_vertex(const Task &task) {
    graph[task].is_existent = false;
    // boost::clear_vertex(task, graph);
    // boost::remove_vertex(task, graph);
    // --task_num;
}

/**
 * @brief Calculate GC1(greedy criteria 1)
 *
 * Greedy criteria 1 is chooses the task from \f$ D \f$ that has the highest
 * `out_degree`
 *
 * @param D \f$ D \f$ set calculated by `get_top_vertices()`
 * @return Schedule::Task
 */
Schedule::Task Schedule::GC1(std::vector<Task> D) {
    return *std::max_element(D.begin(), D.end(), [&](Task task1, Task task2) {
        return boost::out_degree(task1, graph) <
               boost::out_degree(task2, graph);
    });
}

/**
 * @brief Get all edges that have the set task as target
 *
 * @param task Task to get edges for
 * @return std::pair<boost::graph_traits<Schedule::Graph>::in_edge_iterator,
 * boost::graph_traits<Schedule::Graph>::in_edge_iterator>
 */
std::pair<boost::graph_traits<Schedule::Graph>::in_edge_iterator,
          boost::graph_traits<Schedule::Graph>::in_edge_iterator>
Schedule::get_in_edges(const Task &task) const {
    return boost::in_edges(task, graph);
}

/**
 * @brief Get the whole underlying graph
 * @todo Remove in future versions
 *
 * @return Schedule::Graph
 */
const Schedule::Graph &Schedule::get_graph() const { return graph; }