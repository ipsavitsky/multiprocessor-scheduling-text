#include "schedule/schedule.hpp"
#include "logging/boost_logger.hpp"

#include <fstream>

// move to public header of logging
#include <boost/log/sources/severity_logger.hpp>
#include <boost/log/sources/record_ostream.hpp>

Schedule input_schedule(int criterion, std::ifstream &input) {
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
                    tran_time, criterion);
}

int main() {

    init_logging();
    // severity_level should be in a namespace!
    boost::log::sources::severity_logger<severity_level> slg;

    BOOST_LOG_SEV(slg, normal) << "hooray!";
    std::ifstream input;
    input.open("../input.txt"); 
    Schedule schedule = input_schedule(Schedule::NO, input);
    schedule.print_graph();
    input.close();
}