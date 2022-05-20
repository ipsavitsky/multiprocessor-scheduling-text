#ifndef SCHEDULE_HPP
#define SCHEDULE_HPP

#include <algorithm>
#include <boost/graph/adjacency_list.hpp>
#include <boost/numeric/ublas/matrix.hpp>
#include <iostream>
#include <map>
#include <vector>

class Schedule {
  public:
    using Proc = int;

    // TODO: make this a enum class
    enum extra_criterion {
        NO,
        CR,
        BF,
    };

    // TODO: move to config file
    double BF_BOUND = 0.1;
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

    Graph graph;

    boost::numeric::ublas::matrix<int>
        task_times; // C (size: proc_num x task_num)
    boost::numeric::ublas::matrix<int>
        tran_times; // D (size: proc_num x proc_num)
                    // std::vector<std::vector<int>> task_times;
                    // std::vector<std::vector<int>> tran_times;

    boost::numeric::ublas::matrix<int> long_transmition;

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

    bool is_direct_connection(const Schedule::Proc &proc1,
                              const Schedule::Proc &proc2);

    void init_transmition_matrices(boost::numeric::ublas::matrix<int> tran);

    std::vector<Task> get_top_vertices();

    Schedule() = default;

    using edge_it = std::vector<std::pair<int, int>>::iterator;
    Schedule(edge_it edge_iterator_start, edge_it edge_iterator_end,
             int task_num, int proc_num,
             boost::numeric::ublas::matrix<int> &task_times,
             boost::numeric::ublas::matrix<int> &tran_times, int criteria = NO);

    Schedule(const Schedule &schedule);

    void create_fictive_node(std::vector<Task> D);

    void set_up_critical_paths();

    Task GC1(std::vector<Task> D);
};

#endif // SCHEDULE_HPP