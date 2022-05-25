#include "input_reader.h"

#include <algorithm>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#include "transport_catalogue.h"

// Comment this out because practicum's platform doesn't support custom files.
// #include "log_duration.h"

namespace detail {
    // We have these functions here, because Yandex practicum doesn't support custom files.
    // Ideally they should be in a separate file.

    std::vector<std::string> SplitString(const std::string& str, char delimiter) {
        std::stringstream ss(str);
        std::string segment;
        std::vector<std::string> seglist;

        while(std::getline(ss, segment, delimiter))
        {
           seglist.push_back(segment);
        }
        return seglist;
    }

    // trim from start (in place)
    void ltrim(std::string &s) {
        s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](unsigned char ch) {
            return !std::isspace(ch);
        }));
    }

    // trim from end (in place)
    void rtrim(std::string &s) {
        s.erase(std::find_if(s.rbegin(), s.rend(), [](unsigned char ch) {
            return !std::isspace(ch);
        }).base(), s.end());
    }

    // trim from both ends (in place)
    void trim(std::string &s) {
        ltrim(s);
        rtrim(s);
    }
}

namespace input_reader {
    using namespace detail;
    using namespace transport_catalogue;

    void AddStop(std::string& request, TransportCatalogue& tc) {
        // 5 == "Stop ".size()
        std::vector<std::string> name_coords = SplitString(request.substr(5), ':');

        std::string name = name_coords[0];

        std::vector<std::string> coords = SplitString(name_coords[1], ',');

        double lat = std::stod(coords[0]);
        double lng = std::stod(coords[1]);

        tc.AddStop({name, lat, lng});
    }

    void AddDist(std::string& request, transport_catalogue::TransportCatalogue& tc) {
        std::vector<std::string> name_coords = SplitString(request.substr(5), ':');
        std::string name = name_coords[0];
        Stop* this_stop = tc.StopByName(name);
        std::vector<std::string> dists = SplitString(name_coords[1], ',');

        for (int i = 2; i < dists.size(); ++i) {
            std::string dist_string = dists[i];
            trim(dist_string);
            size_t pos = dist_string.find("m to ");
            int dist = stoi(dist_string.substr(0, pos));
            std::string to_name = dist_string.substr(pos + 5);
            Stop* to_stop = tc.StopByName(to_name);
            tc.AddDistance(this_stop, to_stop, dist);
        }
    }

    void AddBus(std::string& request, TransportCatalogue& tc) {
        // 4 == "Bus ".size()
        // LOG_DURATION("AllAddBus");
        std::vector<std::string> name_stops = SplitString(request.substr(4), ':');

        std::string name = name_stops[0];

        std::vector<Stop*> stops;

        if (name_stops[1].find(" - ") != -1) {
            // Two-way route
            std::vector<std::string> stop_names = SplitString(name_stops[1], '-');
            for (auto& stop_name : stop_names) {
                trim(stop_name);
                stops.push_back(tc.StopByName(stop_name));
            }
            for (int i=stop_names.size()-2;i>=0;--i) {
                stops.push_back(tc.StopByName(stop_names[i]));
            }
        } else {
            // One-way route
            std::vector<std::string> stop_names = SplitString(name_stops[1], '>');
            for (auto stop_name : stop_names) {
                trim(stop_name);
                stops.push_back(tc.StopByName(stop_name));
            }
        }

        tc.AddBus({name, stops, tc});
    }

    void ReadInput(std::istream& stream, TransportCatalogue& tc) {
        int num_requests = 0;
        stream >> num_requests;
        std::vector<std::string> requests;
        std::string request;
        for (;num_requests >= 0;--num_requests) {
            std::getline(stream, request);
            requests.push_back(request);
        }

        {
            // LOG_DURATION("AddStop");
            for (auto req : requests) {
                if (req.rfind("Stop", 0) == 0) {
                    AddStop(req, tc);
                }
            }
        }

        {
            // LOG_DURATION("AddDist");
            for (auto req : requests) {
                if (req.rfind("Stop", 0) == 0) {
                    AddDist(req, tc);
                }
            }
        }

        {
            // LOG_DURATION("AddBus");
            for (auto req : requests) {
                if (req.rfind("Bus", 0) == 0) {
                    AddBus(req, tc);
                }
            }
        }
    }
}
