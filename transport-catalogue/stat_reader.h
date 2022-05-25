#pragma once

#include <string>

#include "transport_catalogue.h"

namespace stat_reader {
    void PrintBus(std::string& request, transport_catalogue::TransportCatalogue& tc);

    void PrintStop(std::string& request, transport_catalogue::TransportCatalogue& tc);

    void ReadStat(transport_catalogue::TransportCatalogue& tc);
}
