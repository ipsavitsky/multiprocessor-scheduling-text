#include "schedule/schedule.hpp"
#include "schedule_io/schedule_io.hpp"
#include <iostream>
#include <fstream>

int main() {
    std::ifstream input;
    input.open("input.txt");
    Schedule schedule = input_schedule(Schedule::NO, input);
    schedule.print_graph();
    input.close();
}