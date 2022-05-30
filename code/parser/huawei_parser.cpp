#include "huawei_parser.hpp"
#include "../logging/boost_logger.hpp"

#include <boost/numeric/ublas/io.hpp>
#include <boost/numeric/ublas/matrix.hpp>

#include <filesystem>
#include <fstream>

/**
 * @brief Generate a schedule from a file in Huawei format for data class 1
 *
 * This function expects that the provided directory contains
 * 1. `<directory_name>.txt` - a file with amount of tasks and edges + all edges,
 * 2. `<directory_name>_dly.txt` - a file with \f$D_{ij}\f$ matrix,
 * 3. `<directory_name>_com.txt` - a file with task execution vector. It is assumed that all processors are equal in power.
 *
 * @param path Path to the directory
 * @return Schedule
 */
Schedule input_schedule_class_1(std::string path) {
    std::filesystem::path p(path);
    if (!std::filesystem::is_directory(p)) {
        throw std::runtime_error("Not a directory");
    }
    LOG_DEBUG << "path: " << p;
    std::filesystem::path last;
    last = p.filename();
    if (last == "") {
        last = p.parent_path().filename();
    }
    LOG_DEBUG << "last: " << last;

    std::string base = last.string() + ".txt";
    std::string dly = last.string() + "_dly.txt";
    std::string com = last.string() + "_com.txt";

    std::size_t task_num = 0;
    std::size_t proc_num = 0;
    std::size_t edge_num = 0;

    std::vector<Schedule::Edge> edges;
    boost::numeric::ublas::matrix<int> task_time;
    boost::numeric::ublas::matrix<int> tran_time;

    if (std::filesystem::exists(p / base)) {
        LOG_INFO << base << " found";
        std::ifstream base_stream(p / base);
        base_stream >> task_num >> edge_num;
        for (int i = 0; i < edge_num; ++i) {
            int a, b;
            base_stream >> a >> b;
            edges.push_back({a, b});
        }
        LOG_DEBUG << "there are " << edges.size() << " edges";
        base_stream.close();
    }
    if (std::filesystem::exists(p / dly)) {
        LOG_INFO << dly << " found";
        std::ifstream dly_stream(p / dly);
        std::string garbage;
        // this is too evil but there is no other way
        for (; std::getline(dly_stream, garbage); proc_num++)
            ;
        LOG_DEBUG << "proc_num: " << proc_num;
        dly_stream.clear();
        dly_stream.seekg(0, std::ios_base::beg);
        tran_time.resize(proc_num, proc_num);
        for (int i = 0; i < proc_num; ++i) {
            for (int j = 0; j < proc_num; ++j) {
                dly_stream >> tran_time(i, j);
            }
        }
        dly_stream.close();
        LOG_DEBUG << tran_time;
    }
    if (std::filesystem::exists(p / com)) {
        LOG_INFO << com << " found";
        std::ifstream com_stream(p / com);
        task_time.resize(proc_num, task_num);
        int tmp;
        for (int i = 0; i < task_num; ++i) {
            com_stream >> tmp;
            for (int j = 0; j < proc_num; ++j) {
                task_time(j, i) = tmp;
            }
        }
        com_stream.close();
        LOG_DEBUG << task_time;
    }

    return Schedule(edges, task_time, tran_time);
}

/**
 * @brief Generate a schedule from a file in Huawei format for data class 2
 * 
 * This function expects that the provided directory contains
 * 1. `<directory_name>.txt` - a file with amount of tasks and edges + all edges,
 * 2. `<directory_name>_dly.txt` - a file with \f$D_{ij}\f$ matrix,
 *
 * This data class presumes that all tasks are equal in processing requirements, therefore \f$C_{ij} \f$ matrix is \f$proc\_num \cdot task\_num \f$ matrix of ones.
 * 
 * @bug Because of the way it handles \f$C\f$ matrix, it is not possible to load `DAG_D` yet because of its size.
 *  
 * @param path Path to directory
 * @return Schedule 
 */
Schedule input_schedule_class_2(std::string path) {
    std::filesystem::path p(path);
    if (!std::filesystem::is_directory(p)) {
        throw std::runtime_error("Not a directory");
    }
    LOG_DEBUG << "path: " << p;
    std::filesystem::path last;
    last = p.filename();
    if (last == "") {
        last = p.parent_path().filename();
    }
    LOG_DEBUG << "last: " << last;

    std::string base = last.string() + ".txt";
    std::string dly = last.string() + "_dly.txt";
    std::string com = last.string() + "_com.txt";

    std::size_t task_num = 0;
    std::size_t proc_num = 0;
    std::size_t edge_num = 0;

    std::vector<Schedule::Edge> edges;
    boost::numeric::ublas::matrix<int> tran_time;
    boost::numeric::ublas::matrix<int> task_time;

    if (std::filesystem::exists(p / base)) {
        LOG_INFO << base << " found";
        std::ifstream base_stream(p / base);
        base_stream >> task_num >> edge_num;
        for (int i = 0; i < edge_num; ++i) {
            int a, b;
            base_stream >> a >> b;
            edges.push_back({a, b});
        }
        LOG_DEBUG << "there are " << edges.size() << " edges";
        base_stream.close();
    }
    if (std::filesystem::exists(p / dly)) {
        LOG_INFO << dly << " found";
        std::ifstream dly_stream(p / dly);
        std::string garbage;
        // this is too evil but there is no other way
        for (; std::getline(dly_stream, garbage); proc_num++)
            ;
        LOG_DEBUG << "proc_num: " << proc_num;
        dly_stream.clear();
        dly_stream.seekg(0, std::ios_base::beg);
        tran_time.resize(proc_num, proc_num);
        for (int i = 0; i < proc_num; ++i) {
            for (int j = 0; j < proc_num; ++j) {
                dly_stream >> tran_time(i, j);
            }
        }
        dly_stream.close();
        // LOG_DEBUG << tran_time;
    }

    task_time.resize(proc_num, task_num);
    std::fill(task_time.data().begin(), task_time.data().end(), 1);
    // LOG_DEBUG << task_time;
    return Schedule(edges, task_time, tran_time);
}