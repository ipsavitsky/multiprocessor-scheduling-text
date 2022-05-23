#include "logging/boost_logger.hpp"
#include "schedule/schedule.hpp"
#include "time_schedule/time_schedule.hpp"

#include <boost/numeric/ublas/matrix.hpp>
#include <boost/program_options.hpp>

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

int main(int argc, char *argv[]) {

    boost::program_options::options_description desc("General options");
    std::string str_criteria;
    std::string filename;
    desc.add_options()("help,h", "Print help")(
        "input,i",
        boost::program_options::value<std::string>(&filename)->default_value(
            "../input.txt"),
        "Input file")("criteria,c",
                      boost::program_options::value<std::string>(&str_criteria)
                          ->default_value("NO"),
                      "Extra criteria for time schedule");
    boost::program_options::variables_map vm;
    boost::program_options::store(
        boost::program_options::parse_command_line(argc, argv, desc), vm);
    boost::program_options::notify(vm);

    if (vm.count("help")) {
        std::cout << desc << std::endl;
        return 0;
    }

    logger::debug = false;
    logger::init();

    LOG_INFO << "Starting";

    TimeSchedule::extra_criteria criteria;
    if (str_criteria == "NO") {
        criteria = TimeSchedule::extra_criteria::NO;
    } else if (str_criteria == "BF") {
        criteria = TimeSchedule::extra_criteria::BF;
    } else if (str_criteria == "CR") {
        criteria = TimeSchedule::extra_criteria::CR;
    } else {
        LOG_ERROR << "Unknown criteria: " << str_criteria;
        return 1;
    }

    std::ifstream input;
    LOG_INFO << "Opening file " << filename;
    input.open(filename);
    if (!input.is_open()) {
        LOG_ERROR << "Can't open file " << filename;
        return 1;
    }

    Schedule schedule = input_schedule(input);
    input.close();
    TimeSchedule time_schedule(schedule.get_proc_num());

    auto D = schedule.get_top_vertices();

    LOG_INFO << "D updated";

    schedule.create_fictive_node(D);

    LOG_INFO << "Fictive node created";

    schedule.set_up_critical_paths();

    LOG_INFO << "Calculated critical paths";

    schedule.hard_remove_fictive_vertices();

    // schedule.print_graph();

    while (!D.empty()) {
        auto chosen_task = schedule.GC1(D);
        LOG_INFO << "GC1 chosen " << chosen_task;
        Schedule::Proc chosen_proc;
        switch (criteria) {
        case TimeSchedule::extra_criteria::NO:
            chosen_proc = time_schedule.GC2(schedule, chosen_task);
            LOG_INFO << "GC2 chosen " << chosen_proc;
            time_schedule.add_task(schedule, chosen_task, chosen_proc);
            break;
        case TimeSchedule::extra_criteria::CR:
            throw std::runtime_error("Not implemented");
            break;
        case TimeSchedule::extra_criteria::BF:
            throw std::runtime_error("Not implemented"); 
            break;
        }
        // time_schedule.test_add_task(schedule, 4, 2);
        schedule.remove_vertex(chosen_task);
        D = schedule.get_top_vertices();
    }
    LOG_INFO << "final time is " << time_schedule.get_time();
    return 0;
}