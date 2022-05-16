#include "schedule/schedule.hpp"
#include <iostream>
#include <fstream>

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
    std::ifstream input;
    input.open("../input.txt");
    Schedule schedule = input_schedule(Schedule::NO, input);
    schedule.print_graph();
    input.close();
}