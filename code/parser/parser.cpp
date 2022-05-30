#include "../logging/boost_logger.hpp"
#include "huawei_parser.hpp"

#include <boost/numeric/ublas/io.hpp>
#include <boost/numeric/ublas/matrix.hpp>

#include <filesystem>
#include <fstream>

/**
 * @brief Upload a schedule from file
 *
 * The format of the file is the following: \n
 * {task_num} {proc_num} {edge_num} \n
 * {C_ij matrix, proc_num x task_num} \n
 * {D_ij matrix, proc_num x proc_num} \n
 * {Pairs of edges are separated by a space, edge_num x 2}
 *
 * @param path Path to file
 * @return Schedule
 */
Schedule input_schedule_regular(std::string path) {
    std::filesystem::path p(path);
    if (!std::filesystem::is_regular_file(p)) {
        throw std::runtime_error("Not a regular file");
    }
    LOG_DEBUG << "path: " << p;

    std::ifstream input(p);

    int task_num, proc_num, edge_num;
    input >> task_num >> proc_num >> edge_num;
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
    std::vector<Schedule::Edge> edges;
    for (int i = 0; i < edge_num; ++i) {
        int a, b;
        input >> a >> b;
        edges.push_back({a, b});
    }
    return Schedule(edges, task_time, tran_time);
}