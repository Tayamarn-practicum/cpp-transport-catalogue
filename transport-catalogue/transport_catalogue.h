#pragma once

#include <deque>
#include <map>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <utility>

#include "domain.h"

namespace transport_catalogue {
    class TransportCatalogue {
    public:
        TransportCatalogue();

        Stop* AddStop(const Stop& stop);

        Bus* AddBus(const Bus& bus);

        void AddDistance(Stop* s1, Stop* s2, int dist);

        void AddEdgeSpanToBus(size_t edge, Bus* bus, int span);

        std::deque<Stop> GetStops() const;

        std::deque<Stop>* GetStopsPtr();

        std::map<std::string_view, Stop*> GetStopnames() const;

        const std::map<std::string_view, Stop*>* GetStopnamesPtr() const;

        Stop* StopByName(const std::string& stop_name) const;

        std::deque<Bus> GetBuses() const;

        std::deque<Bus>* GetBusesPtr();

        std::map<std::string_view, Bus*> GetBusnames() const;

        const std::map<std::string_view, Bus*>* GetBusnamesPtr() const;

        const std::unordered_set<Bus*>* GetBusesByStop(Stop* stop) const;

        std::unordered_map<std::pair<Stop*, Stop*>, int, StopPointerPairHasher>* GetDists();

        const std::unordered_map<size_t, Stop*>* GetVertexToStops() const;

        const std::unordered_map<size_t, std::pair<Bus*, int>>* GetEdgeSpanToBuses() const;

    private:
        std::deque<Stop> stops_;
        std::map<std::string_view, Stop*> stopname_to_stop_;
        std::deque<Bus> buses_;
        std::map<std::string_view, Bus*> busname_to_bus_;
        std::unordered_map<Stop*, std::unordered_set<Bus*>> stop_to_buses_;
        std::unordered_map<std::pair<Stop*, Stop*>, int, StopPointerPairHasher> dist_between_stops_;
        std::unordered_map<size_t, Stop*> vertex_to_stop_;
        std::unordered_map<size_t, std::pair<Bus*, int>> edge_to_bus_and_span_;
    };
}
