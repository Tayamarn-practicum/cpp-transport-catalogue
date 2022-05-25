#pragma once

#include <string>

#include "transport_catalogue.h"

namespace stat_reader {
    void PrintBus(std::ostream& _ostream, std::string& request, transport_catalogue::TransportCatalogue& tc);

    void PrintStop(std::ostream& _ostream, std::string& request, transport_catalogue::TransportCatalogue& tc);

    void ReadStat(std::istream& _istream, std::ostream& _ostream, transport_catalogue::TransportCatalogue& tc);
}
