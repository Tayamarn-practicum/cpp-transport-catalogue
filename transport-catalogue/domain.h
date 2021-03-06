#pragma once

#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "geo.h"

namespace transport_catalogue {
    struct Stop {
        std::string name;
        geo::Coordinates coords;
        size_t in_vertex;
        size_t out_vertex;

        Stop(std::string& name, double lat, double lng, size_t in_vertex);
    };

    class StopPointerPairHasher {
    public:
        size_t operator()(const std::pair<Stop*, Stop*>& obj) const;
    };

    struct Bus {
        std::string name;
        std::vector<Stop*> stops;
        std::unordered_set<Stop*> unique_stops;
        double true_dist;
        double geo_dist;
        bool is_roundtrip;
        Stop* first;
        Stop* last;

        Bus(std::string& name,
            std::vector<Stop*>& stops,
            std::unordered_map<std::pair<Stop*, Stop*>, int, StopPointerPairHasher>* dists,
            bool is_roundtrip,
            Stop* first, Stop* last
        );

        double CalculateGeoDistance();
        double CalculateTrueDistance(std::unordered_map<std::pair<Stop*, Stop*>, int, StopPointerPairHasher>* dists);
        double GetCurvature();
        double GetGeoDistance();
        double GetTrueDistance();
    };
}
