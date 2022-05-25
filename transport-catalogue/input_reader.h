#pragma once

#include <string>

#include "transport_catalogue.h"

namespace detail {
    // We have these functions here, because Yandex practicum doesn't support custom files.

    std::vector<std::string> SplitString(std::string str, char delimiter);

    // trim from start (in place)
    void ltrim(std::string &s);

    // trim from end (in place)
    void rtrim(std::string &s);

    // trim from both ends (in place)
    void trim(std::string &s);
}

namespace input_reader {
    void AddStop(std::string& request, transport_catalogue::TransportCatalogue& tc);

    void AddDist(std::string& request, transport_catalogue::TransportCatalogue& tc);

    void AddBus(std::string& request, transport_catalogue::TransportCatalogue& tc);

    void ReadInput(transport_catalogue::TransportCatalogue& tc);
}
