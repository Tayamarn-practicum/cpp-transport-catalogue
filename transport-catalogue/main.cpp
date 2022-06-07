#include <iostream>

#include "json_reader.h"

// Comment these out because practicum's platform doesn't support custom files.
#include "tests.h"
// #include "log_duration.h"

int main() {
    tests::RunTests();
    std::cerr << "Tests OK!" << std::endl;

    // LOG_DURATION("Test");
    transport_catalogue::TransportCatalogue tc;
    json_reader::ProcessInput(std::cin, std::cout, tc);

    std::cerr << "OK!" << std::endl;
}
