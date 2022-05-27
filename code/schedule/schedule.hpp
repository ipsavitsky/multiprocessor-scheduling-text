#ifndef SCHEDULE_HPP
#define SCHEDULE_HPP

#include <algorithm>
#include <boost/graph/adjacency_list.hpp>
#include <boost/numeric/ublas/matrix.hpp>
#include <iostream>
#include <map>
#include <vector>

/**
 * @brief Schedule class
 *
 * Contains a task graph, \f$C\f$ and \f$D\f$ matrices
 *
 */
class Schedule {
  public:
    /**
     * @brief Type used to represent a processor
     *
     */
    using Proc = size_t;

    /**
     * @brief Internal type used to represent data in a vertex
     *
     */
    struct VertexData {
        /** Shortest path from a fictive root to this vertex */
        int shortest_path_length;
        /** Whether this vertex is fictive */
        bool is_fictive = false;
        /** Existent vertex */
        bool is_existent = true;
    };

    /**
     * @brief Internal type used to represent data in an edge
     *
     */
    struct EdgeData {
        /** Minimum time of the parent vertex. Used in Dijksta's algorithm */
        int min_time = 0;
    };

    /**
     * @brief Boost::Graph graph type shortened
     *
     */
    using Graph =
        boost::adjacency_list<boost::vecS, boost::vecS, boost::bidirectionalS,
                              VertexData, EdgeData>;

    /**
     * @brief Internal type used to represent a task
     *
     */
    using Task = Graph::vertex_descriptor;

  private:
    using edge_it = std::vector<std::pair<int, int>>::iterator;
    int task_num;
    int proc_num;
    int edges{0};

    Graph graph;

    boost::numeric::ublas::matrix<int>
        task_times; // C (size: proc_num x task_num)
    boost::numeric::ublas::matrix<int>
        tran_times; // D (size: proc_num x proc_num)

    boost::numeric::ublas::matrix<int> long_transmition;

  public:
    void print_graph();

    int get_task_num() const;

    int get_proc_num() const;

    int get_tran_time(const Proc &from, const Proc &to) const;

    int get_task_time(const Proc &proc, const Task &task) const;

    int get_out_degree(const Task &task) const;

    int get_in_degree(const Task &task) const;

    // TODO: REMOVE THIS IN THE FUTURE
    const Graph &get_graph() const;

    int get_number_of_edges() const;

    void hard_remove_fictive_vertices();

    std::pair<boost::graph_traits<Graph>::in_edge_iterator,
              boost::graph_traits<Graph>::in_edge_iterator>
    get_in_edges(const Task &task) const;

    void remove_vertex(const Task &task);

    bool is_direct_connection(const Schedule::Proc &proc1,
                              const Schedule::Proc &proc2) const;

    void init_transmition_matrices(boost::numeric::ublas::matrix<int> tran);

    std::vector<Task> get_top_vertices();

    Schedule() = default;

    Schedule(edge_it edge_iterator_start, edge_it edge_iterator_end,
             int task_num, int proc_num,
             boost::numeric::ublas::matrix<int> &task_times,
             boost::numeric::ublas::matrix<int> &tran_times);

    Schedule(const Schedule &schedule);

    void create_fictive_node(std::vector<Task> D);

    void set_up_critical_paths();

    Task GC1(std::vector<Task> D);
};

#endif // SCHEDULE_HPP