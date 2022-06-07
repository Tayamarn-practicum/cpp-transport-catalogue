#include "transport_catalogue.h"

#include <iostream>

// Comment this out because practicum's platform doesn't support custom files.
// #include "log_duration.h"

namespace transport_catalogue {
    TransportCatalogue::TransportCatalogue() {}

    void TransportCatalogue::AddStop(const Stop& stop) {
        stops_.push_back(stop);
        Stop* current_stop = &(stops_[stops_.size() - 1]);
        stopname_to_stop_[current_stop->name] = current_stop;
        stop_to_buses_[current_stop] = {};
    }

    void TransportCatalogue::AddBus(const Bus& bus) {
        buses_.push_back(bus);
        Bus* current_bus = &(buses_[buses_.size() - 1]);
        busname_to_bus_[current_bus->name] = current_bus;
        for (Stop* s : bus.stops) {
            stop_to_buses_.at(s).insert(current_bus);
        }
    }

    void TransportCatalogue::AddDistance(Stop* s1, Stop* s2, int dist) {
        dist_between_stops_[{s1, s2}] = dist;
    }

    std::deque<Stop> TransportCatalogue::GetStops() const {
        return stops_;
    }

    std::deque<Stop>* TransportCatalogue::GetStopsPtr() {
        return &stops_;
    }

    std::map<std::string_view, Stop*> TransportCatalogue::GetStopnames() const {
        return stopname_to_stop_;
    }

    const std::map<std::string_view, Stop*>* TransportCatalogue::GetStopnamesPtr() const {
        return &stopname_to_stop_;
    }

    Stop* TransportCatalogue::StopByName(const std::string& stop_name) const {
        return stopname_to_stop_.at(stop_name);
    }

    std::deque<Bus> TransportCatalogue::GetBuses() const {
        return buses_;
    }

    std::deque<Bus>* TransportCatalogue::GetBusesPtr() {
        return &buses_;
    }

    std::map<std::string_view, Bus*> TransportCatalogue::GetBusnames() const {
        return busname_to_bus_;
    }

    const std::map<std::string_view, Bus*>* TransportCatalogue::GetBusnamesPtr() const {
        return &busname_to_bus_;
    }

    const std::unordered_set<Bus*>* TransportCatalogue::GetBusesByStop(Stop* stop) const {
        auto iter = stop_to_buses_.find(stop);
        return iter != stop_to_buses_.end() ? &iter->second : nullptr;
    }

    std::unordered_map<std::pair<Stop*, Stop*>, int, StopPointerPairHasher>* TransportCatalogue::GetDists() {
        return &dist_between_stops_;
    }
}
