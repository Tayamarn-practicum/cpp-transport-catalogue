#include <iostream>

// #include "input_reader.h"
// #include "transport_catalogue.h"
// #include "stat_reader.h"
#include "json_reader.h"

// Comment these out because practicum's platform doesn't support custom files.
#include "tests.h"
// #include "log_duration.h"

int main() {
    tests::RunTests();
    std::cerr << "Tests OK!" << std::endl;

    transport_catalogue::TransportCatalogue tc;
    json_reader::ProcessInput(std::cin, std::cout, tc);

    // {
    //     // LOG_DURATION("Input");
    //     input_reader::ReadInput(std::cin, tc);
    // }

    // {
    //     // LOG_DURATION("Stat");
    //     stat_reader::ReadStat(std::cin, std::cout, tc);
    // }
    std::cerr << "OK!" << std::endl;
}
