#pragma once

#include <deque>
#include <map>
#include <set>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

#include "geo.h"

namespace transport_catalogue {
    class TransportCatalogue;

    struct Stop {
        std::string name;
        Coordinates coords;

        Stop(std::string& name, double lat, double lng);
    };

    struct Bus {
        std::string name;
        std::vector<Stop*> stops;
        std::set<Stop*> unique_stops;
        double true_dist;
        double geo_dist;

        Bus(std::string& name, std::vector<Stop*>& stops, TransportCatalogue& tc);

        double CalculateGeoDistance();
        double CalculateTrueDistance(TransportCatalogue& tc);
        double GetCurvature();
        double GetGeoDistance();
        double GetTrueDistance();
    };

    class StopPointerPairHasher {
    public:
        size_t operator()(const std::pair<Stop*, Stop*>& obj) const;
    };

    class TransportCatalogue {
    public:
        TransportCatalogue();

        void AddStop(const Stop& stop);

        void AddBus(const Bus& bus);

        void AddDistance(Stop* s1, Stop* s2, int dist);

        std::deque<Stop> GetStops();

        std::map<std::string_view, Stop*> GetStopnames();

        Stop* StopByName(const std::string& stop_name);

        std::deque<Bus> GetBuses();

        std::map<std::string_view, Bus*> GetBusnames();

        std::unordered_set<Bus*> GetBusesByStop(Stop* stop);

        std::unordered_map<std::pair<Stop*, Stop*>, int, StopPointerPairHasher>* GetDists();

    private:
        std::deque<Stop> stops_;
        std::map<std::string_view, Stop*> stopname_to_stop_;
        std::deque<Bus> buses_;
        std::map<std::string_view, Bus*> busname_to_bus_;
        std::unordered_map<Stop*, std::unordered_set<Bus*>> stop_to_buses_;
        std::unordered_map<std::pair<Stop*, Stop*>, int, StopPointerPairHasher> dist_between_stops_;
    };
}
