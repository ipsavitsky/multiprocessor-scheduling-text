#ifndef SCHEDULE_HPP
#define SCHEDULE_HPP

#include <algorithm>
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/graph_traits.hpp>
#include <boost/graph/topological_sort.hpp>
#include <boost/property_map/property_map.hpp>
#include <iostream>
#include <set>
#include <vector>

class Schedule {
  public:
    using Proc = int;

    enum extra_criterion {
        NO,
        CR,
        BF,
    };

    double BF_BOUND = 0.1; // move to config file later
    double CR_BOUND = 0.4;
    double CR2_BOUND = 0.05;

    struct VertexData {
        Proc proc;
        int shortest_path_length;
        bool is_fictive = false;
    };

    struct EdgeData {
        int min_time = 0;
    };

    using Graph =
        boost::adjacency_list<boost::vecS, boost::vecS, boost::bidirectionalS,
                              VertexData, EdgeData>;
    using GraphTraits = boost::graph_traits<Graph>;

    using Task = Graph::vertex_descriptor;

  private:
    int task_num;
    int proc_num;
    int criteria{NO};
    std::map<Proc, int> proc_load; // for BF criteria
    int transmitions{0};           // for CR criteria
    int double_transmitions{0};    // for CR2 in CR criteria
    int edges{0};

    std::vector<std::size_t> critical_paths;

    Graph graph; // Tasks numerates from 0 to task_num - 1
                 // Processors numerates from 0 to proc_num - 1

    std::vector<std::vector<int>> task_times; // C (size: proc_num x task_num)
    std::vector<std::vector<int>> tran_times; // D (size: proc_num x proc_num)
    std::vector<std::vector<int>>
        long_transmition; // (size: proc_num x proc_num)
                          // long_transmition[i][j] = -1  if i and j are
                          // directly connected processors
                          // long_transmition[i][j] = k   if i -> k -> j is the
                          // best path

  public:
    void print_graph();

    void set_task_on_proc(Task &task, Proc &proc);

    int get_task_num() const;

    int get_proc_num() const;

    Proc get_proc_by_task(const Task &task) const;

    int get_tran_time(const Proc &from, const Proc &to) const;

    int get_task_time(const Proc &proc, const Task &task) const;

    int get_out_degree(const Task &task) const;

    int get_in_degree(const Task &task) const;

    int get_number_of_edges() const;

    void remove_fictive_vertices();

    void remove_vertex(const Task &task);

    bool is_direct_connection(const Proc &proc1, const Proc &proc2);

    double calculate_BF();

    double calculate_CR() const;
    double calculate_CR2() const;

    void init_transmition_matrices(std::vector<std::vector<int>> tran);

    std::vector<Task> get_top_vertices();

    Schedule() = default;

    using edge_it = std::vector<std::pair<int, int>>::iterator;
    Schedule(edge_it edge_iterator_start, edge_it edge_iterator_end,
             int task_num, int proc_num,
             std::vector<std::vector<int>> &task_times,
             std::vector<std::vector<int>> &tran_times, int criteria = NO);

    Schedule(const Schedule &schedule);

    class Task_in_iterator { // doesn't change anything in graph
        const Graph &graph;
        GraphTraits::in_edge_iterator
            in; // better be reference, but '!=' is not working because of local
                // objects (can't return it from get_predecessors_of_task)
      public:
        Task_in_iterator(Task_in_iterator &it) : graph(it.graph), in(it.in) {}
        Task_in_iterator(GraphTraits::in_edge_iterator &in_edge,
                         const Graph &in_graph)
            : in(in_edge), graph(in_graph) {}

        Task operator*();
        Task_in_iterator &operator++();
        bool operator!=(const Task_in_iterator &rhs);
    };

    class Task_out_iterator { // doesn't change anything in graph
        const Graph &graph;
        GraphTraits::out_edge_iterator out;

      public:
        Task_out_iterator(Task_out_iterator &it)
            : graph(it.graph), out(it.out) {}
        Task_out_iterator(GraphTraits::out_edge_iterator &out_edge,
                          const Graph &out_graph)
            : graph(out_graph), out(out_edge) {}
        Task operator*();
        Task_out_iterator &operator++();
        bool operator!=(const Task_out_iterator &rhs);
    };

    std::pair<Task_in_iterator, Task_in_iterator>
    get_predecessors_of_task(Task task) const;

    std::pair<Task_out_iterator, Task_out_iterator>
    get_successors_of_task(Task task) const;

    void create_fictive_node(std::vector<Task> D);

    void set_up_critical_paths();

    Task GC1(std::vector<Task> D);
};

#endif // SCHEDULE_HPP