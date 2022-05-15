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
    enum operation_types {
        SWITCH_PROCESSOR,
        SWITCH_TASK,
    };
    enum extra_criterion {
        NO,
        CR,
        BF,
    };

    double BF_BOUND = 0.1; // move to config file later
    double CR_BOUND = 0.4;
    double CR2_BOUND = 0.05;

  private:
    struct proc_t {
        typedef boost::vertex_property_tag kind;
    };
    struct tier_t {
        typedef boost::vertex_property_tag kind;
    };
    struct edge_t {
        typedef boost::edge_property_tag kind;
    };

    using TierProperty = boost::property<tier_t, int>;
    using ProcProperty = boost::property<proc_t, Proc, TierProperty>;
    using EdgeProperty = boost::property<edge_t, bool>;

    using Graph =
        boost::adjacency_list<boost::vecS, boost::vecS, boost::bidirectionalS,
                              ProcProperty, EdgeProperty>;
    using GraphTraits = boost::graph_traits<Graph>;

  public:
    using Task = Graph::vertex_descriptor;

  private:
    int task_num;
    int proc_num;
    int criterion{NO};
    std::map<Proc, int> proc_load; // for BF criterion
    int transmitions{0};           // for CR criterion
    int double_transmitions{0};    // for CR2 in CR criterion
    int non_fictive_edges{0};
    int lower_bound;

    Graph graph; // Tasks numerates from 0 to task_num - 1
                 // Processors numerates from 0 to proc_num - 1
    // boost::property_map<Graph, proc_t>::type procs;  // task i assigned on
    // processor procs[i] (length of procs is task_num)
    std::vector<Task>
        task_by_tier; // Task task_by_tier[i] on tier i
                      // Must be changed along with TierProperty of Task
    // boost::property_map<Graph, tier_t>::type tiers;  // task i assigned on
    // tier tiers[i]

    std::vector<std::vector<int>> task_times; // C (size: proc_num x task_num)
    std::vector<std::vector<int>> tran_times; // D (size: proc_num x proc_num)
    std::vector<std::vector<int>>
        long_transmition; // (size: proc_num x proc_num)
                          // long_transmition[i][j] = -1  if i and j are
                          // directly connected processors
                          // long_transmition[i][j] = k   if i -> k -> j is the
                          // best path
    struct LastOperation {
        int operation_type;
        std::pair<Proc, Proc> proc_switch; // SWITCH_PROCESSOR
        Task last_task;                    // SWITCH_PROCESSOR
        std::pair<int, int> task_switch;   // SWITCH_TASK
    } last_operation;

  public:
    void print_graph();

    void set_task_on_tier(Task &task, int &tier);

    void set_task_on_proc(Task &task, Proc &proc);

    int get_task_num() const;

    int get_proc_num() const;

    Task get_task_by_tier(const int &tier) const;

    int get_tier_by_task(const Task &task) const;

    Proc get_proc_by_task(const Task &task) const;

    Proc get_proc_by_tier(const int &tier) const;

    int get_tran_time(const Proc &from, const Proc &to) const;

    int get_task_time(const Proc &proc, const Task &task) const;

    int get_out_degree(const Task &task) const;

    int get_in_degree(const Task &task) const;

    int get_number_of_edges() const;

    void add_fictive_edge(const Task &t1, const Task &t2);

    bool is_direct_connection(const Proc &proc1, const Proc &proc2);

    double calculate_BF();

    double calculate_CR() const;
    double calculate_CR2() const;
    void set_lower_bound(int bound);
    int get_lower_bound() const;

    void init_tiers_by_topologic();
    void init_procs();
    void init_tiers_procs();
    void init_transmition_matrices(std::vector<std::vector<int>> tran);

    Schedule() = default;
    template <typename edge_it>
    Schedule(edge_it edge_iterator_start, edge_it edge_iterator_end,
             int task_num, int proc_num,
             std::vector<std::vector<int>> &task_times,
             std::vector<std::vector<int>> &tran_times, int criterion = NO);

    Schedule(const Schedule &schedule);

    void reinitiate();

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
        bool from_fictive();
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
        bool to_fictive();
    };

    std::pair<Task_in_iterator, Task_in_iterator>
    get_predecessors_of_task(Task task) const;

    std::pair<Task_out_iterator, Task_out_iterator>
    get_successors_of_task(Task task) const;

    int switch_processor(int curr_tier, bool restore = false);
    int switch_tasks(int curr_tier, bool restore = false);
    int transform(bool restore = false);
};