#include "logging/boost_logger.hpp"
#include "schedule/schedule.hpp"
#include "time_schedule/time_schedule.hpp"

#include <boost/numeric/ublas/matrix.hpp>

#include <fstream>

Schedule input_schedule(std::ifstream &input) {
    LOG_INFO << "Parsing input file";
    int task_num, proc_num;
    input >> task_num >> proc_num;
    boost::numeric::ublas::matrix<int> task_time(proc_num, task_num); // C
    for (int i = 0; i < task_time.size1(); ++i) {
        for (int j = 0; j < task_time.size2(); ++j) {
            input >> task_time(i, j);
        }
    }
    boost::numeric::ublas::matrix<int> tran_time(proc_num, proc_num); // D
    for (int i = 0; i < tran_time.size1(); ++i) {
        for (int j = 0; j < tran_time.size2(); ++j) {
            input >> tran_time(i, j);
        }
    }
    std::vector<std::pair<int, int>> edges;
    for (int i = 0; i < task_num; ++i) {
        for (int j = 0; j < task_num; ++j) {
            int r;
            input >> r;
            if (r) {
                edges.push_back({i, j});
            }
        }
    }
    return Schedule(edges.begin(), edges.end(), task_num, proc_num, task_time,
                    tran_time);
}

int main() {
    logger::init();

    LOG_INFO << "Starting";

    std::ifstream input;
    std::string filename = "../input.txt";
    LOG_INFO << "Opening file " << filename;
    input.open(filename);
    if (!input.is_open()) {
        LOG_ERROR << "Can't open file " << filename;
        return 1;
    }
    Schedule schedule = input_schedule(input);
    input.close();

    auto D = schedule.get_top_vertices();

    LOG_INFO << "D updated";

    schedule.create_fictive_node(D);

    LOG_INFO << "Fictive node created";

    schedule.set_up_critical_paths();

    LOG_INFO << "Calculated critical paths";

    schedule.hard_remove_fictive_vertices();

    schedule.print_graph();

    auto chosen_task = schedule.GC1(D);

    LOG_INFO << "GC1 chose " << chosen_task;

    TimeSchedule time_schedule(schedule.get_proc_num());

    time_schedule.test_add_task(schedule, 4, 2);

    return 0;
}