#include <iostream>

#include "input_reader.h"
#include "transport_catalogue.h"
#include "stat_reader.h"

// Comment these out because practicum's platform doesn't support custom files.
// #include "tests.h"
// #include "log_duration.h"

int main() {
    // tests::RunTests();
    // std::cout << "Tests OK!" << std::endl;

    transport_catalogue::TransportCatalogue tc;

    {
        // LOG_DURATION("Input");
        input_reader::ReadInput(tc);
    }

    {
        // LOG_DURATION("Stat");
        stat_reader::ReadStat(tc);
    }
    // std::cout << "OK!" << std::endl;
}
