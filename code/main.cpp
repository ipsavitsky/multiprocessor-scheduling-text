#include "logging/boost_logger.hpp"
#include "schedule/schedule.hpp"

#include <fstream>

Schedule input_schedule(int criteria, std::ifstream &input) {
    LOG_INFO << "Parsing input file";
    int task_num, proc_num;
    input >> task_num >> proc_num;
    std::vector<std::vector<int>> task_time(proc_num,
                                            std::vector<int>(task_num, 0)); // C
    for (int i = 0; i < task_time.size(); ++i) {
        for (int j = 0; j < task_time[i].size(); ++j) {
            input >> task_time[i][j];
        }
    }
    std::vector<std::vector<int>> tran_time(proc_num,
                                            std::vector<int>(proc_num, 0)); // D
    for (int i = 0; i < tran_time.size(); ++i) {
        for (int j = 0; j < tran_time[i].size(); ++j) {
            input >> tran_time[i][j];
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
                    tran_time, criteria);
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
    Schedule schedule = input_schedule(Schedule::NO, input);
    input.close();

    // schedule.print_graph();

    auto D = schedule.get_top_vertices();

    // std::for_each(D.begin(), D.end(), [](Schedule::Task task) {
    //     LOG_INFO << "Task " << task << " is top vertex";
    // });

    LOG_INFO << "D updated";

    schedule.create_fictive_node(D);

    LOG_INFO << "Fictive node created";

    schedule.print_graph(std::cout);

    schedule.calculate_critical_paths();

    return 0;
}